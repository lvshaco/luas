/**
 * @file luas.cpp
 * @brief   lua脚本模块
 * @author lvxiaojun
 * @version 
 * @Copyright shengjoy.com
 * @date 2012-09-21
 */

#include "luas.h"
#include "util.h"
#include "Eagle.h"
#include "Logger.h"


#define LOG_ERROR s->c_logger->error
#define LOG_TRACE s->c_logger->trace

#define LUA_LOGGER s->lua_logger

/*****************************************************************************/
static int _traceback(lua_State* L) {
    const char* msg = lua_tostring(L, 1);
    if (msg)
        luaL_traceback(L, L, msg, 1);
    else
        lua_pushliteral(L, "no error msg");
    return 1;
}

static int _load_file(const char* file, void* state) {
    int r;
    luas_state* s = (luas_state*)state;
    lua_State* L = s->l;
    r = luaL_loadfile(L, file);
    if (r != LUA_OK) {
        LOG_ERROR("lua load file error : %s", lua_tostring(L, -1));
        lua_pop(L, 1);        
        return -1;
    }

    r = lua_pcall(L, 0, 0, s->traceback);
    if (r != LUA_OK) {
        LOG_ERROR("lua call file error : %s", lua_tostring(L, -1));
        lua_pop(L, 1);
        return -1;
    }

    LOG_TRACE("lua load file [%s] succeed!", file);
    return 0;

}

int luas_load_file(luas_state* s, const char* file) {
    return _load_file(file, s);
}

int luas_load_dir(luas_state* s, const char* path, bool recursive) {
    return for_each_file(path, ".lua", recursive, _load_file, s);
}

static int _newmetatable(lua_State* L, const char* mod, const luaL_Reg* funcs, int func_count) {
    luaL_newmetatable(L, mod);
    luaL_setfuncs(L, funcs, 0);
    return 0;
}

int luas_reg_global(luas_state* s, const char* mod, const luaL_Reg* funcs, int func_count) {
    lua_State* L = s->l;
    lua_pushlightuserdata(L, NULL);
    _newmetatable(L, mod, funcs, func_count);
    lua_setmetatable(L, -2);
    lua_pop(L, 1);
    return 0;
}

int luas_reg_module(luas_state* s, const char* mod, const luaL_Reg* funcs, int func_count) {
    lua_State* L = s->l;
    _newmetatable(L, mod, funcs, func_count);
    lua_pop(L, 1);
    return 0;
}

int luas_expand_module(luas_state* s, const char* mod, const luaL_Reg* funcs, int func_count) {
    lua_State* L = s->l;
    luaL_getmetatable(L, mod);
    luaL_setfuncs(L, funcs, 0);
    lua_pop(L, 1);
    return 0;
}

int luas_reg_const(luas_state* s, const char* mod, const luas_const* consts, int count) {
    lua_State* L = s->l;
    if (mod == NULL || mod[0] == '\0')
        lua_pushglobaltable(L);
    else
        luaL_newmetatable(L, mod);
    for (int i=0; i<count; ++i) {
        lua_pushnumber(L, consts[i].value);
        lua_setfield(L, -2, consts[i].name);
    }
    lua_pop(L, 1);
    return 0;
}

int luas_push_obj(lua_State* L, void* obj, const char* mod) {
    void* udata = lua_newuserdata(L, sizeof(void*));
    *(void**)udata = obj;
    
    lua_createtable(L, 0, 2);
    lua_pushliteral(L, "__index");
    luaL_getmetatable(L, mod);
    lua_settable(L, -3);
    lua_setmetatable(L, -2);
    return 0;
}

static int _get_function(lua_State* L, const char* mod, const char* func) {
    if (mod[0] == '\0') {
        lua_getglobal(L, func);
    } else {
        lua_getglobal(L, mod);
        if (!lua_istable(L, -1)) {
            lua_pop(L, 1);
            return -1;
        }
        lua_pushstring(L, func);
        lua_gettable(L, -2);
        lua_replace(L, -2);
    }
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        return -1;
    }
    return 0;
}

int luas_call(luas_state* s, const char* mod, const char* func, const char* sig, ...) {
    lua_State* L = s->l;
    if (_get_function(L, mod, func) != 0) {
        LOG_ERROR("lua call function [%s.%s] error : no function", mod, func);
        return -1;
    }
    
    int top = lua_gettop(L);
    int res;
    int residx;
    int r;

    va_list ap;
    va_start(ap, sig);
    int args = 0;
    
    while(*sig) {
        switch(*sig++) {
        case 'b':
            lua_pushboolean(L, va_arg(ap, int));
            break;
        case 'd':
            lua_pushnumber(L, va_arg(ap, int));
            break;
        case 'u':
            lua_pushnumber(L, va_arg(ap, unsigned int));
            break;
        case 'f':
            lua_pushnumber(L, va_arg(ap, double));
            break;
        case 's':
            lua_pushstring(L, va_arg(ap, const char*));
            break;
        case 'P':
            lua_pushlightuserdata(L, va_arg(ap, void*));
            break;
        case 'p':
            luas_push_obj(L, va_arg(ap, void*), va_arg(ap, const char*));
            break;
        case ':':
            goto call;
        default:
            goto err_exit;
        }
        ++args;
    }

call:
    res = strlen(sig);
    residx = -res;
    r = lua_pcall(L, args, res, s->traceback);
    if (r != LUA_OK) {
        LOG_ERROR("lua call function [%s.%s] error : %s", mod, func, lua_tostring(L, -1));
        goto err_exit;
    }

    while (*sig) {
        switch (*sig++) {
        case 'b':
            if (!lua_isboolean(L, residx))
                goto err_res;
            *va_arg(ap, int*) = (int)lua_toboolean(L, residx);
            break;
        case 'd':
            if (!lua_isnumber(L, residx))
                goto err_res;
            *va_arg(ap, int*) = (int)lua_tointeger(L, residx);
            break;
        case 'u':
            if (!lua_isnumber(L, residx))
                goto err_res;
            *va_arg(ap, unsigned int*) = (unsigned int)lua_tounsigned(L, residx);
            break;
        case 'f':
            if (!lua_isnumber(L, residx))
                goto err_res;
            *va_arg(ap, double*) = (double)lua_tonumber(L, residx);
            break;
        case 's':
            if (!lua_isstring(L, residx))
                goto err_res;
            *va_arg(ap, const char**) = lua_tostring(L, residx);
            break;
        default:
            goto err_exit;
        }
        ++residx;
    }
    return 0;

err_res:
    LOG_ERROR("lua call function [%s.%s] error : return need '%c'#%d", 
            mod, func, *(sig-1), res + residx + 1);
err_exit:
    lua_settop(L, top);
    return -1;
}

