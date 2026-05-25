#include "texture.h"

std::unordered_map<std::string, SDL_Texture*> Texture::s_cache;

#ifdef __3DS__
std::unordered_map<std::string, C2D_SpriteSheet> Texture::s_sheetCache;

C2D_SpriteSheet Texture::loadSheet(const char* path)
{
    auto it = s_sheetCache.find(path);
    if (it != s_sheetCache.end()) return it->second;

    C2D_SpriteSheet sheet = C2D_SpriteSheetLoad(path);
    if (!sheet) { printf("Failed to load: %s\n", path); exit(1); }

    s_sheetCache[path] = sheet;
    return sheet;
}

#else
SDL_Texture *Texture::loadTex(SDL_Renderer *ren, const char *path)
{
    auto it = s_cache.find(path);
    if (it != s_cache.end())
        return it->second;

    SDL_Surface* surf = IMG_Load(path);

    if (!surf)
    {
        std::cout << "IMG_Load Failure: " << IMG_GetError() << "\n";
        exit(1);
    }

    SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surf);
    SDL_FreeSurface(surf);

    s_cache[path] = tex;
    return tex;
}

void Texture::clearCache()
{
    for (auto& [path, tex] : s_cache)
        SDL_DestroyTexture(tex);
    s_cache.clear();
}
#endif
