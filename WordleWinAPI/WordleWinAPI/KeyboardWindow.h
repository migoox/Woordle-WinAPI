#pragma once
#include "Window.h"
#include "Difficulty.h"
#include <array>

#include "Board.h"

struct kbrd_tile
{
    std::array<letter_state, 4> states;
    RECT position;
    char letter;

    kbrd_tile();

    // save old pen and brush before this call
    void draw(HDC hdc, INT posx = 0, INT posy = 0);

    static constexpr int letters_count = 26;
    static constexpr int tile_size = Board::tile_size;
    static constexpr int margin = Board::margin;

    static const char letters[letters_count + 1];
};

class kbrd_win : public win
{
public:
    kbrd_win(std::shared_ptr<win_class> wclass);
    ~kbrd_win() = default;
    
    virtual LRESULT window_proc(
        HWND window,
        UINT message,
        WPARAM wParam,
        LPARAM lParam) override;

    void init_difficulty();
    void init_kbrd_tiles();
    virtual void init() override;

    void reset();

    void enter_word_easy(char* word, int row);
    void enter_word_medium(char* word, int row);
    void enter_word_hard(char* word, int row);

private:

	ULONGLONG m_timer_start;
    std::array<kbrd_tile, kbrd_tile::letters_count> m_kbrd_tiles;
    int m_current_row;
    int m_current_column;
};