#include "apply_patches.hpp"

#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <commctrl.h>
#include <malloc.h>
#include <shobjidl.h>

#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

using SetThreadDpiAwarenessContextProc = decltype(&SetThreadDpiAwarenessContext);
using GetDpiForWindowProc = decltype(&GetDpiForWindow);
using SystemParametersInfoForDpiProc = decltype(&SystemParametersInfoForDpi);

static const SetThreadDpiAwarenessContextProc setThreadDpiAwarenessContext =
   reinterpret_cast<SetThreadDpiAwarenessContextProc>(
      GetProcAddress(LoadLibraryW(L"User32.dll"), "SetThreadDpiAwarenessContext"));
static const GetDpiForWindowProc getDpiForWindow = reinterpret_cast<GetDpiForWindowProc>(
   GetProcAddress(LoadLibraryW(L"User32.dll"), "GetDpiForWindow"));
static const SystemParametersInfoForDpiProc systemParametersInfoForDpi =
   reinterpret_cast<SystemParametersInfoForDpiProc>(
      GetProcAddress(LoadLibraryW(L"User32.dll"), "SystemParametersInfoForDpi"));

static const wchar_t className[] = L"BF2MemExt";

static const HINSTANCE hInstance = GetModuleHandle(nullptr);
static HWND hwndMain;
static HWND hwndTextBox;
static HWND hwndOpenButton;
static HWND hwndOpenTooltip;
static int g_dpi = 96;

static char* textBoxBuffer = nullptr;
static size_t textBoxBufferSize = 0;

static auto RegisterWindowClass() noexcept -> ATOM;

[[nodiscard]] static bool InitWindows() noexcept;

static void Cleanup() noexcept;

static void UpdateControlFont(HWND window, const LOGFONT& logFont) noexcept;

static void UpdateControlFonts() noexcept;

static int PrintToTextBox(_In_z_ _Printf_format_string_ const char* format, ...) noexcept;

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM) noexcept;

[[nodiscard]] static auto ScaleForDpi(int size) noexcept -> int;

[[nodiscard]] static auto GetFont(HWND window) noexcept -> HFONT;

[[nodiscard]] static auto PickFile(HWND window) noexcept -> char*;

int show_gui() noexcept
{
   if (setThreadDpiAwarenessContext) {
      setThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
   }

   INITCOMMONCONTROLSEX common_controls_init;
   common_controls_init.dwSize = sizeof(INITCOMMONCONTROLSEX);
   common_controls_init.dwICC = ICC_STANDARD_CLASSES;

   if (not InitCommonControlsEx(&common_controls_init)) {
      return 1;
   }

   RegisterWindowClass();

   if (not InitWindows()) {
      Cleanup();

      return 1;
   }

   MSG msg;

   while (GetMessageW(&msg, nullptr, 0, 0)) {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
   }

   Cleanup();

   return (int)msg.wParam;
}

static auto RegisterWindowClass() noexcept -> ATOM
{
   WNDCLASSEXW wcex;

   HICON icon = nullptr; // LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32UI));

   wcex.cbSize = sizeof(WNDCLASSEX);

   wcex.style = CS_HREDRAW | CS_VREDRAW;
   wcex.lpfnWndProc = WndProc;
   wcex.cbClsExtra = 0;
   wcex.cbWndExtra = 0;
   wcex.hInstance = hInstance;
   wcex.hIcon = icon;
   wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
   wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
   wcex.lpszMenuName = nullptr;
   wcex.lpszClassName = className;
   wcex.hIconSm = icon;

   return RegisterClassExW(&wcex);
}

