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

#define CTR_GL_C

/* TODO: linearAllocMutex */

/* TODO: get rid of these */
#define GPU_TEXTURE_MAG_FILTER(v) (((v)&0x1)<<1) //takes a GPU_TEXTURE_FILTER_PARAM
#define GPU_TEXTURE_MIN_FILTER(v) (((v)&0x1)<<2) //takes a GPU_TEXTURE_FILTER_PARAM
#define GPU_TEXTURE_WRAP_S(v) (((v)&0x3)<<8) //takes a GPU_TEXTURE_WRAP_PARAM
#define GPU_TEXTURE_WRAP_T(v) (((v)&0x3)<<12) //takes a GPU_TEXTURE_WRAP_PARAM

#include "gl.h"

typedef float mat4x4[4][4];

extern u32* __linear_heap;

static u32* gpuFrameBuffer = (u32*) 0x1F119400;
static u32* gpuDepthBuffer = (u32*) 0x1F370800;

/* state */
static GLalphaTestStateCTR      alphaTestState;
static GLblendStateCTR          blendState;
static GLcullStateCTR           cullState;
static GLdepthTestStateCTR      depthTestState;
static GLshaderStateCTR         shaderState;
static GLstencilStateCTR        stencilState;
static GLstereoStateCTR         stereoState;
static GLtexturingStateCTR      texturingState;
static GLvertexArraysStateCTR   vertexArraysState;

static mat4x4 matrices[2];
static GLbufferCTR* boundBuffer;
static u32 clearColor;

static uint32_t dirtyState;
static uint32_t enableState;
static uint8_t dirtyMatrices, dirtyTexEnv, dirtyTexUnits;

static u32 gpuCmdSize;
static u32* gpuCmd;
static u32* gpuCmdRight;

#include "gl_helperfunctions_.h"
#include "gl_matrix_.h"
#include "gl_setup_.h"

#include "gl_ctrgl_.h"

void glEnable(GLenum cap)
{
    if (cap == GL_TEXTURE_2D)
    {
        texturingState.enableTextures |= (1 << texturingState.activeTexture);
        dirtyState |= GL_TEXTURING_CTR;
        dirtyTexUnits |= (1 << texturingState.activeTexture);
        return;
    }

    if (!(enableState & cap))
    {
        enableState |= cap;
        dirtyState |= cap;
    }
}

void glDisable(GLenum cap)
{
    if (cap == GL_TEXTURE_2D)
    {
        texturingState.enableTextures &= ~(1 << texturingState.activeTexture);
        dirtyState |= GL_TEXTURING_CTR;
        dirtyTexUnits |= (1 << texturingState.activeTexture);
        return;
    }

    if (enableState & cap)
    {
        enableState &= ~cap;
        dirtyState |= cap;
    }
}

void glClearColorRgba8CTR(uint32_t rgba)
{
    clearColor = rgba;
}

/* Alpha Test */
void glAlphaFunc(GLenum func, GLclampf ref)
{
    glAlphaFuncubCTR(func, clampf2ubyte(ref));
}

void glAlphaFuncubCTR(GLenum func, GLubyte ref)
{
    alphaTestState.func = func;
    alphaTestState.ref = ref;
    dirtyState |= GL_ALPHA_TEST;
}

/* Culling */
void glCullFace(GLenum mode)
{
    cullState.cullFace = mode;
    dirtyState |= GL_CULL_FACE;
}

void glFrontFace(GLenum mode)
{
    cullState.frontFace = mode;
    dirtyState |= GL_CULL_FACE;
}

/* Depth Test */
void glDepthFunc(GLenum func)
{
    depthTestState.func = func;
    dirtyState |= GL_DEPTH_TEST;
}

void glDepthMask(GLboolean flag)
{
    depthTestState.mask = flag;
    dirtyState |= GL_DEPTH_TEST;
}

void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    depthTestState.colorMask = (red | (green << 1) | (blue << 2) | (alpha << 3));
    dirtyState |= GL_DEPTH_TEST;
}

/* Stencil */
void glStencilFunc(GLenum func, GLint ref, GLuint mask)
{
    stencilState.func = func;
    stencilState.ref = ref;
    stencilState.mask = mask;
    dirtyState |= GL_STENCIL_TEST;
}

