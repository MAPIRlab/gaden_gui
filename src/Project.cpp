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
        if (!std::filesystem::exists(configurationsDir))
            return false;
        auto subDirs = gaden::paths::GetAllFilesInDirectory(configurationsDir);

        for (auto const& dir : subDirs)
        {
            gaden::EnvironmentConfigMetadata config(dir);
            GADEN_CHECK_RESULT(config.ReadDirectory());
            configurations.insert({config.GetName(), config});
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
        configurations.insert({config.GetName(), config});
    else
    {
        GADEN_ERROR("Failed to create sample configuration at '{}'", configPath);
        return false;
    }

    return true;
}
