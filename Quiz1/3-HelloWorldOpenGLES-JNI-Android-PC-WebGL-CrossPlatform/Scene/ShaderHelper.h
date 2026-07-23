#pragma once

#include "Platform.h"

#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class ShaderHelper
{
public:
    static GLuint compileShader(GLenum type, const char* src)
    {
        GLuint shader = glCreateShader(type);

        if (!shader)
        {
            LOGE("glCreateShader failed");
            return 0;
        }

        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        GLint ok = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);

        if (!ok)
        {
            GLint len = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);

            std::string log(static_cast<size_t>(len), '\0');
            glGetShaderInfoLog(shader, len, nullptr, &log[0]);

            LOGE("Shader compile error:\n%s", log.c_str());

            glDeleteShader(shader);
            return 0;
        }

        return shader;
    }

    static GLuint linkProgram(GLuint vert, GLuint frag)
    {
        GLuint program = glCreateProgram();

        glAttachShader(program, vert);
        glAttachShader(program, frag);
        glLinkProgram(program);

        GLint ok = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &ok);

        if (!ok)
        {
            GLint len = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);

            std::string log(static_cast<size_t>(len), '\0');
            glGetProgramInfoLog(program, len, nullptr, &log[0]);

            LOGE("Program link error:\n%s", log.c_str());

            glDeleteProgram(program);
            return 0;
        }

        glDetachShader(program, vert);
        glDetachShader(program, frag);
        glDeleteShader(vert);
        glDeleteShader(frag);

        return program;
    }

    static GLuint buildProgram(const char* vertSrc, const char* fragSrc)
    {
        GLuint vert = compileShader(GL_VERTEX_SHADER, vertSrc);

        if (!vert)
        {
            return 0;
        }

        GLuint frag = compileShader(GL_FRAGMENT_SHADER, fragSrc);

        if (!frag)
        {
            glDeleteShader(vert);
            return 0;
        }

        return linkProgram(vert, frag);
    }

#ifdef PLATFORM_ANDROID

    static std::string loadAsset(AAssetManager* mgr, const char* path)
    {
        if (mgr == nullptr)
        {
            LOGE("AAssetManager is null");
            return "";
        }

        AAsset* asset = AAssetManager_open(mgr, path, AASSET_MODE_BUFFER);

        if (asset == nullptr)
        {
            LOGE("Failed to open asset: %s", path);
            return "";
        }

        size_t size = AAsset_getLength(asset);

        std::string result;
        result.resize(size);

        AAsset_read(asset, result.data(), size);
        AAsset_close(asset);

        return result;
    }

    static GLuint buildProgramFromAssets(
            AAssetManager* mgr,
            const char* vertPath,
            const char* fragPath
    )
    {
        std::string vertSrc = loadAsset(mgr, vertPath);
        std::string fragSrc = loadAsset(mgr, fragPath);

        if (vertSrc.empty() || fragSrc.empty())
        {
            LOGE("Shader asset file not found");
            return 0;
        }

        return buildProgram(vertSrc.c_str(), fragSrc.c_str());
    }

#else

    static std::string loadFile(const char* filename)
    {
        std::vector<std::string> paths =
        {
            std::string("assets/shader/") + filename,
            std::string(filename),
            std::string("shader/") + filename,
            std::string("assets/") + filename
        };

        for (const std::string& path : paths)
        {
            std::ifstream file(path);

            if (file.is_open())
            {
                std::stringstream buffer;
                buffer << file.rdbuf();
                return buffer.str();
            }
        }

        LOGE("Shader file not found: %s", filename);
        return "";
    }

    static GLuint buildProgramFromFile(
        const char* vertFile,
        const char* fragFile
    )
    {
        std::string vertSrc = loadFile(vertFile);
        std::string fragSrc = loadFile(fragFile);

        if (vertSrc.empty() || fragSrc.empty())
        {
            LOGE("Shader file source is empty");
            return 0;
        }

        return buildProgram(vertSrc.c_str(), fragSrc.c_str());
    }

#endif
};