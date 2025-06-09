#pragma once
#include <gaden/internal/Triangle.hpp>
#include <glad/include/glad/glad.h>

class Scene
{
public:
    Scene(std::vector<std::vector<gaden::Triangle>> const& models);
    void Render();
    bool active = false;

private:
    void create_framebuffer();

    void unbind_framebuffer();

    void rescale_framebuffer(float width, float height);

    void bind_framebuffer();

private:
    std::vector<float> vertexArray;
    std::vector<uint32_t> triangleArray;

    GLuint VAO; // vertex array object
    GLuint VBO; // vetex buffer object
    GLuint EBO; // element buffer object

    GLuint FBO; // frame buffer object
    GLuint RBO; // render buffer object
    GLuint texture_id;
    
    GLuint shader;
};