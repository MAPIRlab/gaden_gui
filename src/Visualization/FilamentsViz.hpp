#pragma once
#include "Visualization/Shader.hpp"
#include "gaden/datatypes/Filament.hpp"
#include <glm/vec3.hpp>
#include <mutex>
#include <optional>
#include <vector>

class FilamentsViz
{
public:
    FilamentsViz(class Scene& scene);
    void SetUp();
    void SetFilaments(std::vector<gaden::Filament> const& filaments);
    void Draw();
    void CleanUp();

private:
    std::mutex mutex;
    std::vector<glm::vec3> positions;
    std::optional<Shader> shader;
    Scene& scene;

    GLuint instanceVBO;
    GLuint quadVAO, quadVBO;
};