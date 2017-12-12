#pragma once
#include "CurlHttp.h"

extern int CloudRender(const char* exePath, const char* filename, const char* outputprefix, const char* outputtype, const char* outputpath);


extern __declspec(thread) int g_res_x;
extern __declspec(thread) int g_res_y;
