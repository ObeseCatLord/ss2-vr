// Private-lab observer: actual IPC images only; never creates or acknowledges frames.
#include "common/ipc.hpp"
#include "common/frame_policy.hpp"
#include <cstdio>
#include <tlhelp32.h>
#include <shellapi.h>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <string>
using namespace ss2vr;
static void pose(FILE *f,const Pose &p) {
    std::fprintf(f,"{\"p\":[%.9g,%.9g,%.9g],\"q\":[%.9g,%.9g,%.9g,%.9g]}",
        p.p.x,p.p.y,p.p.z,p.q.x,p.q.y,p.q.z,p.q.w);
}
static BOOL CALLBACK closeWindow(HWND window,LPARAM pid) {
    DWORD owner=0;GetWindowThreadProcessId(window,&owner);
    wchar_t title[64]{};GetWindowTextW(window,title,64);
    if(owner==static_cast<DWORD>(pid)&&wcscmp(title,L"Serious Sam 2")==0)
        PostMessageW(window,WM_CLOSE,0,0);
    return TRUE;
}
struct OwnedLoading {
    HWND window=nullptr; DWORD pid=0,thread=0; uint32_t module=0,menu=0;
};
static bool loadingOwner(DWORD pid,const wchar_t *path,OwnedLoading &out) {
    HANDLE process=OpenProcess(PROCESS_VM_READ|PROCESS_QUERY_LIMITED_INFORMATION,FALSE,pid);
    if(!process)return false;
    wchar_t actual[2048]{};DWORD bytes=2048;
    const bool queried=QueryFullProcessImageNameW(process,0,actual,&bytes);
    std::wstring expected(path);
    for(auto &c:expected)if(c==L'/')c=L'\\';
    for(auto &c:actual)if(c==L'/')c=L'\\';
    bool ok=queried&&!_wcsicmp(actual,expected.c_str());
    HANDLE modules=ok?CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,pid):INVALID_HANDLE_VALUE;
    uint32_t base=0;MODULEENTRY32W module{sizeof(module)};
    if(modules!=INVALID_HANDLE_VALUE) {
        if(Module32FirstW(modules,&module))do {
            if(!_wcsicmp(module.szModule,L"Sam2Game.dll"))base=static_cast<uint32_t>(reinterpret_cast<uintptr_t>(module.modBaseAddr));
        }while(Module32NextW(modules,&module));
        CloseHandle(modules);
    }
    auto read=[&](uint32_t address,uint32_t &value) {
        SIZE_T got=0;return ReadProcessMemory(process,reinterpret_cast<void *>(uintptr_t(address)),&value,4,&got)&&got==4;
    };
    uint32_t menu=0,table=0,ready=0,again=0,tableAgain=0,readyAgain=0;
    ok=ok&&base&&read(base+0x40a270,menu)&&menu&&menu<=UINT32_MAX-0x6c&&
        read(menu,table)&&table==base+0x29f148&&read(menu+0x6c,ready)&&ready==1;
    const HWND window=GetForegroundWindow();DWORD owner=0;
    const DWORD thread=window?GetWindowThreadProcessId(window,&owner):0;
    wchar_t title[64]{};if(window)GetWindowTextW(window,title,64);
    ok=ok&&owner==pid&&thread&&wcscmp(title,L"Serious Sam 2")==0&&
        read(base+0x40a270,again)&&again==menu&&read(menu,tableAgain)&&tableAgain==table&&
        read(menu+0x6c,readyAgain)&&readyAgain==1;
    CloseHandle(process);
    if(ok)out={window,pid,thread,base,menu};
    return ok;
}
int wmain(int argc,wchar_t **argv) {
    if(argc<3)return 2;
    Channel channel;if(!channel.open(argv[2],false))return 3;
    if(wcscmp(argv[1],L"status")==0) {
        Lock lock(channel,20);if(!lock)return 4;
        if(argc!=3&&argc!=5)return 2;
        FILE *output=argc==5 ? _wfopen(argv[3],L"wbx") : stdout;
        if(!output)return 6;
        const auto &s=*channel.shared;const auto &i=s.latest;
        const auto now=GetTickCount64(), age=now>=i.tickMs ? now-i.tickMs : ~uint64_t{};
        std::fprintf(output,"{\"abi\":%u,\"game_pid\":%u,\"host_pid\":%u,\"renderer\":%u,\"gameplay\":%u,\"menu\":%u,\"input_sequence\":%llu,\"input_tick_ms\":%llu,\"input_age_ms\":%llu,\"focused\":%u,\"head_valid\":%u,\"head\":",
            s.abi,s.gamePid,s.hostPid,s.rendererReady,s.ui.gameplay,s.menu.visible,
            static_cast<unsigned long long>(i.sequence),static_cast<unsigned long long>(i.tickMs),
            static_cast<unsigned long long>(age),i.focused,i.headValid);
        pose(output,i.head);std::fprintf(output,",\"hands\":[");pose(output,i.hand[0]);std::fprintf(output,",");pose(output,i.hand[1]);
        OwnedLoading loading{};
        const bool ready=argc==5&&loadingOwner(s.gamePid,argv[4],loading);
        std::fprintf(output,"],\"slots\":[%u,%u],\"loading_ready\":%u}\n",static_cast<unsigned>(s.slot[0].state),static_cast<unsigned>(s.slot[1].state),ready);

        return output==stdout ? (std::fflush(output)==0?0:7) : (std::fclose(output)==0?0:7);
    }
    if(wcscmp(argv[1],L"continue-loading")==0) {
        if(argc!=4)return 2;
        DWORD pid=0;{Lock lock(channel,100);if(!lock)return 4;pid=channel.shared->gamePid;}
        OwnedLoading loading{};if(!loadingOwner(pid,argv[3],loading))return 8;
        const bool down=PostMessageW(loading.window,WM_KEYDOWN,VK_RETURN,0x001c0001);
        // Always attempt release; do not retain a key or retry this action.
        const bool up=PostMessageW(loading.window,WM_KEYUP,VK_RETURN,static_cast<LPARAM>(0xc01c0001u));
        return down&&up?0:9;
    }
    if(wcscmp(argv[1],L"close")==0) {
        DWORD pid=0;{Lock lock(channel,100);if(!lock)return 4;pid=channel.shared->gamePid;}
        EnumWindows(closeWindow,static_cast<LPARAM>(pid));return 0;
    }
    if(wcscmp(argv[1],L"capture")!=0||argc!=12)return 2;
    std::vector<uint8_t> pixels[2];for(auto &p:pixels)p.resize(EyeBytes);
    wchar_t *end=nullptr;
    const auto minimumInput=std::wcstoull(argv[4],&end,10);if(!end||*end)return 2;
    float expected[7]{};
    for(unsigned i=0;i<7;++i) {
        expected[i]=std::wcstof(argv[5+i],&end);if(!end||*end||!std::isfinite(expected[i]))return 2;
    }
    Request request{};uint32_t presentation=0;bool found=false;
    const auto deadline=GetTickCount64()+15000;
    while(GetTickCount64()<deadline&&!found) {
        {Lock lock(channel,2);if(lock) {
            const Slot *best=nullptr;
            const auto &shared=*channel.shared;
            for(const auto &s:shared.slot) {
                const auto &head=s.request.input.head;
                const double norm=double(head.q.x)*head.q.x+double(head.q.y)*head.q.y+
                    double(head.q.z)*head.q.z+double(head.q.w)*head.q.w;
                const double dot=double(head.q.x)*expected[3]+double(head.q.y)*expected[4]+
                    double(head.q.z)*expected[5]+double(head.q.w)*expected[6];
                const bool poseMatches=std::isfinite(norm)&&std::abs(norm-1)<.001&&std::isfinite(dot)&&std::abs(dot)>.99999&&
                    std::isfinite(head.p.x)&&std::isfinite(head.p.y)&&std::isfinite(head.p.z)&&
                    std::abs(head.p.x-expected[0])<.001&&std::abs(head.p.y-expected[1])<.001&&std::abs(head.p.z-expected[2])<.001;
                if(s.state==SlotState::Ready&&!s.cancelled&&s.request.input.sequence>=minimumInput&&
                    shared.rendererReady&&shared.ui.gameplay&&!shared.menu.visible&&s.request.input.focused&&s.request.input.headValid&&
                    poseMatches&&!s.request.reserved&&!s.presentationReserved&&s.presentation<=NativeUiComplete&&
                    s.request.trackingGeneration==trackingEpoch(shared)&&
                    s.request.session==shared.latest.session&&s.request.reference==shared.latest.reference&&
                    GetTickCount64()>=s.request.input.tickMs&&GetTickCount64()-s.request.input.tickMs<=FrameAgeMs&&s.request.width&&s.request.height&&
                    s.request.width<=MaxDimension&&s.request.height<=MaxDimension&&
                    (!best||s.request.sequence>best->request.sequence))best=&s;
            }
            if(best) {
                request=best->request;presentation=best->presentation;
                for(unsigned h=0;h<2;++h)std::memcpy(pixels[h].data(),best->pixels[h],size_t(request.width)*request.height*4);
                found=true;
            }
        }}
        if(!found)Sleep(1);
    }
    if(!found)return 5;
    for(unsigned h=0;h<2;++h) {
        auto name=std::wstring(argv[3])+(h?L"-right.ppm":L"-left.ppm");
        auto *f=_wfopen(name.c_str(),L"wbx");if(!f)return 6;
        std::fprintf(f,"P6\n%u %u\n255\n",request.width,request.height);
        std::vector<uint8_t> row(size_t(request.width)*3);
        for(unsigned y=0;y<request.height;++y) {
            for(unsigned x=0;x<request.width;++x) {
                const auto offset=(size_t(y)*request.width+x)*4;
                row[x*3]=pixels[h][offset+2];row[x*3+1]=pixels[h][offset+1];row[x*3+2]=pixels[h][offset];
            }
            if(std::fwrite(row.data(),1,row.size(),f)!=row.size()){std::fclose(f);return 7;}
        }
        if(std::fclose(f)!=0)return 7;
    }
    auto name=std::wstring(argv[3])+L"-metadata.json";
    auto *f=_wfopen(name.c_str(),L"wbx");if(!f)return 6;
    std::fprintf(f,"{\"sequence\":%llu,\"input_sequence\":%llu,\"input_tick_ms\":%llu,\"width\":%u,\"height\":%u,\"presentation\":%u,\"head\":",
        static_cast<unsigned long long>(request.sequence),static_cast<unsigned long long>(request.input.sequence),
        static_cast<unsigned long long>(request.input.tickMs),request.width,request.height,presentation);
    pose(f,request.input.head);std::fprintf(f,",\"eyes\":[");pose(f,request.eye[0]);std::fprintf(f,",");pose(f,request.eye[1]);
    std::fprintf(f,"],\"fov\":[");for(unsigned h=0;h<2;++h) {
        if(h)std::fprintf(f,",");const auto &v=request.fov[h];
        std::fprintf(f,"[%.9g,%.9g,%.9g,%.9g]",v.left,v.right,v.up,v.down);
    }
    std::fprintf(f,"]}\n");return std::fclose(f)==0?0:7;
}

// GUI subsystem avoids creating a console that can steal the owned game's focus.
int WINAPI wWinMain(HINSTANCE,HINSTANCE,wchar_t *,int) {
    int argc=0;auto **argv=CommandLineToArgvW(GetCommandLineW(),&argc);
    if(!argv)return 2;const int result=wmain(argc,argv);LocalFree(argv);return result;
}
