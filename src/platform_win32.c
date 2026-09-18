#include "../include/platform.h"
#include <windows.h>

static HWND g_hwnd = NULL;
static int g_scale = 3;
static BITMAPINFO g_bmi;             // pixel format description for StretchDIBits
static u8 g_bgr[SCREEN_HEIGHT][SCREEN_WIDTH][3]; // scratch buffer, BGR byte order (Windows DIBs, not RGB)
static LARGE_INTEGER g_freq;         // QueryPerformanceCounter ticks per second, cached once
static LARGE_INTEGER g_last_present; // timestamp of the previous platform_present, for pacing
static bool g_quit = false;          // set once the user closes the window

// Every Win32 window needs a procedure to handle messages the OS sends it.
// We only care about being closed; everything else goes to the default
// handler, which does whatever the OS normally expects.
static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp){
  switch (msg) {
    case WM_CLOSE:
      // The user clicked the close button. Just remember it - platform_poll
      // reports this back to main.c's own loop via its return value, so the
      // caller can exit cleanly on its own terms rather than being torn
      // down from inside a message handler.
      g_quit = true;
      DestroyWindow(hwnd);
      return 0;
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
    default:
      return DefWindowProc(hwnd, msg, wp, lp);
  }
}

// Registers a window class and creates a fixed-size, non-resizable window
// scaled up from the native 160x144 resolution.
bool platform_init(int scale){
  g_scale = scale;

  WNDCLASS wc = {0};
  wc.lpfnWndProc = wnd_proc;
  wc.hInstance = GetModuleHandle(NULL);
  wc.lpszClassName = "GBemuWindowClass";
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  if (!RegisterClass(&wc)) return false;

  // We ask for a client area of exactly SCREEN_WIDTH*scale x
  // SCREEN_HEIGHT*scale, but CreateWindow's width/height parameters
  // include the window's title bar and borders too. AdjustWindowRect
  // grows the rect by however much chrome this window style adds, so the
  // *drawable* area ends up exactly the size we want.
  RECT rect = {0, 0, SCREEN_WIDTH * scale, SCREEN_HEIGHT * scale};
  AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, FALSE);

  g_hwnd = CreateWindow(
    wc.lpszClassName, "GBemu",
    (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX) | WS_VISIBLE,
    CW_USEDEFAULT, CW_USEDEFAULT,
    rect.right - rect.left, rect.bottom - rect.top,
    NULL, NULL, wc.hInstance, NULL);
  if (!g_hwnd) return false;

  // Describes the pixel buffer we'll hand StretchDIBits every frame: 24
  // bits per pixel, no compression, and - the detail that actually
  // matters - a *negative* height, which tells Windows our rows go
  // top-to-bottom like the PPU's framebuffer, instead of a plain BMP's
  // native bottom-to-top row order. That saves having to flip every row
  // ourselves on each present.
  g_bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  g_bmi.bmiHeader.biWidth = SCREEN_WIDTH;
  g_bmi.bmiHeader.biHeight = -SCREEN_HEIGHT;
  g_bmi.bmiHeader.biPlanes = 1;
  g_bmi.bmiHeader.biBitCount = 24;
  g_bmi.bmiHeader.biCompression = BI_RGB;

  QueryPerformanceFrequency(&g_freq);
  QueryPerformanceCounter(&g_last_present);
  g_quit = false;
  return true;
}

// Must be called regularly (main.c does it every 256 instructions, not
// just once per frame) regardless of what the emulated game is doing -
// Windows marks a window "Not Responding" within a couple of seconds of
// its message queue going unpumped, even if the process is otherwise
// perfectly healthy.
bool platform_poll(PlatformInput *out_input){
  MSG msg;
  while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  // GetAsyncKeyState reads real, current hardware key state directly from
  // the OS - it works regardless of whether our window has focus, and
  // doesn't depend on the message queue we just pumped above. The high
  // bit (0x8000) means "currently held down".
  out_input->right  = (GetAsyncKeyState(VK_RIGHT) & 0x8000) != 0;
  out_input->left   = (GetAsyncKeyState(VK_LEFT)  & 0x8000) != 0;
  out_input->up     = (GetAsyncKeyState(VK_UP)    & 0x8000) != 0;
  out_input->down   = (GetAsyncKeyState(VK_DOWN)  & 0x8000) != 0;
  out_input->a      = (GetAsyncKeyState('X')      & 0x8000) != 0;
  out_input->b      = (GetAsyncKeyState('Z')      & 0x8000) != 0;
  out_input->select = (GetAsyncKeyState(VK_BACK)  & 0x8000) != 0;
  out_input->start  = (GetAsyncKeyState(VK_RETURN)& 0x8000) != 0;

  return !g_quit;
}

// Copies the framebuffer into the window and paces to ~59.7 FPS (the real
// Game Boy's refresh rate). Should be called once per completed frame
// (PPU.frame_ready), not every instruction.
void platform_present(u8 framebuffer[SCREEN_HEIGHT][SCREEN_WIDTH][3]){
  // Windows DIBs store pixels as BGR, not RGB - swap channels while
  // copying into the scratch buffer StretchDIBits will actually read.
  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
      g_bgr[y][x][0] = framebuffer[y][x][2];
      g_bgr[y][x][1] = framebuffer[y][x][1];
      g_bgr[y][x][2] = framebuffer[y][x][0];
    }
  }

  HDC hdc = GetDC(g_hwnd);
  RECT client;
  GetClientRect(g_hwnd, &client);
  // Source is always the native 160x144 framebuffer; destination is
  // whatever the window's client area currently measures, so this also
  // handles the up-scaling (and would handle window resizing, if the
  // window style allowed it).
  StretchDIBits(hdc,
    0, 0, client.right - client.left, client.bottom - client.top,
    0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
    g_bgr, &g_bmi, DIB_RGB_COLORS, SRCCOPY);
  ReleaseDC(g_hwnd, hdc);

  // Even with cycle-accurate timing, the emulation loop itself runs as
  // fast as the host CPU allows - there's no hardware clock throttling it
  // to real Game Boy speed. Sleeping out whatever's left of a 1/59.7s
  // budget after each frame is what actually paces gameplay to a normal,
  // playable speed instead of finishing in a fraction of a second.
  LARGE_INTEGER now;
  QueryPerformanceCounter(&now);
  double elapsed_ms = (double)(now.QuadPart - g_last_present.QuadPart) * 1000.0 / (double)g_freq.QuadPart;
  double target_ms = 1000.0 / 59.7;
  if (elapsed_ms < target_ms) {
    Sleep((DWORD)(target_ms - elapsed_ms));
  }
  QueryPerformanceCounter(&g_last_present);
}

void platform_shutdown(void){
  if (g_hwnd) DestroyWindow(g_hwnd);
  g_hwnd = NULL;
}