void glStencilOp(GLenum sfail, GLenum dpfail, GLenum dppass)
{
    stencilState.sfail = sfail;
    stencilState.dpfail = dpfail;
    stencilState.dppass = dppass;
    dirtyState |= GL_STENCIL_TEST;
}

/* Blending */
void glBlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    glBlendColorRgba8CTR(glMakeRgba8CTR(
        clampf2ubyte(red), clampf2ubyte(green), clampf2ubyte(blue), clampf2ubyte(alpha)
        ));
}

void glBlendColorRgba8CTR(uint32_t rgba)
{
    blendState.blendColor = rgba;
    dirtyState |= GL_BLEND;
}

void glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha)
{
    blendState.modeRGB = modeRGB;
    blendState.modeAlpha = modeAlpha;
    dirtyState |= GL_BLEND;
}

void glBlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
    blendState.srcRGB = srcRGB;
    blendState.dstRGB = dstRGB;
    blendState.srcAlpha = srcAlpha;
    blendState.dstAlpha = dstAlpha;
    dirtyState |= GL_BLEND;
}

/* **** TEXTURES **** */
GLuint glInitTextureCTR(GLtextureCTR* tex)
{
    tex->w = 0;
    tex->h = 0;
    tex->data = NULL;

    return (GLuint) tex;
}

void glShutdownTextureCTR(GLtextureCTR* tex)
{
    int i;

    for (i = 0; i < NUM_TEXUNITS; i++)
        if (tex == texturingState.texUnits[i].boundTexture)
        {
            texturingState.texUnits[i].boundTexture = NULL;
            dirtyState |= GL_TEXTURING_CTR;
            dirtyTexUnits |= (1 << i);
        }

    linearFree(tex->data);
}

void glGenTextures(GLsizei n, GLuint* textures)
{
    GLtextureCTR* tex;

    while (n-- > 0)
    {
        tex = (GLtextureCTR*) malloc(sizeof(GLtextureCTR));
        glInitTextureCTR(tex);

        *textures = (GLuint) tex;
        textures++;
    }
}

void glDeleteTextures(GLsizei n, const GLuint* textures)
{
    GLtextureCTR* tex;

    for (; n > 0; n--, textures++)
    {
        tex = (GLtextureCTR*) *textures;

        if (tex == NULL)
            continue;

        glShutdownTextureCTR(tex);
        free(tex);
    }
}

void glBindTexture(GLenum target, GLuint texture)
{
    GLtextureCTR* tex;

    tex = (GLtextureCTR*) texture;
    texturingState.texUnits[texturingState.activeTexture].boundTexture = tex;

    dirtyState |= GL_TEXTURING_CTR;
    dirtyTexUnits |= (1 << texturingState.activeTexture);
}

void glGetNamedTexDataPointerCTR(GLuint texture, GLvoid** data_out)
{
    GLtextureCTR* tex;

    tex = (GLtextureCTR*) texture;
    *data_out = tex->data;
}

void glTexImage2D(GLenum target, GLint level, GLint internalFormat,
        GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type,
        const GLvoid* data)
{
    glNamedTexImage2DCTR((GLuint) texturingState.texUnits[texturingState.activeTexture].boundTexture,
            level, internalFormat, width, height, border, format, type, data);
}

void glNamedTexImage2DCTR(GLuint texture, GLint level, GLint internalFormat,
        GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type,
        const GLvoid* data)
{
    GLtextureCTR* tex;
    size_t textureDataSize;
    int i;

    tex = (GLtextureCTR*) texture;

    if (tex->data)
        linearFree(tex->data);

    textureDataSize = width * height * 4;
    tex->data = linearMemAlign(textureDataSize, 0x80);

    if (data)
        memcpy(tex->data, data, textureDataSize);

    tex->w = width;
    tex->h = height;

    for (i = 0; i < NUM_TEXUNITS; i++)
        if (texturingState.texUnits[i].boundTexture == tex)
        {
            dirtyState |= GL_TEXTURING_CTR;
            dirtyTexUnits |= (1 << i);
        }
}

void glTexEnvubvCTR(GLenum target, GLenum pname, const GLubyte* params)
{
    memcpy(&texturingState.env[texturingState.activeTexture].primaryColor, params, 4);
    dirtyState |= GL_TEXTURING_CTR;
    dirtyTexEnv |= (1 << texturingState.activeTexture);
}

