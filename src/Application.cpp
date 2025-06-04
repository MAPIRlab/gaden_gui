#include "Application.hpp"
#include "Modes/DefaultMode.hpp"
#include "ament_imgui/ament_imgui.h"
#include <chrono>
#include <thread>

void Application::Run()
{
    AmentImgui imgui;
    imgui.Setup(nullptr,
                "Gaden",
                900,
                700,
                imgui.FlagsFixedLayout());

    auto defaultMode = std::make_shared<DefaultMode>();
    PushMode(defaultMode);

    while (!shouldClose && !imgui.ShouldClose())
    {
        imgui.StartFrame();
        imgui.SetNextWindowFullscreen();

        ImGui::Begin("Main");
        modeStack.top()->OnGUI();
        ImGui::End();

        imgui.Render();
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
    }

    imgui.Close();
}

void Application::PushMode(std::shared_ptr<Mode> mode)
{
    if (!modeStack.empty())
        modeStack.top()->OnLoseFocus();

    modeStack.push(mode);

    modeStack.top()->OnPush();
    modeStack.top()->OnGainFocus();
}

void Application::PopMode()
{
    modeStack.top()->OnLoseFocus();
    modeStack.top()->OnPop();

    modeStack.pop();

    if (!modeStack.empty())
        modeStack.top()->OnGainFocus();
}

std::shared_ptr<Mode> Application::GetCurrentMode()
{
    return modeStack.top();
}
