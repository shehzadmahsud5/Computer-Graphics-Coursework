#include "texture.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.hpp" // Assuming stb_image.hpp is in the same common/ directory
#include <cstdio>
#include <iostream>
#include <GL/glew.h>

// Helper function to check for GL errors
static void checkGLError(const char* checkpoint_file, int checkpoint_line, const char* call_name = "") {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error after " << call_name << " at " << checkpoint_file << ":" << checkpoint_line << " - Error Code: " << err;
        switch (err) {
        case GL_INVALID_ENUM: std::cerr << " (GL_INVALID_ENUM)"; break;
        case GL_INVALID_VALUE: std::cerr << " (GL_INVALID_VALUE)"; break;
        case GL_INVALID_OPERATION: std::cerr << " (GL_INVALID_OPERATION)"; break;
        case GL_STACK_OVERFLOW: std::cerr << " (GL_STACK_OVERFLOW)"; break;
        case GL_STACK_UNDERFLOW: std::cerr << " (GL_STACK_UNDERFLOW)"; break;
        case GL_OUT_OF_MEMORY: std::cerr << " (GL_OUT_OF_MEMORY)"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: std::cerr << " (GL_INVALID_FRAMEBUFFER_OPERATION)"; break;
        default: std::cerr << " (Unknown Error)"; break;
        }
        std::cerr << std::endl;
    }
}
#define CHECK_GL_ERROR(call) checkGLError(__FILE__, __LINE__, #call)

unsigned int loadTexture(const char* path, bool flipVertically)
{
    unsigned int textureID = 0; // Initialize to 0
    glGenTextures(1, &textureID);
    CHECK_GL_ERROR(glGenTextures);

    if (textureID == 0) {
        std::cerr << "TEXTURE_LOAD_ERROR: Failed to generate texture ID (OpenGL context issue?) for path: " << path << std::endl;
        return 0;
    }

    std::cout << "Attempting to load texture: " << path << std::endl;
    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(flipVertically);
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);

    if (data)
    {
        std::cout << "Loaded texture '" << path << "' - Width: " << width << ", Height: " << height << ", Components: " << nrComponents << std::endl;

        GLenum internalFormat = 0;
        GLenum dataFormat = 0;

        if (nrComponents == 1) {
            internalFormat = GL_R8;
            dataFormat = GL_RED;
        }
        else if (nrComponents == 2) { // ADDED THIS CASE
            internalFormat = GL_RG8;
            dataFormat = GL_RG;
        }
        else if (nrComponents == 3) {
            internalFormat = GL_RGB8;
            dataFormat = GL_RGB;
        }
        else if (nrComponents == 4) {
            internalFormat = GL_RGBA8;
            dataFormat = GL_RGBA;
        }
        else {
            std::cerr << "TEXTURE_LOAD_ERROR: Texture failed to load at path: " << path << " - unsupported number of components: " << nrComponents << std::endl;
            stbi_image_free(data);
            glDeleteTextures(1, &textureID);
            // CHECK_GL_ERROR(glDeleteTextures); // Not strictly necessary before return
            return 0;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        CHECK_GL_ERROR(glBindTexture);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Try setting unpack alignment to 1
        CHECK_GL_ERROR(glPixelStorei_unpack);

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        CHECK_GL_ERROR(glTexImage2D); // <<< THIS IS LIKELY WHERE THE ERROR OCCURS

        glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // Reset to default
        CHECK_GL_ERROR(glPixelStorei_pack);


        glGenerateMipmap(GL_TEXTURE_2D);
        CHECK_GL_ERROR(glGenerateMipmap);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); CHECK_GL_ERROR(glTexParameteri_WRAP_S);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); CHECK_GL_ERROR(glTexParameteri_WRAP_T);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); CHECK_GL_ERROR(glTexParameteri_MIN_FILTER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); CHECK_GL_ERROR(glTexParameteri_MAG_FILTER);

        stbi_image_free(data);
    }
    else
    {
        std::cerr << "TEXTURE_LOAD_ERROR: Texture failed to load at path (stbi_load returned null): " << path << std::endl;
        std::cerr << "STB Image Error: " << stbi_failure_reason() << std::endl;
        glDeleteTextures(1, &textureID);
        // CHECK_GL_ERROR(glDeleteTextures); // Not strictly necessary
        return 0;
    }

    glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture
    // CHECK_GL_ERROR(glBindTexture_Unbind); // Usually not problematic
    std::cout << "Successfully configured texture '" << path << "', ID: " << textureID << std::endl;
    return textureID;
}