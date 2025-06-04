#pragma once
#include "Modes/Mode.hpp"
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

private:
    std::stack<std::shared_ptr<Mode>> modeStack;
};

inline std::unique_ptr<Application> g_app;