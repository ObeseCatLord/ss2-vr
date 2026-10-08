// Private Proton display prerequisite only; no game, OpenXR or input is driven.
#include <windows.h>
#include <d3d9.h>
#include <cstdio>
struct Receipt {
    HANDLE file{INVALID_HANDLE_VALUE};
    ~Receipt() { if (file!=INVALID_HANDLE_VALUE) CloseHandle(file); }
    template<class... Args> bool emit(const char *format, Args... args) {
        char row[1024]{};
        const int size=std::snprintf(row,sizeof(row),format,args...);
        if (size<0 || size>=int(sizeof(row))) return false;
        DWORD written=0;
        return WriteFile(file,row,DWORD(size),&written,nullptr) && written==DWORD(size) && FlushFileBuffers(file);
    }
};
int main() {
    wchar_t selected[2]{};
    if (GetEnvironmentVariableW(L"SS2VR_LAB_PRIVATE_DISPLAY", selected, 2)!=1 || selected[0]!=L'1') {
        std::fputs("Explicit private-display selection required\n",stderr);return 2;
    }
    wchar_t destination[1024]{};
    const DWORD length=GetEnvironmentVariableW(L"SS2VR_DISPLAY_TIMING_OUTPUT",destination,1024);
    if (!length || length>=1024 || destination[0]!=L'Z' || destination[1]!=L':') return 9;
    Receipt receipt;
    receipt.file=CreateFileW(destination,GENERIC_WRITE,FILE_SHARE_READ,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);
    if (receipt.file==INVALID_HANDLE_VALUE) return 9;
    DEVMODEW desktop{};desktop.dmSize=sizeof(desktop);
    if (!EnumDisplaySettingsW(nullptr,ENUM_CURRENT_SETTINGS,&desktop)) {
        receipt.emit("{\"schema\":1,\"stage\":\"desktop-query-failed\",\"raster_called\":false}");return 3;
    }
    auto api=Direct3DCreate9(D3D_SDK_VERSION);
    if (!api) {receipt.emit("{\"schema\":1,\"stage\":\"d3d9-unavailable\",\"raster_called\":false}");return 4;}
    D3DDISPLAYMODE adapter{};HRESULT hr=api->GetAdapterDisplayMode(0,&adapter);
    if (FAILED(hr) || !adapter.Width || !adapter.Height || adapter.RefreshRate<=1 || desktop.dmDisplayFrequency<=1) {
        receipt.emit("{\"schema\":1,\"stage\":\"invalid-current-mode\",\"desktop_hz\":%lu,\"adapter_hz\":%u,\"raster_called\":false}\n",
                    desktop.dmDisplayFrequency,adapter.RefreshRate);api->Release();return 5;
    }
    // Match the private fixture's full-screen current mode. It is never shown
    // outside the separately owned display/prefix selected by the launcher.
    WNDCLASSW wc{};wc.lpfnWndProc=DefWindowProcW;wc.hInstance=GetModuleHandleW(nullptr);
    wc.lpszClassName=L"SS2VRPrivateDisplayTiming";
    if (!RegisterClassW(&wc)) {
        receipt.emit("{\"schema\":1,\"stage\":\"window-registration-failed\",\"win32_error\":%lu,\"raster_called\":false}\n",GetLastError());
        api->Release();return 6;
    }
    HWND window=CreateWindowW(wc.lpszClassName,L"SS2VR private display prerequisite",WS_POPUP,
                             0,0,int(adapter.Width),int(adapter.Height),nullptr,nullptr,wc.hInstance,nullptr);
    if (!window) {
        receipt.emit("{\"schema\":1,\"stage\":\"window-create-failed\",\"win32_error\":%lu,\"raster_called\":false}\n",GetLastError());
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
    if (chain) chain->Release();
    if (device) device->Release();
    api->Release();
    const bool windowDestroyed=DestroyWindow(window)!=FALSE;
    const bool classRetired=UnregisterClassW(wc.lpszClassName,wc.hInstance)!=FALSE;
    const bool cleaned=windowDestroyed && classRetired;
    const bool emitted=receipt.emit("{\"schema\":1,\"stage\":\"swapchain-query\",\"operation\":\"%s\",\"desktop_hz\":%lu,\"adapter_hz\":%u,\"swapchain_hz\":%u,"
                "\"width\":%u,\"height\":%u,\"raster_called\":%s,\"hresult\":%lu,\"scanline\":%u,\"in_vblank\":%s,\"cleanup_completed\":%s}\n",
                operation,desktop.dmDisplayFrequency,adapter.RefreshRate,actual.RefreshRate,actual.Width,actual.Height,
                called?"true":"false",static_cast<unsigned long>(hr),raster.ScanLine,raster.InVBlank?"true":"false",cleaned?"true":"false");
    return emitted ? (SUCCEEDED(hr)&&called&&cleaned?0:8) : 9;
}
