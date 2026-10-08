// Private Proton display prerequisite only; no game, OpenXR or input is driven.
#include <windows.h>
#include <d3d9.h>
#include <cstdio>
int main() {
    wchar_t selected[2]{};
    if (GetEnvironmentVariableW(L"SS2VR_LAB_PRIVATE_DISPLAY", selected, 2)!=1 || selected[0]!=L'1') {
        std::fputs("Explicit private-display selection required\n",stderr);return 2;
    }
    DEVMODEW desktop{};desktop.dmSize=sizeof(desktop);
    if (!EnumDisplaySettingsW(nullptr,ENUM_CURRENT_SETTINGS,&desktop)) {
        std::puts("{\"schema\":1,\"stage\":\"desktop-query-failed\",\"raster_called\":false}");return 3;
    }
    auto api=Direct3DCreate9(D3D_SDK_VERSION);
    if (!api) {std::puts("{\"schema\":1,\"stage\":\"d3d9-unavailable\",\"raster_called\":false}");return 4;}
    D3DDISPLAYMODE adapter{};HRESULT hr=api->GetAdapterDisplayMode(0,&adapter);
    if (FAILED(hr) || !adapter.Width || !adapter.Height || adapter.RefreshRate<=1 || desktop.dmDisplayFrequency<=1) {
        std::printf("{\"schema\":1,\"stage\":\"invalid-current-mode\",\"desktop_hz\":%lu,\"adapter_hz\":%u,\"raster_called\":false}\n",
                    desktop.dmDisplayFrequency,adapter.RefreshRate);api->Release();return 5;
    }
    // Match the private fixture's full-screen current mode. It is never shown
    // outside the separately owned display/prefix selected by the launcher.
    WNDCLASSW wc{};wc.lpfnWndProc=DefWindowProcW;wc.hInstance=GetModuleHandleW(nullptr);
    wc.lpszClassName=L"SS2VRPrivateDisplayTiming";
    if (!RegisterClassW(&wc)) {
        std::printf("{\"schema\":1,\"stage\":\"window-registration-failed\",\"win32_error\":%lu,\"raster_called\":false}\n",GetLastError());
        api->Release();return 6;
    }
    HWND window=CreateWindowW(wc.lpszClassName,L"SS2VR private display prerequisite",WS_POPUP,
                             0,0,int(adapter.Width),int(adapter.Height),nullptr,nullptr,wc.hInstance,nullptr);
    if (!window) {
        std::printf("{\"schema\":1,\"stage\":\"window-create-failed\",\"win32_error\":%lu,\"raster_called\":false}\n",GetLastError());
        UnregisterClassW(wc.lpszClassName,wc.hInstance);api->Release();return 7;
    }
    D3DPRESENT_PARAMETERS pp{};pp.hDeviceWindow=window;pp.Windowed=FALSE;
    pp.BackBufferWidth=adapter.Width;pp.BackBufferHeight=adapter.Height;pp.BackBufferFormat=adapter.Format;
    pp.BackBufferCount=1;pp.SwapEffect=D3DSWAPEFFECT_DISCARD;pp.PresentationInterval=D3DPRESENT_INTERVAL_IMMEDIATE;
    IDirect3DDevice9 *device=nullptr;
    const char *operation="create-device";
    hr=api->CreateDevice(0,D3DDEVTYPE_HAL,window,D3DCREATE_SOFTWARE_VERTEXPROCESSING,&pp,&device);
    IDirect3DSwapChain9 *chain=nullptr;D3DDISPLAYMODE actual{};D3DRASTER_STATUS raster{};bool called=false;
    if (SUCCEEDED(hr)) {operation="get-swap-chain";hr=device->GetSwapChain(0,&chain);}
    if (SUCCEEDED(hr)) {operation="get-display-mode";hr=chain->GetDisplayMode(&actual);}
    if (SUCCEEDED(hr)) {
        operation="validate-swapchain-mode";
        if (!actual.Width || !actual.Height || actual.RefreshRate<=1) hr=D3DERR_INVALIDCALL;
        else {operation="get-raster-status";called=true;hr=chain->GetRasterStatus(&raster);}
    }
    std::printf("{\"schema\":1,\"stage\":\"swapchain-query\",\"operation\":\"%s\",\"desktop_hz\":%lu,\"adapter_hz\":%u,\"swapchain_hz\":%u,"
                "\"width\":%u,\"height\":%u,\"raster_called\":%s,\"hresult\":%lu,\"scanline\":%u,\"in_vblank\":%s}\n",
                operation,desktop.dmDisplayFrequency,adapter.RefreshRate,actual.RefreshRate,actual.Width,actual.Height,
                called?"true":"false",static_cast<unsigned long>(hr),raster.ScanLine,raster.InVBlank?"true":"false");
    if (chain) chain->Release();
    if (device) device->Release();
    api->Release();
    DestroyWindow(window);UnregisterClassW(wc.lpszClassName,wc.hInstance);
    return SUCCEEDED(hr)&&called?0:8;
}
