#ifndef _SNAKE_ASSETMANAGER_H
#define _SLIDDERBOA_ASSETMANAGER_H
#include "slidderboa.h"
#ifdef _WIN32
#include <windows.h>
#include <dirent.h>
#endif

void slidderboa_assetmanager_init(slidderboa_t* game);
char* slidderboa_assetmanager_widetoutf8(wchar_t* wstring);
wchar_t* slidderboa_assetmanager_stringtowide(const char* string); 
SDL_Texture* slidderboa_assetmanager_loadtexture(slidderboa_t* game, const char* asset_path);
void slidderboa_assetmanager_loadassets(slidderboa_t* game);
void slidderboa_assetmanager_addasset(slidderboa_t* game, int asset_type, const char* asset_path);
void slidderboa_assetmanager_destroy(slidderboa_t* game);
#endif