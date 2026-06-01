#pragma once
const int ROWS = 5;
const int COLUMNS = 5;
const int PAIRS = 12;



class board
{
public:

    board();
    ~board();

    void reset_game();

    void random_create();

    int get_shape(int index) const;

    int get_cell_state(int index) const;

    void set_face_up(int index);

    bool compare_selection();

    void hide_mismatched();

    int get_first_selection() const;

    int get_second_selection() const;

    int count_face_up() const;

    int get_pairs_matched() const;

    int get_pairs_remaining() const;

    bool is_game_won() const;

private:
   
    int pattern[ROWS][COLUMNS];   
    int cell_state[ROWS][COLUMNS];

    int pairs_matched; 
    int first_sel;     
    int second_sel;     

    int index_to_row(int index) const;

    int index_to_column(int index) const;

    bool is_valid_index(int index) const;
};
