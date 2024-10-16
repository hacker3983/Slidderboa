#ifndef _SLIDDERBOA_ASSETMANAGER_TYPES_H
#define _SLIDDERBOA_ASSETMANAGER_TYPES_H
#include <SDL2/SDL.h>
#define SLIDDERBOA_ASSETCOUNT 2
enum slidderboa_asset_types {
    SLIDDERBOA_ASSET_FOOD,
    SLIDDERBOA_ASSET_BODY
};

typedef struct slidderboa_asset {
    SDL_Texture** textures;
    size_t texture_count;
} slidderboa_asset_t;

typedef struct slidderboa_assetmanager {
    slidderboa_asset_t assets[SLIDDERBOA_ASSETCOUNT];
} slidderboa_assetmanager_t;
#endif