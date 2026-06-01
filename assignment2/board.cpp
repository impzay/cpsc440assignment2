#include "board.h"
#include <cstdlib>
#include <ctime>

const int SHAPE_EMPTY = 0;

const int FACE_DOWN = 0;
const int FACE_UP = 1;
const int FOUND = 2;

board::board() {
    reset_game();
}

board::~board() {
}

void board::reset_game() {
    //wipe both arrays and reset counters so a fresh game can start
    int row;
    int col;

    for (row = 0; row < ROWS; row++) {
        for (col = 0; col < COLUMNS; col++) {
            pattern[row][col] = SHAPE_EMPTY;
            cell_state[row][col] = FACE_DOWN;
        }
    }

    pairs_matched = 0;
    first_sel = -1;
    second_sel = -1;
}

void board::random_create() {
    //fills 24 cells with 12 pairs of shape ids and shuffles them randomly. cell 24 is left empty for the status panel
    int pool[24];
    int i;
    int swap;
    int temp;

    srand(time(0));

    //fill the pool so each id from 1-12 appears twice
    for (i = 0; i < PAIRS; i++) {
        pool[i * 2] = i + 1;
        pool[i * 2 + 1] = i + 1;
    }

    //shuffle the pool by swapping random pairs a bunch of times 
    for (i = 23; i > 0; i--) {
        swap = rand() % (i + 1);
        temp = pool[i];
        pool[i] = pool[swap];
        pool[swap] = temp;
    }

    //load shuffled ids into the board
    for (i = 0; i < 24; i++) {
        pattern[index_to_row(i)][index_to_column(i)] = pool[i];
    }
}

int board::get_shape(int index) const {
    if (!is_valid_index(index))
        return SHAPE_EMPTY;

    return pattern[index_to_row(index)][index_to_column(index)];
}

int board::get_cell_state(int index) const {
    if (!is_valid_index(index))
        return FACE_DOWN;

    return cell_state[index_to_row(index)][index_to_column(index)];
}

void board::set_face_up(int index) {
    //flips a card up and remembers it as the 1st or 2nd selection for this turn
    int row;
    int col;

    if (!is_valid_index(index))
        return;

    row = index_to_row(index);
    col = index_to_column(index);

    //only flip face-down cards, ignore the rest
    if (cell_state[row][col] != FACE_DOWN)
        return;

    cell_state[row][col] = FACE_UP;

    if (first_sel == -1)
        first_sel = index;
    else if (second_sel == -1)
        second_sel = index;
}

bool board::compare_selection() {
    //compares the two cards the player flipped. if they match, lock em in. if not return false so main can show them for 5 sec
    int a;
    int b;

    if (first_sel == -1 || second_sel == -1)
        return false;

    a = get_shape(first_sel);
    b = get_shape(second_sel);

    if (a == b && a != SHAPE_EMPTY) {
        cell_state[index_to_row(first_sel)][index_to_column(first_sel)] = FOUND;
        cell_state[index_to_row(second_sel)][index_to_column(second_sel)] = FOUND;
        pairs_matched++;
        first_sel = -1;
        second_sel = -1;
        return true;
    }

    return false;
}

void board::hide_mismatched() {
    //flip any cards thats still face up back down (the 5 sec timer ran out without a match)
    int row;
    int col;

    for (row = 0; row < ROWS; row++) {
        for (col = 0; col < COLUMNS; col++) {
            if (cell_state[row][col] == FACE_UP)
                cell_state[row][col] = FACE_DOWN;
        }
    }

    first_sel = -1;
    second_sel = -1;
}

int board::get_first_selection() const {
    return first_sel;
}

int board::get_second_selection() const {
    return second_sel;
}

int board::count_face_up() const {
    int row;
    int col;
    int count = 0;

    for (row = 0; row < ROWS; row++) {
        for (col = 0; col < COLUMNS; col++) {
            if (cell_state[row][col] == FACE_UP)
                count++;
        }
    }

    return count;
}

int board::get_pairs_matched() const {
    return pairs_matched;
}

int board::get_pairs_remaining() const {
    return PAIRS - pairs_matched;
}

bool board::is_game_won() const {
    return (pairs_matched == PAIRS);
}

//converts a linear cell index into a row or column
int board::index_to_row(int index) const {
    return index / COLUMNS;
}

int board::index_to_column(int index) const {
    return index % COLUMNS;
}

bool board::is_valid_index(int index) const {
    //cells 0-23 are playable, 24 is the status panel
    return (index >= 0 && index < 24);
}