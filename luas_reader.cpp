/**
 * @file luas_reader.cpp
 * @brief   lua配置读取
 * @author lvxiaojun
 * @version 
 * @Copyright shengjoy.com
 * @date 2012-12-28
 */

#include "luas_reader.h"
#include "luas.h"

inline static int 
_push_value(lua_State* L, const char* table, const char* key) { 
    lua_getglobal(L, table);
    if (!lua_istable(L, -1)) {
        return -1;
    }
    lua_pushstring(L, key);
    lua_gettable(L, -2);
    return 0;
}

int32_t 
luas_read_int32(luas_state* s, const char* table, const char* key, int32_t d) {
    lua_State* L = s->l;
    int r = d;
    if (_push_value(L, table, key) == 0) {
        if (lua_isnumber(L, -1))
            r = (int32_t)lua_tonumber(L, -1);
        lua_pop(L, 2);
    }
    return r;
}

uint32_t 
luas_read_uint32(luas_state* s, const char* table, const char* key, uint32_t d) {
    lua_State* L = s->l;
    int r = d;
    if (_push_value(L, table, key) == 0) {
        if (lua_isnumber(L, -1))
            r = (uint32_t)lua_tonumber(L, -1);
        lua_pop(L, 2);
    }
    return r;
}

float 
luas_read_float(luas_state* s, const char* table, const char* key, float d) {
    lua_State* L = s->l;
    int r = d;
    if (_push_value(L, table, key) == 0) {
        if (lua_isnumber(L, -1))
            r = (float)lua_tonumber(L, -1);
        lua_pop(L, 2);
    }
    return r;
}

const char* 
luas_read_string(luas_state* s, const char* table, const char* key, const char* d) {
    lua_State* L = s->l;
    const char* r = d;
    if (_push_value(L, table, key) == 0) {
        if (lua_isnumber(L, -1)) {
            size_t len;
            r = lua_tolstring(L, -1, &len);
        }
        lua_pop(L, 2);
    }
    return r;
}
