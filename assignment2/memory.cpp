#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <cstdio>
#include "board.h"

int mx = 0;
int my = 0;

const int CELL_SIZE = 120;
const int MARGIN = 10;
const int DISPLAY_W = 800;
const int DISPLAY_H = 600;

const int FACE_DOWN = 0;
const int FACE_UP = 1;
const int FOUND = 2;

const int STATUS_CELL = 24;

//0 = first click, 1 = second click, 2 = mismatch showing
const int LVL_1 = 0;
const int LVL_2 = 1;
const int LVL_MISS = 2;

int getCell(int px, int py);
void drawGrid(const board& b);
void drawStatus(const board& b, ALLEGRO_FONT* f);
void drawWin(ALLEGRO_FONT* f);
void drawShape(int idx, const board& b);
void drawCount(float cx, float cy, int kind, int n);

int main() {
    ALLEGRO_DISPLAY* display = NULL;
    ALLEGRO_EVENT_QUEUE* queue = NULL;
    ALLEGRO_TIMER* timer = NULL;
    ALLEGRO_FONT* font = NULL;
    ALLEGRO_EVENT ev;
    board b;

    int levels = LVL_1;
    int idx = 0;
    double miss_time = 0.0;
    bool redraw = true;
    bool done = false;
    bool game_over = false;

    //allegro setup
    if (!al_init()) {
        printf("al_init failed\n");
        return -1;
    }
    al_init_primitives_addon();
    al_init_font_addon();
    al_init_ttf_addon();
    al_install_keyboard();
    al_install_mouse();

    display = al_create_display(DISPLAY_W, DISPLAY_H);
    timer = al_create_timer(1.0 / 60.0);
    queue = al_create_event_queue();
    font = al_create_builtin_font();

    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_mouse_event_source());

    //set up new game and start the timer
    b.reset_game();
    b.random_create();
    al_start_timer(timer);

    while (!done && !game_over) {
        al_wait_for_event(queue, &ev);

        if (ev.type == ALLEGRO_EVENT_TIMER) {
            //if 5 sec have passed since the mismatch flip em back over
            if (levels == LVL_MISS && al_get_time() - miss_time >= 5.0) {
                b.hide_mismatched();
                levels = LVL_1;
            }
            redraw = true;
        }
        else if (ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
            mx = ev.mouse.x;
            my = ev.mouse.y;
        }
        else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && ev.mouse.button == 1) {
            mx = ev.mouse.x;
            my = ev.mouse.y;

            //only respond if were waiting for a click
            if (levels == LVL_1 || levels == LVL_2) {
                idx = getCell(mx, my);

                //skip bad clicks and cards already revealed
                if (idx >= 0 && b.get_cell_state(idx) == FACE_DOWN) {
                    b.set_face_up(idx);

                    if (levels == LVL_1) {
                        levels = LVL_2;
                    }
                    else {
                        //second card flipped, check if it matches
                        if (b.compare_selection()) {
                            if (b.is_game_won())
                                game_over = true;
                            else
                                levels = LVL_1;
                        }
                        else {
                            miss_time = al_get_time();
                            levels = LVL_MISS;
                        }
                    }
                }
            }
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
                done = true;
            else if (ev.keyboard.keycode == ALLEGRO_KEY_R)
                b.reset_game();
            b.random_create();
            levels = LVL_1;
        }
        else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            done = true;
        }

        if (redraw && al_is_event_queue_empty(queue)) {
            al_clear_to_color(al_map_rgb(100, 156, 68));
            drawGrid(b);
            drawStatus(b, font);
            if (game_over)
                drawWin(font);
            al_flip_display();
            redraw = false;
        }
    }

    //hold the win screen for a few seconds so they can see they won
    if (game_over)
        al_rest(5.0);

    al_destroy_font(font);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    al_destroy_display(display);
    return 0;
}

int getCell(int px, int py) {
    //convert pixel pos to grid index
    int x = px - MARGIN;
    int y = py - MARGIN;

    if (x < 0 || y < 0 || x >= CELL_SIZE * COLUMNS || y >= CELL_SIZE * ROWS)
        return -1;

    int col = x / CELL_SIZE;
    int row = y / CELL_SIZE;
    int i = row * COLUMNS + col;

    //status cell isnt clickable so ignore it
    if (i == STATUS_CELL)
        return -1;

    return i;
}

void drawGrid(const board& b) {
    //loops through every cell and decides what to draw based on its state
    int i;
    int st;
    float l, t, r, btm;
    ALLEGRO_COLOR bg;

    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLUMNS; col++) {
            i = row * COLUMNS + col;
            if (i == STATUS_CELL) continue;

            l = static_cast<float>(MARGIN + col * CELL_SIZE);
            t = static_cast<float>(MARGIN + row * CELL_SIZE);
            r = l + CELL_SIZE;
            btm = t + CELL_SIZE;

            st = b.get_cell_state(i);

            if (st == FOUND)
                bg = al_map_rgb(35, 75, 35);
            else if (st == FACE_UP)
                bg = al_map_rgb(75, 75, 110);
            else
                bg = al_map_rgb(55, 55, 85);

            al_draw_filled_rectangle(l + 2, t + 2, r - 2, btm - 2, bg);
            al_draw_rectangle(l + 1, t + 1, r - 1, btm - 1, al_map_rgb(90, 90, 130), 2.0f);

            //only draw the shape if the card is up or matched
            if (st == FACE_UP || st == FOUND)
                drawShape(i, b);
        }
    }
}

