#define STB_IMAGE_IMPLEMENTATION

#include "texture.h"

void
load_texture(GLenum texture_unit, const char *filename) {
    int width, height, channels_number;

    unsigned int texture;
    glGenTextures(1, &texture);
    glActiveTexture(texture_unit);
    glBindTexture(GL_TEXTURE_2D, texture);

    // configure texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // creating from external file
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(filename, &width, &height, &channels_number, 0);
    if (data == NULL) {
        SDL_Die("Error loading texture %s", filename);
    } else {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                    "Loaded %s: %u x %u, channels %u", filename, width, height, channels_number);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    stbi_image_free(data);
}
