#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <shlobj.h>
#include <fstream>
#include <sstream>
#pragma comment(lib, "shell32.lib")

// 将字符串转换为小写
std::string toLower(const std::string& str) {
    std::string lowerStr;
    for (char c : str) {
        lowerStr += tolower(c);
    }
    return lowerStr;
}

// 将宽字符串转换为小写
std::wstring toLower(const std::wstring& str) {
    std::wstring lowerStr;
    for (wchar_t c : str) {
        lowerStr += towlower(c);
    }
    return lowerStr;
}

// 将宽字符串转换为UTF-8字符串
std::string wstringToUTF8(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

// 将UTF-8字符串转换为宽字符串
std::wstring utf8ToWstring(const std::string& str) {
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
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

// 从URL中提取域名
std::string extractDomain(const std::string& url) {
    std::string domain;
    
    // 查找协议部分（http://或https://）
    size_t protocolPos = url.find("://");
    size_t startPos = 0;
    
    if (protocolPos != std::string::npos) {
        startPos = protocolPos + 3; // 跳过 "://"
    }
    
    // 查找域名结束位置（第一个斜杠或问号）
    size_t endPos = url.find_first_of("/?", startPos);
    
    if (endPos != std::string::npos) {
        domain = url.substr(startPos, endPos - startPos);
    } else {
        domain = url.substr(startPos);
    }
    
    return domain;
}

// 构造可能的logo URL
std::string getLogoUrl(const std::string& url) {
    std::string domain = extractDomain(url);
    
    // 如果域名为空，返回空字符串
    if (domain.empty()) {
        return "";
    }
    
    // 构造favicon URL
    std::string logoUrl = "http://" + domain + "/favicon.ico";
    
    return logoUrl;
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
    WIN32_FIND_DATAW findData;
    HANDLE hFind;
    std::wstring wPath = utf8ToWstring(path);
    std::wstring searchPath = wPath + L"\\*";

    hFind = FindFirstFileW(searchPath.c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }

    do {
        if (findData.cFileName[0] == L'.') {
            continue; // 跳过 "." 和 ".."
        }

        std::wstring wFileName(findData.cFileName);
        std::wstring wFullPath = wPath + L"\\" + wFileName;
        std::string fullPath = wstringToUTF8(wFullPath);
        std::string filename = wstringToUTF8(wFileName);

        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // 递归搜索子目录
            searchDirectory(fullPath, keyword);
        }
        else {
            // 检查文件扩展名
            if (!isValidExtension(filename)) {
                continue;
            }

            // 检查文件名是否包含关键字
            std::string lowerName = toLower(filename);

            if (lowerName.find(keyword) != std::string::npos) {
                // 构造打开浏览器的命令
                std::string openCmd = "explorer.exe \"" + fullPath + "\"";
                
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
    } while (FindNextFileW(hFind, &findData));

    FindClose(hFind);
}

// 获取Edge浏览器收藏夹文件路径
std::string getEdgeBookmarksPath() {
    WCHAR userProfile[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, userProfile))) {
        std::wstring wBookmarksPath(userProfile);
        wBookmarksPath += L"\\AppData\\Local\\Microsoft\\Edge\\User Data\\Default\\Bookmarks";
        return wstringToUTF8(wBookmarksPath);
    }
    return "";
}

// 解析Edge收藏夹JSON文件
void parseEdgeBookmarks(const std::string& bookmarksPath, const std::string& keyword) {
    std::ifstream file(bookmarksPath);
    if (!file.is_open()) {
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();
    
    size_t pos = 0;
    while ((pos = content.find("\"name\"", pos)) != std::string::npos) {
        // 找到name字段的值
        size_t nameStart = content.find("\"", pos + 6);
        if (nameStart == std::string::npos) break;
        nameStart++; // 跳过开头的引号
        
        size_t nameEnd = content.find("\"", nameStart);
        if (nameEnd == std::string::npos) break;
        
        std::string name = content.substr(nameStart, nameEnd - nameStart);
        std::string lowerName = toLower(name);
        
        // 检查名称是否包含关键字
        if (lowerName.find(keyword) != std::string::npos) {
            // 找到对应的URL
            size_t urlPos = content.find("\"url\"", nameEnd);
            if (urlPos != std::string::npos) {
                size_t colonPos = content.find(":", urlPos + 5); // 找到冒号
                if (colonPos != std::string::npos) {
                    size_t urlStart = content.find("\"", colonPos); // 找到URL值的开始引号
                    if (urlStart != std::string::npos) {
                        urlStart++; // 跳过开头的引号
                        size_t urlEnd = content.find("\"", urlStart); // 找到URL值的结束引号
                        if (urlEnd != std::string::npos) {
                            std::string url = content.substr(urlStart, urlEnd - urlStart);
                            
                            // 获取网站logo URL
                            std::string logoUrl = getLogoUrl(url);
                            
                            // 输出JSON格式结果
                            std::cout << "{"
                                << "\"title\":\"" << escapeJsonString(name) << "\","
                                << "\"content\":\"" << escapeJsonString(url) << "\","
                                << "\"cmd\":\"explorer.exe \\\"" + escapeJsonString(url) + "\\\"\"";
                            
                            // 如果logo URL不为空，则添加preview_path字段，值为logo URL
                            if (!logoUrl.empty()) {
                                std::cout << ",\"preview_path\":\"" << escapeJsonString(logoUrl) << "\"";
                            }
                            
                            std::cout << "}\n"
                                << "\nnext_result\n";
                        }
                    }
                }
            }
        }
        
        pos = nameEnd;
    }
}

int main(int argc, char* argv[]) {
    // 设置控制台代码页为UTF-8以支持中文字符
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    
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

    // 获取Edge浏览器收藏夹文件路径
    std::string bookmarksPath = getEdgeBookmarksPath();
    
// 检查文件是否存在
    std::wstring wBookmarksPath = utf8ToWstring(bookmarksPath);
    DWORD attributes = GetFileAttributesW(wBookmarksPath.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES || (attributes & FILE_ATTRIBUTE_DIRECTORY)) {
        std::cerr << "Edge bookmarks file not found\n";
        return 1;
    }

    // 解析Edge收藏夹文件
    parseEdgeBookmarks(bookmarksPath, keyword);

    return 0;
}