static bool InitWindows() noexcept
{
   hwndMain = CreateWindowExW(0, className, L"BF2 Memory Extender", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                              CW_USEDEFAULT, 440, 256, nullptr, nullptr, hInstance, nullptr);

   if (not hwndMain) return false;

   hwndTextBox = CreateWindowExW(0, L"EDIT", nullptr,
                                 WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE |
                                    ES_AUTOVSCROLL | ES_READONLY,
                                 0, 0, 0, 0, // Size set in WM_SIZE message.
                                 hwndMain, nullptr, hInstance, nullptr);

   if (not hwndTextBox) return false;

   hwndOpenButton = CreateWindowExW(0, L"BUTTON", L"Patch Executable",
                                    WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 0, 0, 0,
                                    0,        // Size set in WM_SIZE message.
                                    hwndMain, // Parent window
                                    nullptr, hInstance, nullptr);

   if (not hwndOpenButton) return false;

   hwndOpenTooltip = CreateWindowExW(0, TOOLTIPS_CLASS, nullptr, WS_POPUP | TTS_ALWAYSTIP,
                                     CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                     hwndMain, nullptr, hInstance, nullptr);

   if (not hwndOpenTooltip) return false;

   wchar_t str[] = L"Open a file picker and select an executable to patch.";

   TOOLINFOW toolInfo = {
      .cbSize = sizeof(toolInfo),
      .uFlags = TTF_IDISHWND | TTF_SUBCLASS,
      .hwnd = hwndMain,
      .uId = (UINT_PTR)hwndOpenButton,
      .lpszText = str,
   };

   SendMessageW(hwndOpenTooltip, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);

   if (getDpiForWindow) g_dpi = getDpiForWindow(hwndMain);

   if (g_dpi != 96) {
      if (RECT rect{}; GetWindowRect(hwndMain, &rect)) {
         MoveWindow(hwndMain, rect.left, rect.top, ScaleForDpi(rect.right - rect.left),
                    ScaleForDpi(rect.bottom - rect.top), false);
      }
   }

   UpdateControlFonts();
   ShowWindow(hwndMain, SW_NORMAL);
   UpdateWindow(hwndMain);

   return TRUE;
}

static void Cleanup() noexcept
{
   if (textBoxBuffer) free(textBoxBuffer);

   if (not hwndMain) return;

   if (hwndTextBox) {
      if (HFONT font = GetFont(hwndTextBox); font) DeleteObject(font);
   }

   if (hwndOpenButton) {
      if (HFONT font = GetFont(hwndOpenButton); font) DeleteObject(font);
   }

   if (hwndOpenTooltip) {
      if (HFONT font = GetFont(hwndOpenTooltip); font) DeleteObject(font);
   }

   DestroyWindow(hwndMain);
}

static void UpdateControlFont(HWND window, const LOGFONT& logFont) noexcept
{
   HFONT oldFont = GetFont(window);

   if (HFONT newFont = CreateFontIndirectW(&logFont); newFont) {
      SendMessageW(window, WM_SETFONT, (WPARAM)newFont, MAKELPARAM(TRUE, 0));

      if (oldFont) DeleteObject(oldFont);
   }
}

static void UpdateControlFonts() noexcept
{
   if (not systemParametersInfoForDpi) return;

   NONCLIENTMETRICS metrics{.cbSize = sizeof(NONCLIENTMETRICS)};
   systemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS, sizeof(metrics), &metrics, false, g_dpi);

   UpdateControlFont(hwndTextBox, metrics.lfMessageFont);
   UpdateControlFont(hwndOpenButton, metrics.lfCaptionFont);
   UpdateControlFont(hwndOpenTooltip, metrics.lfStatusFont);
}

