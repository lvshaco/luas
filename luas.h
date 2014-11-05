/**
 * @file luas.h
 * @brief   lua脚本模块
 * @author lvxiaojun
 * @version 
 * @Copyright shengjoy.com
 * @date 2012-09-21
 */
#ifndef __LUAS_H__
#define __LUAS_H__

#include "lua_inc.h"

class Logger;

// luas配置
struct luas_configure {
    Logger* c_logger;
    Logger* lua_logger;
    const char* path;
};

struct luas_state {
    lua_State* l;
    Logger* c_logger;
    int traceback;
};

struct luas_const {
    const char* name;
    int value;
};

// 环境创建
luas_state* luas_create(const luas_configure* conf);

// 环境销毁
void luas_destroy(luas_state* s);

// 加载目录下脚本
int luas_load_dir(luas_state* s, const char* path, bool recursive);

// 加载脚本
int luas_load_file(luas_state* s, const char* file);

// 注册全局模块
int luas_reg_global(luas_state* s, const char* mod, const luaL_Reg* funcs, int func_count);

// 注册模块
int luas_reg_module(luas_state* s, const char* mod, const luaL_Reg* funcs, int func_count);
int luas_expand_module(luas_state* s, const char* mod, const luaL_Reg* funcs, int func_count);

// 注册常量
int luas_reg_const(luas_state* s, const char* mod, const luas_const* consts, int count);

// 压入对象 
int luas_push_obj(lua_State* L, void* obj, const char* mod);

// 调用脚本函数
// eg: luas_call("", "lua_do_someting", "pu:b", "A", a, 123) 
//  传递参数为A* a, unsigned int 123; 返回为bool
// module : lua模块名
// func   : lua函数名
// sig    : args:rets
//          其中args和rets
//          b:bool
//          d:int, 
//          u:unsigned int, 
//          f:float, 
//          s:const char*, 
//          p:obj pointer(需要连续传递对象类名和对象指针)
int luas_call(luas_state* s, const char* mod, const char* func, const char* sig, ...);

int luas_checklightuserdata(luas_state* s, int idx);

#endif // __LUA_SCRIPT_H__

