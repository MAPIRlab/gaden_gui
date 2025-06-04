#pragma once
#include <cstdio>
#include <filesystem>
#include <fmt/format.h>

namespace Utils
{
    namespace internal
    {
        inline std::string Zenity(const char* command)
        {
            char buffer[1024];
            FILE* f = popen(command, "r");
            fgets(buffer, 1024, f);

            // zenity returns a newline character at the end!
            std::string response(buffer);
            response.erase(response.end() - 1);
            return response;
        }

    } // namespace internal
    inline std::filesystem::path FileDialog()
    {
        return std::filesystem::path(internal::Zenity("zenity --file-selection"));
    }

    inline std::filesystem::path DirectoryDialog()
    {
        return std::filesystem::path(internal::Zenity("zenity --file-selection --directory"));
    }

    inline std::string TextInput(std::string_view title, std::string_view prompt)
    {
        std::string command = fmt::format("zenity --entry --title={} --text={}", title, prompt);
        return internal::Zenity(command.c_str());
    }
} // namespace Utils