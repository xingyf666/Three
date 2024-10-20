#pragma once

#include <GLFW/glfw3.h>
#include <iostream>
#include <string>

__declspec(noinline) std::string opengl_errno_name(int err)
{
    switch (err)
    {
#define PRE_GL_ERROR(name)                                                                                             \
    case GL_##name:                                                                                                    \
        return #name;

        PRE_GL_ERROR(NO_ERROR)
        PRE_GL_ERROR(INVALID_ENUM)
        PRE_GL_ERROR(INVALID_VALUE)
        PRE_GL_ERROR(INVALID_OPERATION)
        PRE_GL_ERROR(STACK_OVERFLOW)
        PRE_GL_ERROR(STACK_UNDERFLOW)
        PRE_GL_ERROR(OUT_OF_MEMORY)
#undef PRE_GL_ERROR
    }
    return "Unknown error";
}

void check_gl_error(const char *filename, int lineno, const char *expr)
{
    int err = glGetError();
    if (err != 0) [[unlikely]]
    {
        std::cerr << filename << ":" << lineno << ": " << expr << " failed: " << opengl_errno_name(err) << std::endl;
        std::terminate();
    }
}

#if NDEBUG
#define CHECK_GL(x)                                                                                                    \
    do                                                                                                                 \
    {                                                                                                                  \
        (x);                                                                                                           \
    } while (0)
#else
#define CHECK_GL(x)                                                                                                    \
    do                                                                                                                 \
    {                                                                                                                  \
        (x);                                                                                                           \
        check_gl_error(__FILE__, __LINE__, #x);                                                                        \
    } while (0)
#endif