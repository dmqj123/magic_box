#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <knownfolders.h>
#include <objbase.h>
#include <ShlObj_core.h>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")

using namespace std;

// 函数声明
wstring GetCurrentUserFolder();
bool ParseCommandLine(int argc, wchar_t* argv[], wstring& keyword);
wstring ReadFileContent(const wstring& filePath);
void ParseJsonAndSearch(const wstring& jsonContent, const wstring& keyword);
void ProcessNode(const wstring& nodeContent, const wstring& keyword, bool& firstResult);
wstring ExtractValue(const wstring& content, const wstring& key);
vector<wstring> ExtractChildren(const wstring& content);
string WideToUtf8(const wstring& wstr);  // 修正返回类型为string
wstring Utf8ToWide(const string& str);

int wmain(int argc, wchar_t* argv[]) {
    // 设置控制台输出为UTF-8
    SetConsoleOutputCP(CP_UTF8);

    // 解析命令行参数
    wstring keyword;
    if (!ParseCommandLine(argc, argv, keyword)) {
        return 0;
    }

    // 获取当前用户的收藏夹路径
    wstring userFolder = GetCurrentUserFolder();
    if (userFolder.empty()) {
        wcerr << L"无法获取当前用户目录" << endl;
        return 1;
    }

    wstring bookmarksPath = userFolder + L"AppData\\Local\\Microsoft\\Edge\\User Data\\Default\\Bookmarks";

    // 读取收藏夹内容
    wstring jsonContent = ReadFileContent(bookmarksPath);
    if (jsonContent.empty()) {
        wcerr << L"无法读取收藏夹文件: " << bookmarksPath << endl;
        return 1;
    }

    // 解析JSON并搜索匹配项
    ParseJsonAndSearch(jsonContent, keyword);

    return 0;
}

// 获取当前用户目录（使用新的API）
wstring GetCurrentUserFolder() {
    PWSTR path = nullptr;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_Profile, 0, NULL, &path);
    if (SUCCEEDED(hr) && path != nullptr) {
        wstring result(path);
        CoTaskMemFree(path);  // 释放内存
        return result + L"\\";
    }
    return L"";
}

// 解析命令行参数
bool ParseCommandLine(int argc, wchar_t* argv[], wstring& keyword) {
    if (argc < 3 || wcscmp(argv[1], L"-k") != 0) {
        return false;
    }

    keyword = argv[2];
    return !keyword.empty();
}

// 读取文件内容（UTF-8）
wstring ReadFileContent(const wstring& filePath) {
    // 改用ifstream（窄字符流）读取二进制内容，避免宽字符流的类型问题
    ifstream file(filePath, ios::binary);
    if (!file.is_open()) {
        return L"";
    }

    // 获取文件大小，显式转换避免警告
    file.seekg(0, ios::end);
    size_t fileSize = static_cast<size_t>(file.tellg());
    file.seekg(0, ios::beg);

    // 读取原始字节到string（UTF-8）
    string utf8Content(fileSize, '\0');
    file.read(&utf8Content[0], fileSize);
    file.close();

    // 转换为宽字符串
    return Utf8ToWide(utf8Content);
}

// 解析JSON并搜索匹配项
void ParseJsonAndSearch(const wstring& jsonContent, const wstring& keyword) {
    // 找到roots节点
    size_t rootsPos = jsonContent.find(L"\"roots\"");
    if (rootsPos == wstring::npos) {
        return;
    }

    size_t start = jsonContent.find(L'{', rootsPos);
    if (start == wstring::npos) {
        return;
    }

    size_t end = jsonContent.find(L'}', start);
    if (end == wstring::npos) {
        return;
    }

    wstring rootsContent = jsonContent.substr(start, end - start + 1);
    bool firstResult = true;

    // 处理各个根节点
    vector<wstring> rootNodes = { L"bookmark_bar", L"other", L"synced", L"workspaces" };
    for (const auto& node : rootNodes) {
        size_t nodePos = rootsContent.find(L"\"" + node + L"\"");
        if (nodePos == wstring::npos) {
            continue;
        }

        size_t nodeStart = rootsContent.find(L'{', nodePos);
        if (nodeStart == wstring::npos) {
            continue;
        }

        // 找到对应的闭合括号
        int braceCount = 1;
        size_t nodeEnd = nodeStart + 1;
        while (nodeEnd < rootsContent.size() && braceCount > 0) {
            if (rootsContent[nodeEnd] == L'{') {
                braceCount++;
            }
            else if (rootsContent[nodeEnd] == L'}') {
                braceCount--;
            }
            nodeEnd++;
        }

        wstring nodeContent = rootsContent.substr(nodeStart, nodeEnd - nodeStart);
        ProcessNode(nodeContent, keyword, firstResult);
    }
}

