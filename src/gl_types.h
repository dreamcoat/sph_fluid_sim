#pragma once

#include <GL/glew.h>

enum 
{
    oglERR_CLEAR = 0,
    oglERR_JUSTLOG,

    oglERR_SHADERCREATE,
    oglERR_SHADERCOMPILE,
    oglERR_SHADERLINK,
    oglERR_SHADERLOCATION,

    oglERR_BUFFER,
    oglERR_TEXTIMAGE,

    oglERR_DRAWING_TRI,
    oglERR_DRAWING_STR
};

typedef void gl_error_handler_t(int err, int glerr, const GLchar* glMsg);

// External function for GL errors
static gl_error_handler_t* external_gl_error_handler;