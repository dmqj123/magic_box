#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <shlobj.h>
#include <io.h>
#include <fcntl.h>
#include <codecvt>
#include <locale>

// 确保所有CSIDL常量都有定义
#ifndef CSIDL_DESKTOP
#define CSIDL_DESKTOP 0x0000
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
        ".txt", ".doc", ".docx", ".ppt", ".pptx", ".mp3", ".mp4", ".md", ".cpp", ".zip", ".7z", ".png", ".gif", ".jpg", ".jpeg"
    };

    // 检查是否在允许列表中
    for (const auto& allowed : allowedExtensions) {
        if (ext == allowed) return true;
    }

    return false;
}

// 递归搜索目录
void searchDirectory(const std::string& path, const std::string& keyword) {
    // 转换路径为宽字符
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring pathWide = converter.from_bytes(path);
    std::wstring searchPath = pathWide + L"\\*";

    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }

    do {
        if (findData.cFileName[0] == L'.') {
            continue; // 跳过 "." 和 ".."
        }

        std::wstring fullPath = pathWide + L"\\" + findData.cFileName;

        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // 递归搜索子目录
            searchDirectory(converter.to_bytes(fullPath), keyword);
        }
        else {
            std::string filename = converter.to_bytes(findData.cFileName);

            // 检查文件扩展名
            if (!isValidExtension(filename)) {
                continue;
            }

            // 检查文件名是否包含关键字
            std::string lowerName = toLower(filename);

            if (lowerName.find(keyword) != std::string::npos) {
                // 构造打开文件资源管理器的命令
                std::string openCmd = "explorer.exe \"" + converter.to_bytes(fullPath) + "\"";
                
                // 输出JSON格式结果，使用logo URL代替preview_path
                std::wcout << L"{"
                    << L"\"title\":\"" << converter.from_bytes(escapeJsonString(filename)) << L"\","
                    << L"\"content\":\"" << converter.from_bytes(escapeJsonString(converter.to_bytes(fullPath))) << L"\","
                    << L"\"cmd\":\"" << converter.from_bytes(escapeJsonString(openCmd)) << L"\""
                    << L"}\n"
                    << L"\nnext_result\n";
            }
        }
    } while (FindNextFileW(hFind, &findData));

    FindClose(hFind);
}

int main(int argc, char* argv[]) {
    // 设置控制台输出为UTF-8
    SetConsoleOutputCP(CP_UTF8);
    _setmode(_fileno(stdout), _O_U8TEXT);
    std::wcout.imbue(std::locale("en_US.UTF-8"));
    std::wcin.imbue(std::locale("en_US.UTF-8"));

    // 检查命令行参数
    if (argc != 3 || std::string(argv[1]) != "-k") {
        std::cerr << "Usage: " << argv[0] << " -k <keyword>\n";
        return 1;
    }

    // 转换关键字为UTF-8
    std::string keywordArg = argv[2];
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring keywordWide = converter.from_bytes(keywordArg);
    std::string keyword = toLower(converter.to_bytes(keywordWide));  // 获取并转换为小写关键字

    // 如果关键字为空，则直接退出
    if (keyword.empty()) {
        return 0;
    }

    // 定义要搜索的目录类型
    const std::vector<int> folderTypes = {
        CSIDL_DESKTOP,         // 桌面
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
