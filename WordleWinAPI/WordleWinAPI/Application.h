#pragma once
#include <string>
#include <windows.h>
#include <dwmapi.h>
#include "board.h"
#include "KeyboardWindow.h"
#include "GameWindow.h"
#include "Difficulty.h"
#include "board.h"

#include <unordered_set>
class app
{
public:

	app(HINSTANCE, int);
	~app();
	int run();

	static HINSTANCE get_instance_handler() { return s_instance; }

	static constexpr UINT game_window_count = 4;


	static std::shared_ptr<kbrd_win>& get_keyboard_window()
	{
		return m_keyboard_win;
	}
	static std::array<std::shared_ptr<game_win>, game_window_count>& get_game_windows()
	{
		return m_game_wins;
	}

	static difficulty& get_difficulty() { return m_current_difficulty; }

	static char* get_input() { return input; }

	static constexpr COLORREF blank_letter_color = RGB(251, 252, 255);
	static constexpr COLORREF wrong_letter_color = RGB(164, 174, 196);
	static constexpr COLORREF wrong_position_letter_color = RGB(243, 194, 55);
	static constexpr COLORREF correct_letter_color = RGB(121, 184, 81);
	static constexpr COLORREF frame_letter_color = RGB(222, 225, 233);
	static constexpr COLORREF background_color = RGB(255, 255, 255);

	static void init_font();

	static INT get_rows_count()
	{
		if (m_current_difficulty == difficulty::easy)
			return Board::easy_rows;
		else if (m_current_difficulty == difficulty::medium)
			return Board::medium_rows;
		return Board::hard_rows;
	}

	static HFONT get_font_handler() { return font_handler; }

	static void load_words();
	static std::string random_word();
	static bool is_word_valid(std::string word);

	static bool is_game_won() 
	{
		if (m_current_difficulty == difficulty::easy)
		{
			return m_game_wins[0]->is_word_guessed();
		}
		else if (m_current_difficulty == difficulty::medium)
		{
			return m_game_wins[0]->is_word_guessed()
					&& m_game_wins[1]->is_word_guessed();

		}
		return m_game_wins[0]->is_word_guessed()
			&& m_game_wins[1]->is_word_guessed()
			&& m_game_wins[2]->is_word_guessed()
			&& m_game_wins[3]->is_word_guessed();
	}

	static void end_game()
	{
		game_ended = true;
		for (auto& w : m_game_wins)
		{
			w->send_draw_message();
		}
	}
	static void reset();
	static void clear_input();

	static bool is_ended() { return game_ended; }
private:
	static char input[Board::max_tile_count];
	static HINSTANCE s_instance;

	static std::shared_ptr<kbrd_win> m_keyboard_win;
	std::shared_ptr<win_class> m_keyboard_win_class;

	static std::array<std::shared_ptr<game_win>, game_window_count> m_game_wins;
	std::shared_ptr<win_class> m_game_win_class;

	static difficulty m_current_difficulty;

	static HFONT font_handler;

	static std::unordered_set<std::string> words;
	static bool game_ended;
};