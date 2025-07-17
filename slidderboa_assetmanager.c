#include "slidderboa_assetmanager.h"
const char* slidderboa_assetpaths[] = {"assets/food", "assets/body"};

void slidderboa_assetmanager_init(slidderboa_t* game) {
    memset(game->asset_manager.assets, 0, SLIDDERBOA_ASSETCOUNT * sizeof(slidderboa_asset_t));
}

char* slidderboa_assetmanager_widetoutf8(wchar_t* wstring) {
    size_t len_wstr = 0;
    char* string = NULL;
    #ifdef _WIN32
    len_wstr = WideCharToMultiByte(CP_UTF8, 0, wstring, -1, NULL, 0, NULL, NULL);
    string = calloc(len_wstr+1, sizeof(char));
    WideCharToMultiByte(CP_UTF8, 0, wstring, -1, string, len_wstr, NULL, NULL);
    #else
    len_wstr = wcstombs(NULL, wstring, 0);
    string = calloc(len_wstr+1, sizeof(char));
    wcstombs(string, wstring, len_wstr);
    #endif
    return string;
}

wchar_t* slidderboa_assetmanager_stringtowide(const char* string) {
    if(!string) {
        printf("From mplayer string to wide(): the string give as a parameter is NULL\n");
        return NULL;
    }
    size_t wstr_len = mbstowcs(NULL, string, 0)+1; // get the length of the string in wide characters
    size_t str_len = strlen(string);
    if(wstr_len == -1) {
        printf("mbstowcs(): failed at first\n");
        wstr_len = str_len;
    }
    wchar_t* wstring = calloc(wstr_len+1, sizeof(wchar_t));
    size_t ret = mbstowcs(wstring, string, wstr_len);
    if(ret == -1) {
        printf("mbstowc(): failed again\n");
        wchar_t wc = 0;
        for(size_t i=0;i<wstr_len;i++) {
            if(mbtowc(&wc, &string[i], 1) < 0) {
                printf("Failed to convert to from string to wide as mbtowc() failed\n");
            }
            wstring[i] = wc;
        }
    }
    return wstring;
}

SDL_Texture* slidderboa_assetmanager_loadtexture(slidderboa_t* game, const char* asset_path) {
    SDL_Texture* texture = IMG_LoadTexture(game->renderer, asset_path);
    return texture;
}

void slidderboa_assetmanager_loadassets(slidderboa_t* game) {
    for(size_t asset_type=0;asset_type<SLIDDERBOA_ASSETCOUNT;asset_type++) {
        #ifdef _WIN32
        wchar_t searchdir_pattern[14] = {0},
                *assetpath_widestr = slidderboa_assetmanager_stringtowide(slidderboa_assetpaths[asset_type]);
        wcscpy(searchdir_pattern, assetpath_widestr);
        wcscat(searchdir_pattern, L"\\*");
        free(assetpath_widestr); assetpath_widestr = NULL;
        WIN32_FIND_DATAW find_filedata;
        HANDLE hfind = FindFirstFileW(searchdir_pattern, &find_filedata);
        do {
            char* asset_name = slidderboa_assetmanager_widetoutf8(find_filedata.cFileName);
            if(!strcmp(asset_name, ".") || !strcmp(asset_name, "..")) {
                free(asset_name); asset_name = NULL;
                continue;
            }
            size_t full_assetpath_len = strlen(slidderboa_assetpaths[asset_type]) + strlen(asset_name) + 1;
            char full_assetpath[full_assetpath_len];
            memset(full_assetpath, 0, full_assetpath_len);
            strcpy(full_assetpath, slidderboa_assetpaths[asset_type]);
            strcat(full_assetpath, "\\");
            strcat(full_assetpath, asset_name);
            free(asset_name); asset_name = NULL;
            slidderboa_assetmanager_addasset(game, asset_type, full_assetpath);
        } while(FindNextFileW(hfind, &find_filedata));
        FindClose(hfind);
        #else
        DIR* directory = opendir(slidderboa_assetpaths[asset_type]);
        struct dirent* direntry = NULL;
        while((direntry = readdir(directory))) {
            char* asset_name = direntry->d_name;
            if(!strcmp(asset_name,  ".") || !strcmp(asset_name, "..")) {
                continue;
            }
            size_t full_assetpath_len = strlen(slidderboa_assetpaths[asset_type]) + strlen(asset_name) + 1;
            char full_assetpath[full_assetpath_len];
            memset(full_assetpath, 0, full_assetpath_len);
            strcpy(full_assetpath, slidderboa_assetpaths[asset_type]);
            strcat(full_assetpath, "/");
            strcat(full_assetpath, asset_name);
	    printf("loaded asset path %s\n", full_assetpath);
            slidderboa_assetmanager_addasset(game, asset_type, full_assetpath);
        }
        closedir(directory);
        #endif
    }
}

void slidderboa_assetmanager_addasset(slidderboa_t* game, int asset_type, const char* asset_path) {
    slidderboa_asset_t* asset = &game->asset_manager.assets[asset_type];
    SDL_Texture** new_ptr = NULL;
    if(!asset->textures) {
        new_ptr = malloc(sizeof(SDL_Texture*));
        new_ptr[0] = slidderboa_assetmanager_loadtexture(game, asset_path);
    } else {
        new_ptr = realloc(asset->textures, (asset->texture_count+1) * sizeof(SDL_Texture*));
        new_ptr[asset->texture_count++] = slidderboa_assetmanager_loadtexture(game, asset_path);
    }
    if(!new_ptr) {
        return;
    }
    asset->textures = new_ptr;
}

void slidderboa_assetmanager_destroy(slidderboa_t* game) {
    for(size_t i=0;i<SLIDDERBOA_ASSETCOUNT;i++) {
        for(size_t j=0;j<game->asset_manager.assets[i].texture_count;j++) {
            SDL_DestroyTexture(game->asset_manager.assets[i].textures[j]);
        }
        free(game->asset_manager.assets[i].textures);
        game->asset_manager.assets[i].texture_count = 0;
    }
    slidderboa_assetmanager_init(game);
}
