#include "Std.h"
#include <QLibraryInfo>
#include <QFileSystemWatcher>

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

QFileSystemWatcher* fileWatcher = 0;

std::string to_hex(int d, size_t digits = 2) {
    std::stringstream ss;
    ss << std::hex <<
          std::setfill('0') <<
          std::uppercase <<
          std::setw(digits) << d;
    return ss.str();
}

static std::set<std::string> commands;
static std::set<std::string> workspace_set;
static std::vector<std::string> workspace;

std::thread chaiAsyncThread;
duint chaiEvalDirect(const char* cmd) {
    try {
        auto bv = chai.eval(cmd);
        if(bv.get_type_info().is_arithmetic()) {            
            return chai.boxed_cast<duint>(bv);
        }
        if(bv.get_type_info().name() == "bool") {
            return chai.boxed_cast<bool>(bv);
        }
        if(bv.get_type_info().name() == "void") {
            return 0;
        }
        _plugin_logprintf(" >>> bv type is: %s \n" , bv.get_type_info().name().c_str() );
        return 0;
    }
    catch ( const std::exception &e ) {
        _plugin_logprintf(" >>> Exception thrown: %s \n" , e.what( ) );
    }

    return 0;
}

duint chaiEvalDirect(const char* cmd, bool async) {
    if(async) {
        if(chaiAsyncThread.joinable())
            chaiAsyncThread.join();
        chaiAsyncThread = std::thread([] (std::string cmd) {
            chaiEvalDirect(cmd.c_str());
        }, std::string(cmd));
        return 0;
    } else {
        return chaiEvalDirect(cmd);
    }
}

bool chaiEvalCommand(int argc, char* argv[]) {
    if(argc == 0)
        return false;
	std::string cmdName = argv[0];    
    auto pos = cmdName.find_first_of(" \t");
    if(pos != std::string::npos)
        cmdName = cmdName.substr(0, pos);

    std::stringstream cmd;
    cmd << cmdName << "(";
    for(int i = 1;i < argc;i++) {
        if(i > 1)
            cmd << ", ";
        cmd << argv[i];
    }
    cmd << ")";
    return chaiEvalDirect(cmd.str().c_str());
}

bool chaiEval(int argc, char* argv[]) {
    if(argc < 2)
        return false;

    return chaiEvalDirect(argv[1]);
}

bool chaiEvalAsync(int argc, char* argv[]) {
    if(argc < 2)
        return false;

    return chaiEvalDirect(argv[1], true);
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

    _plugin_logprintf("\nWorkspace: \n");
    for(const auto &script : workspace) {
        if(std::regex_search(script, reg)) {
            _plugin_logprintf("\t%s\n", script.c_str() );
        }
    }

    _plugin_logprintf("\nCommands: \n");
    for(const auto &command : commands) {
        if(std::regex_search(command, reg)) {
            _plugin_logprintf("\t%s\n", command.c_str() );
        }
    }
    _plugin_logprintf("\n\n");

    return true;
}

