#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#define SIZEX 80
#define SIZEY 70
#define MAX_ROWS 100
#define MAX_COLS 100
#define FPS 60

#define VITESSE 4.0
#define SAUT -10.0
#define GRAVITE 0.6
#define MAX_VY 12.0

int map[MAX_ROWS][MAX_COLS];
int rows = 0, cols = 0;

ALLEGRO_BITMAP *img_bloc1 = NULL;
ALLEGRO_BITMAP *img_bloc2 = NULL;
ALLEGRO_BITMAP *img_bloc3 = NULL;
ALLEGRO_BITMAP *background = NULL;

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
                al_draw_filled_rectangle(x * SIZEX, y * SIZEY,
                    (x + 1) * SIZEX, (y + 1) * SIZEY,
                    al_map_rgb(0, 0, 255));
            } else {
                al_draw_filled_rectangle(x * SIZEX, y * SIZEY,
                    (x + 1) * SIZEX, (y + 1) * SIZEY,
                    al_map_rgb(255, 0, 255));
            }
        }
    }
}

void afficher_menu(ALLEGRO_FONT *font, int screen_w, int screen_h, ALLEGRO_BITMAP *bg) {
    // Afficher le background en arrière-plan du menu
    if (bg) {
        float scale_x = (float)screen_w / al_get_bitmap_width(bg);
        float scale_y = (float)screen_h / al_get_bitmap_height(bg);
        al_draw_scaled_bitmap(bg, 0, 0,
                            al_get_bitmap_width(bg), al_get_bitmap_height(bg),
                            0, 0, screen_w, screen_h, 0);
    } else {
        al_clear_to_color(al_map_rgb(0, 0, 0));
    }

    ALLEGRO_COLOR blanc = al_map_rgb(255, 255, 255);
    ALLEGRO_COLOR noir = al_map_rgb(0, 0, 0);

    // Titre avec fond semi-transparent pour meilleure lisibilité
    al_draw_filled_rectangle(screen_w/2 - 150, screen_h/4 - 30, screen_w/2 + 150, screen_h/4 + 10, al_map_rgba(0, 0, 0, 180));
    al_draw_text(font, blanc, screen_w / 2, screen_h / 4 - 25, ALLEGRO_ALIGN_CENTER, "MENU PRINCIPAL");

    // Bouton Jouer
    al_draw_filled_rectangle(screen_w/2 - 100, screen_h/2 - 30, screen_w/2 + 100, screen_h/2 + 10, al_map_rgb(50, 150, 50));
    al_draw_text(font, blanc, screen_w / 2, screen_h/2 - 25, ALLEGRO_ALIGN_CENTER, "JOUER");

    // Bouton Quitter
    al_draw_filled_rectangle(screen_w/2 - 100, screen_h/2 + 40, screen_w/2 + 100, screen_h/2 + 80, al_map_rgb(150, 50, 50));
    al_draw_text(font, blanc, screen_w / 2, screen_h/2 + 45, ALLEGRO_ALIGN_CENTER, "QUITTER");

    al_flip_display();
}

