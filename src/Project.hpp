#pragma once
#include <gaden/EnvironmentConfigMetadata.hpp>

class Project
{
public:
    Project(std::filesystem::path const& path);
    bool ReadDirectory();
    bool CreateTemplate();
    std::filesystem::path GetRoot() { return rootDirectory; }
    std::string GetConfigurationPath(std::string const& name) { return rootDirectory / "environment_configurations" / name; }

public:
    std::vector<gaden::EnvironmentConfigMetadata> configurations;

private:
    std::filesystem::path rootDirectory;
};