#pragma once

#include <windows.h>
#include <chaiscript/chaiscript.hpp>
#include <chaiscript/chaiscript_stdlib.hpp>

#include <QFileDialog>
#include <pluginsdk/_plugins.h>
#include <pluginsdk/_scriptapi.h>
#include <pluginsdk/_scriptapi_debug.h>
#include <pluginsdk/_scriptapi_register.h>
#include <pluginsdk/_scriptapi_stack.h>
#include <pluginsdk/_scriptapi_assembler.h>

#include <functional>
#include <iomanip>

#include <regex>

#ifndef DLL_EXPORT
#define DLL_EXPORT __declspec(dllexport)
#endif //DLL_EXPORT

template <typename T> struct TypeWrapper {
    typedef T f_arg;
    typedef T chai_arg;
    static inline f_arg convert(chai_arg a) { return a; }
};

template <> struct TypeWrapper<void> {
    typedef void f_arg;
    typedef void chai_arg;
    static inline f_arg convert(chai_arg ) { }
};

template <> struct TypeWrapper <const char*> {
    typedef const char* f_arg;
    typedef const std::string& chai_arg;
    static inline f_arg convert(chai_arg a) { return a.c_str(); }
};

template <> struct TypeWrapper <const unsigned char*> {
    typedef const unsigned char* f_arg;
    typedef const std::vector<unsigned char>& chai_arg;
    static inline f_arg convert(chai_arg a) { return &a[0]; }
};

template <> struct TypeWrapper <unsigned char*> { };
template <> struct TypeWrapper <char*> { };

template < typename... args >
static inline std::function< void (typename TypeWrapper<args>::chai_arg...) >
FunctionWrapper( void (*fn)(args...), int __pref  ) {
    return [=] (typename TypeWrapper<args>::chai_arg... in) {
        fn( (TypeWrapper<args>::convert(in))... );
    };
};

template < typename rtn, typename... args >
static inline std::function< typename TypeWrapper<rtn>::chai_arg (typename TypeWrapper<args>::chai_arg...) >
FunctionWrapper( rtn (*fn)(args...), int __pref) {
    return [=] (typename TypeWrapper<args>::chai_arg... in) {
        return TypeWrapper<rtn>::convert( fn( (TypeWrapper<args>::convert(in))... ) );
    };
};
