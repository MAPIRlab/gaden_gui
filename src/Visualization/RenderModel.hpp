#pragma once

#include "Visualization/Utils.hpp"
#include "gaden/datatypes/Model3D.hpp"
#include "gaden/internal/Triangle.hpp"
#include <glad/include/glad/glad.h>

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec3 Color;
};

class RenderModel
{
public:
    std::vector<Vertex> vertexArray;
    std::vector<uint32_t> triangleArray;

    RenderModel(std::vector<gaden::Triangle> const& triangles, gaden::Color color)
    {
        uint32_t vertexIndex = 0;
        for (auto const& triangle : triangles)
        {
            // opengl coordinates are weird
            auto p1 = VizUtils::toGL(triangle.p1);
            auto p2 = VizUtils::toGL(triangle.p2);
            auto p3 = VizUtils::toGL(triangle.p3);

            auto normal = VizUtils::toGL(triangle.normal());

            vertexArray.push_back(Vertex{.Position = p1, .Normal = normal, .Color = {color.r, color.g, color.b}});
            vertexArray.push_back(Vertex{.Position = p2, .Normal = normal, .Color = {color.r, color.g, color.b}});
            vertexArray.push_back(Vertex{.Position = p3, .Normal = normal, .Color = {color.r, color.g, color.b}});

            for (size_t i = 0; i < 9; i++)
                triangleArray.push_back(vertexIndex++);
        }

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertexArray.size() * sizeof(Vertex), vertexArray.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangleArray.size() * sizeof(uint32_t), triangleArray.data(), GL_STATIC_DRAW);

        // attributes
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Color));

        glBindVertexArray(0);
    }

    void Draw() const
    {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, triangleArray.size(), GL_UNSIGNED_INT, 0);
    }

private:
    GLuint VAO; // vertex array object
    GLuint VBO; // vetex buffer object
    GLuint EBO; // element buffer object
};