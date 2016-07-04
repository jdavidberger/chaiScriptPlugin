# chaiScriptPlugin
Plugin which enables [chai scripts](https://github.com/ChaiScript/ChaiScript) to run inside of [x64dbg](https://github.com/x64dbg/x64dbg). 

# Why

x64dbg has a basic scripting language, but without most control flow or ability to make functions. The plugin API is very thorough but isn't well suited for rapid prototyping. 

# Basics

The plugin adds three commands: 

- chaiLoad <filename>: If given a filename as an argument, it evals the file. Without a filename specified, it opens a file dialog. 
- chaiEval <statement>: Uses the chai engine to evaluate the given statement. 
- chaiShowEnv <regex>: Shows all the locals / functions currently defined. The regex is optional, and defaults to showing everything. 

The same file can be loaded multiple times; and if there are top level statements then they will be ran. If there are globals set, they will be loaded into the current global scope. 

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

Available functions are:

- DbgMemWrite
- DbgMemRead
- DbgMemGetPageSize
- DbgCmdExec
- DbgCmdExecDirect
- DbgIsValidExpression
- DbgIsDebugging
- DbgIsJumpGoingToExecute
- DbgSetLabelAt
- DbgClearLabelRange
- DbgSetCommentAt
- DbgClearCommentRange
- DbgGetBookmarkAt
- DbgSetBookmarkAt
- DbgClearBookmarkRange
- DbgGetBpxTypeAt
- DbgValFromString
- DbgGetRegDump
- DbgValToString
- DbgMemIsValidReadPtr
- DbgGetFunctionTypeAt
- DbgGetLoopTypeAt
- DbgGetBranchDestination
- DbgScriptLoad
- DbgScriptUnload
- DbgScriptRun
- DbgScriptStep
- DbgScriptBpToggle
- DbgScriptBpGet
- DbgScriptCmdExec
- DbgScriptAbort
- DbgScriptGetLineType
- DbgScriptSetIp
- DbgSymbolEnum
- DbgAssembleAt
- DbgModBaseFromName
- DbgSettingsUpdated
- DbgMenuEntryClicked
- DbgFunctionOverlaps
- DbgFunctionAdd
- DbgFunctionDel
- DbgArgumentOverlaps
- DbgArgumentAdd
- DbgArgumentDel
- DbgLoopOverlaps
- DbgLoopAdd
- DbgLoopDel
- DbgXrefAdd
- DbgXrefDelAll
- DbgGetXrefCountAt
- DbgGetXrefTypeAt
- DbgIsRunLocked
- DbgIsBpDisabled
- DbgSetAutoCommentAt
- DbgClearAutoCommentRange
- DbgSetAutoLabelAt
- DbgClearAutoLabelRange
- DbgSetAutoBookmarkAt
- DbgClearAutoBookmarkRange
- DbgSetAutoFunctionAt
- DbgClearAutoFunctionRange
- DbgWinEvent
- DbgWinEventGlobal
- DbgIsRunning
- DbgGetTimeWastedCounter
- DbgGetArgTypeAt
- DbgReleaseEncodeTypeBuffer
- DbgGetEncodeTypeAt
- DbgGetEncodeSizeAt
- DbgSetEncodeType
- DbgDelEncodeTypeRange
- DbgDelEncodeTypeSegment

