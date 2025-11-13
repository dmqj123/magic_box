#include <Windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <objbase.h>
#include <shellapi.h>
#include <comdef.h>
#include <gdiplus.h>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <codecvt>
#include <locale>
#include <iostream>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")

using namespace Gdiplus;
using namespace std;

// 定义ICO文件结构
#pragma pack(push, 2)
typedef struct {
    WORD idReserved;
    WORD idType;
    WORD idCount;
} ICONDIR;

typedef struct {
    BYTE bWidth;
    BYTE bHeight;
    BYTE bColorCount;
    BYTE bReserved;
    WORD wPlanes;
    WORD wBitCount;
    DWORD dwBytesInRes;
    DWORD dwImageOffset;
} ICONDIRENTRY;
#pragma pack(pop)

wstring_convert<codecvt_utf8<wchar_t>> utf8_converter;

// 转义JSON字符串中的特殊字符
string escapeJsonString(const std::string& str) {
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



// 清理临时图标文件夹
void CleanTempIcons() {
    wchar_t tempPath[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);
    wstring iconDir = wstring(tempPath) + L"app_icons_cache\\";

    // 删除整个图标缓存目录
    // 确保路径以双空字符结尾，这是SHFileOperationW的要求
    // SHFileOperationW需要一个以双空字符结尾的字符串列表
    // 因此，我们需要创建一个新的缓冲区来存储路径并添加双空字符
    size_t pathLen = iconDir.length();
    std::vector<wchar_t> fromPath(pathLen + 2);
    wcsncpy_s(fromPath.data(), fromPath.size(), iconDir.c_str(), pathLen);
    fromPath[pathLen] = L'\0';
    fromPath[pathLen + 1] = L'\0';

    SHFILEOPSTRUCTW fileOp = { 0 };
    fileOp.wFunc = FO_DELETE;
    fileOp.pFrom = fromPath.data();
    fileOp.fFlags = FOF_NO_UI | FOF_NOCONFIRMATION | FOF_SILENT;
    SHFileOperationW(&fileOp);

    // 等待文件操作完成，确保目录被完全删除
    Sleep(100); // 100毫秒

    // 重新创建目录
    CreateDirectoryW(iconDir.c_str(), nullptr);
}

// 获取图标缓存目录
wstring GetIconCacheDir() {
    wchar_t tempPath[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);
    wstring iconDir = wstring(tempPath) + L"app_icons_cache\\";
    // 确保目录存在
    CreateDirectoryW(iconDir.c_str(), nullptr);
    return iconDir;
}

// 保存图标到临时文件(PNG格式)
wstring SaveIconToTemp(const wstring& filePath, int iconIndex) {
    // 创建唯一的文件名基于文件路径和索引
    wstring uniqueId = filePath;
    for (auto& ch : uniqueId) {
        if (ch == L'\\' || ch == L'/' || ch == L':' || ch == L'*' || ch == L'?' || ch == L'\"' || ch == L'<' || ch == L'>' || ch == L'|') {
            ch = L'_';
        }
    }
    
    wchar_t indexStr[16];
    swprintf(indexStr, 16, L"_%d", iconIndex);
    uniqueId += indexStr;
    
    wstring iconDir = GetIconCacheDir();
    wstring pngPath = iconDir + uniqueId + L".png";
    
    // 检查文件是否已存在
    if (PathFileExistsW(pngPath.c_str())) {
        return pngPath;
    }

    // 提取最大的图标
    UINT iconCount = ExtractIconExW(filePath.c_str(), -1, nullptr, nullptr, 0);
    if (iconCount == 0) return L"";

    // 尝试提取不同尺寸的图标，选择最大的
    HICON hIcon = nullptr;
    int maxIconSize = 0;
    
    vector<HICON> icons(iconCount);
    ExtractIconExW(filePath.c_str(), 0, nullptr, icons.data(), iconCount);
    
    for (UINT i = 0; i < iconCount; i++) {
        if (icons[i]) {
            ICONINFO iconInfo;
            if (GetIconInfo(icons[i], &iconInfo)) {
                BITMAP bmp;
                GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bmp);
                
                int size = max(bmp.bmWidth, bmp.bmHeight);
                if (size > maxIconSize) {
                    maxIconSize = size;
                    if (hIcon) DestroyIcon(hIcon);
                    hIcon = CopyIcon(icons[i]);
                }
                
                DeleteObject(iconInfo.hbmColor);
                DeleteObject(iconInfo.hbmMask);
            }
        }
    }
    
    // 清理所有图标
    for (UINT i = 0; i < iconCount; i++) {
        if (icons[i]) DestroyIcon(icons[i]);
    }
    
    if (!hIcon) return L"";

    // 将图标转换为PNG格式并保存
    HDC hdcScreen = GetDC(NULL);
    HDC hdc = CreateCompatibleDC(hdcScreen);
    ReleaseDC(NULL, hdcScreen);
    
    ICONINFO iconInfo;
    if (!GetIconInfo(hIcon, &iconInfo)) {
        DestroyIcon(hIcon);
        DeleteDC(hdc);
        return L"";
    }

    BITMAP bmp;
    GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bmp);

    // 创建带alpha通道的位图
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = bmp.bmWidth;
    bmi.bmiHeader.biHeight = -bmp.bmHeight; // Top-down DIB
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    BYTE* pBits = nullptr;
    HBITMAP hBmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, (void**)&pBits, NULL, 0);
    
    if (!hBmp) {
        DeleteObject(iconInfo.hbmColor);
        DeleteObject(iconInfo.hbmMask);
        DeleteDC(hdc);
        DestroyIcon(hIcon);
        return L"";
    }
    
    HGDIOBJ oldObj = SelectObject(hdc, hBmp);
    
    // 使用正确的参数绘制图标，包括透明度支持
    DrawIconEx(hdc, 0, 0, hIcon, bmp.bmWidth, bmp.bmHeight, 0, NULL, DI_NORMAL);
    
    // 创建GDI+ bitmap，显式指定像素格式
    Bitmap* pBitmap = new Bitmap(bmp.bmWidth, bmp.bmHeight, bmi.bmiHeader.biWidth * 4, PixelFormat32bppARGB, pBits);
    if (pBitmap) {
        // 保存为PNG
        CLSID clsid;
        HRESULT hr = CLSIDFromString(L"{557CF406-1A04-11D3-9A73-0000F81EF32E}", &clsid); // PNG encoder
        
        if (SUCCEEDED(hr)) {
            // 确保目录存在
            CreateDirectoryW(iconDir.c_str(), nullptr);
            
            // 保存图像
            pBitmap->Save(pngPath.c_str(), &clsid, nullptr);
        }
        
        delete pBitmap;
    }
    
    // 清理资源
    SelectObject(hdc, oldObj);
    DeleteObject(hBmp);
    DeleteObject(iconInfo.hbmColor);
    DeleteObject(iconInfo.hbmMask);
    DeleteDC(hdc);
    DestroyIcon(hIcon);
    
    return pngPath;
}

