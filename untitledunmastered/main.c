#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


#define SIZEX 80
#define SIZEY 70
#define MAX_ROWS 100
#define MAX_COLS 100
#define FPS 60

#define VITESSE 4.0
#define SAUT -10.0
#define GRAVITE 0.6
#define MAX_VY 12.0

typedef enum {
    MENU_START,
    MENU_OPTIONS,
    MENU_QUIT,
    GAME_RUNNING,
    GAME_PAUSED
} GameState;

int map[MAX_ROWS][MAX_COLS];
int rows = 0, cols = 0;

ALLEGRO_BITMAP *img_bloc1 = NULL;
ALLEGRO_BITMAP *img_bloc2 = NULL;
ALLEGRO_BITMAP *img_bloc3 = NULL;
ALLEGRO_BITMAP *img_bloc4 = NULL;
ALLEGRO_BITMAP *img_bloc5 = NULL;
ALLEGRO_BITMAP *img_bloc6 = NULL;
ALLEGRO_BITMAP *img_bloc7 = NULL;
ALLEGRO_BITMAP *background = NULL;
ALLEGRO_FONT *menu_font = NULL;

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

bool collision(float x, float y, float w, float h, Direction dir) {
    int left = x / SIZEX;
    int right = (x + w - 1) / SIZEX;
    int top = y / SIZEY;
    int bottom = (y + h - 1) / SIZEY;

    for (int i = top; i <= bottom; i++) {
        for (int j = left; j <= right; j++) {
            if (i < 0 || j < 0 || i >= rows || j >= cols) return true;
            int val = map[i][j];
            if (val == 1) return true;
        }
    }
    return false;
}

void lire_matrice(const char *filename, int *spawn_x, int *spawn_y) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Erreur ouverture du fichier %s\n", filename);
        exit(1);
    }
    int val;
    char line[1024];
    rows = 0; cols = 0;
    while (fgets(line, sizeof(line), file)) {
        int current_col = 0;
        char *ptr = line;
        while (sscanf(ptr, "%d", &val) == 1) {
            map[rows][current_col] = val;
            if (val == 9) { *spawn_x = current_col; *spawn_y = rows; }
            current_col++;
            while (*ptr != ' ' && *ptr != '\0') ptr++;
            while (*ptr == ' ') ptr++;
        }
        if (current_col > cols) cols = current_col;
        rows++;
    }
    fclose(file);
}

void dessiner_matrice() {
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            int val = map[y][x];
            if (val == 0 || val == 9) continue;

            if (val == 1) {
                al_draw_scaled_bitmap(img_bloc1, 0, 0,
                    al_get_bitmap_width(img_bloc1), al_get_bitmap_height(img_bloc1),
                    x * SIZEX, y * SIZEY, SIZEX, SIZEY, 0);
            } else if (val == 2) {
                al_draw_scaled_bitmap(img_bloc2, 0, 0,
                    al_get_bitmap_width(img_bloc2), al_get_bitmap_height(img_bloc2),
                    x * SIZEX, y * SIZEY, SIZEX, SIZEY, 0);
            } else if (val == 3) {
                al_draw_scaled_bitmap(img_bloc3, 0, 0,
                    al_get_bitmap_width(img_bloc3), al_get_bitmap_height(img_bloc3),
                    x * SIZEX, y * SIZEY, SIZEX, SIZEY, 0);
            } else if (val == 4) {
                if (img_bloc4) {
                    al_draw_scaled_bitmap(img_bloc4, 0, 0,
                        al_get_bitmap_width(img_bloc4), al_get_bitmap_height(img_bloc4),
                        x * SIZEX, y * SIZEY, SIZEX, SIZEY, 0);
                } else {
                    al_draw_filled_rectangle(x * SIZEX, y * SIZEY,
                        (x + 1) * SIZEX, (y + 1) * SIZEY,
                        al_map_rgb(0, 0, 255));
                }
            } else if (val == 5) {
                if (img_bloc5) {
                    al_draw_scaled_bitmap(img_bloc5, 0, 0,
                        al_get_bitmap_width(img_bloc5), al_get_bitmap_height(img_bloc5),
                        x * SIZEX, y * SIZEY, SIZEX, SIZEY, 0);
                } else {
                    al_draw_filled_rectangle(x * SIZEX, y * SIZEY,
                        (x + 1) * SIZEX, (y + 1) * SIZEY,
                        al_map_rgb(0, 255, 0));
                }
            } else if (val == 6) {
                if (img_bloc6) {
                    al_draw_scaled_bitmap(img_bloc6, 0, 0,
                        al_get_bitmap_width(img_bloc6), al_get_bitmap_height(img_bloc6),
                        x * SIZEX, y * SIZEY, SIZEX, SIZEY, 0);
                } else {
                    al_draw_filled_rectangle(x * SIZEX, y * SIZEY,
                        (x + 1) * SIZEX, (y + 1) * SIZEY,
                        al_map_rgb(255, 0, 255));
                }
            } else if (val == 7) {
                if (img_bloc7) {
                    al_draw_scaled_bitmap(img_bloc7, 0, 0,
                        al_get_bitmap_width(img_bloc7), al_get_bitmap_height(img_bloc7),
                        x * SIZEX, y * SIZEY, SIZEX, SIZEY, 0);
                } else {
                    al_draw_filled_rectangle(x * SIZEX, y * SIZEY,
                        (x + 1) * SIZEX, (y + 1) * SIZEY,
                        al_map_rgb(255, 255, 0));
                }
            } else {
                al_draw_filled_rectangle(x * SIZEX, y * SIZEY,
                    (x + 1) * SIZEX, (y + 1) * SIZEY,
                    al_map_rgb(255, 0, 255));
            }
        }
    }
}

