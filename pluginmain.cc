#include <functional>
#include <windows.h>
#include "pluginsdk/_plugins.h"
#include <iomanip>

#include <QFileDialog>

#include <chaiscript/chaiscript.hpp>
#include <chaiscript/chaiscript_stdlib.hpp>

#include <chaiinterops.h>
#include <regex>

chaiscript::ChaiScript chai(chaiscript::Std_Lib::library());

#ifndef DLL_EXPORT
#define DLL_EXPORT __declspec(dllexport)
#endif //DLL_EXPORT

#define plugin_name "CHAISCRIPTPLUGIN"
#define plugin_version 1

int pluginHandle;
HWND hwndDlg;
int hMenu;
int hMenuDisasm;
int hMenuDump;
int hMenuStack;

std::string to_hex(int d, size_t digits = 2) {
    std::stringstream ss;
    ss << std::hex <<
          std::setfill('0') <<
          std::uppercase <<
          std::setw(digits) << d;
    return ss.str();
}

bool chaiEval(int argc, char* argv[]) {
    if(argc < 2) {
        _plugin_logprintf("Please specify a chai command\n");
        return true;
    }

    try {
        chai.eval(argv[1]);
    }
    catch ( const std::exception &e ) {
        _plugin_logprintf(" >>> Exception thrown: %s \n" , e.what( ) );
    }

    return true;
}

bool chaiShowEnv(int argc, char* argv[]) {
    _plugin_logprintf("Chai locals: \n");

    std::regex reg(argc > 1 ? argv[1] : ".*");
    for(const auto &sym : chai.get_locals()) {
        if(std::regex_search(sym.first, reg)) {
            const chaiscript::Boxed_Value& bv = sym.second;
            _plugin_logprintf("\t%s: %s\n", sym.first.c_str(), bv.get_type_info().name().c_str() );
        }
    }

    _plugin_logprintf("\nChai functions: \n\t");
    const auto funcs = chai.get_state().engine_state.m_boxed_functions;
    for(const auto &func : funcs) {
        if(std::regex_search(func.first, reg)) {
            const chaiscript::Boxed_Value& bv = func.second;
            _plugin_logprintf("%s, ", func.first.c_str() );
        }
    }
    _plugin_logprintf("\n\n");

    return true;
}

bool chaiLoad(int argc, char* argv[]) {
    std::string fileName = "";
    if(argc < 2) {
        auto qfileName = QFileDialog::getOpenFileName(nullptr,
                 "Open chai File", "", "CHAI Files (*.chai)");

        fileName = qfileName.toStdString();
    } else {
        fileName = argv[1];
    }

    try {
        chai.eval_file(fileName);
        _plugin_logprintf(" >>> [" plugin_name "] Loaded %s \n" , fileName.c_str() );
    }
    catch ( const std::exception &e ) {
        _plugin_logprintf(" >>> Exception thrown: %s \n" , e.what( ) );
    }

    return true;
}

extern "C" DLL_EXPORT bool pluginit(PLUG_INITSTRUCT* initStruct) {
    initStruct->pluginVersion = plugin_version;
    initStruct->sdkVersion = PLUG_SDKVERSION;
    strcpy(initStruct->pluginName, plugin_name);
    pluginHandle = initStruct->pluginHandle;

    if(!_plugin_registercommand(pluginHandle, "chaiEval", chaiEval, false))
        _plugin_logputs("error registering the \"exec\" command!");

   if(!_plugin_registercommand(pluginHandle, "chaiLoad", chaiLoad, false))
       _plugin_logputs("error registering the \"exec\" command!");

   if(!_plugin_registercommand(pluginHandle, "chaiShowEnv", chaiShowEnv, false))
       _plugin_logputs("error registering the \"exec\" command!");

   return true;
}

extern "C" DLL_EXPORT bool plugstop() {
    return true;
}

static void _puts(const std::string& v) {
    _plugin_logprintf("%s", v.c_str());
}

static void print(const std::string& v) {
    _plugin_logputs(v.c_str());    
}

std::vector<unsigned char> _DbgMemRead(duint va, duint size) {
    std::vector<unsigned char> rtn;
    rtn.resize(size);
    if(DbgMemRead(va, &rtn[0], size) == false)
        rtn.clear();
    return rtn;
}

static void registerChaiFunctions() {
    chai.add(chaiscript::bootstrap::standard_library::vector_type<std::vector<uint8_t> >("UCharVector"));
    chai.add(chaiscript::fun(&print), "print");
    chai.add(chaiscript::fun(&_puts), "puts");
    chai.add(chaiscript::fun(&to_hex), "to_hex");
    chai.add(chaiscript::fun(&_DbgMemRead), "DbgMemRead");

#define DBG_FUNCTION(x) chai.add(chaiscript::fun(FunctionWrapper(&x, 0)), #x);
#include "dbgops.h"

}

extern "C" DLL_EXPORT void plugsetup(PLUG_SETUPSTRUCT* setupStruct) {
    hwndDlg = setupStruct->hwndDlg;
    hMenu = setupStruct->hMenu;
    hMenuDisasm = setupStruct->hMenuDisasm;
    hMenuDump = setupStruct->hMenuDump;
    hMenuStack = setupStruct->hMenuStack;

    registerChaiFunctions();
}

extern "C" DLL_EXPORT BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    return TRUE;
}