void drawStatus(const board& b, ALLEGRO_FONT* f) {
    float l = static_cast<float>(MARGIN + 4 * CELL_SIZE);
    float t = static_cast<float>(MARGIN + 4 * CELL_SIZE);
    float r = l + CELL_SIZE;
    float btm = t + CELL_SIZE;
    float tcx = l + CELL_SIZE / 2.0f;
    float tt = t + CELL_SIZE / 2.0f - 24.0f;

    al_draw_filled_rectangle(l + 2, t + 2, r - 2, btm - 2, al_map_rgb(20, 55, 20));
    al_draw_rectangle(l + 1, t + 1, r - 1, btm - 1, al_map_rgb(180, 180, 60), 2.5f);

    al_draw_text(f, al_map_rgb(200, 200, 200), tcx, tt, ALLEGRO_ALIGN_CENTRE, "STATUS");
    al_draw_text(f, al_map_rgb(200, 200, 200), tcx, tt + 14, ALLEGRO_ALIGN_CENTRE, "Matched:");
    al_draw_textf(f, al_map_rgb(220, 220, 80), tcx, tt + 26, ALLEGRO_ALIGN_CENTRE, "%d / %d", b.get_pairs_matched(), PAIRS);
    al_draw_textf(f, al_map_rgb(200, 200, 200), tcx, tt + 40, ALLEGRO_ALIGN_CENTRE, "Left: %d", b.get_pairs_remaining());
}

void drawWin(ALLEGRO_FONT* f) {
    float cx = DISPLAY_W / 2.0f;
    float cy = DISPLAY_H / 2.0f;

    al_draw_filled_rectangle(0, 0, DISPLAY_W, DISPLAY_H, al_map_rgba(0, 0, 0, 190));
    al_draw_textf(f, al_map_rgb(220, 220, 60), cx, cy - 10, ALLEGRO_ALIGN_CENTRE, "YOU WIN! All %d pairs matched!", PAIRS);
    al_draw_text(f, al_map_rgb(200, 200, 200), cx, cy + 10, ALLEGRO_ALIGN_CENTRE, "Press R to play again or ESC to quit.");
}

void drawShape(int idx, const board& b) {
    //figure out what shape and how many to draw from the cards id
    if (idx < 0 || idx >= 24) return;

    int sid = b.get_shape(idx);
    if (sid == 0) return;

    int row = idx / COLUMNS;
    int col = idx % COLUMNS;
    float cx = static_cast<float>(MARGIN + col * CELL_SIZE + CELL_SIZE / 2);
    float cy = static_cast<float>(MARGIN + row * CELL_SIZE + CELL_SIZE / 2);

    //1-4 = circles, 5-8 = squares, 9-12 = rectangles. the count is whats left over after subtracting
    if (sid <= 4)
        drawCount(cx, cy, 0, sid);
    else if (sid <= 8)
        drawCount(cx, cy, 1, sid - 4);
    else
        drawCount(cx, cy, 2, sid - 8);
}

//kind: 0 = circle, 1 = square, 2 = rectangle | n is how many to draw
void drawCount(float cx, float cy, int kind, int n) {
    if (n < 1 || n > 4) return;

    ALLEGRO_COLOR clr;
    if (kind == 0)
        clr = al_map_rgb(220, 80, 80);
    else if (kind == 1)
        clr = al_map_rgb(80, 160, 220);
    else
        clr = al_map_rgb(80, 200, 120);

    ALLEGRO_COLOR white = al_map_rgb(255, 255, 255);

    //one big shape if its by itself, otherwise smaller so they all fit
    float s;
    float w;
    float h;

    if (n == 1)
        s = 28.0f;
    else
        s = 18.0f;

    //rectangles use w and h since theyre wider than tall, circles + squares just use s
    if (kind == 2) {
        if (n == 1) {
            w = 36.0f;
            h = 16.0f;
        }
        else {
            w = 20.0f;
            h = 10.0f;
        }
    }
    else {
        w = s;
        h = s;
    }

    //setting up where each shape will go inside the cell
    float px[4];
    float py[4];

    if (n == 1) {
        px[0] = cx; py[0] = cy;
    }
    else if (n == 2) {
        px[0] = cx - 22; py[0] = cy;
        px[1] = cx + 22; py[1] = cy;
    }
    else if (n == 3) {
        px[0] = cx - 22; py[0] = cy - 18;
        px[1] = cx + 22; py[1] = cy - 18;
        px[2] = cx;      py[2] = cy + 18;
    }
    else {
        px[0] = cx - 22; py[0] = cy - 18;
        px[1] = cx + 22; py[1] = cy - 18;
        px[2] = cx - 22; py[2] = cy + 18;
        px[3] = cx + 22; py[3] = cy + 18;
    }
    //loops through the positions and draws each one based on kind
    for (int i = 0; i < n; i++) {
        float x = px[i];
        float y = py[i];

        if (kind == 0) {
            al_draw_filled_circle(x, y, s, clr);
            al_draw_circle(x, y, s, white, 1.5f);
        }
        else {
            al_draw_filled_rectangle(x - w, y - h, x + w, y + h, clr);
            al_draw_rectangle(x - w, y - h, x + w, y + h, white, 1.5f);
        }
    }
}