#include "Common.h"



std::vector<std::string> GetFileList(std::string path, std::string extension)
{
    std::vector<std::string> files;
   
    if (std::filesystem::exists(path))		//必须先检测目录是否存在才能使用文件入口.
    {
        std::filesystem::directory_entry entry(path);
        //if (entry.status().type() == std::filesystem::file_type::directory)	//这里用了C++11的强枚举类型
            //std::cout << "该路径是一个目录" << std::endl;
        std::filesystem::directory_iterator list(path);
        for (auto& p : list)
        {
            if (!extension.empty() && extension != p.path().extension().string())
                continue;

            files.push_back(p.path().string());
        }
    }
     //note: assuming already sorted alpha numerically

    return files;
}

std::tuple<std::vector<std::string>, std::vector<std::string>> ReadRGBDColorFiles(std::string path)
{
    std::vector<std::string> colorFiles, depthFiles;

    colorFiles = GetFileList(path + "rgb");
    depthFiles = GetFileList(path + "depth");

    return std::make_tuple(colorFiles, depthFiles);
}

void MakeFolder(std::string path_folder)
{
    CreateDirectoryA(path_folder.c_str(), nullptr);
}

std::string FullPath(std::string path)
{
    char fullPath[MAX_PATH];

    GetFullPathNameA(path.c_str(), MAX_PATH, fullPath, NULL);

    return fullPath;
}