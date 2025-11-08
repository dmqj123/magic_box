#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <shlobj.h>
#include <locale>
#include <codecvt>

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

// 设置控制台代码页为UTF-8
void setConsoleToUTF8() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
}

// 将UTF-8字符串转换为wstring
std::wstring utf8ToWstring(const std::string& str) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

// 将wstring转换为UTF-8字符串
std::string wstringToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

// 将wstring转换为小写
std::wstring toLowerW(const std::wstring& wstr) {
    std::wstring lowerStr;
    for (wchar_t c : wstr) {
        lowerStr += std::towlower(c);
    }
    return lowerStr;
}

// 转义JSON字符串中的特殊字符 (UTF-8 version)
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
bool isValidExtension(const std::wstring& filename) {
    // 获取文件扩展名（小写）
    size_t dotPos = filename.find_last_of(L'.');
    if (dotPos == std::wstring::npos) return false;

    std::wstring ext = toLowerW(filename.substr(dotPos));

    // 允许的扩展名列表
    const std::vector<std::wstring> allowedExtensions = {
        L".txt", L".doc", L".docx", L".ppt", L".pptx", L".mp3", L".mp4", L".md", L".cpp", L".zip", L".7z", L".png", L".gif", L".jpg", L".jpeg"
    };

    // 检查是否在允许列表中
    for (const auto& allowed : allowedExtensions) {
        if (ext == allowed) return true;
    }

    return false;
}

// 递归搜索目录
void searchDirectory(const std::wstring& path, const std::wstring& keyword) {
    WIN32_FIND_DATAW findData;
    HANDLE hFind;
    std::wstring searchPath = path + L"\\*";

    hFind = FindFirstFileW(searchPath.c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }

    do {
        if (findData.cFileName[0] == L'.') {
            continue; // 跳过 "." 和 ".."
        }

        std::wstring fullPath = path + L"\\" + findData.cFileName;

        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // 递归搜索子目录
            searchDirectory(fullPath, keyword);
        }
        else {
            std::wstring filename = findData.cFileName;

            // 检查文件扩展名
            if (!isValidExtension(filename)) {
                continue;
            }

            // 检查文件名是否包含关键字
            std::wstring lowerName = toLowerW(filename);

            if (lowerName.find(keyword) != std::wstring::npos) {
                // Convert to UTF-8 for output
                std::string utf8Filename = wstringToUtf8(filename);
                std::string utf8FullPath = wstringToUtf8(fullPath);
                std::string utf8Keyword = wstringToUtf8(keyword);
                
                // 构造打开文件资源管理器的命令
                std::string openCmd = "explorer.exe \"" + utf8FullPath + "\"";
                // 获取文件扩展名（小写）
                size_t dotPos = filename.find_last_of(L'.');
                std::wstring extW = dotPos != std::wstring::npos ? toLowerW(filename.substr(dotPos)) : L"";
                std::string ext = wstringToUtf8(extW);

                // 输出JSON格式结果
                std::cout << "{"
                    << "\"title\":\"" << escapeJsonString(utf8Filename) << "\","
                    << "\"content\":\"" << escapeJsonString(utf8FullPath) << "\","
                    << "\"cmd\":\"" << escapeJsonString(openCmd) << "\""
                    << (ext == ".png" || ext == ".jpg" || ext == ".jpeg" ? ",\"preview_path\":\"" + escapeJsonString(utf8FullPath) + "\"" : "")
                    << "}\n"
                    << "\nnext_result\n";
            }
        }
    } while (FindNextFileW(hFind, &findData));

    FindClose(hFind);
}

int main(int argc, char* argv[]) {
    // 设置控制台为UTF-8模式
    setConsoleToUTF8();
    
    // 检查命令行参数
    if (argc != 3 || std::string(argv[1]) != "-k") {
        std::cerr << "Usage: " << argv[0] << " -k <keyword>\n";
        return 1;
    }

    // Convert keyword from command line to wide string for Unicode support
    std::wstring keywordW;
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, argv[2], -1, NULL, 0);
    if (size_needed > 1) {
        keywordW.resize(size_needed - 1); // Remove null terminator from count
        MultiByteToWideChar(CP_UTF8, 0, argv[2], -1, &keywordW[0], size_needed);
    } else {
        // If conversion fails, try default ANSI code page
        int size_needed_ansi = MultiByteToWideChar(CP_ACP, 0, argv[2], -1, NULL, 0);
        if (size_needed_ansi > 1) {
            keywordW.resize(size_needed_ansi - 1);
            MultiByteToWideChar(CP_ACP, 0, argv[2], -1, &keywordW[0], size_needed_ansi);
        } else {
            // Fallback to original string as wide string
            for (char c : std::string(argv[2])) {
                keywordW += static_cast<wchar_t>(static_cast<unsigned char>(c));
            }
        }
    }

    // 转换为小写
    keywordW = toLowerW(keywordW);

    // 如果关键字为空，则直接退出
    if (keywordW.empty()) {
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
        WCHAR folderPath[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(NULL, folderType, NULL, 0, folderPath))) {
            std::wstring wstrFolderPath(folderPath);
            searchDirectory(wstrFolderPath, keywordW);
        }
        else {
            // 特殊处理下载文件夹（某些旧系统可能不支持CSIDL_DOWNLOADS）
            if (folderType == CSIDL_DOWNLOADS) {
                // 尝试替代方法获取下载文件夹
                if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, folderPath))) {
                    std::wstring downloadsPath = std::wstring(folderPath) + L"\\Downloads";
                    searchDirectory(downloadsPath, keywordW);
                }
            }
        }
    }

    return 0;
}