#include <SDL2/SDL_ttf.h>
#include "lib/game_object.h"

/* 
* resolution : 320x240
* ligne de commande pour la compilation : gcc -o New_Game_Object New_Game_Object.c lib/game_object.c lib/hitbox.c lib/texture.c -lm $(sdl2-config --cflags --libs) -l SDL2_ttf
* pour utiliser valgrind (memoire) : 
*   valgrind -s --tool=memcheck --leak-check=yes|no|full|summary --leak-resolution=low|med|high --show-reachable=yes --track-origins=yes ./New_Game_Object
*/

typedef struct Input
{
    SDL_bool key[SDL_NUM_SCANCODES];
    SDL_bool quit;
    SDL_Keycode key_up, key_down, key_left, key_right, key_Validate, key_Summit, key_Delete, key_Help;  /* /;8;7;9;Space;Return;D;H */
    SDL_bool up, down, left, right, Validate, Summit, Delete, Help;
}Input;

typedef struct Point
{
    SDL_Point point;
    struct Point *suivant;
}Point;

typedef struct Cursor
{
    SDL_Texture *texture;
    SDL_Rect target_render_texture;
    int x, y;
}Cursor;

typedef struct Everything
{
    Game_object *game_object;
    Point *list_points;
    Cursor cursor;
    char texture_file[GAME_OBJECT_LEN_TEXTURE_FILE];
    int mode, current_row;
    Input input;
    SDL_Renderer *renderer;
    SDL_Window *window;
}Everything;

void updateEvent(Input *input)
{
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        if(event.type == SDL_QUIT)
            input->quit = SDL_TRUE;
        else if(event.type == SDL_KEYDOWN)
        {
            input->key[event.key.keysym.scancode] = SDL_TRUE;
        }
        else if(event.type == SDL_KEYUP)
            input->key[event.key.keysym.scancode] = SDL_FALSE;
    }
    input->Validate = input->key[SDL_GetScancodeFromKey(input->key_Validate)];
    input->Summit = input->key[SDL_GetScancodeFromKey(input->key_Summit)];
    input->Delete = input->key[SDL_GetScancodeFromKey(input->key_Delete)];
    input->Help = input->key[SDL_GetScancodeFromKey(input->key_Help)];
    input->down = input->key[SDL_GetScancodeFromKey(input->key_down)];
    input->left = input->key[SDL_GetScancodeFromKey(input->key_left)];
    input->right = input->key[SDL_GetScancodeFromKey(input->key_right)];
    input->up = input->key[SDL_GetScancodeFromKey(input->key_up)];
}

void loadKeys(Everything *all)
{
    all->input.key_Validate = SDLK_SPACE;
    all->input.key_Summit = SDLK_RETURN;
    all->input.key_Delete = SDLK_d;
    all->input.key_Help = SDLK_h;
    all->input.key_down = SDLK_KP_8;
    all->input.key_left = SDLK_KP_7;
    all->input.key_right = SDLK_KP_9;
    all->input.key_up = SDLK_KP_DIVIDE;
}

void destroy_Point(Point *point, Everything *all)
{
    Point *liste = all->list_points;
    if (point == NULL)
    {
        printf("Erreur dans destroy_Point : le point passée en paramètre est un pointeur null \n");
        return;
    }
    while (point != liste->suivant)
    {
        liste = liste->suivant;
    }
    Point *next = liste->suivant->suivant;
    free(liste->suivant);
    liste->suivant = next;
}

void destroy_Cursor(Everything *all)
{
    if (all->cursor.texture != NULL)
    {
        SDL_DestroyTexture(all->cursor.texture);
    }
}

void display_Cursor(Everything *all)
{
    if (all->cursor.texture != NULL)
    {
        SDL_RenderCopy(all->renderer, all->cursor.texture, NULL, &all->cursor.target_render_texture);
    }
}

void display_Point(Point *point, Everything *all)
{
    SDL_SetRenderDrawColor(all->renderer, 255, 0, 0, 255);
    SDL_RenderDrawLine(all->renderer, point->point.x - 3, point->point.y, point->point.x + 3, point->point.y);
    SDL_RenderDrawLine(all->renderer, point->point.x, point->point.y - 3, point->point.x, point->point.y + 3);
    SDL_RenderDrawLine(all->renderer, point->point.x - 3, point->point.y - 3, point->point.x + 3, point->point.y + 3);
    SDL_RenderDrawLine(all->renderer, point->point.x + 3, point->point.y - 3, point->point.x - 3, point->point.y + 3);
    SDL_SetRenderDrawColor(all->renderer, 0, 0, 0, 255);
}

void display_Points(Everything *all)
{
    Point *point = all->list_points;
    while (point->suivant != NULL)
    {
        display_Point(point->suivant, all);
        point = point->suivant;
    }
}

