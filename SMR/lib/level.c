#include "level.h"

SDL_bool collision_Level(Hitbox *hitbox, Level *level)
{
    for (int i = 0; i < level->nb_hitboxes; i++)
    {
        if (sat(hitbox, &level->hitboxes[i]))
        {
            return SDL_TRUE;
        }
    }
    return SDL_FALSE;
}

SDL_Texture **load_Tilemaps(SDL_Renderer *renderer)
{
    SDL_Texture **tilemaps = malloc(sizeof(SDL_Texture*) * NB_TILEMAPS);
    if (tilemaps == NULL && NB_TILEMAPS != 0)
    {
        fprintf(stderr, "load_Tilemaps : Out of Memory\n");
        return NULL;
    }
    tilemaps[0] = loadImage("assets/tilemaps/grass.bmp", renderer);
    return tilemaps;
}

void destroy_Tilemaps(SDL_Texture **tilemaps)
{
    for (int i = 0; i < NB_TILEMAPS; i++)
    {
        SDL_DestroyTexture(tilemaps[i]);
    }
    free(tilemaps);
}

void load_Level(int level_id, Level *level, SDL_Renderer *renderer)
{
    FILE *file = fopen("assets/levels/platform.dat", "rb");
    int nb_tile;
    Tile *tiles;
    SDL_Texture **tilemaps;
    fread(level, sizeof(Level), 1, file);
    
    // lecture des tiles et construction de la texture

    fread(&nb_tile, sizeof(int), 1, file);
    tiles = malloc(sizeof(Tile) * nb_tile);
    if (tiles == NULL && nb_tile != 0)
    {
        fprintf(stderr, "load_Level : Out of Memory\n");
        return;
    }
    fread(tiles, sizeof(Tile), nb_tile, file);
    level->texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, level->size_x, level->size_y);
    if(NULL == level->texture)
    {
        fprintf(stderr, "Erreur SDL_CreateTexture : %s\n", SDL_GetError());
        free(tiles);
        return;
    }
    SDL_SetTextureBlendMode(level->texture, SDL_BLENDMODE_BLEND);
    tilemaps = load_Tilemaps(renderer);
    SDL_SetRenderTarget(renderer, level->texture);
    for (int i = 0; i < nb_tile; i++)
    {
        SDL_RenderCopy(renderer, tilemaps[tiles[i].tilemap_id], &tiles[i].src, &tiles[i].dst);
    }
    destroy_Tilemaps(tilemaps);
    SDL_SetRenderTarget(renderer, NULL);
    free(tiles);

    // lecture des hitboxes

    level->hitboxes = NULL;
    level->hitboxes = malloc(sizeof(Hitbox) * level->nb_hitboxes);
    if (level->hitboxes == NULL && level->nb_hitboxes != 0)
    {
        fprintf(stderr, "load_Level : Out of Memory\n");
        return;
    }
    fread(level->hitboxes, sizeof(Hitbox), level->nb_hitboxes, file);
    fclose(file);
}

void destroy_Level(Level *level)
{
    if (level->texture != NULL)
    {
        SDL_DestroyTexture(level->texture);
    }
    if (level->hitboxes != NULL)
    {
        free(level->hitboxes);
    }
}

void move_cam_Level(int x, int y, SDL_bool *bool_x, SDL_bool *bool_y, Level *level)
{
    level->src.x += x;
    level->src.y += y;
    *bool_x = SDL_TRUE;
    *bool_y = SDL_TRUE;
    if (level->src.x < 0)
    {
        level->src.x = 0;
        *bool_x = SDL_FALSE;
    }
    if (level->src.y < 0)
    {
        level->src.y = 0;
        *bool_y = SDL_FALSE;
    }
    if (level->src.x > level->size_x - level->src.w)
    {
        level->src.x = level->size_x - level->src.w;
        *bool_x = SDL_FALSE;
    }
    if (level->src.y > level->size_y - level->src.h)
    {
        level->src.y = level->size_y - level->src.h;
        *bool_y = SDL_FALSE;
    }
}

void display_Level(Level *level, SDL_Renderer *renderer)
{
    if (level->texture != NULL)
    {
        SDL_RenderCopy(renderer, level->texture, &level->src, NULL);
    }
}