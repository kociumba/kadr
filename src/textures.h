#ifndef TEXTURES_H
#define TEXTURES_H

#include <stdint.h>
#include "graphics.h"

// Uploads an SDL3 surface to an OpenGL 2D texture.
//
// Parameters:
//   surface          - the SDL_Surface to upload
//   existing_texture - an existing GL texture ID to update (0 to create new)
//
// Returns:
//   OpenGL texture ID on success, 0 on failure.
//
// Notes:
//   - Requires an active OpenGL context.
//   - Converts to SDL_PIXELFORMAT_RGBA32 (RGBA in memory order) if needed.
//   - Handles SDL surface pitch (row padding) via GL_UNPACK_ROW_LENGTH.
inline GLuint surface_to_gl_tex(SDL_Surface* surface, GLuint existing_texture = 0) {
    if (!surface || !surface->pixels || surface->w <= 0 || surface->h <= 0) return 0;

    SDL_Surface* upload = surface;
    SDL_Surface* converted = nullptr;
    GLenum gl_format = 0;

    if (surface->format == SDL_PIXELFORMAT_BGRA32) {
        gl_format = GL_BGRA;
    } else if (surface->format == SDL_PIXELFORMAT_RGBA32) {
        gl_format = GL_RGBA;
    } else {
        converted = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
        if (!converted) return 0;
        upload = converted;
        gl_format = GL_RGBA;
    }

    GLuint texture = existing_texture;
    if (texture == 0) { glGenTextures(1, &texture); }

    // Move declarations BEFORE any potential early-exit/goto
    GLint prev_tex = 0, prev_row_length = 0, prev_align = 0;

    if (texture == 0) {
        if (converted) SDL_DestroySurface(converted);
        SDL_Log("failed to generate a texture for upload: %s", SDL_GetError());
        return 0;
    }

    glGetIntegerv(GL_TEXTURE_BINDING_2D, &prev_tex);
    glGetIntegerv(GL_UNPACK_ROW_LENGTH, &prev_row_length);
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &prev_align);

    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, (upload->pitch == upload->w * 4) ? 0 : upload->pitch / 4);

    SDL_Log("=== surface_to_gl_tex diagnostics ===");
    SDL_Log("surface->format: %s (enum value: %u)",
        SDL_GetPixelFormatName(upload->format),
        (unsigned)upload->format);
    SDL_Log("surface->w: %d, surface->h: %d", upload->w, upload->h);
    SDL_Log("surface->pitch: %d (expected for w*4: %d)", upload->pitch, upload->w * 4);
    SDL_Log("surface->pixels ptr: %p", upload->pixels);
    SDL_Log("gl_format: 0x%X (GL_RGBA=0x%X, GL_BGRA=0x%X)", (unsigned)gl_format, GL_RGBA, GL_BGRA);
    SDL_Log("GL_UNPACK_ROW_LENGTH set to: %d",
        (upload->pitch == upload->w * 4) ? 0 : upload->pitch / 4);

    glTexImage2D(GL_TEXTURE_2D,
        0,
        GL_RGBA8,
        upload->w,
        upload->h,
        0,
        gl_format,
        GL_UNSIGNED_BYTE,
        upload->pixels);

    GLenum err = glGetError();
    SDL_Log("glTexImage2D: 0x%X (%s)", err, err == GL_NO_ERROR ? "GL_NO_ERROR" : "ERROR");

    GLint tex_w = 0, tex_h = 0;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &tex_w);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &tex_h);
    SDL_Log("GL reports texture dimensions: %d x %d", tex_w, tex_h);
    SDL_Log("returned texture ID: %u", texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glPixelStorei(GL_UNPACK_ROW_LENGTH, prev_row_length);
    glPixelStorei(GL_UNPACK_ALIGNMENT, prev_align);
    glBindTexture(GL_TEXTURE_2D, prev_tex);

    if (converted) SDL_DestroySurface(converted);
    return texture;
}

// ------------------------------------------------------------------------
// ImGui helpers
// ------------------------------------------------------------------------

// Convenience wrapper that returns an ImTextureID directly.
inline ImTextureID surface_to_imgui(SDL_Surface* surface) {
    GLuint tex = surface_to_gl_tex(surface);
    return (ImTextureID) static_cast<uintptr_t>(tex);
}

// Call this when you're done with the texture (e.g., on app shutdown).
inline void destroy_tex(ImTextureID tex_id) {
    GLuint gl_id = static_cast<GLuint>((uintptr_t)tex_id);
    if (gl_id != 0) glDeleteTextures(1, &gl_id);
}

#endif /* TEXTURES_H */