/* **** SHADERS **** */
GLuint glInitProgramCTR(GLprogramCTR* prog)
{
    prog->dvlb = NULL;
    prog->projectionUniform = -1;
    prog->modelviewUniform = -1;

    return (GLuint) prog;
}

void glShutdownProgramCTR(GLprogramCTR* prog)
{
    if (shaderState.program == prog)
    {
        shaderState.program = NULL;
        dirtyState |= GL_SHADER_PROGRAM_CTR;
    }

    SHDR_FreeDVLB(prog->dvlb);
}

GLuint glCreateProgram(void)
{
    GLprogramCTR* prog;

    prog = (GLprogramCTR*) malloc(sizeof(GLprogramCTR));
    glInitProgramCTR(prog);

    return (GLuint) prog;
}

void glDeleteProgram(GLuint program)
{
    GLprogramCTR* prog;

    prog = (GLprogramCTR*) program;

    if (prog == NULL)
        return;

    glShutdownProgramCTR(prog);
    free(prog);
}

void glUseProgram(GLuint program)
{
    GLprogramCTR* prog;

    prog = (GLprogramCTR*) program;
    shaderState.program = prog;

    matrixUniforms[0] = prog->projectionUniform;
    matrixUniforms[1] = prog->modelviewUniform;

    dirtyState |= GL_SHADER_PROGRAM_CTR;
    dirtyMatrices = 0xff;
}

void glGetProgramDvlbCTR(GLuint program, DVLB_s** dvlb_out)
{
    GLprogramCTR* prog;

    prog = (GLprogramCTR*) program;
    *dvlb_out = prog->dvlb;
}

GLint glGetUniformLocation(GLuint program, const GLchar* name)
{
    GLprogramCTR* prog;

    prog = (GLprogramCTR*) program;
    return SHDR_GetUniformRegister(prog->dvlb, name, 0);
}

void glLoadProgramBinaryCTR(GLuint program, const void* shbin, GLsize size)
{
    GLprogramCTR* prog;

    prog = (GLprogramCTR*) program;
    prog->dvlb = SHDR_ParseSHBIN((u32*) shbin, size);

    prog->projectionUniform = glGetUniformLocation(program, "projection");
    prog->modelviewUniform = glGetUniformLocation(program, "modelview");
}

void glUniform4fv(GLint location, GLsizei count, const GLfloat* value)
{
    ctrglFlushState(GL_SHADER_PROGRAM_CTR);

    GPUCMD_AddSingleParam(0x000F02C0, 0x80000000 | location);
    GPUCMD_Add(0x000F02C1, (u32*) value, count * 4);
}

void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if (transpose)
    {
        float param[16];

        param[0x0] = value[3];
        param[0x1] = value[2];
        param[0x2] = value[1];
        param[0x3] = value[0];

        param[0x4] = value[7];
        param[0x5] = value[6];
        param[0x6] = value[5];
        param[0x7] = value[4];
        
        param[0x8] = value[11];
        param[0x9] = value[10];
        param[0xa] = value[9];
        param[0xb] = value[8];

        param[0xc] = value[15];
        param[0xd] = value[14];
        param[0xe] = value[13];
        param[0xf] = value[12];

        GPU_SetUniform(location, (u32*) param, 4);
    }
    else
        GPU_SetUniform(location, (u32*) value, 4);
}

/* **** VERTEX ARRAYS **** */
void glVertexFormatCTR(GLuint numAttribs, GLuint vertexSize)
{
    vertexArraysState.numAttribs = numAttribs;
    vertexArraysState.vertexSize = vertexSize;
    dirtyState |= GL_VERTEX_ARRAYS_CTR;
}

void glVertexAttribCTR(GLuint index, GLint size, GLenum type)
{
    vertexArraysState.attribs[index].size = size - 1;
    vertexArraysState.attribs[index].type = type;
    dirtyState |= GL_VERTEX_ARRAYS_CTR;
}

/* **** BUFFERS **** */
GLuint glInitBufferCTR(GLbufferCTR* buf)
{
    buf->data = NULL;
    buf->size = 0;

    return (GLuint) buf;
}

void glShutdownBufferCTR(GLbufferCTR* buf)
{
    if (buf == boundBuffer)
        boundBuffer = NULL;

    linearFree(buf->data);
}

