// 指定子系统为Windows，防止链接器错误                                                                                                                                                                                                           │
#pragma comment(linker, "/subsystem:windows")
#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <vector>
#include <psapi.h>

// 服务名称
const char* SERVICE_NAME = "MagicBoxSearchService";

// 全局变量
HHOOK g_keyboardHook = NULL;
HINSTANCE g_hInstance = NULL;

// 获取当前可执行文件路径
std::string GetCurrentExePath() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string path(buffer);
    return path;
}

// 获取当前可执行文件所在目录
std::string GetExeDirectory() {
    std::string fullPath = GetCurrentExePath();
    size_t lastSlash = fullPath.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        return fullPath.substr(0, lastSlash);
    }
    return "";
}

// 检查是否已注册到启动项
bool IsRegisteredInStartup() {
    HKEY hKey;
    bool found = false;

    if (RegOpenKeyExA(HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        char valueData[MAX_PATH];
        DWORD valueSize = sizeof(valueData);

        if (RegQueryValueExA(hKey, SERVICE_NAME, NULL, NULL,
            (LPBYTE)valueData, &valueSize) == ERROR_SUCCESS) {
            found = true;
        }
        RegCloseKey(hKey);
    }

    return found;
}

// 添加到注册表启动项
bool RegisterInStartup() {
    HKEY hKey;
    std::string exePath = GetCurrentExePath();

    if (RegOpenKeyExA(HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {

        LONG result = RegSetValueExA(hKey,
            SERVICE_NAME,
            0,
            REG_SZ,
            (const BYTE*)exePath.c_str(),
            exePath.length() + 1);

        RegCloseKey(hKey);

        return (result == ERROR_SUCCESS);
    }

    return false;
}

// 启动主程序
void LaunchMainProgram() {
    std::string dir = GetExeDirectory();
    std::string mainExe = dir + "\\magic_box.exe";

    // 使用 ShellExecute 启动主程序
    HINSTANCE result = ShellExecuteA(NULL, "open", mainExe.c_str(), NULL, dir.c_str(), SW_SHOWNORMAL);
    if ((int)result <= 32) {
        // 启动失败，可能是因为文件不存在
        // 尝试使用当前目录
        char currentDir[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, currentDir);
        std::string altExe = std::string(currentDir) + "\\magic_box.exe";
        ShellExecuteA(NULL, "open", altExe.c_str(), NULL, currentDir, SW_SHOWNORMAL);
    }
}

// 全局键盘钩子过程
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        if (wParam == WM_KEYDOWN) {
            KBDLLHOOKSTRUCT* pKeyboard = (KBDLLHOOKSTRUCT*)lParam;

            // 检测 Ctrl+~ 组合键
            // ~ 键通常对应 VK_OEM_3 (主键盘的反引号/波浪符键)，也可能为其他键码    
            if ((pKeyboard->vkCode == VK_OEM_3 || pKeyboard->vkCode == VK_OEM_2 || pKeyboard->vkCode == 0xBD || pKeyboard->vkCode == 0xC0) &&  // 可能的波浪号键 
                (GetAsyncKeyState(VK_CONTROL) & 0x8000)) {  // Ctrl 键被按下

                // 再次确认 Ctrl 键仍被按下
                if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
                    LaunchMainProgram();
                }
                // 阻止按键事件继续传递，防止触发其他应用
                return 1;
            }
        }
    }

    return CallNextHookEx(g_keyboardHook, nCode, wParam, lParam);
}

// 安装全局键盘钩子
bool InstallKeyboardHook() {
    g_keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);
    return (g_keyboardHook != NULL);
}

// 卸载全局键盘钩子
bool UninstallKeyboardHook() {
    if (g_keyboardHook) {
        bool result = UnhookWindowsHookEx(g_keyboardHook);
        g_keyboardHook = NULL;
        return result;
    }
    return true;
}

// 检查是否已经运行
bool IsAlreadyRunning() {
    HANDLE hMutex = CreateMutexA(NULL, TRUE, SERVICE_NAME);
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(hMutex);
        return true;
    }
    return false;
}

// 主窗口过程（实际上不显示窗口）
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        // 初始化钩子
        if (!InstallKeyboardHook()) {
            MessageBoxA(NULL, "Failed to install keyboard hook!", "Error", MB_OK | MB_ICONERROR);
            PostQuitMessage(1);
        }
        break;

    case WM_DESTROY:
        // 清理钩子
        UninstallKeyboardHook();
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// 程序入口
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 检查是否已经运行
    if (IsAlreadyRunning()) {
        return 0;
    }

    // 检查是否已注册到启动项，如果没有则注册
    if (!IsRegisteredInStartup()) {
        if (RegisterInStartup()) {
            // 注册成功，继续执行
        }
        else {
            // 注册失败，也可以继续运行，只是下次重启不会自动启动
        }
    }

    // 保存实例句柄
    g_hInstance = hInstance;

    // 注册窗口类
    WNDCLASSA wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = SERVICE_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClassA(&wc)) {
        MessageBoxA(NULL, "Failed to register window class!", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // 创建隐藏窗口
    HWND hwnd = CreateWindowExA(
        0,
        wc.lpszClassName,
        SERVICE_NAME,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd) {
        MessageBoxA(NULL, "Failed to create window!", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // 隐藏窗口
    ShowWindow(hwnd, SW_HIDE);

    // 消息循环
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}