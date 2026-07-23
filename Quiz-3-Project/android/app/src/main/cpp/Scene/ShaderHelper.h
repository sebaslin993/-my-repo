#pragma once

/**
 * ShaderHelper.h
 *
 * Self-contained helper for compiling GLSL shaders and linking programs.
 * On Android, shaders are loaded from the APK assets via AAssetManager.
 *
 * Replaces the old GLPIFramework ShaderManager + ProgramManager combination
 * with a single, lightweight C++17 class.
 */

#include <GLES3/gl3.h>
#include <android/asset_manager.h>
#include <android/log.h>
#include <string>

#define SH_TAG "ShaderHelper"
#define SH_LOGI(...) __android_log_print(ANDROID_LOG_INFO,  SH_TAG, __VA_ARGS__)
#define SH_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, SH_TAG, __VA_ARGS__)

class ShaderHelper {
public:
    /**
     * Load a text file from APK assets.
     * Returns empty string on failure.
     */
    static std::string loadAsset(AAssetManager* mgr, const char* path) {
        if (!mgr || !path) return {};
        AAsset* asset = AAssetManager_open(mgr, path, AASSET_MODE_BUFFER);
        if (!asset) {
            SH_LOGE("Cannot open asset: %s", path);
            return {};
        }
        size_t size = static_cast<size_t>(AAsset_getLength(asset));
        std::string src(size, '\0');
        AAsset_read(asset, &src[0], size);
        AAsset_close(asset);
        return src;
    }

    /**
     * Compile a single shader from source.
     * Returns 0 on failure.
     */
    static GLuint compileShader(GLenum type, const char* src) {
        GLuint shader = glCreateShader(type);
        if (!shader) {
            SH_LOGE("glCreateShader failed");
            return 0;
        }
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        GLint ok = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            GLint len = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
            std::string log(len, '\0');
            glGetShaderInfoLog(shader, len, nullptr, &log[0]);
            SH_LOGE("Shader compile error:\n%s", log.c_str());
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }

    /**
     * Link a vertex + fragment shader into a GL program.
     * Returns 0 on failure.
     */
    static GLuint linkProgram(GLuint vert, GLuint frag) {
        GLuint prog = glCreateProgram();
        glAttachShader(prog, vert);
        glAttachShader(prog, frag);
        glLinkProgram(prog);

        GLint ok = GL_FALSE;
        glGetProgramiv(prog, GL_LINK_STATUS, &ok);
        if (!ok) {
            GLint len = 0;
            glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
            std::string log(len, '\0');
            glGetProgramInfoLog(prog, len, nullptr, &log[0]);
            SH_LOGE("Program link error:\n%s", log.c_str());
            glDeleteProgram(prog);
            return 0;
        }
        // Shaders can be detached/deleted after linking
        glDetachShader(prog, vert);
        glDetachShader(prog, frag);
        glDeleteShader(vert);
        glDeleteShader(frag);
        return prog;
    }

    /**
     * Convenience: load from assets, compile, and link in one call.
     * Returns 0 on any failure.
     */
    static GLuint buildProgramFromAssets(AAssetManager* mgr,
                                         const char* vertPath,
                                         const char* fragPath) {
        std::string vertSrc = loadAsset(mgr, vertPath);
        std::string fragSrc = loadAsset(mgr, fragPath);
        if (vertSrc.empty() || fragSrc.empty()) return 0;

        GLuint vert = compileShader(GL_VERTEX_SHADER,   vertSrc.c_str());
        if (!vert) return 0;
        GLuint frag = compileShader(GL_FRAGMENT_SHADER, fragSrc.c_str());
        if (!frag) { glDeleteShader(vert); return 0; }

        return linkProgram(vert, frag);
    }
};
