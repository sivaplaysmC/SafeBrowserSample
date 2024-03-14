#include <combaseapi.h>
#include <cstddef>
#include <sstream>
#include <stdlib.h>
#include <tchar.h>
#include <wil/com.h>
#include <windef.h>
#include <windows.h>
#include <wingdi.h>
#include <winuser.h>
#include <wrl.h>

#include <debugapi.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include "WebView2.h"

using namespace Microsoft::WRL;
static TCHAR szWindowClass[] = _T("DesktopApp");

static TCHAR szTitle[] = _T("WebView sample");

HINSTANCE hInst;
HHOOK keyboardHook;

HWND CreateFullscreenWindow(HINSTANCE hinst);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
void SetFullscreen(HWND hWnd);

static wil::com_ptr<ICoreWebView2Controller> webviewController;

static wil::com_ptr<ICoreWebView2> webview;

wil::com_ptr<ICoreWebView2Settings7> settings7;

int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_ HINSTANCE hPrevInstance,
                     _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {

  SetProcessDPIAware();

  WNDCLASS wc = {};
  wc.lpfnWndProc = WndProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = szWindowClass;
  RegisterClass(&wc);

  hInst = hInstance;

  keyboardHook =
      SetWindowsHookExA(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);

  HWND hWnd = CreateFullscreenWindow(hInst);
  /* HWND hWnd = */
  /* CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
     CW_USEDEFAULT, */
  /*              CW_USEDEFAULT, 1200, 900, NULL, NULL, hInstance, NULL); */

  /* SetFullscreen(hWnd); */

  if (!hWnd) {
    MessageBox(NULL, _T("Call to CreateWindow failed!"),
               _T("Windows Desktop Guided Tour"), NULL);

    return 1;
  }

  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);

  CreateCoreWebView2EnvironmentWithOptions(
      nullptr, nullptr, nullptr,
      Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
          [hWnd](HRESULT result, ICoreWebView2Environment *env) -> HRESULT {
            // Create a CoreWebView2Controller and get the associated
            // CoreWebView2 whose parent is the main window hWnd
            env->CreateCoreWebView2Controller(
                hWnd,
                Callback<
                    ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                    [hWnd](HRESULT result,
                           ICoreWebView2Controller *controller) -> HRESULT {
                      if (controller != nullptr) {
                        webviewController = controller;
                        webviewController->get_CoreWebView2(&webview);
                      }

                      // Add a few settings for the webview
                      // The demo step is redundant since the values are the
                      // default settings
                      wil::com_ptr<ICoreWebView2Settings> settings;
                      webview->get_Settings(&settings);
                      settings->put_IsScriptEnabled(TRUE);
                      settings->put_AreDefaultScriptDialogsEnabled(TRUE);
                      settings->put_IsWebMessageEnabled(TRUE);
                      settings->put_AreDevToolsEnabled(FALSE);
                      settings->put_AreDefaultContextMenusEnabled(FALSE);

                      settings->QueryInterface(IID_PPV_ARGS(&settings7));
                      if (!settings7)
                        OutputDebugStringA("You are fucked");
                      else {
                        settings7->put_IsBuiltInErrorPageEnabled(FALSE);
                        settings7->put_AreBrowserAcceleratorKeysEnabled(FALSE);
                      }

                      // Resize WebView to fit the bounds of the parent window
                      RECT bounds;
                      GetClientRect(hWnd, &bounds);
                      webviewController->put_Bounds(bounds);

                      // Schedule an async task to navigate to Bing
                      webview->Navigate(
                          L"https://www.youtube.com/watch?v=xvFZjo5PgG0");
                      return S_OK;
                    })
                    .Get());
            return S_OK;
          })
          .Get());

  // Main message loop:

  OutputDebugStringA("Opening the window ^_^");

  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    if (settings7) {
      settings7->put_AreBrowserAcceleratorKeysEnabled(FALSE);
      BOOL flag;
      settings7->get_AreBrowserAcceleratorKeysEnabled(&flag);
      if (flag == FALSE)
        OutputDebugStringA("Got fucked up real quick ._.\n");
      settings7->put_AreBrowserAcceleratorKeysEnabled(FALSE);
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam,
                         LPARAM lParam) {
  switch (message) {
  case WM_DESTROY:
    OutputDebugStringA("Closing the window ^_^");
    UnhookWindowsHookEx(keyboardHook);
    PostQuitMessage(0);
    break;
  default:
    return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
}

std::stringstream strstream;

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode == HC_ACTION) {
    KBDLLHOOKSTRUCT *pKeyInfo = (KBDLLHOOKSTRUCT *)lParam;
    auto AltDown = GetAsyncKeyState(VK_MENU);
    auto CtrlDown = GetAsyncKeyState(VK_CONTROL);
    auto vkCode = pKeyInfo->vkCode;
    /* AltDown = CtrlDown = 0; */

    // restrict function keys
    if (vkCode >= VK_F1 && vkCode <= VK_F24)
      return 1;
#if 0
    if (GetAsyncKeyState(VK_LWIN))
      return 1;
#endif

    if (AltDown) {
      switch (vkCode) {
      case VK_TAB:
      case VK_LEFT:
      case VK_RIGHT:
        return 1;
      }
    }
    if (CtrlDown) {
      switch (vkCode) {
      case VK_TAB:
      case VK_LEFT:
      case VK_RIGHT:
        return 1;
      }
    }
    if (AltDown && CtrlDown) {
      switch (vkCode) {
      case VK_TAB:
      case VK_LEFT:
      case VK_RIGHT:
        return 1;
      }
    }
  }
  return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

HWND CreateFullscreenWindow(HINSTANCE hinst) {
  HMONITOR hmon = MonitorFromWindow(0, MONITOR_DEFAULTTONEAREST);
  MONITORINFO mi = {sizeof(mi)};
  if (!GetMonitorInfo(hmon, &mi))
    return NULL;
  return CreateWindow(
      szWindowClass, szTitle, WS_POPUP | WS_VISIBLE, mi.rcMonitor.left,
      mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left,
      mi.rcMonitor.bottom - mi.rcMonitor.top, 0, NULL, hinst, 0);
}
