#ifdef GL_ES
precision lowp float;
#endif
uniform sampler2D tex0;

varying vec4 colorVarying;
varying vec2 uvVarying;

void main()
{
    gl_FragColor = colorVarying * texture2D(tex0, uvVarying);
}

