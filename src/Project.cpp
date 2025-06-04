#include "Project.hpp"
#include "gaden/internal/PathUtils.hpp"

Project::Project(std::filesystem::path const& path)
    : rootDirectory(path)
{
}

bool Project::ReadDirectory()
{
    try
    {
        std::filesystem::path configurationsDir = rootDirectory / "environment_configurations";
        auto subDirs = gaden::paths::GetAllFilesInDirectory(configurationsDir);

        configurations.reserve(subDirs.size());

        for (auto const& dir : subDirs)
        {
            gaden::EnvironmentConfigMetadata config(dir);
            GADEN_CHECK_RESULT(config.ReadDirectory());
            configurations.push_back(config);
        }
    }
    catch (std::exception const& e)
    {
        GADEN_ERROR("An error ocurred while trying to parse directory '{}':\n\t{}", rootDirectory, e.what());
        return false;
    }
    return true;
}

bool Project::CreateTemplate()
{
    std::filesystem::create_directories(rootDirectory / "wind_simulations");
    std::filesystem::create_directories(rootDirectory / "cad_models");

    std::filesystem::path configPath = GetConfigurationPath("config1");
    std::filesystem::create_directories(configPath);

    gaden::EnvironmentConfigMetadata config(configPath);
    if (config.CreateTemplate())
        configurations.push_back(config);
    else
    {
        GADEN_ERROR("Failed to create sample configuration at '{}'", configPath);
        return false;
    }
    
    return true;
}