void Quit(Everything *all, int status)
{
    /* liberation de la RAM allouee */

    while (all->list_points != NULL)
    {
        destroy_Point(all->list_points, all);
    }
    destroy_Game_object(&all->game_object);

    /* destruction du renderer et de la fenetre, fermeture de la SDL puis sortie du programme */

    if (NULL != all->renderer)
        SDL_DestroyRenderer(all->renderer);
    if (NULL != all->window)
        SDL_DestroyWindow(all->window);
    TTF_Quit();
    SDL_Quit();
    if (status == EXIT_SUCCESS)
        printf("L'exécution du programme s'est bien terminée.\n");
    else
        printf("L'exécution du programme s'est soldée par une erreur.\n");
    exit(status);
}

void add_Point(Everything *all)
{
    Point *liste = all->list_points;
    if (all->input.Validate)
    {
        all->input.Validate = SDL_FALSE;
        while (liste->suivant != NULL)
        {
            liste = liste->suivant;
        }
        liste->suivant = malloc(sizeof(Point));
        if (liste->suivant == NULL)
        {
            fprintf(stderr, "Erreur dans add_point : Out of Memory\n");
            Quit(all, EXIT_FAILURE);
        }
        liste = liste->suivant;
        liste->suivant = NULL;
        liste->point.x = all->cursor.x;
        liste->point.y = all->cursor.y;
    }
}

void del_Point(Everything *all)
{
    Point *liste = all->list_points->suivant;
    if (all->input.Delete)
    {
        all->input.Delete = SDL_FALSE;
        if (liste == NULL)
                return;
        while (liste->point.x != all->cursor.x || liste->point.y != all->cursor.y)
        {
            if (liste->suivant == NULL)
                return;
            liste = liste->suivant;
        }
        destroy_Point(liste, all);
    }
}

void build_Hitbox(Everything *all)
{
    Point *point = all->list_points;
    int nb_points = 0, x_min = 0, x_max = 0, y_min = 0, y_max = 0;
    while (point->suivant != NULL)
    {
        nb_points += 1;
        point = point->suivant;
    }
    if (3 > nb_points)
    {
        point = all->list_points;
        while (point->suivant != NULL)
            destroy_Point(point->suivant, all);
        return;
    }
    point = all->list_points->suivant;
    Hitbox *new_hitbox = malloc(sizeof(Hitbox));
    if (new_hitbox == NULL)
    {
        fprintf(stderr, "Erreur dans build_Hitbox : Out of Memory\n");
        Quit(all, EXIT_FAILURE);
    }
    all->game_object->hitboxes[all->current_row] = new_hitbox;
    new_hitbox->points = malloc(nb_points * sizeof(SDL_Point));
    new_hitbox->nb_points = nb_points;
    if (new_hitbox->points == NULL)
    {
        fprintf(stderr, "Erreur dans build_Hitbox : Out of Memory\n");
        Quit(all, EXIT_FAILURE);
    }
    Point *tmp = NULL;
    for (int i = 0; i < new_hitbox->nb_points; i++)
    {
        new_hitbox->points[i].x = point->point.x;
        new_hitbox->points[i].y = point->point.y;
        if (x_min > point->point.x)
            x_min = point->point.x;
        if (point->point.x > x_max)
            x_max = point->point.x;
        if (y_min > point->point.y)
            y_min = point->point.y;
        if (point->point.y > y_max)
            y_max = point->point.y;
        tmp = point;
        point = point->suivant;
        destroy_Point(tmp, all);
    }
    new_hitbox->cercle_x = x_min + (x_max - x_min) / 2;
    new_hitbox->cercle_y = y_min + (y_max - y_min) / 2;
    new_hitbox->cercle_rayon = (int)sqrt((double)((x_max-x_min) * (x_max-x_min) + (y_max-y_min) * (y_max-y_min))) / 2;
}

void Init(Everything *all)
{
    /* initialisation de la SDL et de la TTF */

    if (0 != SDL_Init(SDL_INIT_VIDEO))
    {
        fprintf(stderr, "Erreur SDL_Init : %s\n", SDL_GetError());
        Quit(all, EXIT_FAILURE);
    }
    if (0 != TTF_Init())
    {
        fprintf(stderr, "Erreur TTF_Init : %s\n", TTF_GetError());
        Quit(all, EXIT_FAILURE);
    }

    /* creation de la fenetre et du renderer associé */

    all->window = SDL_CreateWindow("New Game Object", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              4*320, 4*240, SDL_WINDOW_SHOWN /*| SDL_WINDOW_FULLSCREEN*/);
    if (NULL == all->window)
    {
        fprintf(stderr, "Erreur SDL_CreateWindow : %s\n", SDL_GetError());
        Quit(all, EXIT_FAILURE);
    }
    all->renderer = SDL_CreateRenderer(all->window, -1, SDL_RENDERER_ACCELERATED);
    if (NULL == all->renderer)
    {
        fprintf(stderr, "Erreur SDL_CreateRenderer : %s\n", SDL_GetError());
        Quit(all, EXIT_FAILURE);
    }
    SDL_RenderSetLogicalSize(all->renderer, 320, 240);

    /* chargement et affichage de l'icone de la fenetre */

    SDL_Surface *icone;
    icone = SDL_LoadBMP("icone.bmp");
    if (NULL == icone)
    {
        fprintf(stderr, "Erreur SDL_LoadBMP : %s\n", SDL_GetError());
        Quit(all, EXIT_FAILURE);
    }
    SDL_SetWindowIcon(all->window, icone);
    SDL_FreeSurface(icone);
}

