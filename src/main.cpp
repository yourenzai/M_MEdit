#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlwapi.h>
#include <string>

#include "WebView2.h"
#include "WebView2Helper.h"

// WebView2 全局控制接口指针
static ICoreWebView2Controller* g_controller = nullptr;
static ICoreWebView2* g_webview = nullptr;

// 获取当前运行目录下的本地 HTML 绝对路径
std::wstring GetLocalAssetUrl(const std::wstring& relativePath) {
    wchar_t exePath[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    PathRemoveFileSpecW(exePath);

    std::wstring fullPath = exePath;
    fullPath += L"\\" + relativePath;

    // 转换为标准 file:/// 协议 URL
    constexpr DWORD MAX_URL = 2048;
    wchar_t fileUrl[MAX_URL] = { 0 };
    DWORD urlLen = MAX_URL;
    UrlCreateFromPathW(fullPath.c_str(), fileUrl, &urlLen, 0);

    return fileUrl;
}

// 初始化 WebView2 运行时并绑定到指定的窗口 HWND
void InitWebView(HWND hWnd) {
    // 步骤 1：异步创建 WebView2 环境
    auto onEnvCreated = Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
        [hWnd](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
            if (FAILED(result) || !env) {
                MessageBoxW(hWnd, L"创建 WebView2 环境失败！请检查系统是否安装了 WebView2 Runtime。", L"错误", MB_ICONERROR);
                return result;
            }

            // 步骤 2：环境创建成功后，异步创建 Controller（关联到宿主窗口句柄）
            auto onControllerCreated = Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                [hWnd](HRESULT res, ICoreWebView2Controller* controller) -> HRESULT {
                    if (FAILED(res) || !controller) {
                        MessageBoxW(hWnd, L"创建 WebView2 Controller 失败！", L"错误", MB_ICONERROR);
                        return res;
                    }

                    g_controller = controller;
                    g_controller->AddRef(); // 增加引用计数持有指针
                    g_controller->get_CoreWebView2(&g_webview);

                    // 调整 WebView2 区域使其贴合窗口客户区
                    RECT bounds;
                    GetClientRect(hWnd, &bounds);
                    g_controller->put_Bounds(bounds);

                    // 步骤 3：加载本地前端页面
                    std::wstring startUrl = GetLocalAssetUrl(L"assets\\index.html");
                    g_webview->Navigate(startUrl.c_str());

                    return S_OK;
                }
            );

            env->CreateCoreWebView2Controller(hWnd, onControllerCreated);
            return S_OK;
        }
    );

    // 调用官方加载器接口启动初始化流程
    CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr, onEnvCreated);
}

// 窗口消息处理过程
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_SIZE:
        // 当宿主窗口大小发生改变时，同步更新 WebView2 的尺寸
        if (g_controller != nullptr) {
            RECT bounds;
            GetClientRect(hWnd, &bounds);
            g_controller->put_Bounds(bounds);
        }
        return 0;

    case WM_DESTROY:
        // 释放 WebView2 接口对象
        if (g_webview) {
            g_webview->Release();
            g_webview = nullptr;
        }
        if (g_controller) {
            g_controller->Close();
            g_controller->Release();
            g_controller = nullptr;
        }
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
}

// 应用程序入口点 (Win32 GUI)
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    // 初始化 COM 库（WebView2 必须要求）
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        MessageBoxW(NULL, L"COM 库初始化失败！", L"错误", MB_ICONERROR);
        return 1;
    }

    // 1. 注册窗口类
    const wchar_t CLASS_NAME[] = L"M_MEdit_MainWindow";
    WNDCLASSEXW wcex = { sizeof(WNDCLASSEXW) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = CLASS_NAME;
    RegisterClassExW(&wcex);

    // 2. 创建主窗口 (初始尺寸 1000x700，居中显示)
    HWND hWnd = CreateWindowExW(
        0, CLASS_NAME, L"M_MEdit - Markdown 编辑器",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1000, 700,
        NULL, NULL, hInstance, NULL
    );

    if (!hWnd) {
        CoUninitialize();
        return 1;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // 3. 启动 WebView2
    InitWebView(hWnd);

    // 4. 标准 Windows 消息循环
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    // 反初始化 COM
    CoUninitialize();
    return (int)msg.wParam;
}

// 兼容 ANSI 链接器入口
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR, int nCmdShow) {
    return wWinMain(hInstance, hPrevInstance, GetCommandLineW(), nCmdShow);
}
