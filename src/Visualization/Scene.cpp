#include "Scene.hpp"
#include "Application.hpp"
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

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 color;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

out vec4 vertexColor; 
out vec3 vertexNormal;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0f);
    vertexColor = vec4(color,1);
    vertexNormal = normal;
})";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec4 vertexColor;
in vec3 vertexNormal;
uniform vec3 lightDir = vec3(1, -1, 0);

void main()
{
    float diff = max(dot(normalize(vertexNormal), normalize(lightDir)), 0.0) * 0.5 + 0.5;
	FragColor = vertexColor * diff;
})";

constexpr ImVec2 windowSize = ImVec2(800, 600);

Scene::Scene(std::vector<std::vector<gaden::Triangle>> const& models, std::vector<gaden::Color> const& colors)
{
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        raise(SIGTRAP);
    }

    for (size_t i = 0; i < models.size(); i++)
        renderModels.push_back({models[i], colors[i]});

    sphereMarker = RenderModel::CreateSphere(0.2f, 20, 20, gaden::Color{1, 0, 1, 1});

    shader.emplace(vertexShaderSource, fragmentShaderSource);

    create_framebuffer();
}

void Scene::Render(glm::vec3 markerPosition)
{
    ImGui::SetNextWindowSize(windowSize);
    ImGui::Begin("Scene", &active, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    // currently unnecessary, we have a fixed size window!
    const float window_width = ImGui::GetContentRegionAvail().x;
    const float window_height = ImGui::GetContentRegionAvail().y;
    //  rescale_framebuffer(window_width, window_height);

    camera.HandleInput(g_app->GetDeltaTime());

    // render into a framebuffer (texture), which will then be used by ImGui::Image
    bind_framebuffer();

    shader->use();

    // calculate the camera matrices and send them to the shader
    glm::mat4 projection = glm::perspective(glm::radians(60.f), 1.f, 0.1f, 100.0f);
    projection[1] *= -1;
    shader->setMat4("projection", projection);

    glm::mat4 view = camera.GetViewMatrix();
    shader->setMat4("view", view);
    shader->setVec3("lightDir", glm::vec3(0.3, 1, 0.5));

    // clear the buffer
    glClearColor(0.2f, 0.4f, 0.7f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    // draw
    for (auto const& model : renderModels)
        model.Draw(*shader);

    sphereMarker.transform.position = VizUtils::toGL(markerPosition);
    sphereMarker.Draw(*shader);
    // cleanup
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