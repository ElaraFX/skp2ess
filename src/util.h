#ifndef S2E_UTIL_H
#define S2E_UTIL_H


#ifdef _WIN32
#include <direct.h>
#include <io.h>
#define ACCESS _access
#define MKDIR(a) _mkdir((a))
#elif _LINUX
#include <stdarg.h>
#include <sys/stat.h>
#define ACCESS access
#define MKDIR(a) mkdir((a),0755)
#endif




#endif