#include <windows.h>
#include "pluginsdk/_plugins.h"

#ifndef DLL_EXPORT
#define DLL_EXPORT __declspec(dllexport)
#endif //DLL_EXPORT

#define plugin_name "testplugin"
#define plugin_version 1

int pluginHandle;
HWND hwndDlg;
int hMenu;
int hMenuDisasm;
int hMenuDump;
int hMenuStack;

bool exec(int argc, char* argv[]) {
    for(unsigned i = 1;i < argc;i++) {
        DbgCmdExec(argv[i]);
    }
    return true;
}

bool printMemoryRegion(int argc, char* argv[]) {
    size_t length = 16;
    if(argc == 1)
        return true;
    if(argc > 2) {
        length = atoi(argv[2]);
    }

    auto memPtr = DbgValFromString(argv[1]);

    _plugin_logprintf("0x%x (%d):", memPtr, length);
    for(unsigned i = 0;i < length;i++) {
        uint8_t buf = 0;
        DbgMemRead(memPtr + i, &buf, 1);
        _plugin_logprintf("%02x ", buf);
    }
    _plugin_logprintf("\n");
    return true;
}

extern "C" DLL_EXPORT bool pluginit(PLUG_INITSTRUCT* initStruct)
{
    initStruct->pluginVersion = plugin_version;
    initStruct->sdkVersion = PLUG_SDKVERSION;
    strcpy(initStruct->pluginName, plugin_name);
    pluginHandle = initStruct->pluginHandle;

    _plugin_logprintf("[TEST] pluginHandle: %d\n", pluginHandle);
   if(!_plugin_registercommand(pluginHandle, "printMemoryRegion", printMemoryRegion, false))
       _plugin_logputs("[TEST] error registering the \"printMemoryRegion\" command!");
   if(!_plugin_registercommand(pluginHandle, "exec", exec, false))
       _plugin_logputs("[TEST] error registering the \"exec\" command!");

   return true;
}

extern "C" DLL_EXPORT bool plugstop()
{
    return true;
}

extern "C" DLL_EXPORT void plugsetup(PLUG_SETUPSTRUCT* setupStruct)
{
    hwndDlg = setupStruct->hwndDlg;
    hMenu = setupStruct->hMenu;
    hMenuDisasm = setupStruct->hMenuDisasm;
    hMenuDump = setupStruct->hMenuDump;
    hMenuStack = setupStruct->hMenuStack;
}

extern "C" DLL_EXPORT BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    return TRUE;
}
