#pragma once
#include "KeyboardWindow.h"
#include "Difficulty.h"
#include "board.h"

class game_win : public win
{
public:
    game_win(std::shared_ptr<win_class> wclass);
    ~game_win() = default;

    virtual LRESULT window_proc(
        HWND window,
        UINT message,
        WPARAM wParam,
        LPARAM lParam);

    void set_difficulty_overlay(difficulty d);
    virtual void init() override;

    void reset();

    Board& get_board() { return m_board; }

    void set_word(const std::string& word);

    std::array<letter_state, Board::cols> check_word(char* guess);

    bool is_word_guessed() const { return m_word_guessed; }

private:
    void draw(HDC hdc);
    void draw_overlay(HDC hdc);
    bool m_word_guessed;
    ULONGLONG m_timer_start;
    Board m_board;
    std::string m_word;
};