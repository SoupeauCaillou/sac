#pragma once

#if !defined(ANDROID) && !defined(EMSCRIPTEN)
#define CHECK_GL_ERROR
#endif

#ifdef CHECK_GL_ERROR
 #define GL_OPERATION(x) \
     (x); \
     RenderingSystem::check_GL_errors(#x);
#else
 #define GL_OPERATION(x) \
     (x);
#endif

#define EndFrameMarker -10
#define ToggleZWriteMarker -11
#define BeginFrameMarker -12
#define EnableBlending -13
#define ToggleColorWriteMarker -14

enum {
    ATTRIB_VERTEX = 0,
    ATTRIB_UV,
    ATTRIB_POS_ROT,
    ATTRIB_SCALE,
    NUM_ATTRIBS
};

#define L_RENDER  0
#define L_QUEUE   1
#define L_TEXTURE 2

#define C_RENDER_DONE 0
#define C_FRAME_READY 1