void init_Game_object(Everything *all)
{
    all->game_object = malloc(sizeof(Game_object));
    if (all->game_object == NULL)
    {
        fprintf(stderr, "Erreur dans init_Game_object : Out of Memory\n");
        Quit(all, EXIT_FAILURE);
    }
    all->game_object->nb_col = 0;
    all->game_object->nb_row = 0;
    all->game_object->texture = NULL;
    all->game_object->hitboxes = NULL;
}

void query_texture_path(Everything *all)
{
    char input[GAME_OBJECT_LEN_TEXTURE_FILE];
    scanf(" %s", input);
    strcpy(all->texture_file, input);
}

SDL_bool fill_Game_object(Everything *all)
{
    if (all->game_object->nb_col == 0 || all->game_object->nb_row == 0)
    {
        return SDL_FALSE;
    }
    all->game_object->texture = loadImage(all->texture_file, all->renderer);
    if (all->game_object->texture == NULL)
    {
        return SDL_FALSE;
    }
    all->game_object->hitboxes = malloc(all->game_object->nb_row * sizeof(Hitbox*));
    if (all->game_object->hitboxes == NULL)
    {
        fprintf(stderr, "Erreur dans fill_Game_object : Out of Memory\n");
        Quit(all, EXIT_FAILURE);
    }
    SDL_QueryTexture(all->game_object->texture, NULL, NULL, &all->game_object->src_rect.w, &all->game_object->src_rect.h);
    all->game_object->src_rect.x = 0;
    all->game_object->src_rect.y = 0;
    all->game_object->dst_rect.h = all->game_object->src_rect.h;
    all->game_object->dst_rect.w = all->game_object->src_rect.w;
    all->game_object->dst_rect.x = 0;
    all->game_object->dst_rect.y = 0;
    return SDL_TRUE;
}

void updateMode(Everything *all)
{
    switch (all->mode)
    {   
        case 0 :        /* obtention des données de base */
        {
            if (all->input.Summit)
            {
                all->input.Summit = SDL_FALSE;
                if (fill_Game_object(all))
                {
                    all->mode = 1;
                }
            }
        }
        case 1 :        /* écran d'aide */
        {
            if (all->input.Help)
            {
                all->input.Help = SDL_FALSE;
                all->mode = 2;
            }
            break;
        }
        case 2 :        /* choix de l'animation */
        {
            if (all->input.Help)
            {
                all->input.Help = SDL_FALSE;
                all->mode = 1;
            }
            break;
        }
        case 3 :        /* choix de l'animation avec affichage de la hitbox */
        {
            if (all->input.Help)
            {
                all->input.Help = SDL_FALSE;
                all->mode = 1;
            }
            break;
        }
        case 4 :        /* édition d'une hitbox */
        {
            break;
        }
    }
}

void runGame(Everything *all)
{
    SDL_RenderClear(all->renderer);

    switch (all->mode)
    {
        case 0 :        /* obtention des données de base */
        {
            break;
        }
        case 1 :        /* écran d'aide */
        {
            break;
        }
        case 2 :        /* choix de l'animation */
        {
            animate_Game_object(all->game_object);
            display_Game_object(all->game_object, all->renderer);
            break;
        }
        case 3 :        /* choix de l'animation avec affichage de la hitbox */
        {
            animate_Game_object(all->game_object);
            display_Game_object(all->game_object, all->renderer);
            if (all->game_object->current_hitbox != NULL)
            {
                display_Hitbox(all->game_object->current_hitbox, all->renderer);
            }
            break;
        }
        case 4 :        /* édition d'une hitbox */
        {
            add_Point(all);
            del_Point(all);
            animate_Game_object(all->game_object);
            display_Game_object(all->game_object, all->renderer);
            if (all->game_object->current_hitbox != NULL)
            {
                display_Hitbox(all->game_object->current_hitbox, all->renderer);
            }
            display_Points(all);
            break;
        }
    }
    updateMode(all);

    SDL_RenderPresent(all->renderer);
}

int main(int argc, char *argv[])
{
    /* Création des variables */

    Everything all = {.renderer = NULL, .window = NULL, .game_object = NULL, .list_points = NULL};
    all.input.quit = SDL_FALSE;
    all.mode = 0;
    for (int i = 0; i < SDL_NUM_SCANCODES; i++)
        all.input.key[i] = SDL_FALSE;

    /* Initialisations, création de la fenêtre et du renderer. */

    Init(&all);
    loadKeys(&all);
    init_Game_object(&all);

    /* Boucle principale du jeu */

    while (!all.input.quit)
    {
        updateEvent(&all.input);
        runGame(&all);
        SDL_Delay( (int)(1000 / 60) );
    }

    /* Fermeture du logiciel et libération de la mémoire */

    Quit(&all, EXIT_SUCCESS);
}