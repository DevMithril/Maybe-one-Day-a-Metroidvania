#ifndef TEXT__LIB__H
#define TEXT__LIB__H
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include "my_standard.h"

#define LEN_MAX_TEXT 50

typedef struct Text
{
    SDL_Texture *texture;
    SDL_Rect src_rect, dst_rect;
}Text;

/* permet d'allouer une structure Text, renvoie NULL en cas d'erreur */
Text *create_Text(TTF_Font *font, const char text[], SDL_Color color, SDL_Renderer *renderer);

/* permet d'écrire les données d'un text dans un fichier */
void save_Text(FILE **file, const char *text, const char *font_file, int font_size, int a, int r, int g, int b);

/* permet d'allouer un text à partir de données présentes dans un fichier ; renvoie NULL en cas d'erreur */
Text *load_Text(FILE **file, Text *text, SDL_Renderer *renderer);

/* permet de libèrer la mémoire allouée pour un text */
void destroy_Text(Text **text);

/* permet d'afficher un text */
void display_Text(Text *text, SDL_Renderer *renderer);

/* permet de déplacer un text */
void move_Text(int x, int y, Text *text);

#endif