static void _chaiLoad(const std::string& fileName, bool reload = false) {
    try {
        chai.eval_file(fileName);
        if(workspace_set.find(fileName) == workspace_set.end()) {
            workspace.push_back(fileName);
            workspace_set.insert(fileName);
            if(fileWatcher) {
                QString qtFileName(fileName.c_str());
                fileWatcher->addPath(qtFileName);
            }
        }
        if(reload) {
            _plugin_logprintf(" >>> [" plugin_name "] Reloaded %s on modifcation\n" , fileName.c_str() );
        } else {
            _plugin_logprintf(" >>> [" plugin_name "] Loaded %s \n" , fileName.c_str() );
        }
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
bool chaiClearWorkspace(int argc, char* argv[]) {
    for(auto script : workspace) {
        QString path(script.c_str());
        if(fileWatcher)
            fileWatcher->removePath(path);
    }
    commands.clear();
    workspace.clear();
    workspace_set.clear();
    return true;
}

duint chaiExpr(int argc, const duint* argv, void * usrdata) {
    std::string& cmdName = *(std::string*)usrdata;
    std::stringstream cmd;
    cmd << cmdName << "(";
    for(int i = 0;i < argc;i++) {
        if(i != 0)
            cmd << ", ";
        cmd << argv[i];
    }
    cmd << ")";
    return chaiEvalDirect(cmd.str().c_str());
}

bool chaiRegisterCommand(const char* cmd) {
    commands.insert(cmd);

    const auto funcs = chai.get_state().engine_state.m_boxed_functions;
    try {
        const chaiscript::Boxed_Value& c = chai.eval(std::string(cmd) + ".get_arity()");
        int arity = chaiscript::boxed_cast<int>(c);
        _plugin_registerexprfunction(pluginHandle, cmd, arity, chaiExpr, new std::string(cmd));
    }
    catch ( const std::exception &e ) {
        _plugin_logprintf(" >>> Exception thrown: %s \n" , e.what( ) );
    }
    return _plugin_registercommand(pluginHandle, cmd, chaiEvalCommand, false);
}

bool chaiRegisterCommand(int argc, char* argv[]) {
    if(argc < 2) {
        return false;
    }
    return chaiRegisterCommand(argv[1]);
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

    chai.add(chaiscript::type_conversion<int, duint>());
    chai.add(chaiscript::type_conversion<unsigned int, duint>());
}

static void saveCache(CBTYPE cbType, void* callbackInfo) {
    PLUG_CB_LOADSAVEDB* loadSaveInfo = (PLUG_CB_LOADSAVEDB*)callbackInfo;

    json_t* chaiScriptRoot = json_object();

    if(!workspace.empty()) {
        json_t* loadedScripts = json_array();
        for(auto script : workspace) {
            json_array_append_new(loadedScripts, json_string(script.c_str()));
        }
        json_object_set(chaiScriptRoot, "workspace", loadedScripts);
    }

    if(!commands.empty()) {
        json_t* definedCommands = json_array();
        for(auto command : commands) {
            json_array_append_new(definedCommands, json_string(command.c_str()));
        }
        json_object_set(chaiScriptRoot, "commands", definedCommands);
    }

    json_object_set(loadSaveInfo->root, "chaiScript", chaiScriptRoot);
}

static void loadCache(CBTYPE cbType, void* callbackInfo) {
    PLUG_CB_LOADSAVEDB* loadSaveInfo = (PLUG_CB_LOADSAVEDB*)callbackInfo;

    json_t* chaiScriptRoot = json_object_get(loadSaveInfo->root, "chaiScript");
    if(chaiScriptRoot == 0)
        return;

    json_t* loadedScripts = json_object_get(chaiScriptRoot, "workspace");

    for(size_t i = 0;i < json_array_size(loadedScripts);i++) {
        _chaiLoad( json_string_value( json_array_get(loadedScripts, i) ) );
    }

    json_t* definedCommands = json_object_get(chaiScriptRoot, "commands");

    for(size_t i = 0;i < json_array_size(definedCommands);i++) {
		chaiRegisterCommand(json_string_value(json_array_get(definedCommands, i)));
    }
}


extern "C" DLL_EXPORT bool pluginit(PLUG_INITSTRUCT* initStruct) {
    initStruct->pluginVersion = plugin_version;
    initStruct->sdkVersion = PLUG_SDKVERSION;
    strncpy_s(initStruct->pluginName, sizeof(initStruct->pluginName), plugin_name, sizeof(initStruct->pluginName));
    pluginHandle = initStruct->pluginHandle;

    _plugin_logprintf("[ChaiScript] Qt version %s\n", QLibraryInfo::build());

    if(!_plugin_registercommand(pluginHandle, "chaiEval", chaiEval, false))
        _plugin_logputs("error registering the \"chaiEval\" command!");

    if(!_plugin_registercommand(pluginHandle, "chaiEvalAsync", chaiEvalAsync, false))
        _plugin_logputs("error registering the \"chaiEvalAsync\" command!");

   if(!_plugin_registercommand(pluginHandle, "chaiLoad", chaiLoad, false))
       _plugin_logputs("error registering the \"chaiLoad\" command!");

   if(!_plugin_registercommand(pluginHandle, "chaiShowEnv", chaiShowEnv, false))
       _plugin_logputs("error registering the \"chaiShowEnv\" command!");

   if(!_plugin_registercommand(pluginHandle, "chaiClearWorkspace", chaiClearWorkspace, false))
       _plugin_logputs("error registering the \"chaiClearWorkspace\" command!");

   if(!_plugin_registercommand(pluginHandle, "chaiRegisterCommand", chaiRegisterCommand, false))
       _plugin_logputs("error registering the \"chaiRegisterCommand\" command!");

   fileWatcher = new QFileSystemWatcher();
   QObject::connect(fileWatcher, &QFileSystemWatcher::fileChanged, [] (const QString& path) {
       _chaiLoad(path.toStdString(), true);
   });

   _plugin_registercallback(pluginHandle, CBTYPE::CB_SAVEDB, saveCache);
   _plugin_registercallback(pluginHandle, CBTYPE::CB_LOADDB, loadCache);
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

extern "C" DLL_EXPORT bool plugstop() {
    delete fileWatcher;
    fileWatcher = 0;
    if(chaiAsyncThread.joinable())
        chaiAsyncThread.join();
    return true;
}

extern "C" DLL_EXPORT BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    return TRUE;
}
