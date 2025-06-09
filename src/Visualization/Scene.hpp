#pragma once
#include "Visualization/Camera.hpp"
#include "Visualization/RenderModel.hpp"
#include "Visualization/Shader.hpp"
#include <gaden/internal/Triangle.hpp>
#include <glad/include/glad/glad.h>
#include <optional>

class Scene
{
public:
    Scene(std::vector<std::vector<gaden::Triangle>> const& models, std::vector<gaden::Color> const& colors);
    void Render();
    bool active = false;

private:
    void create_framebuffer();

    void unbind_framebuffer();

    void rescale_framebuffer(float width, float height);

    void bind_framebuffer();

private:

    std::vector<RenderModel> renderModels;
    GLuint FBO; // frame buffer object
    GLuint RBO; // render buffer object
    GLuint texture_id;
    
    std::optional<Shader> shader;

    Camera camera;
};