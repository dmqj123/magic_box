#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <shlobj.h>

// 确保所有CSIDL常量都有定义
#ifndef CSIDL_DESKTOP
#define CSIDL_DESKTOP 0x0000
#endif
#ifndef CSIDL_MYDOCUMENTS
#define CSIDL_MYDOCUMENTS 0x0005
#endif
#ifndef CSIDL_MYPICTURES
#define CSIDL_MYPICTURES 0x0027
#endif
#ifndef CSIDL_MYMUSIC
#define CSIDL_MYMUSIC 0x000D
#endif
#ifndef CSIDL_MYVIDEO
#define CSIDL_MYVIDEO 0x000E
#endif
#ifndef CSIDL_DOWNLOADS
#define CSIDL_DOWNLOADS 0x0040
#endif

// 将字符串转换为小写
std::string toLower(const std::string& str) {
    std::string lowerStr;
    for (char c : str) {
        lowerStr += tolower(c);
    }
    return lowerStr;
}

// 转义JSON字符串中的特殊字符
std::string escapeJsonString(const std::string& str) {
    std::string output;
    for (char c : str) {
        switch (c) {
        case '"':  output += "\\\""; break;
        case '\\': output += "\\\\"; break;
        case '\b': output += "\\b";  break;
        case '\f': output += "\\f";  break;
        case '\n': output += "\\n";  break;
        case '\r': output += "\\r";  break;
        case '\t': output += "\\t";  break;
        default:   output += c;      break;
        }
    }
    return output;
}

// 检查文件扩展名是否符合要求
bool isValidExtension(const std::string& filename) {
    // 获取文件扩展名（小写）
    size_t dotPos = filename.find_last_of('.');
    if (dotPos == std::string::npos) return false;

    std::string ext = toLower(filename.substr(dotPos));

    // 允许的扩展名列表
    const std::vector<std::string> allowedExtensions = {
        ".txt", ".doc", ".docx", ".ppt", ".pptx", ".mp3", ".mp4", ".md", ".cpp", ".zip", ".7z", ".exe", ".png", ".gif", ".jpg", ".jpeg"
    };

    // 检查是否在允许列表中
    for (const auto& allowed : allowedExtensions) {
        if (ext == allowed) return true;
    }

    return false;
}

// 递归搜索目录
void searchDirectory(const std::string& path, const std::string& keyword) {
    WIN32_FIND_DATAA findData;
    HANDLE hFind;
    std::string searchPath = path + "\\*";

    hFind = FindFirstFileA(searchPath.c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }

    do {
        if (findData.cFileName[0] == '.') {
            continue; // 跳过 "." 和 ".."
        }

        std::string fullPath = path + "\\" + findData.cFileName;

        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // 递归搜索子目录
            searchDirectory(fullPath, keyword);
        }
        else {
            std::string filename = findData.cFileName;

            // 检查文件扩展名
            if (!isValidExtension(filename)) {
                continue;
            }

            // 检查文件名是否包含关键字
            std::string lowerName = toLower(filename);

            if (lowerName.find(keyword) != std::string::npos) {
                // 构造打开文件资源管理器的命令
                std::string openCmd = "explorer.exe \"" + fullPath + "\"";

                // 输出JSON格式结果
                std::cout << "{"
                    << "\"title\":\"" << escapeJsonString(filename) << "\","
                    << "\"content\":\"" << escapeJsonString(fullPath) << "\","
                    << "\"cmd\":\"" << escapeJsonString(openCmd) << "\""
                    << "}\n"
                    << "\nnext_result\n";
            }
        }
    } while (FindNextFileA(hFind, &findData));

    FindClose(hFind);
}

int main(int argc, char* argv[]) {
    // 检查命令行参数
    if (argc != 3 || std::string(argv[1]) != "-k") {
        std::cerr << "Usage: " << argv[0] << " -k <keyword>\n";
        return 1;
    }

    std::string keyword = toLower(argv[2]);  // 获取并转换为小写关键字

    // 如果关键字为空，则直接退出
    if (keyword.empty()) {
        return 0;
    }

    // 定义要搜索的目录类型
    const std::vector<int> folderTypes = {
        CSIDL_DESKTOP,         // 桌面
        CSIDL_MYDOCUMENTS,      // 文档
        CSIDL_MYPICTURES,       // 图片
        CSIDL_MYMUSIC,          // 音乐
        CSIDL_MYVIDEO,          // 视频
        CSIDL_DOWNLOADS         // 下载
    };

    // 搜索所有指定目录
    for (int folderType : folderTypes) {
        CHAR folderPath[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(NULL, folderType, NULL, 0, folderPath))) {
            searchDirectory(folderPath, keyword);
        }
        else {
            // 特殊处理下载文件夹（某些旧系统可能不支持CSIDL_DOWNLOADS）
            if (folderType == CSIDL_DOWNLOADS) {
                // 尝试替代方法获取下载文件夹
                if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, folderPath))) {
                    std::string downloadsPath = std::string(folderPath) + "\\Downloads";
                    searchDirectory(downloadsPath, keyword);
                }
            }
        }
    }

    return 0;
}