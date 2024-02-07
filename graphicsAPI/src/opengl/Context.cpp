//
// Created by Jonathan Richard on 2024-02-03.
//

#include "graphicsAPI/opengl/Context.h"

#include <iostream>

#define DEBUG_FUNCTION(func, ...) \
do { \
    printf("Calling function: %s\n", #func); \
    printf("Parameters: "); \
    printf(__VA_ARGS__); \
    printf("\n"); \
} while(0)

// Macro to print which line has an error
#ifndef Debug

// X MACRO to define all of the OpenGL errors
#define _LIST_OF_OPENGL_ERRORS           \
        X(GL_NO_ERROR)                       \
        X(GL_INVALID_VALUE)                  \
        X(GL_INVALID_OPERATION)              \
        X(GL_STACK_OVERFLOW)                 \
        X(GL_STACK_UNDERFLOW)                \
        X(GL_OUT_OF_MEMORY)                  \
        X(GL_INVALID_FRAMEBUFFER_OPERATION)  \
        X(GL_CONTEXT_LOST)                   \
        X(GL_TABLE_TOO_LARGE)

// Returns a string of the X Macro Error
static const char* _glStringError(const GLenum err)
{
    switch (err)
    {
#           define X(name) case name: return #name;
        _LIST_OF_OPENGL_ERRORS
#           undef X
    }
    return "UNDEFINED";
}

// Fetching all of the errors and printing them
static void _glPrintErrors(const char* func, const char* file, const int line)
{
    GLenum err = glGetError();
    while(err != GL_NO_ERROR)
    {
        std::cerr << "\nOpenGL ERROR:"
                     "\n  Func - " << func <<
                "\n  File - " << file <<
                "\n  Line - " << line <<
                "\n  Type - " << _glStringError(err)
                  << std::endl;
        err = glGetError();
    }
}

#   define glLog(x)  x; _glPrintErrors(#x, __FILE__, __LINE__); std::cout << #x << std::endl
#else
#   define glLog(x) x
#endif

namespace opengl {

void Context::init()
{
    if (this->isInit)
    {
        return;
    }

    if (this->isInit = glewInit(); this->isInit != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
    }
}

void Context::clipControl(GLenum origin, GLenum depth)
{
    glLog(glClipControl(origin, depth));
}

void Context::enable(GLenum cap)
{
    glLog(glEnable(cap));
}

void Context::disable(GLenum cap)
{
    glLog(glDisable(cap));
}

void Context::clearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    glLog(glClearColor(red, green, blue, alpha));
}

void Context::clear(GLbitfield mask)
{
    glLog(glClear(mask));
}

void Context::viewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    glLog(glViewport(x, y, width, height));
}

void Context::blendFunc(GLenum sfactor, GLenum dfactor)
{
    glLog(glBlendFunc(sfactor, dfactor));
}

void Context::frontFace(GLenum mode)
{
    glLog(glFrontFace(mode));
}

void Context::cullFace(GLenum mode)
{
    glLog(glCullFace(mode));
}

void Context::depthFunc(GLenum func)
{
    glLog(glDepthFunc(func));
}

void Context::depthMask(GLboolean flag)
{
    glLog(glDepthMask(flag));
}

void Context::drawArrays(GLenum mode, GLint first, GLsizei count)
{
    glLog(glDrawArrays(mode, first, count));
}

void Context::drawElements(GLenum mode, GLsizei count, GLenum type, const void* indices)
{
    glLog(glDrawElements(mode, count, type, indices));
}

void Context::useProgram(GLuint program)
{
    glLog(glUseProgram(program));
}

void Context::bindVertexArray(GLuint array)
{
    glLog(glBindVertexArray(array));
}

void Context::bindBuffer(GLenum target, GLuint buffer)
{
    glLog(glBindBuffer(target, buffer));
}

void Context::bindTexture(GLenum target, GLuint texture)
{
    glLog(glBindTexture(target, texture));
}

void Context::activeTexture(GLenum texture)
{
    glLog(glActiveTexture(texture));
}

void Context::uniform1i(GLint location, GLint v0)
{
    glLog(glUniform1i(location, v0));
}

void Context::uniform1f(GLint location, GLfloat v0)
{
    glLog(glUniform1f(location, v0));
}

void Context::uniform2f(GLint location, GLfloat v0, GLfloat v1)
{
    glLog(glUniform2f(location, v0, v1));
}

void Context::uniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
    glLog(glUniform3f(location, v0, v1, v2));
}

void Context::uniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    glLog(glUniform4f(location, v0, v1, v2, v3));
}

void Context::uniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    glLog(glUniformMatrix4fv(location, count, transpose, value));
}

void Context::bindImageTexture(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format)
{
    glLog(glBindImageTexture(unit, texture, level, layered, layer, access, format));
}

void Context::deleteShader(GLuint shader)
{
    glLog(glDeleteShader(shader));
}

void Context::deleteProgram(GLuint program)
{
    glLog(glDeleteProgram(program));
}

void Context::linkProgram(GLuint program)
{
    glLog(glLinkProgram(program));
}

void Context::shaderSource(GLuint shader, GLsizei count, const GLchar** string, const GLint* length)
{
    glLog(glShaderSource(shader, count, string, length));
}

void Context::compileShader(GLuint shader)
{
    glLog(glCompileShader(shader));
}

