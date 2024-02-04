//
// Created by Jonathan Richard on 2024-02-03.
//

#pragma once

#include "GL/glew.h"
#include "spirv_cross.hpp"

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
    void bindTexture(GLenum target, GLuint texture);
    void activeTexture(GLenum texture);
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
