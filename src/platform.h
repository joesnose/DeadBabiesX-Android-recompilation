#pragma once
#include <cstdint>
#include <cstdio>
#include <sys/stat.h>
#include <SDL.h>
#include <string>
using WORD=uint16_t; using BYTE=uint8_t; using DWORD=uint32_t; using HANDLE=void*;
using ULONGLONG=uint64_t;
union LARGE_INTEGER {long long QuadPart=0;struct{uint32_t LowPart;int32_t HighPart;};};
inline uint64_t GetTickCount64(){return SDL_GetTicks64();}
constexpr unsigned INVALID_FILE_ATTRIBUTES=~0u;
inline unsigned GetFileAttributesA(const char*p){struct stat s;if(!stat(p,&s))return 0;
#ifdef __vita__
 if(p[0]!='/'&&std::string(p).find(':')==std::string::npos&&!stat((std::string("app0:/")+p).c_str(),&s))return 0;
#endif
 return INVALID_FILE_ATTRIBUTES;}
inline bool MoveFileExA(const char*a,const char*b,int){return rename(a,b)==0;}
constexpr int MOVEFILE_REPLACE_EXISTING=1,MOVEFILE_WRITE_THROUGH=2;