void Context::getShaderiv(GLuint shader, GLenum pname, GLint* params)
{
    glLog(glGetShaderiv(shader, pname, params));
}

void Context::getShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
    glLog(glGetShaderInfoLog(shader, bufSize, length, infoLog));
}

void Context::getProgramiv(GLuint program, GLenum pname, GLint* params)
{
    glLog(glGetProgramiv(program, pname, params));
}

void Context::getProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
    glLog(glGetProgramInfoLog(program, bufSize, length, infoLog));
}

void Context::attachShader(GLuint program, GLuint shader)
{
    glLog(glAttachShader(program, shader));
}

void Context::detachShader(GLuint program, GLuint shader)
{
    glLog(glDetachShader(program, shader));
}

GLuint Context::createShader(GLenum type)
{
    return glLog(glCreateShader(type));
}

GLuint Context::createProgram()
{
    return glLog(glCreateProgram());
}

GLboolean Context::isBuffer(GLuint buffer)
{
    return glLog(glIsBuffer(buffer));
}

GLboolean Context::isEnabled(GLenum cap)
{
    return glLog(glIsEnabled(cap));
}

GLboolean Context::isFramebuffer(GLuint framebuffer)
{
    return glLog(glIsFramebuffer(framebuffer));
}

GLboolean Context::isProgram(GLuint program)
{
    return glLog(glIsProgram(program));
}

GLboolean Context::isRenderbuffer(GLuint renderbuffer)
{
    return glLog(glIsRenderbuffer(renderbuffer));
}

GLboolean Context::isShader(GLuint shader)
{
    return glLog(glIsShader(shader));
}

GLboolean Context::isTexture(GLuint texture)
{
    return glLog(glIsTexture(texture));
}

void Context::enableVertexAttribArray(GLuint index)
{
    glLog(glEnableVertexAttribArray(index));
}

void Context::disableVertexAttribArray(GLuint index)
{
    glLog(glDisableVertexAttribArray(index));
}

void Context::vertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer)
{
    glLog(glVertexAttribPointer(index, size, type, normalized, stride, pointer));
}

void Context::vertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer)
{
    glLog(glVertexAttribIPointer(index, size, type, stride, pointer));
}

void Context::vertexAttribDivisor(GLuint index, GLuint divisor)
{
    glLog(glVertexAttribDivisor(index, divisor));
}

void Context::vertexAttribFormat(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)
{
    glLog(glVertexAttribFormat(attribindex, size, type, normalized, relativeoffset));
}

void Context::vertexAttribBinding(GLuint attribindex, GLuint bindingindex)
{
    glLog(glVertexAttribBinding(attribindex, bindingindex));
}

void Context::genVertexArrays(GLsizei n, GLuint* arrays)
{
    glLog(glGenVertexArrays(n, arrays));
}

void Context::genBuffers(GLsizei n, GLuint* buffers)
{
    glLog(glGenBuffers(n, buffers));
}

void Context::genTextures(GLsizei n, GLuint* textures)
{
    glLog(glGenTextures(n, textures));
}

void Context::genFramebuffers(GLsizei n, GLuint* framebuffers)
{
    glLog(glGenFramebuffers(n, framebuffers));
}

void Context::genRenderbuffers(GLsizei n, GLuint* renderbuffers)
{
    glLog(glGenRenderbuffers(n, renderbuffers));
}

void Context::deleteVertexArrays(GLsizei n, const GLuint* arrays)
{
    glLog(glDeleteVertexArrays(n, arrays));
}

void Context::deleteBuffers(GLsizei n, const GLuint* buffers)
{
    glLog(glDeleteBuffers(n, buffers));
}

void Context::deleteTextures(GLsizei n, const GLuint* textures)
{
    glLog(glDeleteTextures(n, textures));
}

void Context::deleteFramebuffers(GLsizei n, const GLuint* framebuffers)
{
    glLog(glDeleteFramebuffers(n, framebuffers));
}

void Context::deleteRenderbuffers(GLsizei n, const GLuint* renderbuffers)
{
    glLog(glDeleteRenderbuffers(n, renderbuffers));
}

void Context::bindFramebuffer(GLenum target, GLuint framebuffer)
{
    glLog(glBindFramebuffer(target, framebuffer));
}

void Context::deleteBuffer(GLuint id)
{
    glLog(glDeleteBuffers(1, &id));
}

void Context::unbindBuffer(GLenum target)
{
    glLog(glBindBuffer(target, 0));
}

void Context::bufferData(GLenum target, uint32_t size, const void* data, GLenum usage)
{
    glLog(glBufferData(target, size, data, usage));
}

void Context::bufferSubData(GLenum get_target, uint32_t uint32, uint32_t size, const void* data)
{
    glLog(glBufferSubData(get_target, uint32, size, data));
}

void* Context::mapBufferRange(GLenum get_target, uint32_t uint32, uint32_t size, int i)
{
    return glLog(glMapBufferRange(get_target, uint32, size, i));
}

void Context::unmapBuffer(GLenum target)
{
    glLog(glUnmapBuffer(target));
}

void Context::bindBufferBase(GLenum target, GLuint index, GLuint id)
{
    glLog(glBindBufferBase(target, index, id));
}

WithContext::WithContext(Context& context) : context_(&context)
{
}

WithContext::~WithContext()
{
}

Context& WithContext::getContext() const
{
    return *context_;
}

} // namespace opengl