// 处理节点内容
void ProcessNode(const wstring& nodeContent, const wstring& keyword, bool& firstResult) {
    // 提取类型
    wstring type = ExtractValue(nodeContent, L"type");

    // 如果是URL类型，检查是否匹配
    if (type == L"url") {
        wstring name = ExtractValue(nodeContent, L"name");
        wstring url = ExtractValue(nodeContent, L"url");

        // 检查名称是否包含关键词（不区分大小写）
        if (name.find(keyword) != wstring::npos) {
            // 准备输出
            if (!firstResult) {
                cout << "\n\nnext_result" << endl;
            }
            firstResult = false;

            // 转换为UTF-8
            string title = WideToUtf8(name);
            string content = WideToUtf8(url);
            string cmd = "explorer.exe \"" + content + "\"";

            // 输出JSON
            cout << "{";
            cout << "\"title\":\"" << title << "\",";
            cout << "\"content\":\"" << content << "\",";
            cout << "\"cmd\":\"" << cmd << "\"";
            cout << "}" << endl;

            // 刷新输出缓冲区，确保实时输出
            cout.flush();
        }
    }
    // 如果是文件夹类型，处理子节点
    else if (type == L"folder" || type == L"workspace") {
        vector<wstring> children = ExtractChildren(nodeContent);
        for (const auto& child : children) {
            ProcessNode(child, keyword, firstResult);
        }
    }
}

// 提取JSON值
wstring ExtractValue(const wstring& content, const wstring& key) {
    size_t keyPos = content.find(L"\"" + key + L"\":");
    if (keyPos == wstring::npos) {
        return L"";
    }

    size_t valueStart = content.find(L"\"", keyPos + key.length() + 3);
    if (valueStart == wstring::npos) {
        return L"";
    }

    size_t valueEnd = content.find(L"\"", valueStart + 1);
    if (valueEnd == wstring::npos) {
        return L"";
    }

    return content.substr(valueStart + 1, valueEnd - valueStart - 1);
}

// 提取子节点
vector<wstring> ExtractChildren(const wstring& content) {
    vector<wstring> children;

    size_t childrenPos = content.find(L"\"children\"");
    if (childrenPos == wstring::npos) {
        return children;
    }

    size_t arrayStart = content.find(L'[', childrenPos);
    if (arrayStart == wstring::npos) {
        return children;
    }

    size_t arrayEnd = content.find(L']', arrayStart);
    if (arrayEnd == wstring::npos) {
        return children;
    }

    wstring arrayContent = content.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
    size_t pos = 0;
    int braceCount = 0;
    size_t nodeStart = wstring::npos;

    while (pos < arrayContent.size()) {
        if (arrayContent[pos] == L'{') {
            if (nodeStart == wstring::npos) {
                nodeStart = pos;
            }
            braceCount++;
        }
        else if (arrayContent[pos] == L'}') {
            braceCount--;
            if (braceCount == 0 && nodeStart != wstring::npos) {
                children.push_back(arrayContent.substr(nodeStart, pos - nodeStart + 1));
                nodeStart = wstring::npos;
            }
        }
        pos++;
    }

    return children;
}

// 宽字符串转UTF-8（修正为返回string）
string WideToUtf8(const wstring& wstr) {
    if (wstr.empty()) return "";
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

// UTF-8转宽字符串
wstring Utf8ToWide(const string& str) {
    if (str.empty()) return L"";
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}