void dessiner_menu(int screen_w, int screen_h, GameState current_selection) {

    al_draw_scaled_bitmap(background, 0, 0,
        al_get_bitmap_width(background), al_get_bitmap_height(background),
        0, 0, screen_w, screen_h, 0);


    ALLEGRO_COLOR titre_couleur = al_map_rgb(255, 255, 255);
    al_draw_text(menu_font, titre_couleur,
        screen_w / 2, screen_h / 4,
        ALLEGRO_ALIGN_CENTER, "E'CELESTE");


    ALLEGRO_COLOR couleur_start = current_selection == MENU_START ?
        al_map_rgb(255, 255, 0) : al_map_rgb(255, 255, 255);
    ALLEGRO_COLOR couleur_options = current_selection == MENU_OPTIONS ?
        al_map_rgb(255, 255, 0) : al_map_rgb(255, 255, 255);
    ALLEGRO_COLOR couleur_quit = current_selection == MENU_QUIT ?
        al_map_rgb(255, 255, 0) : al_map_rgb(255, 255, 255);

    al_draw_text(menu_font, couleur_start,
        screen_w / 2, screen_h / 2,
        ALLEGRO_ALIGN_CENTER, "Commencer");
    al_draw_text(menu_font, couleur_options,
        screen_w / 2, screen_h / 2 + 50,
        ALLEGRO_ALIGN_CENTER, "Options");
    al_draw_text(menu_font, couleur_quit,
        screen_w / 2, screen_h / 2 + 100,
        ALLEGRO_ALIGN_CENTER, "Quitter");
}

