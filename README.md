# chaiScriptPlugin
Plugin which enables [chai scripts](https://github.com/ChaiScript/ChaiScript) to run inside of [x64dbg](https://github.com/x64dbg/x64dbg). 

# Why

x64dbg has a basic scripting language, but without most control flow or ability to make functions. The plugin API is very thorough but isn't well suited for rapid prototyping. 

# Basics

The plugin adds three commands: 

- chaiLoad <filename>: If given a filename as an argument, it evals the file. Without a filename specified, it opens a file dialog. After the file is picked, any changes to that file reload the script. 
- chaiEval <statement>: Uses the chai engine to evaluate the given statement. 
- chaiShowEnv <regex>: Shows all the locals / functions currently defined. The regex is optional, and defaults to showing everything. 
- chaiClearWorkspace: Clears the existing workspace. No longer watchs any loaded files, and the save state for the debugee is cleared.
- chaiRegisterCommand: Registers a chai function as a top level command in the debugger, and possibly a function available for use in expressions if the functions arguments are all numeric. 

Given a file: 

~~~~
global hello_world = fun() {
    print("hello world!"); 
};
hello_world(); 
~~~~

loading it with 'chaiLoad' will print "hello world!". It also has loaded that function into scope, so chaiEval hello_world() will print the message again. 

# Debugger Interaction

Many functions available for plugins to the debugger are exposed to chaiscript. For instance: 

~~~~
    var thisPtr = DbgValFromString("ecx");
    var firstArg = DbgValFromString("[esp + 4]"); 
    var mem = DbgMemRead(thisPtr, 16);
    
    for(var i = 0;i < v.size();++i) {
        puts("${to_hex(v[i], 2)}, "); 
    }
    print(" ${to_hex(thisPtr, 8)}: firstArg ${firstArg} ");       
~~~~

might be useful at the top of a thiscall invocation as it prints the first argument given, as well as the first 16 bytes of what ecx points to. 

# Workspace 

When you detach from a program, any files you had loaded are saved along with things like breakpoints. All commands specified with chaiRegisterCommand are also saved. When you re-attach, the plugin will load in those settings to return the chai engine to basically the same state as it was left in. 

# Building

The easiest way to build is to check out the repo into the x64dbg directory; alongside the pluginsdk and release folder. You'll need to install QT 5.6 with QT creator. Open up the project folder in QT creator and the project should build. If you checked it out somewhere besides in the x64dbg directory; in QT creator go into build settings and add an entry in the CMake configuration called 'X64DBGFOLDER', setting it to the x64dbg path. 

If you are interested in running it against a source-built version of x64dbg, point that setting at the root directory for the x64dbg repo and be sure to run 'release.bat' at least once. This sets up the correct directory structure for the necessary headers. 

Either way, your build configuration (Debug vs Release vs Profile) MUST match your build configuration for x64dbg. If you are using a released version of x64dbg, that is Release. If you need debug for some reason, you'll need to build x64dbg from source. 
