// Read-only receipts on an explicitly selected private X11 display.
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/XRes.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

static bool failed=false;
static int error(Display*, XErrorEvent*) { failed=true; return 0; }
static unsigned long property(Display* d, Window w, Atom key) {
    Atom type{}; int format{}; unsigned long count{},remaining{}; unsigned char* data=nullptr;
    unsigned long value=0;
    if (XGetWindowProperty(d,w,key,0,1,False,AnyPropertyType,&type,&format,&count,&remaining,&data)==Success &&
        format==32 && count==1 && data) value=*reinterpret_cast<unsigned long*>(data);
    if(data) XFree(data);
    return value;
}
int main() {
    const char* selected=std::getenv("SS2VR_LAB_PRIVATE_DISPLAY");
    if(!selected || std::strcmp(selected,"1") || !std::getenv("DISPLAY") || std::getenv("WAYLAND_DISPLAY")) return 2;
    Display* d=XOpenDisplay(nullptr); if(!d)return 3;
    XSetErrorHandler(error);
    int event{},err{},major{},minor{};
    if(!XResQueryExtension(d,&event,&err) || !XResQueryVersion(d,&major,&minor) || major<1 || (major==1 && minor<2)) {
        XCloseDisplay(d);return 4;
    }
    const Window root=DefaultRootWindow(d);
    const auto active=property(d,root,XInternAtom(d,"_NET_ACTIVE_WINDOW",False));
    std::vector<Window> pending{root}; unsigned visited=0,found=0;
    std::printf("{\"schema\":1,\"active_xid\":%lu,\"windows\":[",active);
    while(!pending.empty() && visited++<4096) {
        const Window w=pending.back();pending.pop_back();
        char* title=nullptr;
        if(XFetchName(d,w,&title) && title) {
            const unsigned role=!std::strcmp(title,"Monado")?1:
                (!std::strcmp(title,"SS2VR private display prerequisite")?2:0);
            if(role) {
                XWindowAttributes attributes{};
                const bool observed=XGetWindowAttributes(d,w,&attributes)!=0;
                XResClientIdSpec spec{w,XRES_CLIENT_ID_PID_MASK};
                long count=0; XResClientIdValue* ids=nullptr; pid_t pid=-1;
                if(XResQueryClientIds(d,1,&spec,&count,&ids)==Success) {
                    for(long i=0;i<count;++i)if(XResGetClientIdType(&ids[i])==XRES_CLIENT_ID_PID)pid=XResGetClientPid(&ids[i]);
                }
                if(ids)XResClientIdsDestroy(count,ids);
                std::printf("%s{\"xid\":%lu,\"role\":%u,\"pid\":%ld,\"viewable\":%s,\"input_output\":%s,\"width\":%d,\"height\":%d}",
                    found++?",":"",w,role,static_cast<long>(pid),observed&&attributes.map_state==IsViewable?"true":"false",
                    observed&&attributes.c_class==InputOutput?"true":"false",attributes.width,attributes.height);
            }
            XFree(title);
        }
        Window rootReturn{},parent{};Window* children=nullptr;unsigned count=0;
        if(XQueryTree(d,w,&rootReturn,&parent,&children,&count)) {
            if(pending.size()+count>4096)failed=true;
            else for(unsigned i=0;i<count;++i)pending.push_back(children[i]);
        }
        if(children)XFree(children);
    }
    if(!pending.empty())failed=true;
    XSync(d,False);XCloseDisplay(d);
    std::printf("],\"complete\":%s}\n",failed?"false":"true");
    return failed?5:0;
}