// 解析快捷方式
wstring ParseShortcut(const wstring& lnkPath, wstring& iconPath) {
    IShellLinkW* psl = nullptr;
    wstring targetPath;

    if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (void**)&psl))) {
        IPersistFile* ppf = nullptr;
        if (SUCCEEDED(psl->QueryInterface(IID_IPersistFile, (void**)&ppf))) {
            if (SUCCEEDED(ppf->Load(lnkPath.c_str(), STGM_READ))) {
                wchar_t path[MAX_PATH];
                int iconIndex = 0;
                if (SUCCEEDED(psl->GetPath(path, MAX_PATH, nullptr, 0))) {
                    targetPath = path;
                }
                if (SUCCEEDED(psl->GetIconLocation(path, MAX_PATH, &iconIndex))) {
                    if (wcslen(path) > 0) {
                        iconPath = SaveIconToTemp(path, iconIndex);
                    }
                }
                else {
                    // 如果没有指定图标，则从目标文件提取
                    if (!targetPath.empty()) {
                        iconPath = SaveIconToTemp(targetPath, 0);
                    }
                }
            }
            ppf->Release();
        }
        psl->Release();
    }
    return targetPath;
}

// 扫描目录中的快捷方式
void ScanShortcuts(const wstring& searchPath, const wstring& keyword, set<wstring>& exportedApps, bool& firstResult) {
    WIN32_FIND_DATAW findData;
    wstring searchPattern = searchPath + L"\\*.lnk";
    HANDLE hFind = FindFirstFileW(searchPattern.c_str(), &findData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                wstring lnkPath = searchPath + L"\\" + findData.cFileName;
                wstring iconPath;
                wstring targetPath = ParseShortcut(lnkPath, iconPath);

                if (!targetPath.empty() && PathFileExistsW(targetPath.c_str())) {
                    // 检查目标文件是否是exe
                    if (targetPath.length() < 4 || _wcsicmp(targetPath.c_str() + targetPath.length() - 4, L".exe") != 0) {
                        continue;
                    }
                    wstring appName = findData.cFileName;
                    appName = appName.substr(0, appName.find_last_of(L'.'));

                    // 检查名称匹配
                    if (keyword.empty() ||
                        StrStrIW(appName.c_str(), keyword.c_str()) ||
                        StrStrIW(targetPath.c_str(), keyword.c_str())) {

                        // 去重检查
                        wstring key = targetPath + L"|" + appName;
                        if (exportedApps.find(key) == exportedApps.end()) {
                            exportedApps.insert(key);

                            // 如果图标没有从快捷方式中获取，则尝试从目标文件获取
                            if (iconPath.empty()) {
                                iconPath = SaveIconToTemp(targetPath, 0);
                            }

                            // 生成JSON输出
                            string escapedAppName = escapeJsonString(utf8_converter.to_bytes(appName));
                            string escapedTargetPath = escapeJsonString(utf8_converter.to_bytes(targetPath));
                            string escapedIconPath = escapeJsonString(utf8_converter.to_bytes(iconPath));
                            wstring json = L"{\"title\":\"" + utf8_converter.from_bytes(escapedAppName) +
                                L"\",\"content\":\"" + utf8_converter.from_bytes(escapedTargetPath) +
                                L"\",\"cmd\":\"explorer.exe \\\"" + utf8_converter.from_bytes(escapedTargetPath) + L"\\\"\"" +
                                L",\"preview_path\":\"" + utf8_converter.from_bytes(escapedIconPath) + L"\"" +
                                L",\"auto_close\":true}";

                            cout << utf8_converter.to_bytes(json);
                            cout << "\n\nnext_result\n";
                            firstResult = false;
                        }
                    }
                }
            }
        } while (FindNextFileW(hFind, &findData));
        FindClose(hFind);
    }
}

