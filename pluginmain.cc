#include "Std.h"
#include <QLibraryInfo>
#include <pluginsdk/bridgemain.h>

chaiscript::ChaiScript chai(chaiscript::Std_Lib::library());

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
        auto bv = chai.eval(argv[1]);
        if(bv.get_type_info().is_arithmetic()) {
            return chai.boxed_cast<int>(bv);
        }
        return true;
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

static void _chaiLoad(const std::string& fileName) {
    try {
        chai.eval_file(fileName);
        _plugin_logprintf(" >>> [" plugin_name "] Loaded %s \n" , fileName.c_str() );
    }
    catch ( const std::exception &e ) {
        _plugin_logprintf(" >>> Exception thrown: %s \n" , e.what( ) );
    }
}

static void _chaiLoad() {
    QString qfileName = QFileDialog::getOpenFileName(nullptr,
             "Open chai File", "", "CHAI Files (*.chai)");

    if(qfileName.size()) {
        std::string fileName = qfileName.toStdString();
        _chaiLoad(fileName);
    }
}

bool chaiLoad(int argc, char* argv[]) {       
    std::string fileName = "";
    if(argc < 2) {
        GuiExecuteOnGuiThread(_chaiLoad);
    } else {
        _chaiLoad(argv[1]);
    }
    return true;
}

static void _puts(const std::string& v) {
    _plugin_logprintf("%s", v.c_str());
}

static void print(const std::string& v) {
    std::string msg = v + "\n";
    _plugin_logputs(msg.c_str());
}

std::vector<unsigned char> _DbgMemRead(duint va, duint size) {
    std::vector<unsigned char> rtn;
    rtn.resize(size);

    if(DbgMemRead(va, &rtn[0], size) == false)
        rtn.clear();
    return rtn;
}

static std::string Sanitize(const std::string& name) {
    return std::regex_replace(name, std::regex("\\::"), "_");
}

static void registerChaiFunctions() {
    chai.add(chaiscript::bootstrap::standard_library::vector_type<std::vector<uint8_t> >("UCharVector"));
    chai.add(chaiscript::fun(&print), "print");
    chai.add(chaiscript::fun(&_puts), "puts");
    chai.add(chaiscript::fun(&to_hex), "to_hex");
    chai.add(chaiscript::fun(&_DbgMemRead), "DbgMemRead");

#define DBG_FUNCTION(x) chai.add(chaiscript::fun(FunctionWrapper(&x, 0)), Sanitize(#x));
#include "dbgops.h"

}

extern "C" DLL_EXPORT bool plugstop() {
    return true;
}

extern "C" DLL_EXPORT bool pluginit(PLUG_INITSTRUCT* initStruct) {
    initStruct->pluginVersion = plugin_version;
    initStruct->sdkVersion = PLUG_SDKVERSION;
    strcpy(initStruct->pluginName, plugin_name);
    pluginHandle = initStruct->pluginHandle;

    _plugin_logprintf("[ChaiScript] Qt version %s\n", QLibraryInfo::build());

    if(!_plugin_registercommand(pluginHandle, "chaiEval", chaiEval, false))
        _plugin_logputs("error registering the \"exec\" command!");

   if(!_plugin_registercommand(pluginHandle, "chaiLoad", chaiLoad, false))
       _plugin_logputs("error registering the \"exec\" command!");

   if(!_plugin_registercommand(pluginHandle, "chaiShowEnv", chaiShowEnv, false))
       _plugin_logputs("error registering the \"exec\" command!");

   return true;
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
