#include "Scene.hpp"
#include "GLFW/glfw3.h"
#include "imgui.h"
#include <iostream>

// this file has a hell of a lot of boilerplate to set up OpenGL rendering...
// the actual meat is actually very little code:
//      - give the correct format to the triangle arrays
//      - create bind array and index buffers
//      - create and bind the framebuffer to a textureID
//      - render into framebuffer
//      - pass the textureID to ImGui through ImGui::Image

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\n\0";

constexpr ImVec2 windowSize = ImVec2(800, 600);

void add_shader(GLuint program, const char* shader_code, GLenum type)
{
    GLuint current_shader = glCreateShader(type);

    const GLchar* code[1];
    code[0] = shader_code;

    GLint code_length[1];
    code_length[0] = strlen(shader_code);

    glShaderSource(current_shader, 1, code, code_length);
    glCompileShader(current_shader);

    GLint result = 0;
    GLchar log[1024] = {0};

    glGetShaderiv(current_shader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(current_shader, sizeof(log), NULL, log);
        std::cerr << "Error compiling " << type << " shader: " << log << "\n";
        return;
    }

    glAttachShader(program, current_shader);
}

GLuint create_shaders()
{
    GLuint shader = glCreateProgram();
    if (!shader)
    {
        std::cout << "Error creating shader program!\n";
        exit(1);
    }

    add_shader(shader, vertexShaderSource, GL_VERTEX_SHADER);
    add_shader(shader, fragmentShaderSource, GL_FRAGMENT_SHADER);

    GLint result = 0;
    GLchar log[1024] = {0};

    glLinkProgram(shader);
    glGetProgramiv(shader, GL_LINK_STATUS, &result);
    if (!result)
    {
        glGetProgramInfoLog(shader, sizeof(log), NULL, log);
        std::cout << "Error linking program:\n"
                  << log << '\n';
        return 0;
    }

    glValidateProgram(shader);
    glGetProgramiv(shader, GL_VALIDATE_STATUS, &result);
    if (!result)
    {
        glGetProgramInfoLog(shader, sizeof(log), NULL, log);
        std::cout << "Error validating program:\n"
                  << log << '\n';
        return 0;
    }

    return shader;
}

Scene::Scene(std::vector<std::vector<gaden::Triangle>> const& models)
{
    uint32_t vertexIndex = 0;
    for (auto const& model : models)
        for (auto const& triangle : model)
        {
            vertexArray.push_back(triangle.p1.x);
            vertexArray.push_back(triangle.p1.y);
            vertexArray.push_back(triangle.p1.z);

            vertexArray.push_back(triangle.p2.x);
            vertexArray.push_back(triangle.p2.y);
            vertexArray.push_back(triangle.p2.z);

            vertexArray.push_back(triangle.p3.x);
            vertexArray.push_back(triangle.p3.y);
            vertexArray.push_back(triangle.p3.z);

            for (size_t i = 0; i < 9; i++)
                triangleArray.push_back(vertexIndex++);
        }

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        raise(SIGTRAP);
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexArray.size() * sizeof(float), vertexArray.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangleArray.size() * sizeof(uint32_t), triangleArray.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    shader = create_shaders();
    create_framebuffer();
}

void Scene::Render()
{
    ImGui::SetNextWindowSize(windowSize);
    ImGui::Begin("Scene", &active, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    const float window_width = ImGui::GetContentRegionAvail().x;
    const float window_height = ImGui::GetContentRegionAvail().y;

    // currently unnecessary, we have a fixed size window!
    //  rescale_framebuffer(window_width, window_height);

    bind_framebuffer();

    glUseProgram(shader);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, triangleArray.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    unbind_framebuffer();

    ImGui::Image((ImTextureID)(intptr_t)texture_id, ImVec2(window_width, window_height));
    ImGui::End();
}

void Scene::create_framebuffer()
{
    const GLint WIDTH = windowSize.x;
    const GLint HEIGHT = windowSize.y;

    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id, 0);

    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n";

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void Scene::bind_framebuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
}

void Scene::unbind_framebuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Scene::rescale_framebuffer(float width, float height)
{
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
}