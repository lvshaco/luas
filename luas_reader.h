/**
 * @file luas_reader.h
 * @brief   lua配置读取
 * @author lvxiaojun
 * @version 
 * @Copyright shengjoy.com
 * @date 2012-12-28
 */

#include <stdint.h>

struct luas_state;

int32_t luas_read_int32(luas_state* s, const char* table, const char* key, int32_t d);
uint32_t luas_read_uint32(luas_state* s, const char* table, const char* key, uint32_t d);
float luas_read_float(luas_state* s, const char* table, const char* key, float d);
const char* luas_read_string(luas_state* s, const char* table, const char* key, const char* d);