int main(void) {
    assert(al_init());
    assert(al_install_keyboard());
    assert(al_install_mouse());
    assert(al_init_primitives_addon());
    assert(al_init_image_addon());
    al_init_font_addon();
    assert(al_init_ttf_addon());

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
    ALLEGRO_FONT *font = al_create_builtin_font();

    al_register_event_source(file, al_get_keyboard_event_source());
    al_register_event_source(file, al_get_mouse_event_source());
    al_register_event_source(file, al_get_display_event_source(fenetre));
    al_register_event_source(file, al_get_timer_event_source(timer));
    al_start_timer(timer);

    img_bloc1 = al_load_bitmap("../image/bloc1.png");
    img_bloc2 = al_load_bitmap("../image/bloc2.png");
    img_bloc3 = al_load_bitmap("../image/bloc3.png");
    background = al_load_bitmap("../image/background.png");
    ALLEGRO_BITMAP *L_a = al_load_bitmap("../image/a.png");
    ALLEGRO_BITMAP *L_b = al_load_bitmap("../image/b.png");
    ALLEGRO_BITMAP *L_c = al_load_bitmap("../image/c.png");
    ALLEGRO_BITMAP *L_d = al_load_bitmap("../image/d.png");
    ALLEGRO_BITMAP *L_e = al_load_bitmap("../image/e.png");
    ALLEGRO_BITMAP *L_f = al_load_bitmap("../image/f.png");
    assert(img_bloc1 && img_bloc2 && img_bloc3 && background && L_a && L_b && L_c && L_d && L_e && L_f);
    ALLEGRO_BITMAP *L = L_a;

    Player player = {spawn_x * SIZEX, spawn_y * SIZEY, 60, 60, 0, 0, false, false, true};
    float initial_spawn_x = player.x;
    float initial_spawn_y = player.y;
    bool keys[ALLEGRO_KEY_MAX] = {false};
    bool fini = false;
    bool vers_droite = true;
    float frame_timer = 0;
    float frame_interval = 0.15;

    Etat_Jeu etat = ETAT_MENU;
    bool redessiner = true;

    while (!fini) {
        ALLEGRO_EVENT event;
        al_wait_for_event(file, &event);

        if (event.type == ALLEGRO_EVENT_TIMER) {
            redessiner = true;
        } else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            fini = true;
        } else if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
            keys[event.keyboard.keycode] = true;

            // Échap pour quitter le jeu ou revenir au menu
            if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                if (etat == ETAT_JEU) {
                    etat = ETAT_MENU;
                    // Réinitialiser les positions du joueur pour le prochain jeu
                    player.x = initial_spawn_x;
                    player.y = initial_spawn_y;
                    player.vx = 0;
                    player.vy = 0;
                } else {
                    fini = true;
                }
            }
        } else if (event.type == ALLEGRO_EVENT_KEY_UP) {
            keys[event.keyboard.keycode] = false;
        } else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            if (etat == ETAT_MENU) {
                int mx = event.mouse.x;
                int my = event.mouse.y;

                // Vérification si clique sur le bouton JOUER
                if (mx >= screen_w/2 - 100 && mx <= screen_w/2 + 100 &&
                    my >= screen_h/2 - 30 && my <= screen_h/2 + 10) {
                    etat = ETAT_JEU;
                    // Réinitialiser les positions du joueur pour le nouveau jeu
                    player.x = initial_spawn_x;
                    player.y = initial_spawn_y;
                    player.vx = 0;
                    player.vy = 0;
                }
                // Vérification si clique sur le bouton QUITTER
                else if (mx >= screen_w/2 - 100 && mx <= screen_w/2 + 100 &&
                        my >= screen_h/2 + 40 && my <= screen_h/2 + 80) {
                    fini = true;
                }
            }
        }

        if (redessiner && al_is_event_queue_empty(file)) {
            redessiner = false;

            if (etat == ETAT_MENU) {
                afficher_menu(font, screen_w, screen_h, background);
            } else if (etat == ETAT_JEU) {
                // Logique originale du jeu
                float old_x = player.x;
                float old_y = player.y;
                float vx = 0;

                // Déplacement
                if (keys[ALLEGRO_KEY_D]) { vx += VITESSE; vers_droite = true; }
                if (keys[ALLEGRO_KEY_Q]) { vx -= VITESSE; vers_droite = false; }
                player.x += vx;
                if (collision(player.x, player.y, player.w, player.h, DIR_HORIZ))
                    player.x = old_x;

                // Gravité et saut
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

                // Téléportation vers la map2 quand on touche un bloc de type 3
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
                    }
                }

                // Retour au point de spawn si touche un bloc de type 2 (bloc danger)
                int left = (player.x) / SIZEX;
                int right = (player.x + player.w - 1) / SIZEX;
                int top = (player.y) / SIZEY;
                int bottom = (player.y + player.h - 1) / SIZEY;

                for (int i = top; i <= bottom; i++) {
                    for (int j = left; j <= right; j++) {
                        if (i >= 0 && j >= 0 && i < rows && j < cols) {
                            if (map[i][j] == 2) {
                                player.x = initial_spawn_x;
                                player.y = initial_spawn_y;
                                player.vx = 0;
                                player.vy = 0;
                            }
                        }
                    }
                }

                // Grimper sur un mur (bloc type 4)
                player.grimpe = false;
                for (int i = top; i <= bottom; i++) {
                    for (int j = left; j <= right; j++) {
                        if (i >= 0 && j >= 0 && i < rows && j < cols) {
                            if (map[i][j] == 4) {
                                if ((player.x + player.w >= j * SIZEX && player.x <= (j + 1) * SIZEX)) {
                                    player.grimpe = true;
                                    break;
                                }
                            }
                        }
                    }
                }
                if (player.grimpe && keys[ALLEGRO_KEY_Z]) {
                    player.vy = -VITESSE;
                }

                // Animation du joueur
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

                // Dessin du jeu
                al_draw_scaled_bitmap(background, 0, 0,
                    al_get_bitmap_width(background), al_get_bitmap_height(background),
                    0, 0, screen_w, screen_h, 0);
                dessiner_matrice();
                al_draw_scaled_bitmap(L, 0, 0,
                    al_get_bitmap_width(L), al_get_bitmap_height(L),
                    player.x, player.y, player.w, player.h, 0);
                al_flip_display();
            }
        }
    }

    al_destroy_bitmap(img_bloc1); al_destroy_bitmap(img_bloc2); al_destroy_bitmap(img_bloc3);
    al_destroy_bitmap(background);
    al_destroy_bitmap(L_a); al_destroy_bitmap(L_b); al_destroy_bitmap(L_c);
    al_destroy_bitmap(L_d); al_destroy_bitmap(L_e); al_destroy_bitmap(L_f);
    al_destroy_timer(timer);
    al_destroy_event_queue(file);
    al_destroy_display(fenetre);
    al_destroy_font(font);
    return 0;
}