/*****************************************************************************/
enum LUA_LOG_LEVEL {
    LUA_LOG_DEBUG = 1,
    LUA_LOG_TRACE = 2,
    LUA_LOG_ERROR = 3,
};

static int _log(lua_State* L, int log_level, int begin_arg) {
    Logger* logger;
    char buf[1024] = {0};
    char* p = buf;
    int n;
    int size = sizeof(buf);
    int nargs = lua_gettop(L);
    for (int i = begin_arg; i <= nargs; ++i) {
        if (lua_type(L, i) == LUA_TNUMBER) {
            n = snprintf(p, size, LUA_NUMBER_FMT, lua_tonumber(L, i));
        } else {
            size_t l;
            const char* s = luaL_tolstring(L, i, &l);
            n = snprintf(p, size, "%s", s);
        }
        if (n < 0 || n >= size)
            break;
        p += n;
        size -= n;
    }
    buf[sizeof(buf)-1] = '\0';

    lua_getfield(L, LUA_REGISTRYINDEX, "lua_logger");
    if (!lua_islightuserdata(L, -1))
        luaL_error(L, "lua log error : lua logger is not a lightuserdata");
    logger = (Logger*)lua_touserdata(L, -1);
    switch(log_level) {
    case LUA_LOG_ERROR:
        logger->error(buf);
        break;
    case LUA_LOG_TRACE:
        logger->trace(buf);
        break;
    default:
        logger->debug(buf);
        break;
    }
    lua_pop(L, 1);
    return 0;
}

static int _luas_log(lua_State* L) {
    if (lua_gettop(L) < 2) {
        luaL_error(L, "no enough arguments (2 at least)");
        return 0;
    }
    return _log(L, luaL_checknumber(L, 1), 2);
}

static int _luas_print(lua_State* L) {
    if (lua_gettop(L) < 1) {
        luaL_error(L, "no enough argument (1 at least)");
        return 0;
    }
    return _log(L, LUA_LOG_DEBUG, 1);
}

static int _luas_reg_log(lua_State* L) {
    lua_pushglobaltable(L);
    lua_pushnumber(L, LUA_LOG_ERROR);
    lua_setfield(L, -2, "LUAS_ERROR");
    lua_pushnumber(L, LUA_LOG_TRACE);
    lua_setfield(L, -2, "LUAS_TRACE");
    lua_pushnumber(L, LUA_LOG_DEBUG);
    lua_setfield(L, -2, "LUAS_DEBUG");

    lua_pushcfunction(L, _luas_log);
    lua_setfield(L, -2, "LUAS_LOG");
    lua_pushcfunction(L, _luas_print);
    lua_setfield(L, -2, "print");
    lua_pop(L, 1);

    lua_getglobal(L, "io");
    lua_pushliteral(L, "write");
    lua_pushcfunction(L, _luas_print);
    lua_settable(L, -3);
    lua_pop(L, 1);
    return 0;
}

static void _add_path(lua_State* L, const char* path, const char* pathfield) {
    lua_getglobal(L, "package");
    lua_getfield(L, -1, pathfield);
    
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    luaL_addlstring(&b, path, strlen(path));
    luaL_addchar(&b, ';');
    luaL_addvalue(&b);
    luaL_pushresult(&b);
    lua_setfield(L, -2, pathfield);
    lua_pop(L, 1);
}

/*****************************************************************************/
luas_state* luas_create(const luas_configure* conf) {
    luas_state* s;
    lua_State* L = luaL_newstate();
    lua_gc(L, LUA_GCSTOP, 0);
    luaL_openlibs(L);
    _luas_reg_log(L);
    lua_pushlightuserdata(L, conf->lua_logger);
    lua_setfield(L, LUA_REGISTRYINDEX, "lua_logger");
    lua_gc(L, LUA_GCRESTART, 0);
    lua_pushcfunction(L, _traceback);
    char path[1024] = {0};
    strcat(path, conf->path);
    strcat(path, "/?.lua");
    _add_path(L, path, "path");

    s = (luas_state*)malloc(sizeof(luas_state));
    s->l = L;
    s->traceback = lua_gettop(L);
    s->c_logger = conf->c_logger;
    return s;
}

void luas_destroy(luas_state* s) {
    if (s != NULL) {
        if (s->l)
            lua_close(s->l);
        free(s);
    }
}

/****************************************************************************/
int luas_checklightuserdata(luas_state* s, int idx) {
    lua_State* L = s->l;
    if (!lua_islightuserdata(L, idx)) {
        luaL_error(L, "luas error : lightuserdata expected");
        return -1;
    }
    return 0;
}

