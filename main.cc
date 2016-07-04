#include <windows.h>
#include "pluginsdk/_plugins.h"

extern "C" void plugsetup(PLUG_SETUPSTRUCT* setupStruct);
bool chaiRun(int argc, char* argv[]);

int main(int argc, char** argv) {
    PLUG_SETUPSTRUCT setupStruct;
    plugsetup(&setupStruct);
    char* args[2] = {"", "C:\\tools\\x64dbg\\release\\x32\\scripts\\example.chai"};
    chaiRun(2, args);

    return 0;
}
