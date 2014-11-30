/*
    Boost Software License - Version 1.0 - August 17th, 2003

    Permission is hereby granted, free of charge, to any person or organization
    obtaining a copy of the software and accompanying documentation covered by
    this license (the "Software") to use, reproduce, display, distribute,
    execute, and transmit the Software, and to prepare derivative works of the
    Software, and to permit third-parties to whom the Software is furnished to
    do so, all subject to the following:

    The copyright notices in the Software and this entire statement, including
    the above license grant, this restriction and the following disclaimer,
    must be included in all copies of the Software, in whole or in part, and
    all derivative works of the Software, unless such copies or derivative
    works are solely in the form of machine-executable object code generated by
    a source language processor.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
    SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
    FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
    ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

#ifndef GL_CTR_H
#define GL_CTR_H

#include <3ds.h>

#define NUM_TEXENV          6
#define NUM_TEXUNITS        3
#define MAX_VERTEX_ATTRIBS  4

typedef float GLmat4x4[4][4];

typedef enum
{
    GL_STEREO_NONE_CTR,
    GL_STEREO_PERSPECTIVE_CTR,
    GL_STEREO_ORTHO_CTR,
}
GLstereoModeCTR;

/* **** TEXTURES **** */
typedef struct
{
    uint16_t w, h;
    void* data;

    /* TODO: border */
}
GLtextureCTR;

/* **** SHADERS **** */
typedef struct
{
    DVLB_s* dvlb;
    void* shbin;

    GLint projectionUniform;
    GLint modelviewUniform;
}
GLprogramCTR;

/* **** VERTEX ARRAYS **** */
typedef struct
{
    GLuint size : 2;
    GLenum type : 2;
}
GLvertexAttribCTR;

/* **** VERTEX BUFFERS **** */
typedef struct
{
    void* data;
    size_t size;
}
GLbufferCTR;

/* **** STATE **** */

typedef struct
{
    GLenum func         : 3;
    GLubyte ref;
}
GLalphaTestStateCTR;

typedef struct
{
    uint32_t blendColor;

    GLenum modeRGB      : 4;
    GLenum modeAlpha    : 4;

    GLenum srcRGB       : 4;
    GLenum dstRGB       : 4;
    GLenum srcAlpha     : 4;
    GLenum dstAlpha     : 4;
}
GLblendStateCTR;

typedef struct
{
    GLenum cullFace     : 1;
    GLenum frontFace    : 1;
}
GLcullStateCTR;

typedef struct
{
    GLenum func         : 3;
    GLboolean mask      : 1;
    GLubyte colorMask   : 4;
}
GLdepthTestStateCTR;

typedef struct
{
    GLmat4x4 projection;
    GLmat4x4 modelview;

    GLstereoModeCTR stereoMode;
    float screenZ;

    union
    {
        struct
        {
            float skew;
        }
        ortho;

        struct
        {
            float nearZ, scale;
        }
        perspective;
    }
    stereoParams;
}
GLmatricesStateCTR;

typedef struct
{
    GLprogramCTR* program;
}
GLshaderStateCTR;

typedef struct
{
    GLenum func         : 3;
    GLubyte ref;
    GLubyte mask;

    GLenum sfail        : 4;
    GLenum dpfail       : 4;
    GLenum dppass       : 4;
}
GLstencilStateCTR;

typedef struct
{
    GLboolean enabled   : 1;
    GLfloat interaxial;
}
GLstereoStateCTR;

typedef struct
{
    uint32_t primaryColor;
    GLenum combineRGB   : 4;
    GLenum combineAlpha : 4;
    GLenum src0RGB      : 4;
    GLenum src1RGB      : 4;
    GLenum src2RGB      : 4;
    GLenum src0Alpha    : 4;
    GLenum src1Alpha    : 4;
    GLenum src2Alpha    : 4;
    GLenum operand0RGB  : 4;
    GLenum operand1RGB  : 4;
    GLenum operand2RGB  : 4;
    GLenum operand0Alpha: 4;
    GLenum operand1Alpha: 4;
    GLenum operand2Alpha: 4;
}
GLtexEnvStateCTR;

typedef struct
{
    GLtextureCTR* boundTexture;
}
GLtexUnitStateCTR;

typedef struct
{
    GLtexEnvStateCTR env[NUM_TEXENV];
    GLtexUnitStateCTR texUnits[NUM_TEXUNITS];

    uint8_t activeTexture;
    uint8_t enableTextures;
}
GLtexturingStateCTR;

typedef struct
{
    GLvertexAttribCTR attribs[MAX_VERTEX_ATTRIBS];

    uint8_t numAttribs;
    uint8_t vertexSize;
}
GLvertexArraysStateCTR;

typedef void (*CTRGLtimeoutHandler)(CTRGLtimeoutType type);

#endif
