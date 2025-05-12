//
// Created by flori on 12/05/2025.
//

#ifndef GAME_TYPES_H
#define GAME_TYPES_H

#include <allegro5/allegro.h>

#define SIZEX 80
#define SIZEY 70
#define MAX_ROWS 100
#define MAX_COLS 100
#define FPS 60

#define VITESSE 4.0
#define SAUT -10.0
#define GRAVITE 0.6
#define MAX_VY 12.0

typedef struct {
    float x, y;
    float w, h;
    float vx, vy;
    bool au_sol;
    bool grimpe;
    bool peut_dash;
} Player;

typedef enum {
    DIR_HORIZ,
    DIR_VERT
} Direction;

typedef enum {
    ETAT_MENU,
    ETAT_JEU
} Etat_Jeu;

#endif // GAME_TYPES_H
