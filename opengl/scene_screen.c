#include "scene_screen.h"
#include "assert.h"
#include "camera.h"

void
destroy_scene_screen_contents(scene_screen_t *scene_screen) {
    if (scene_screen->frame_buffer >= 0) {
        glDeleteFramebuffers(1, &scene_screen->frame_buffer);
        scene_screen->frame_buffer = -1;
    }
    if (scene_screen->texture >= 0) {
        glDeleteTextures(1, &scene_screen->texture);
        scene_screen->texture = -1;
    }
    if (scene_screen->render_buffer >= 0) {
        glDeleteRenderbuffers(1, &scene_screen->render_buffer);
        scene_screen->render_buffer = -1;
    }
}

void
update_scene_screen(scene_screen_t *scene_screen, camera_t *camera) {
    if (camera == NULL) {
        SDL_Die("Attempt to draw without a camera");
        assert(camera != NULL);
    }

    if (scene_screen->width == camera->viewport_width && scene_screen->height == camera->viewport_height) {
        return;
    }

    destroy_scene_screen_contents(scene_screen);

    scene_screen->width = camera->viewport_width;
    scene_screen->height = camera->viewport_height;

    glGenFramebuffers(1, &scene_screen->frame_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, scene_screen->frame_buffer);
    GL_CHECK_ERROR;

    glGenTextures(1, &scene_screen->texture);
    glBindTexture(GL_TEXTURE_2D, scene_screen->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (int) scene_screen->width, (int) scene_screen->height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, scene_screen->texture, 0);
    GL_CHECK_ERROR;

    glGenRenderbuffers(1, &scene_screen->render_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, scene_screen->render_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (int) scene_screen->width, (int) scene_screen->height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              scene_screen->render_buffer);
    if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_FRAMEBUFFER)) {
        SDL_Die("Frame buffer incomplete");
    }
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    GL_CHECK_ERROR;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
