//
// Created by william on 2022/5/24.
//

#ifndef GRAPHICRENDERENGINE_GLCOMMONDEFINE_H
#define GRAPHICRENDERENGINE_GLCOMMONDEFINE_H
#include "commonMacro.h"

//#define GLFW_INCLUDE_GLCOREARB

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <sstream>
#include <string>

#define CHECK_GL(x)                 \
    do                              \
    {                               \
        (x);                        \
        checkGlError(TO_STRING(x)); \
    } while (0)

// For internal debugging of gl errors
inline void checkGlError(std::string_view msg = {}) noexcept
{
    for (GLenum err; (err = glGetError()) != GL_NO_ERROR;)
    {
        if (err != GL_NONE)
        {
            if (!msg.empty()) LOG_ERROR("{}", msg);
        }
        switch (err)
        {
        case GL_INVALID_ENUM:
            LOG_ERROR("GL_INVALID_ENUM");
            break;
        case GL_INVALID_VALUE:
            LOG_ERROR("GL_INVALID_VALUE");
            break;
        case GL_INVALID_OPERATION:
            LOG_ERROR("GL_INVALID_OPERATION");
            break;
        case GL_OUT_OF_MEMORY:
            LOG_ERROR("GL_OUT_OF_MEMORY");
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            LOG_ERROR("GL_INVALID_FRAMEBUFFER_OPERATION");
            break;
        }
    }
}

inline bool hasExtension(std::string_view extensionName)
{
    // std::string extension = (const char*)glGetString(GL_EXTENSIONS); crash !!
    int NumberOfExtensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &NumberOfExtensions);
    for (int i = 0; i < NumberOfExtensions; i++)
    {
        const char* e = (const char*)glGetStringi(GL_EXTENSIONS, i);
        if (extensionName == e)
        {
            LOG_INFO("find Extension:{}", e);
            return true;
        }
    }
    return false;
}
inline std::vector<std::string> listExtension()
{
    // std::string extension = (const char*)glGetString(GL_EXTENSIONS); crash !!
    int NumberOfExtensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &NumberOfExtensions);
    std::vector<std::string> extensions;
    for (int i = 0; i < NumberOfExtensions; i++)
    {
        const char* e = (const char*)glGetStringi(GL_EXTENSIONS, i);
        extensions.emplace_back(e);
    }
    return extensions;
}

inline bool hasSRGB()
{
    static bool res = hasExtension("GL_EXT_sRGB");
    return res;
}

#endif // GRAPHICRENDERENGINE_GLCOMMONDEFINE_H
