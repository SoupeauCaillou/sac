/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/



#include "EffectLibrary.h"

#include "shaders/default_fs.h"
#include "shaders/default_no_alpha_fs.h"
#include "shaders/empty_fs.h"
#include "shaders/default_no_texture_fs.h"
#include "shaders/default_vs.h"
#define VERTEX_SHADER_ARRAY default_vs
#define VERTEX_SHADER_SIZE default_vs_len

static GLuint compileShader(const std::string&, GLuint type, const FileBuffer& fb) {
    LOGV(1, "Compiling " << ((type == GL_VERTEX_SHADER) ? "vertex" : "fragment") << " shader...");;

    GLuint shader = glCreateShader(type);
    GL_OPERATION(glShaderSource(shader, 1, (const char**)&fb.data, &fb.size))
    GL_OPERATION(glCompileShader(shader))

    GLint logLength;
    GL_OPERATION(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength))
    if (logLength > 1) {
        char *log = new char[logLength];
        GL_OPERATION(glGetShaderInfoLog(shader, logLength, &logLength, log))
        LOGW("GL shader error: " << log);
        delete[] log;
    }

   if (!glIsShader(shader)) {
        LOGE("Weird; " << shader << "d is not a shader");
   }
    return shader;
}

static Shader buildShaderFromFileBuffer(const char* vsName, const FileBuffer& fragmentFb) {
    Shader out;
    LOGV(1, "building shader ...");;
    out.program = glCreateProgram();
    check_GL_errors("glCreateProgram");

    FileBuffer vertexFb;
    vertexFb.data = VERTEX_SHADER_ARRAY;
    vertexFb.size = VERTEX_SHADER_SIZE;
    GLuint vs = compileShader(vsName, GL_VERTEX_SHADER, vertexFb);

    GLuint fs = compileShader("unknown.fs", GL_FRAGMENT_SHADER, fragmentFb);

    GL_OPERATION(glAttachShader(out.program, vs))
    GL_OPERATION(glAttachShader(out.program, fs))
    LOGV(2, "Binding GLSL attribs");
    GL_OPERATION(glBindAttribLocation(out.program, EffectLibrary::ATTRIB_VERTEX, "aPosition"))
    GL_OPERATION(glBindAttribLocation(out.program, EffectLibrary::ATTRIB_UV, "aTexCoord"))

    LOGV(2, "Linking GLSL program");
    GL_OPERATION(glLinkProgram(out.program))

    GLint logLength;
    glGetProgramiv(out.program, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 1) {
        char *log = new char[logLength];
        glGetProgramInfoLog(out.program, logLength, &logLength, log);
        LOGW("GL shader (vs='" << vsName << "' program error: '" << log << "'");

        delete[] log;
    }

    out.uniformMatrix = glGetUniformLocation(out.program, "uMvp");
    out.uniformColorSampler = glGetUniformLocation(out.program, "tex0");
    out.uniformAlphaSampler = glGetUniformLocation(out.program, "tex1");
    out.uniformColor= glGetUniformLocation(out.program, "vColor");
    out.uniformCamera = glGetUniformLocation(out.program, "uCamera");
    out.uniformUVScaleOffset = glGetUniformLocation(out.program, "uvScaleOffset");
    out.uniformRotation = glGetUniformLocation(out.program, "uRotation");
    out.uniformScaleZ = glGetUniformLocation(out.program, "uScaleZ");

    glDeleteShader(vs);
    glDeleteShader(fs);

    return out;
}

static Shader buildShaderFromAsset(AssetAPI* assetAPI, const char* vsName, const char* fsName) {
    LOGI("Compiling shaders: " << vsName << '/' << fsName);
    FileBuffer fragmentFb = assetAPI->loadAsset(fsName);
    Shader shader = buildShaderFromFileBuffer(vsName, fragmentFb);
    delete[] fragmentFb.data;
    return shader;
}

void EffectLibrary::init(AssetAPI* pAssetAPI, bool pUseDeferredLoading) {
    NamedAssetLibrary<Shader, EffectRef, FileBuffer>::init(pAssetAPI, pUseDeferredLoading);
    FileBuffer fb;
    fb.data = default_fs;
    fb.size = default_fs_len;
    registerDataSource(name2ref(DEFAULT_FRAGMENT), fb);
    fb.data = default_no_alpha_fs;
    fb.size = default_no_alpha_fs_len;
    registerDataSource(name2ref(DEFAULT_NO_ALPHA_FRAGMENT), fb);
    fb.data = empty_fs;
    fb.size = empty_fs_len;
    registerDataSource(name2ref(EMPTY_FRAGMENT), fb);
    fb.data = default_no_texture_fs;
    fb.size = default_no_texture_fs_len;
    registerDataSource(name2ref(DEFAULT_NO_TEXTURE_FRAGMENT), fb);
}

bool EffectLibrary::doLoad(const char* assetName, Shader& out, const EffectRef& ref) {
    LOGF_IF(assetAPI == 0, "Unitialized assetAPI member");

    std::map<EffectRef, FileBuffer>::iterator it = dataSource.find(ref);
    if (it == dataSource.end()) {
        LOGV(1, "loadShader: '" << assetName << "' from file");
        out = buildShaderFromAsset(assetAPI, "default.vs", assetName);
    } else {
        const FileBuffer& fb = it->second;
        LOGV(1, "loadShader: '" << assetName << "' from InMemoryShader (" << fb.size << ')');
        out = buildShaderFromFileBuffer("default.vs", fb);
    }

    return true;
}

void EffectLibrary::doUnload(const Shader&) {
    LOGT("Effect unloading");
}

void EffectLibrary::doReload(const char* name, const EffectRef& ref) {
    //Shader& info = ref2asset[ref];
    doLoad(name, assets[ref2Index(ref)], ref);
}