// 扫描注册表应用
void ScanRegistry(HKEY hKey, const wstring& keyword, set<wstring>& exportedApps, bool& firstResult) {
    DWORD index = 0;
    wchar_t subKeyName[255];
    DWORD subKeyNameSize = 255;

    while (RegEnumKeyExW(hKey, index++, subKeyName, &subKeyNameSize, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS) {
        HKEY hSubKey;
        if (RegOpenKeyExW(hKey, subKeyName, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
            wchar_t displayName[255] = { 0 };
            wchar_t installLocation[MAX_PATH] = { 0 };
            wchar_t displayIcon[MAX_PATH] = { 0 };
            wchar_t uninstallString[MAX_PATH] = { 0 };
            DWORD size = sizeof(displayName);

            if (RegQueryValueExW(hSubKey, L"DisplayName", nullptr, nullptr, (LPBYTE)displayName, &size) == ERROR_SUCCESS && wcslen(displayName) > 0) {
                size = sizeof(installLocation);
                RegQueryValueExW(hSubKey, L"InstallLocation", nullptr, nullptr, (LPBYTE)installLocation, &size);

                size = sizeof(displayIcon);
                RegQueryValueExW(hSubKey, L"DisplayIcon", nullptr, nullptr, (LPBYTE)displayIcon, &size);

                size = sizeof(uninstallString);
                RegQueryValueExW(hSubKey, L"UninstallString", nullptr, nullptr, (LPBYTE)uninstallString, &size);

                // 检查名称匹配
                if (keyword.empty() || StrStrIW(displayName, keyword.c_str())) {
                    wstring exePath;
                    wstring iconPath;

                    // 获取可执行文件路径
                    if (wcslen(displayIcon) > 0) {
                        wstring iconStr = displayIcon;
                        size_t pos = iconStr.find(L',');
                        exePath = (pos != wstring::npos) ? iconStr.substr(0, pos) : iconStr;
                        // 去除可能的引号
                        if (exePath.length() >= 2 && exePath[0] == L'\"' && exePath[exePath.length() - 1] == L'\"') {
                            exePath = exePath.substr(1, exePath.length() - 2);
                        }
                    }

                    // 尝试从安装位置获取
                    if (exePath.empty() && wcslen(installLocation) > 0) {
                        // 尝试在安装目录下找与显示名称相同的exe
                        wstring exeName = wstring(displayName) + L".exe";
                        wstring candidate = wstring(installLocation) + L"\\" + exeName;
                        if (PathFileExistsW(candidate.c_str())) {
                            exePath = candidate;
                        }
                        // 尝试查找目录中的exe文件
                        else {
                            WIN32_FIND_DATAW findData;
                            wstring searchPattern = wstring(installLocation) + L"\\*.exe";
                            HANDLE hFind = FindFirstFileW(searchPattern.c_str(), &findData);
                            if (hFind != INVALID_HANDLE_VALUE) {
                                do {
                                    if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                                        candidate = wstring(installLocation) + L"\\" + findData.cFileName;
                                        exePath = candidate;
                                        break;
                                    }
                                } while (FindNextFileW(hFind, &findData));
                                FindClose(hFind);
                            }
                        }
                    }

                    // 尝试从卸载字符串获取
                    if (exePath.empty() && wcslen(uninstallString) > 0) {
                        wstring uninstStr = uninstallString;
                        if (uninstStr.find(L".exe") != wstring::npos) {
                            size_t pos = uninstStr.find(L".exe");
                            exePath = uninstStr.substr(0, pos + 4);
                            // 去除可能的引号
                            if (exePath.length() >= 2 && exePath[0] == L'\"' && exePath[exePath.length() - 1] == L'\"') {
                                exePath = exePath.substr(1, exePath.length() - 2);
                            }
                        }
                    }

                    if (!exePath.empty() && PathFileExistsW(exePath.c_str())) {
                        // 去重检查
                        wstring key = exePath + L"|" + displayName;
                        if (exportedApps.find(key) == exportedApps.end()) {
                            exportedApps.insert(key);

                            // 提取图标
                            iconPath = SaveIconToTemp(exePath, 0);

                            // 生成JSON输出
                            string escapedDisplayName = escapeJsonString(utf8_converter.to_bytes(displayName));
                            string escapedExePath = escapeJsonString(utf8_converter.to_bytes(exePath));
                            string escapedIconPath = escapeJsonString(utf8_converter.to_bytes(iconPath));
                            wstring json = L"{\"title\":\"" + utf8_converter.from_bytes(escapedDisplayName) +
                                L"\",\"content\":\"" + utf8_converter.from_bytes(escapedExePath) +
                                L"\",\"cmd\":\"explorer.exe \\\"" + utf8_converter.from_bytes(escapedExePath) + L"\\\"\"" +
                                L",\"preview_path\":\"" + utf8_converter.from_bytes(escapedIconPath) + L"\"" +
                                L",\"auto_close\":true}";

                            cout << utf8_converter.to_bytes(json);
                            cout << "\n\nnext_result\n";
                            firstResult = false;
                        }
                    }
                }
            }
            RegCloseKey(hSubKey);
        }
        subKeyNameSize = 255;
    }
}

int wmain(int argc, wchar_t* argv[]) {
    // 初始化COM
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    // 初始化GDI+
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    // 设置控制台输出UTF-8
    SetConsoleOutputCP(CP_UTF8);

    // 不再每次清理临时文件，而是保持缓存
    // CleanTempIcons();

    // 解析参数
    wstring keyword;
    if (argc == 3 && wcscmp(argv[1], L"-k") == 0) {
        keyword = argv[2];
    }
    else {
        return 0;
    }

    set<wstring> exportedApps;
    bool firstResult = true;

    // 扫描桌面快捷方式
    wchar_t desktopPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_DESKTOPDIRECTORY, nullptr, 0, desktopPath))) {
        ScanShortcuts(desktopPath, keyword, exportedApps, firstResult);
    }

    // 扫描开始菜单快捷方式
    wchar_t startMenuPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_STARTMENU, nullptr, 0, startMenuPath))) {
        ScanShortcuts(startMenuPath, keyword, exportedApps, firstResult);
    }

    // 扫描公共开始菜单
    wchar_t commonStartMenu[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_COMMON_STARTMENU, nullptr, 0, commonStartMenu))) {
        ScanShortcuts(commonStartMenu, keyword, exportedApps, firstResult);
    }

    // 扫描注册表应用
    HKEY hUninstallKey;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", 0, KEY_READ, &hUninstallKey) == ERROR_SUCCESS) {
        ScanRegistry(hUninstallKey, keyword, exportedApps, firstResult);
        RegCloseKey(hUninstallKey);
    }

    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall", 0, KEY_READ, &hUninstallKey) == ERROR_SUCCESS) {
        ScanRegistry(hUninstallKey, keyword, exportedApps, firstResult);
        RegCloseKey(hUninstallKey);
    }

    // 扫描64位注册表应用（64位系统）
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall", 0, KEY_READ, &hUninstallKey) == ERROR_SUCCESS) {
        ScanRegistry(hUninstallKey, keyword, exportedApps, firstResult);
        RegCloseKey(hUninstallKey);
    }

    // 清理资源
    GdiplusShutdown(gdiplusToken);
    CoUninitialize();
    return 0;
}