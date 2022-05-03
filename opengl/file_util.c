#include "file_util.h"

char *
load_text_file(const char *shader_name) {
    char *buffer;
    long length;
    FILE *file = fopen(shader_name, "rb");

    if (file) {
        fseek(file, 0, SEEK_END);
        length = ftell(file);
        fseek(file, 0, SEEK_SET);
        buffer = malloc(length + 1);
        if (buffer) {
            fread(buffer, 1, length, file);
        }
        fclose(file);
        buffer[length] = '\0';
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error reading file: %s, errno: %d", shader_name, errno);
        exit(1);
    }
    return buffer;
}