static int PrintToTextBox(_In_z_ _Printf_format_string_ const char* format, ...) noexcept
{
   va_list args;
   va_start(args, format);

   const int size = vsnprintf(nullptr, 0, format, args);

   char* string = static_cast<char*>(_malloca(size + 1));

   if (not string) goto cleanup;

   if (vsnprintf(string, size + 1, format, args) < 0) goto cleanup;

   string[size] = '\0';

   if (not textBoxBuffer) {
      textBoxBuffer = _strdup(string);
      textBoxBufferSize = size;
   }
   else {
      const size_t newtextBoxBufferSize = textBoxBufferSize + size;

      char* newTextBoxBuffer = static_cast<char*>(malloc(newtextBoxBufferSize + 1));

      if (newTextBoxBuffer == nullptr) goto cleanup;

      memcpy(&newTextBoxBuffer[0], textBoxBuffer, textBoxBufferSize);
      memcpy(&newTextBoxBuffer[textBoxBufferSize], string, size);

      newTextBoxBuffer[newtextBoxBufferSize] = '\0';

      free(textBoxBuffer);

      textBoxBuffer = newTextBoxBuffer;
      textBoxBufferSize = newtextBoxBufferSize;
   }

   if (textBoxBuffer) SetWindowTextA(hwndTextBox, textBoxBuffer);

cleanup:
   va_end(args);

   if (string) _freea(string);

   return 0;
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept
{
   switch (message) {
   case WM_COMMAND: {
      if (lParam == reinterpret_cast<LPARAM>(hwndOpenButton) and HIWORD(wParam) == BN_CLICKED) {
         char* file = PickFile(hWnd);

         if (file) {
            if (apply(file, PrintToTextBox)) {
               MessageBoxW(hwndMain, L"Executable patched successfully. You can now close this tool.",
                           L"Success", MB_OK);
            }
            else {
               MessageBoxW(hwndMain, L"Failed to patch executable. See text output for more info.",
                           L"Failed", MB_OK | MB_ICONERROR);
            }

            free(file);
         }
      }
   }; break;
   case WM_SIZE: {
      const int width = LOWORD(lParam);
      const int height = HIWORD(lParam);

      const int log_width = width;
      const int log_height = height - ScaleForDpi(48);

      const int button_x = ScaleForDpi(8);
      const int button_y = log_height + ScaleForDpi(8);
      const int button_width = width - ScaleForDpi(16);
      const int button_height = ScaleForDpi(32);

      MoveWindow(hwndTextBox, 0, 0, log_width, log_height, TRUE);

      MoveWindow(hwndOpenButton, button_x, button_y, button_width, button_height, TRUE);
   } break;
   case WM_DPICHANGED: {
      g_dpi = HIWORD(wParam);

      RECT* const prcNewWindow = (RECT*)lParam;
      SetWindowPos(hWnd, nullptr, prcNewWindow->left, prcNewWindow->top,
                   prcNewWindow->right - prcNewWindow->left,
                   prcNewWindow->bottom - prcNewWindow->top, SWP_NOZORDER | SWP_NOACTIVATE);

      UpdateControlFonts();

      break;
   }
   case WM_DESTROY:
      PostQuitMessage(0);
      break;
   default:
      return DefWindowProcW(hWnd, message, wParam, lParam);
   }
   return 0;
}

static auto ScaleForDpi(int size) noexcept -> int
{
   return size * g_dpi / 96;
};

static auto GetFont(HWND window) noexcept -> HFONT
{
   return reinterpret_cast<HFONT>(SendMessageW(hwndTextBox, WM_GETFONT, 0, 0));
}

static auto PickFile(HWND window) noexcept -> char*
{
   if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) {
      return nullptr;
   }

   const static COMDLG_FILTERSPEC filter = {L"Executable", L"*.exe"};

   char* result = nullptr;

   IFileOpenDialog* dialog = nullptr;
   IShellItem* item = nullptr;

   wchar_t* name = nullptr;
   int name_size = 0;

   if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog)))) {
      goto cleanup;
   }

   if (FAILED(dialog->SetOptions(FOS_PATHMUSTEXIST | FOS_NOCHANGEDIR | FOS_FILEMUSTEXIST))) {
      goto cleanup;
   }

   if (FAILED(dialog->SetTitle(L"Select BF2 Executable"))) {
      goto cleanup;
   }

   if (FAILED(dialog->SetFileTypes(1, &filter))) {
      goto cleanup;
   }

   if (FAILED(dialog->SetClientGuid(
          {0x15ee4b89, 0xad62, 0x4e42, {0xbf, 0xdb, 0xd6, 0x15, 0x8f, 0xb3, 0x7a, 0x64}}))) {
      goto cleanup;
   }

   if (FAILED(dialog->Show(window))) {
      goto cleanup;
   }

   if (FAILED(dialog->GetResult(&item))) {
      goto cleanup;
   }

   if (FAILED(item->GetDisplayName(SIGDN_FILESYSPATH, &name))) {
      goto cleanup;
   }

   name_size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, name, -1, nullptr, 0, nullptr, nullptr);

   if (name_size == 0) goto cleanup;

   result = static_cast<char*>(malloc(name_size + 1));

   if (not result) goto cleanup;

   result[name_size] = '\0';

   if (WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, name, -1, result, name_size, nullptr,
                           nullptr) == 0) {
      free(result);

      goto cleanup;
   }

cleanup:
   if (name) CoTaskMemFree(name);
   if (item) item->Release();
   if (dialog) dialog->Release();

   CoUninitialize();

   return result;
}