void glGenBuffers(GLsizei n, GLuint* buffers)
{
    GLbufferCTR* buf;

    while (n-- > 0)
    {
        buf = (GLbufferCTR*) malloc(sizeof(GLbufferCTR));
        glInitBufferCTR(buf);

        *buffers = (GLuint) buf;
        buffers++;
    }
}

void glDeleteBuffers(GLsizei n, const GLuint* buffers)
{
    GLbufferCTR* buf;

    for (; n > 0; n--, buffers++)
    {
        buf = (GLbufferCTR*) *buffers;

        if (buf == NULL)
            continue;

        glShutdownBufferCTR(buf);
        free(buf);
    }
}

void glBindBuffer(GLenum target, GLuint buffer)
{
    GLbufferCTR* buf;

    buf = (GLbufferCTR*) buffer;
    boundBuffer = buf;
}

void glBufferData(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage)
{
    glNamedBufferData((GLuint) boundBuffer, size, data, usage);
}

void glNamedBufferData(GLuint buffer, GLsizei size, const void* data, GLenum usage)
{
    GLbufferCTR* buf;

    buf = (GLbufferCTR*) buffer;

    if (buf->data != NULL)
        linearFree(buf->data);

    buf->data = linearMemAlign(size, 128);

    if (data != NULL)
        memcpy(buf->data, data, size);

    buf->size = size;
}

void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data)
{
    glNamedBufferSubData((GLuint) boundBuffer, offset, size, data);
}

void glNamedBufferSubData(GLuint buffer, GLintptr offset, GLsizei size, const void* data)
{
    GLbufferCTR* buf;

    buf = (GLbufferCTR*) buffer;
    memcpy((uint8_t*) buf->data + offset, data, size);
}

void* glMapBuffer(GLenum target, GLenum access)
{
    return glMapNamedBuffer((GLuint) boundBuffer, access);
}

void* glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
    return glMapNamedBufferRange((GLuint) boundBuffer, offset, length, access);
}

void* glMapNamedBuffer(GLuint buffer, GLenum access)
{
    GLbufferCTR* buf;

    buf = (GLbufferCTR*) buffer;
    return buf->data;               /* shared memory rocks */
}

void* glMapNamedBufferRange(GLuint buffer, GLintptr offset, GLsizei length, GLbitfield access)
{
    GLbufferCTR* buf;

    buf = (GLbufferCTR*) buffer;

    if (offset + length > buf->size)
        return NULL;

    if (buf->data)
        return (uint8_t*) buf->data + offset;
    else
        return NULL;
}

GLboolean glUnmapBuffer(GLenum target)
{
    return GL_TRUE;
}

GLboolean glUnmapNamedBuffer(GLuint buffer)
{
    return GL_TRUE;
}

/* **** STEREO **** */
void glStereoDisableCTR(void)
{
    stereoState.enabled = GL_FALSE;
}

void glStereoEnableCTR(GLfloat interaxial, GLfloat nearZ, GLfloat screenZ)
{
    stereoState.enabled = GL_TRUE;
    stereoState.interaxial = interaxial;
    stereoState.nearZ = nearZ;
    stereoState.screenZ = screenZ;
}

/* **** NOT SORTED YET **** */
void glGetDirectMatrixfCTR(GLenum mode, GLfloat* m)
{
    memcpy(m, matrices[mode], sizeof(mat4x4));
}

void glDirectLoadMatrixfCTR(GLenum mode, const GLfloat* m)
{
    memcpy(matrices[mode], m, sizeof(mat4x4));
    dirtyMatrices |= (1 << mode);
}

void glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
    /* TODO: precompute commands */

    ctrglFlushState(0xffffffff);

    flushMatrices();

    //gsVboPrecomputeCommands(vbo);

    // u64 val=svcGetSystemTick();
    /*if(vbo->commands)
    {
        _GPUCMD_AddRawCommands(vbo->commands, vbo->commandsSize);
    }else{*/
        // TODO: cache VBO phys address
        gpuDrawArrayDirectly(GPU_TRIANGLES, (u8*) boundBuffer->data + first * vertexArraysState.vertexSize, count);
    //}
    // debugValue[5]+=(u32)(svcGetSystemTick()-val);
    // debugValue[6]++;
}
