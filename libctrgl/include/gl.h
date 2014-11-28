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

/*
 *  CTRGL - a far-from-complete OpenGL implementation for the POS PICA in the
 *      Nintendo 3DS family of video game consoles.
 *
 *    This project would never have been possible without all the great people
 *    behind 3Dbrew and ctrulib.
 *    Special thanks to Smealum for blessing us with code exec in the first place.
 */

#ifndef CTRGL_H
#define CTRGL_H

#include <3ds.h>
#include <stdint.h>

#define glMakeRgba8CTR(r,g,b,a) (\
    (((r) & 0xFF) << 24)\
    | (((g) & 0xFF) << 16)\
    | (((b) & 0xFF) << 8)\
    | ((a) & 0xFF))\

/* type declarations */

typedef void        GLvoid;
typedef char        GLchar;
typedef size_t      GLuint;
typedef int16_t     GLint;
typedef uint8_t     GLubyte;
typedef float       GLclampf;
typedef float       GLfloat;

typedef int         GLboolean;
typedef uint16_t    GLbitfield;
typedef uint16_t    GLenum;
typedef int32_t     GLintptr;
typedef uint32_t    GLsize;
typedef int32_t     GLsizei;
typedef int32_t     GLsizeiptr;

/* enums */

#include "gl_enums.h"
#include "gl_ctr.h"

/* ctrgl control functions */
void ctrglInit(void);
void ctrglExit(void);

void ctrglAllocateCommandBuffers(GLsize size,
        GLuint count            /* must be 1 or 2 */
        );
void ctrglGetCommandBuffers(u32* size, u32** gpuCmd, u32** gpuCmdRight);

void ctrglResetGPU(void);
void ctrglBeginRendering(void);
void ctrglFlushState(uint32_t mask);
void ctrglFinishRendering(void);

/* **** STATE **** */
void glEnable(GLenum cap);
void glDisable(GLenum cap);
void glClearColorRgba8CTR(uint32_t rgba);

/* Alpha Test */
void glAlphaFunc(GLenum func, GLclampf ref);

void glAlphaFuncubCTR(GLenum func, GLubyte ref);

/* Blending */
void glBlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
void glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha);
void glBlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);

void glBlendColorRgba8CTR(uint32_t rgba);

/* Culling */
void glCullFace(GLenum mode);
void glFrontFace(GLenum mode);

/* Depth Test */
void glDepthFunc(GLenum func);
void glDepthMask(GLboolean flag);

/* Stencil */
void glStencilFunc(GLenum func, GLint ref, GLuint mask);
void glStencilOp(GLenum sfail, GLenum dpfail, GLenum dppass);

/* **** TEXTURES **** */
void glGenTextures(GLsizei n, GLuint* textures);
void glDeleteTextures(GLsizei n, const GLuint* textures);
/*glActiveTexture*/
void glBindTexture(GLenum target, GLuint texture);  /* target must be GL_TEXTURE_2D */
void glTexImage2D(GLenum target,    /* must be GL_TEXTURE_2D */
        GLint level,                /* must be 0 */
        GLint internalFormat,       /* must be GL_BLOCK_RGBA8_CTR */
        GLsizei width, GLsizei height,
        GLint border,               /* must be 0 */
        GLenum format,              /* must be GL_BLOCK_RGBA_CTR */
        GLenum type,                /* must be GL_UNSIGNED_BYTE */
        const GLvoid* data);

void glDirectTexImage2DCTR(GLuint texture, GLint level, GLint internalFormat,
        GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type,
        const GLvoid* data);        /* arguments: see glTexImage2D */
void glGetDirectTexDataPointerCTR(GLuint texture, GLvoid** data_out);
void glTexEnvubvCTR(GLenum target,  /* must be GL_TEXTURE_ENV */
        GLenum pname,               /* must be GL_TEXTURE_ENV_COLOR */
        const GLubyte* params);

/* **** SHADERS **** */
GLuint glCreateProgram(void);
void glUseProgram(GLuint program);  /* this can currently be done only once */
GLint glGetUniformLocation(GLuint program, const GLchar* name);

void glUniform4fv(GLint location, GLsizei count, const GLfloat* value);
void glUniformMatrix4fv(GLint location, GLsizei count,
        GLboolean transpose,        /* must be GL_TRUE */
        const GLfloat* value);

void glLoadProgramBinaryCTR(GLuint program, const void* shbin, GLsize size);
void glGetProgramDvlbCTR(GLuint program, DVLB_s** dvlb_out);

/* **** VERTEX BUFFERS **** */
void glGenBuffers(GLsizei n, GLuint* buffers);
void glDeleteBuffers(GLsizei n, const GLuint* buffers);
void glBindBuffer(GLenum target, GLuint buffer);

void glBufferData(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
void glNamedBufferData(GLuint buffer, GLsizei size, const void* data, GLenum usage);

void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data);
void glNamedBufferSubData(GLuint buffer, GLintptr offset, GLsizei size, const void* data);

void* glMapBuffer(GLenum target, GLenum access);
void* glMapNamedBuffer(GLuint buffer, GLenum access);   /* ayy lmao we OpenGL 4.5 now */

void* glMapBufferRange(GLenum target,
    GLintptr offset,
    GLsizeiptr length,
    GLbitfield access);

void* glMapNamedBufferRange(GLuint buffer,
    GLintptr offset,
    GLsizei length,
    GLbitfield access);

GLboolean glUnmapBuffer(GLenum target);
GLboolean glUnmapNamedBuffer(GLuint buffer);

/* **** VERTEX ARRAYS **** */
void glVertexFormatCTR(GLuint numAttribs, GLuint vertexSize);
void glVertexAttribCTR(GLuint index, GLint size, GLenum type);

/* **** STEREO **** */
void glStereoDisableCTR(void);
void glStereoEnableCTR(GLfloat nearZ, GLfloat screenZ, GLfloat interaxial);

/* **** DRAWING **** */
void glDrawArrays(GLenum mode,
    GLint first,                    /* must be 0 */
    GLsizei count);

/* **** NOT SORTED YET **** */
void glDirectLoadMatrixfCTR(GLenum mode, const GLfloat* m);

#endif
