// 指定子系统为Windows，防止链接器错误
#pragma comment(linker, "/subsystem:windows")
#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <vector>
#include <psapi.h>
#include <algorithm>

// 服务名称
const char* SERVICE_NAME = "MagicBoxSearchService";

// 全局变量
HHOOK g_keyboardHook = NULL;
HINSTANCE g_hInstance = NULL;

// 前向声明
struct WindowData;
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);

// 获取当前可执行文件路径
std::string GetCurrentExePath() {
    char buffer[MAX_PATH] = { 0 };
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    return std::string(buffer);
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

// 检查是否已注册到启动项，并验证路径是否正确
bool IsRegisteredInStartup(bool* pathCorrect = nullptr) {
    HKEY hKey = NULL;
    bool found = false;
    bool isPathCorrect = false;

    // 获取当前程序的路径
    std::string currentExePath = GetCurrentExePath();

    if (RegOpenKeyExA(HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        char valueData[MAX_PATH] = { 0 };
        DWORD valueSize = sizeof(valueData);

        if (RegQueryValueExA(hKey, SERVICE_NAME, NULL, NULL,
            reinterpret_cast<LPBYTE>(valueData), &valueSize) == ERROR_SUCCESS) {
            found = true;
            
            // 比较注册表中的路径和当前程序路径是否相同
            isPathCorrect = (_stricmp(valueData, currentExePath.c_str()) == 0);
        }
        RegCloseKey(hKey);
    }

    // 如果提供了路径正确性的输出参数，则设置它
    if (pathCorrect != nullptr) {
        *pathCorrect = isPathCorrect;
    }

    return found;
}

// 添加或更新注册表启动项
bool RegisterInStartup() {
    HKEY hKey = NULL;
    std::string exePath = GetCurrentExePath();

    if (RegOpenKeyExA(HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {

        // RegSetValueExA 会自动覆盖已存在的值，实现启动项的添加或更新
        LONG result = RegSetValueExA(hKey,
            SERVICE_NAME,
            0,
            REG_SZ,
            reinterpret_cast<const BYTE*>(exePath.c_str()),
            static_cast<DWORD>(exePath.length() + 1));

        RegCloseKey(hKey);

        return (result == ERROR_SUCCESS);
    }

    return false;
}

// WindowData 结构体定义
struct WindowData {
    DWORD processId;
    HWND hwnd;
};

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    WindowData* data = reinterpret_cast<WindowData*>(lParam);

    DWORD processId = 0;
    GetWindowThreadProcessId(hwnd, &processId);

    if (data->processId == processId && IsWindowVisible(hwnd)) {
        // 不再检查窗口标题和尺寸，只要可见就返回
        data->hwnd = hwnd;
        return FALSE; // 找到第一个可见窗口就停止枚举
    }
    return TRUE; // 继续枚举
}

// 查找目标进程的主窗口
HWND FindMainWindow(DWORD dwProcessId) {
    WindowData data = { dwProcessId, NULL };
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&data));
    return data.hwnd;
}

// 检查目标进程是否正在运行
bool IsProcessRunning(const char* processName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    PROCESSENTRY32 pe = { 0 };
    pe.dwSize = sizeof(PROCESSENTRY32);

    bool found = false;
    if (Process32First(hSnapshot, &pe)) {
        do {
            // 使用宽窄字符转换进行比较
            char exeName[MAX_PATH];
            WideCharToMultiByte(CP_ACP, 0, pe.szExeFile, -1, exeName, MAX_PATH, NULL, NULL);

            if (_stricmp(exeName, processName) == 0) {
                found = true;
                break;
            }
        } while (Process32Next(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);
    return found;
}

// 获取目标进程ID
DWORD GetProcessIdByName(const char* processName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    PROCESSENTRY32 pe = { 0 };
    pe.dwSize = sizeof(PROCESSENTRY32);

    DWORD pid = 0;
    if (Process32First(hSnapshot, &pe)) {
        do {
            // 使用宽窄字符转换进行比较
            char exeName[MAX_PATH];
            WideCharToMultiByte(CP_ACP, 0, pe.szExeFile, -1, exeName, MAX_PATH, NULL, NULL);

            if (_stricmp(exeName, processName) == 0) {
                pid = pe.th32ProcessID;
                break;
            }
        } while (Process32Next(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);
    return pid;
}

// 改进的窗口激活函数
void ActivateWindow(HWND hwnd) {
    if (!hwnd || !IsWindow(hwnd)) {
        return;
    }

    // 如果窗口最小化，先恢复
    if (IsIconic(hwnd)) {
        ShowWindow(hwnd, SW_RESTORE);
    }

    // 方法1: 标准的激活方式
    SetForegroundWindow(hwnd);

    // 方法2: 附加线程输入（提高SetForegroundWindow成功率）
    DWORD foregroundThreadId = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
    DWORD targetThreadId = GetWindowThreadProcessId(hwnd, NULL);

    if (foregroundThreadId != targetThreadId) {
        AttachThreadInput(targetThreadId, foregroundThreadId, TRUE);
        SetForegroundWindow(hwnd);
        AttachThreadInput(targetThreadId, foregroundThreadId, FALSE);
    }
    else {
        SetForegroundWindow(hwnd);
    }

    // 方法3: 强制刷新窗口状态
    BringWindowToTop(hwnd);
    SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

    SetFocus(hwnd);
}

// 将窗口移动到鼠标所在位置并激活
void MoveWindowToMousePosition(HWND hwnd) {
    if (!hwnd || !IsWindow(hwnd)) {
        return;
    }

    // 获取鼠标位置
    POINT mousePos = { 0 };
    GetCursorPos(&mousePos);

    // 获取窗口尺寸
    RECT windowRect = { 0 };
    GetWindowRect(hwnd, &windowRect);
    int width = windowRect.right - windowRect.left;
    int height = windowRect.bottom - windowRect.top;

    // 获取鼠标所在屏幕的工作区域（排除任务栏）
    HMONITOR hMonitor = MonitorFromPoint(mousePos, MONITOR_DEFAULTTONEAREST);
    MONITORINFO monitorInfo = { 0 };
    monitorInfo.cbSize = sizeof(MONITORINFO);

    if (GetMonitorInfoA(hMonitor, &monitorInfo)) {
        RECT workArea = monitorInfo.rcWork;

        // 计算新位置，使窗口在鼠标位置附近居中显示
        int newX = mousePos.x - width / 2;
        int newY = mousePos.y - height / 2;

        // 确保窗口不会超出屏幕工作区域
        if (newX < workArea.left) newX = workArea.left;
        if (newX > workArea.right - width) newX = workArea.right - width;
        if (newY < workArea.top) newY = workArea.top;
        if (newY > workArea.bottom - height) newY = workArea.bottom - height;

        // 移动窗口
        SetWindowPos(hwnd, HWND_TOP, newX, newY, width, height,
            SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_SHOWWINDOW);
    }
    else {
        // 如果获取屏幕信息失败，直接使用鼠标位置
        SetWindowPos(hwnd, HWND_TOP,
            mousePos.x - width / 2,
            mousePos.y - height / 2,
            width, height,
            SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_SHOWWINDOW);
    }

    ActivateWindow(hwnd);
}

// 启动主程序或激活现有实例
void LaunchOrActivateMainProgram() {
    const char* processName = "magic_box.exe";

    // 检查进程是否已经在运行
    DWORD pid = GetProcessIdByName(processName);

    if (pid != 0) {
        // 进程已存在，找到其主窗口
        HWND hwnd = FindMainWindow(pid);
        if (hwnd) {
            // 将窗口移动到鼠标位置并激活
            MoveWindowToMousePosition(hwnd);
            return;
        }
    }

    // 启动新实例
    std::string dir = GetExeDirectory();
    std::string mainExe = dir + "\\magic_box.exe";

    // 使用 ShellExecute 启动主程序，并等待一会然后激活
    HINSTANCE result = ShellExecuteA(NULL, "open", mainExe.c_str(), NULL, dir.c_str(), SW_SHOWNORMAL);
    if (reinterpret_cast<INT_PTR>(result) <= 32) {
        // 启动失败，可能是因为文件不存在
        // 尝试使用当前目录
        char currentDir[MAX_PATH] = { 0 };
        GetCurrentDirectoryA(MAX_PATH, currentDir);
        std::string altExe = std::string(currentDir) + "\\magic_box.exe";
        result = ShellExecuteA(NULL, "open", altExe.c_str(), NULL, currentDir, SW_SHOWNORMAL);
    }

    // 如果启动成功，等待一会然后尝试激活窗口
    if (reinterpret_cast<INT_PTR>(result) > 32) {
        Sleep(3000); // 等待让程序启动

        // 重新查找进程和窗口
        DWORD newPid = GetProcessIdByName("magic_box.exe");
        if (newPid != 0) {
            HWND newHwnd = FindMainWindow(newPid);
            if (newHwnd) {
                ActivateWindow(newHwnd);
            }
        }
    }
}

// 全局键盘钩子过程
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        if (wParam == WM_KEYDOWN) {
            KBDLLHOOKSTRUCT* pKeyboard = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);

            // 检测 Ctrl+~ 组合键
            // ~ 键通常对应 VK_OEM_3 (主键盘的反引号/波浪符键)，也可能为其他键码    
            if ((pKeyboard->vkCode == VK_OEM_3 || pKeyboard->vkCode == VK_OEM_8 ||
                pKeyboard->vkCode == 0xBD || pKeyboard->vkCode == 0xC0) &&  // 可能的波浪号键 
                (GetAsyncKeyState(VK_CONTROL) & 0x8000)) {  // Ctrl 键被按下

                // 再次确认 Ctrl 键仍被按下
                if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
                    LaunchOrActivateMainProgram();
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

    // 检查是否已注册到启动项，并验证路径是否正确
    bool pathCorrect = false;
    bool isRegistered = IsRegisteredInStartup(&pathCorrect);
    
    // 如果未注册或路径不正确，则添加或更新启动项
    if (!isRegistered || !pathCorrect) {
        RegisterInStartup(); // 注册失败也继续运行
    }

    // 保存实例句柄
    g_hInstance = hInstance;

    // 注册窗口类
    WNDCLASSA wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = SERVICE_NAME;
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
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
    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return static_cast<int>(msg.wParam);
}