int main(void) {
    if (!al_init()) {
        fprintf(stderr, "Erreur: Impossible d'initialiser Allegro\n");
        return -1;
    }
    if (!al_install_keyboard()) {
        fprintf(stderr, "Erreur: Impossible d'initialiser le clavier\n");
        return -1;
    }
    if (!al_init_primitives_addon()) {
        fprintf(stderr, "Erreur: Impossible d'initialiser les primitives\n");
        return -1;
    }
    if (!al_init_image_addon()) {
        fprintf(stderr, "Erreur: Impossible d'initialiser l'addon image\n");
        return -1;
    }
    al_init_font_addon();
    al_init_ttf_addon();

    int spawn_x = 1, spawn_y = 1;
    char current_map[100] = "../map1.txt";
    lire_matrice(current_map, &spawn_x, &spawn_y);

    ALLEGRO_DISPLAY_MODE mode;
    al_get_display_mode(0, &mode);
    int screen_w = mode.width;
    int screen_h = mode.height;

    al_set_new_display_flags(ALLEGRO_FULLSCREEN);
    ALLEGRO_DISPLAY *fenetre = al_create_display(screen_w, screen_h);
    ALLEGRO_EVENT_QUEUE *file = al_create_event_queue();
    ALLEGRO_TIMER *timer = al_create_timer(1.0 / FPS);
    al_register_event_source(file, al_get_keyboard_event_source());
    al_register_event_source(file, al_get_display_event_source(fenetre));
    al_register_event_source(file, al_get_timer_event_source(timer));
    al_start_timer(timer);


    img_bloc1 = al_load_bitmap("../image/bloc1.png");
    img_bloc2 = al_load_bitmap("../image/bloc2.png");
    img_bloc3 = al_load_bitmap("../image/bloc3.png");
    img_bloc4 = al_load_bitmap("../image/bloc4.png");
    img_bloc5 = al_load_bitmap("../image/bloc5.png");
    img_bloc6 = al_load_bitmap("../image/bloc6.png");
    img_bloc7 = al_load_bitmap("../image/bloc7.png");
    background = al_load_bitmap("../image/background.png");
    ALLEGRO_BITMAP *L_a = al_load_bitmap("../image/a.png");
    ALLEGRO_BITMAP *L_b = al_load_bitmap("../image/b.png");
    ALLEGRO_BITMAP *L_c = al_load_bitmap("../image/c.png");
    ALLEGRO_BITMAP *L_d = al_load_bitmap("../image/d.png");
    ALLEGRO_BITMAP *L_e = al_load_bitmap("../image/e.png");
    ALLEGRO_BITMAP *L_f = al_load_bitmap("../image/f.png");

    if (!img_bloc1 || !img_bloc2 || !img_bloc3 || !background ||
        !L_a || !L_b || !L_c || !L_d || !L_e || !L_f) {
        fprintf(stderr, "Erreur: Impossible de charger une ou plusieurs images\n");
        return -1;
    }

    ALLEGRO_BITMAP *L = L_a;


    menu_font = al_load_font("../fonts/arial.ttf", 40, 0);
    if (!menu_font) {
        menu_font = al_load_font("C:\\Windows\\Fonts\\arial.ttf", 40, 0);
    }
    if (!menu_font) {
        fprintf(stderr, "Erreur: Impossible de charger la police\n");

        al_destroy_bitmap(img_bloc1); al_destroy_bitmap(img_bloc2); al_destroy_bitmap(img_bloc3);
        al_destroy_bitmap(background);
        al_destroy_bitmap(L_a); al_destroy_bitmap(L_b); al_destroy_bitmap(L_c);
        al_destroy_bitmap(L_d); al_destroy_bitmap(L_e); al_destroy_bitmap(L_f);
        al_destroy_timer(timer);
        al_destroy_event_queue(file);
        al_destroy_display(fenetre);
        return -1;
    }

    Player player = {spawn_x * SIZEX, spawn_y * SIZEY, 60, 60, 0, 0, false, false, true};
    float initial_spawn_x = spawn_x * SIZEX;
    float initial_spawn_y = spawn_y * SIZEY;
    bool keys[ALLEGRO_KEY_MAX] = {false};
    bool fini = false;
    bool vers_droite = true;
    float frame_timer = 0;
    float frame_interval = 0.15;


    GameState game_state = MENU_START;
    GameState menu_selection = MENU_START;

    while (!fini) {
        ALLEGRO_EVENT event;
        al_wait_for_event(file, &event);

        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            fini = true;
        }
        else if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
            if (game_state == MENU_START || game_state == MENU_OPTIONS || game_state == MENU_QUIT) {

                switch (event.keyboard.keycode) {
                    case ALLEGRO_KEY_UP:
                        menu_selection = (menu_selection == MENU_START) ?
                            MENU_QUIT : menu_selection - 1;
                        break;
                    case ALLEGRO_KEY_DOWN:
                        menu_selection = (menu_selection == MENU_QUIT) ?
                            MENU_START : menu_selection + 1;
                        break;
                    case ALLEGRO_KEY_ENTER:
                    case ALLEGRO_KEY_SPACE:
                        switch (menu_selection) {
                            case MENU_START:
                                game_state = GAME_RUNNING;
                                break;
                            case MENU_OPTIONS:
                                // TODO: Implémenter un menu d'options
                                break;
                            case MENU_QUIT:
                                fini = true;
                                break;
                        }
                        break;
                    case ALLEGRO_KEY_ESCAPE:
                        game_state = MENU_START;
                        break;
                }
            }
            else if (game_state == GAME_RUNNING) {

                keys[event.keyboard.keycode] = true;
                if (event.keyboard.keycode == ALLEGRO_KEY_Z && (player.au_sol || player.grimpe)) {
                    player.vy = SAUT;
                    player.au_sol = false;
                } else if (event.keyboard.keycode == ALLEGRO_KEY_SPACE && player.peut_dash) {
                    float dash_distance = SIZEX;
                    float new_x = player.x + (vers_droite ? dash_distance : -dash_distance);
                    if (!collision(new_x, player.y, player.w, player.h, DIR_HORIZ)) {
                        player.x = new_x;
                    }
                    player.peut_dash = false;
                } else if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                    game_state = GAME_PAUSED;
                }
            }
            else if (game_state == GAME_PAUSED) {
                if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                    game_state = GAME_RUNNING;
                }
            }
        }
        else if (event.type == ALLEGRO_EVENT_KEY_UP) {
            if (game_state == GAME_RUNNING) {
                keys[event.keyboard.keycode] = false;
            }
        }
        else if (event.type == ALLEGRO_EVENT_TIMER) {
            if (game_state == GAME_RUNNING) {
                // Le code de jeu original
                float old_x = player.x;
                float old_y = player.y;
                float vx = 0;
                if (keys[ALLEGRO_KEY_D]) { vx += VITESSE; vers_droite = true; }
                if (keys[ALLEGRO_KEY_Q]) { vx -= VITESSE; vers_droite = false; }
                player.x += vx;
                if (collision(player.x, player.y, player.w, player.h, DIR_HORIZ))
                    player.x = old_x;

                player.vy += GRAVITE;
                if (player.vy > MAX_VY) player.vy = MAX_VY;
                player.y += player.vy;
                if (collision(player.x, player.y, player.w, player.h, DIR_VERT)) {
                    if (player.vy > 0) {
                        player.au_sol = true;
                        player.peut_dash = true;
                    }
                    player.vy = 0;
                    player.y = old_y;
                } else {
                    player.au_sol = false;
                }

                int px = (player.x + player.w / 2) / SIZEX;
                int py = (player.y + player.h / 2) / SIZEY;
                if (px >= 0 && py >= 0 && px < cols && py < rows) {
                    int val = map[py][px];
                    if (val == 3) {
                        strcpy(current_map, "../map2.txt");
                        lire_matrice(current_map, &spawn_x, &spawn_y);
                        player.x = spawn_x * SIZEX;
                        player.y = spawn_y * SIZEY;
                        player.vx = 0;
                        player.vy = 0;
                        initial_spawn_x = player.x;
                        initial_spawn_y = player.y;
                    } else if (val == 5) {
                        strcpy(current_map, "../map3.txt");
                        lire_matrice(current_map, &spawn_x, &spawn_y);
                        player.x = spawn_x * SIZEX;
                        player.y = spawn_y * SIZEY;
                        player.vx = 0;
                        player.vy = 0;
                        initial_spawn_x = player.x;
                        initial_spawn_y = player.y;
                    } else if (val == 6) {
                        strcpy(current_map, "../map4.txt");
                        lire_matrice(current_map, &spawn_x, &spawn_y);
                        player.x = spawn_x * SIZEX;
                        player.y = spawn_y * SIZEY;
                        player.vx = 0;
                        player.vy = 0;
                        initial_spawn_x = player.x;
                        initial_spawn_y = player.y;
                    } else if (val == 7) {
                        strcpy(current_map, "../map1.txt");
                        lire_matrice(current_map, &spawn_x, &spawn_y);
                        player.x = spawn_x * SIZEX;
                        player.y = spawn_y * SIZEY;
                        player.vx = 0;
                        player.vy = 0;
                        initial_spawn_x = player.x;
                        initial_spawn_y = player.y;
                    }
                }

                int left = (player.x) / SIZEX;
                int right = (player.x + player.w - 1) / SIZEX;
                int top = (player.y) / SIZEY;
                int bottom = (player.y + player.h - 1) / SIZEY;

                for (int i = top; i <= bottom; i++) {
                    for (int j = left; j <= right; j++) {
                        if (map[i][j] == 2) {
                            player.x = initial_spawn_x;
                            player.y = initial_spawn_y;
                            player.vx = 0;
                            player.vy = 0;
                        }
                    }
                }

                player.grimpe = false;
                for (int i = top; i <= bottom; i++) {
                    for (int j = left; j <= right; j++) {
                        if (i >= 0 && j >= 0 && i < rows && j < cols && map[i][j] == 4) {
                            if ((player.x + player.w >= j * SIZEX && player.x <= (j + 1) * SIZEX)) {
                                player.grimpe = true;
                                break;
                            }
                        }
                    }
                }
                if (player.grimpe && keys[ALLEGRO_KEY_Z]) {
                    player.vy = -VITESSE;
                }

                bool en_mouvement = vx != 0;
                if (en_mouvement) {
                    frame_timer += 1.0 / FPS;
                    if (frame_timer >= frame_interval) {
                        frame_timer = 0;
                        static bool toggle = false;
                        toggle = !toggle;
                        if (vers_droite) L = toggle ? L_b : L_c;
                        else L = toggle ? L_e : L_f;
                    }
                } else {
                    L = vers_droite ? L_a : L_d;
                }
            }


            if (game_state == GAME_RUNNING) {
                al_draw_scaled_bitmap(background, 0, 0,
                    al_get_bitmap_width(background), al_get_bitmap_height(background),
                    0, 0, screen_w, screen_h, 0);
                dessiner_matrice();
                al_draw_scaled_bitmap(L, 0, 0, al_get_bitmap_width(L), al_get_bitmap_height(L),
                                      player.x, player.y, player.w, player.h, 0);
            }
            else if (game_state == MENU_START || game_state == MENU_OPTIONS || game_state == MENU_QUIT) {
                dessiner_menu(screen_w, screen_h, menu_selection);
            }
            else if (game_state == GAME_PAUSED) {

                al_draw_scaled_bitmap(background, 0, 0,
                    al_get_bitmap_width(background), al_get_bitmap_height(background),
                    0, 0, screen_w, screen_h, 0);
                dessiner_matrice();
                al_draw_scaled_bitmap(L, 0, 0, al_get_bitmap_width(L), al_get_bitmap_height(L),
                                      player.x, player.y, player.w, player.h, 0);


                al_draw_filled_rectangle(0, 0, screen_w, screen_h,
                    al_map_rgba(0, 0, 0, 128));


                al_draw_text(menu_font, al_map_rgb(255, 255, 255),
                    screen_w / 2, screen_h / 2,
                    ALLEGRO_ALIGN_CENTER, "PAUSE");
                al_draw_text(menu_font, al_map_rgb(200, 200, 200),
                    screen_w / 2, screen_h / 2 + 50,
                    ALLEGRO_ALIGN_CENTER, "Appuyez sur Echap pour continuer");
            }

            al_flip_display();
        }
    }

   
    al_destroy_bitmap(img_bloc1);
    al_destroy_bitmap(img_bloc2);
    al_destroy_bitmap(img_bloc3);
    if (img_bloc4) al_destroy_bitmap(img_bloc4);
    if (img_bloc5) al_destroy_bitmap(img_bloc5);
    if (img_bloc6) al_destroy_bitmap(img_bloc6);
    if (img_bloc7) al_destroy_bitmap(img_bloc7);
    al_destroy_bitmap(background);
    al_destroy_bitmap(L_a); al_destroy_bitmap(L_b); al_destroy_bitmap(L_c);
    al_destroy_bitmap(L_d); al_destroy_bitmap(L_e); al_destroy_bitmap(L_f);
    al_destroy_timer(timer);
    al_destroy_event_queue(file);
    al_destroy_display(fenetre);
    al_destroy_font(menu_font);
    return 0;
}