//
// Created by Jonathan Richard on 2024-02-03.
//

#pragma once

#include "GL/glew.h"
#include "graphicsAPI/common/Buffer.h"

namespace opengl {

class Context
{
public:
    Context() = default;
    ~Context() = default;
    void init();

public: // OpenGL functions
    void clipControl(GLenum origin, GLenum depth);
    void enable(GLenum cap);
    void disable(GLenum cap);
    void clearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    void clearDepth(GLfloat depth);
    void clearStencil(GLint s);
    void colorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
    void stencilMask(GLuint mask);
    void stencilMaskSeparate(GLenum face, GLuint mask);
    void stencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask);
    void stencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
    void clear(GLbitfield mask);
    void viewport(GLint x, GLint y, GLsizei width, GLsizei height);
    void blendFunc(GLenum sfactor, GLenum dfactor);
    void frontFace(GLenum mode);
    void cullFace(GLenum mode);
    void depthFunc(GLenum func);
    void depthMask(GLboolean flag);
    void drawArrays(GLenum mode, GLint first, GLsizei count);
    void drawElements(GLenum mode, GLsizei count, GLenum type, const void* indices);
    void useProgram(GLuint program);
    void bindVertexArray(GLuint array);
    void bindBuffer(GLenum target, GLuint buffer);
    void bindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
    void bindTexture(GLenum target, GLuint texture);
    void activeTexture(GLenum texture);
    void getActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
    GLint getUniformLocation(GLuint program, const GLchar* name);
    void uniform1i(GLint location, GLint v0);
    void uniform1f(GLint location, GLfloat v0);
    void uniform2f(GLint location, GLfloat v0, GLfloat v1);
    void uniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
    void uniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
    void uniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    void bindImageTexture(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
    void deleteShader(GLuint shader);
    void deleteProgram(GLuint program);
    void linkProgram(GLuint program);
    void shaderSource(GLuint shader, GLsizei count, const GLchar** string, const GLint* length);
    void compileShader(GLuint shader);
    void getShaderiv(GLuint shader, GLenum pname, GLint* params);
    void getShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
    void getProgramiv(GLuint program, GLenum pname, GLint* params);
    void getProgramResourceiv(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum* props, GLsizei bufSize, GLsizei* length, GLint* params);
    void getProgramResourceName(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei* length, GLchar* name);
    void getProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
    void attachShader(GLuint program, GLuint shader);
    void detachShader(GLuint program, GLuint shader);
    GLuint createShader(GLenum type);
    GLuint createProgram();
    GLboolean isBuffer(GLuint buffer);
    GLboolean isEnabled(GLenum cap);
    GLboolean isFramebuffer(GLuint framebuffer);
    GLboolean isProgram(GLuint program);
    GLboolean isRenderbuffer(GLuint renderbuffer);
    GLboolean isShader(GLuint shader);
    GLboolean isTexture(GLuint texture);
    void enableVertexAttribArray(GLuint index);
    void disableVertexAttribArray(GLuint index);
    void vertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
    void vertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer);
    void vertexAttribDivisor(GLuint index, GLuint divisor);
    void vertexAttribFormat(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
    void vertexAttribBinding(GLuint attribindex, GLuint bindingindex);
    void genVertexArrays(GLsizei n, GLuint* arrays);
    void genBuffers(GLsizei n, GLuint* buffers);
    void genTextures(GLsizei n, GLuint* textures);
    void genFramebuffers(GLsizei n, GLuint* framebuffers);
    void genRenderbuffers(GLsizei n, GLuint* renderbuffers);
    void deleteVertexArrays(GLsizei n, const GLuint* arrays);
    void deleteBuffers(GLsizei n, const GLuint* buffers);
    void deleteTextures(GLsizei n, const GLuint* textures);
    void deleteFramebuffers(GLsizei n, const GLuint* framebuffers);
    void deleteRenderbuffers(GLsizei n, const GLuint* renderbuffers);
    void bindFramebuffer(GLenum target, GLuint framebuffer);
    void deleteBuffer(GLuint id);
    void unbindBuffer(GLenum target);
    void bufferData(GLenum target, uint32_t size, const void* data, GLenum usage);
    void bufferSubData(GLenum get_target, uint32_t uint32, uint32_t size, const void* data);
    void* mapBufferRange(GLenum get_target, uint32_t uint32, uint32_t size, int i);
    void unmapBuffer(GLenum target);
    void bindBufferBase(GLenum target, GLuint index, GLuint id);
    void framebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
    void framebufferTexture2DMultisample(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples);
    void framebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
    void renderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
    void renderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
    void drawBuffers(GLsizei n, const GLenum* bufs);
    void scissor(GLint x, GLint y, GLsizei width, GLsizei height);
    void texParameteri(GLenum target, GLenum pname, GLint param);
    void texImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels);
    void texImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels);
    void texSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels);
    void texSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels);
    void texStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
    void texStorage3D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
    void pixelStorei(GLenum pname, GLint param);
    void compressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data);
    void compressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data);
    void compressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* data);
    void compressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data);
    void generateMipmap(GLenum target);
    void bindRenderbuffer(GLenum target, GLuint renderbuffer);
    void invalidateFramebuffer(GLenum target, GLsizei numAttachments, const GLenum* attachments);
    GLenum checkFramebufferStatus(GLenum target);


private:
    bool isInit = false;
};

class WithContext {
public:
    explicit WithContext(Context& context);
    virtual ~WithContext();

    // This type is not copyable.
    WithContext(const WithContext&) = delete;
    WithContext& operator=(const WithContext&) = delete;

    Context& getContext() const;

private:
    Context* context_;
};

}
