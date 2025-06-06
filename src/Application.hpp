#pragma once
#include "Modes/Mode.hpp"
#include "Project.hpp"
#include <memory>
#include <stack>

class Application
{
public:
    void Run();
    void PushMode(std::shared_ptr<Mode> mode);
    void PopMode();
    std::shared_ptr<Mode> GetCurrentMode();

public:
    bool shouldClose = false;
    std::shared_ptr<Project> project;

private:
    std::stack<std::shared_ptr<Mode>> modeStack;
};

inline std::unique_ptr<Application> g_app;