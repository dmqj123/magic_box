#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <shlobj.h>
#pragma comment(lib, "shell32.lib")

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

// 检查文件扩展名是否符合收藏夹要求
bool isValidExtension(const std::string& filename) {
    // 获取文件扩展名（小写）
    size_t dotPos = filename.find_last_of('.');
    if (dotPos == std::string::npos) return false;

    std::string ext = toLower(filename.substr(dotPos));

    // 收藏夹文件通常为.url或.htm/.html格式
    const std::vector<std::string> allowedExtensions = {
        ".url", ".htm", ".html"
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
                // 构造打开浏览器的命令
                std::string openCmd = "cmd.exe /c start \"\" \"" + fullPath + "\"";
                
                // 获取文件扩展名（小写）
                size_t dotPos = filename.find_last_of('.');
                std::string ext = dotPos != std::string::npos ? toLower(filename.substr(dotPos)) : "";

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

// 获取Edge浏览器收藏夹目录
std::string getEdgeFavoritesPath() {
    CHAR userProfile[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, userProfile))) {
        std::string favoritesPath = std::string(userProfile) + "\\AppData\\Local\\Microsoft\\Edge\\User Data\\Default\\Favorites";
        return favoritesPath;
    }
    return "";
}

// 获取备选的Edge收藏夹目录
std::string getAlternativeEdgeFavoritesPath() {
    CHAR userProfile[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, userProfile))) {
        // 尝试其他可能的路径
        std::vector<std::string> possiblePaths = {
            std::string(userProfile) + "\\AppData\\Local\\Microsoft\\Edge\\User Data\\Default\\Favorites",
            std::string(userProfile) + "\\AppData\\Roaming\\Microsoft\\Edge\\User Data\\Default\\Favorites",
            std::string(userProfile) + "\\Favorites"  // 浏览器导出的收藏夹可能在这里
        };
        
        for (const auto& path : possiblePaths) {
            DWORD attributes = GetFileAttributesA(path.c_str());
            if (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY)) {
                return path;
            }
        }
    }
    return "";
}

int main(int argc, char* argv[]) {
    // 检查命令行参数
    if (argc < 3 || (std::string(argv[1]) != "-k" && std::string(argv[1]) != "-d")) {
        std::cerr << "Usage: " << argv[0] << " -k <keyword> [-d <directory>]\n";
        std::cerr << "       " << argv[0] << " -d <directory> -k <keyword>\n";
        return 1;
    }

    std::string keyword;
    std::string searchDirectoryPath;

    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "-k" && i + 1 < argc) {
            keyword = toLower(argv[i + 1]);
            i++; // 跳过下一个参数
        } else if (std::string(argv[i]) == "-d" && i + 1 < argc) {
            searchDirectoryPath = argv[i + 1];
            i++; // 跳过下一个参数
        }
    }

    // 如果关键字为空，则直接退出
    if (keyword.empty()) {
        std::cerr << "Keyword cannot be empty\n";
        return 1;
    }

    // 如果指定了搜索目录，则使用指定的目录
    if (!searchDirectoryPath.empty()) {
        DWORD attributes = GetFileAttributesA(searchDirectoryPath.c_str());
        if (attributes == INVALID_FILE_ATTRIBUTES || !(attributes & FILE_ATTRIBUTE_DIRECTORY)) {
            std::cerr << "Specified directory not found: " << searchDirectoryPath << "\n";
            return 1;
        }
    } else {
        // 获取Edge浏览器收藏夹目录
        searchDirectoryPath = getEdgeFavoritesPath();
        
        // 检查目录是否存在
        if (searchDirectoryPath.empty()) {
            std::cerr << "Failed to get Edge favorites path\n";
            return 1;
        }

        DWORD attributes = GetFileAttributesA(searchDirectoryPath.c_str());
        if (attributes == INVALID_FILE_ATTRIBUTES || !(attributes & FILE_ATTRIBUTE_DIRECTORY)) {
            // 尝试备选路径
            searchDirectoryPath = getAlternativeEdgeFavoritesPath();
            if (searchDirectoryPath.empty()) {
                std::cerr << "Edge favorites directory not found\n";
                return 1;
            }
        }
    }

    // 搜索收藏夹目录
    searchDirectory(searchDirectoryPath, keyword);

    return 0;
}
