#pragma once

#include "Visualization/Shader.hpp"
#include "Visualization/Transform.hpp"
#include "Visualization/Utils.hpp"
#include "gaden/datatypes/Color.hpp"
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
    Transform transform;

    RenderModel() {}
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

            for (size_t i = 0; i < 3; i++)
                triangleArray.push_back(vertexIndex++);
        }

        Setup();
    }

    void Setup()
    {
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

    void Draw(Shader const& shader) const
    {
        shader.setMat4("model", transform.GetTransformMatrix());
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, triangleArray.size(), GL_UNSIGNED_INT, 0);
    }

    static RenderModel CreateSphere(float radius, int sectorCount, int stackCount, gaden::Color gadenColor)
    {
        RenderModel sphere;
        glm::vec3 color = {gadenColor.r, gadenColor.g, gadenColor.b};

        /* GENERATE VERTEX ARRAY */
        float x, y, z, xy; // vertex position

        float sectorStep = (float)(2 * M_PI / sectorCount);
        float stackStep = (float)(M_PI / stackCount);
        float sectorAngle, stackAngle;

        for (int i = 0; i <= stackCount; ++i)
        {
            stackAngle = (float)(M_PI / 2 - i * stackStep); // starting from pi/2 to -pi/2
            xy = cosf(stackAngle);                          // r * cos(u)
            z = sinf(stackAngle);                           // r * sin(u)

            // add (sectorCount+1) vertices per stack
            // the first and last vertices have same position and normal, but different tex coords
            for (int j = 0; j <= sectorCount; ++j)
            {
                sectorAngle = j * sectorStep; // starting from 0 to 2pi

                // vertex position (x, y, z)
                x = xy * cosf(sectorAngle); // r * cos(u) * cos(v)
                y = xy * sinf(sectorAngle); // r * cos(u) * sin(v)
                glm::vec3 point = radius * glm::normalize(glm::vec3{x, y, z});
                sphere.vertexArray.push_back({.Position = point, .Normal = glm::normalize(point), .Color = color});
            }
        }

        /* GENERATE INDEX ARRAY */
        int k1, k2;
        for (int i = 0; i < stackCount; ++i)
        {
            k1 = i * (sectorCount + 1); // beginning of current stack
            k2 = k1 + sectorCount + 1;  // beginning of next stack

            for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
            {
                // 2 triangles per sector excluding first and last stacks
                // k1 => k2 => k1+1
                if (i != 0)
                {
                    sphere.triangleArray.push_back(k1);
                    sphere.triangleArray.push_back(k2);
                    sphere.triangleArray.push_back(k1 + 1);
                }

                // k1+1 => k2 => k2+1
                if (i != (stackCount - 1))
                {
                    sphere.triangleArray.push_back(k1 + 1);
                    sphere.triangleArray.push_back(k2);
                    sphere.triangleArray.push_back(k2 + 1);
                }
            }
        }

        sphere.Setup();
        return sphere;
    }

private:
    GLuint VAO; // vertex array object
    GLuint VBO; // vetex buffer object
    GLuint EBO; // element buffer object
};