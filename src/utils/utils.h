//
// Created by william on 2022/6/1.
//

#ifndef GRAPHICRENDERENGINE_UTILS_H
#define GRAPHICRENDERENGINE_UTILS_H
#include "utils/commonMacro.h"

#include <filesystem>
#include <fstream>

static std::string getFileContents(const std::filesystem::path& filename)
{
    std::ifstream in{ filename, std::ios::in | std::ios::binary };
    if (in)
    {
        std::string contents;
        in.seekg(0, std::ios::end);
        auto size = in.tellg();
        if (size > 0)
        {
            contents.resize((std::string::size_type)size);
            in.seekg(0, std::ios::beg);
            in.read(&contents[0], contents.size());
        }
        in.close();
        return contents;
    }
    LOG_ERROR("Error reading {}. Error code: {}", filename.c_str(), errno);
}

static std::string_view getCurrentOperationSystem()
{
#ifdef IS_WINDOWS
    return "Windows";
#elif IS_LINUX
    return "Linux";
#elif IS_MACOS
    return "macOS";
#else
    return "unknown system!";
#endif
}

static std::vector<std::string> getCurrentDirFiles(std::string_view fileDir)
{
    std::vector<std::string> files;
    std::filesystem::directory_iterator lists(fileDir);
    for (const auto& file : lists)
    {
        files.emplace_back(file.path().filename().c_str());
    }
    return files;
}

#endif // GRAPHICRENDERENGINE_UTILS_H
