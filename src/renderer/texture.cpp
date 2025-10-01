#include <iostream>
#include <memory>

#include <GL/glew.h>
#include "renderer/texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

std::unique_ptr<Texture> Texture::LoadFile(const std::string& filename) {
    auto texture = std::make_unique<Texture>();

    int w, h, c;
    unsigned char *data = stbi_load(filename.c_str(), &w, &h, &c, 4);
    if (!data) {
        std::cerr << "ERROR: Failed to load texture under '" << filename << "'" << std::endl;
        std::exit(1);
    }

    glGenTextures(1, &texture.get()->m_id);
    glBindTexture(GL_TEXTURE_2D, texture.get()->m_id);

    // TODO: configure properly
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // TODO: automatically detect values for this function
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    std::cout << "Loaded texture under '" << filename << "' with size of " << sizeof(data) << " bytes" << std::endl;
    stbi_image_free(data);

    return std::move(texture);
}
