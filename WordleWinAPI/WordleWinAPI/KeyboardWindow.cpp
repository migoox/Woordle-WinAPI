#include "KeyboardWindow.h"

#include <codecvt>

#include "resource.h"
#include "Application.h"
#include <locale>
#include <string>


const char kbrd_tile::letters[kbrd_tile::letters_count + 1] { "QWERTYUIOPASDFGHJKLZXCVBNM" };

kbrd_tile::kbrd_tile() :
	letter('a')
{
    states[0] = letter_state::none;
    states[1] = letter_state::none;
    states[2] = letter_state::none;
    states[3] = letter_state::none;
}

void kbrd_tile::draw(HDC hdc, INT posx, INT posy)
{
    static HPEN ghost_pen = CreatePen(PS_NULL, 0, app::frame_letter_color);

    static HPEN frame_pen = CreatePen(PS_SOLID, 2, app::frame_letter_color);
    static HBRUSH brush_none = CreateSolidBrush(app::blank_letter_color);

    static HBRUSH brush_correct = CreateSolidBrush(app::correct_letter_color);
    static HBRUSH brush_wrong_position = CreateSolidBrush(app::wrong_position_letter_color);
    static HBRUSH brush_wrong_letter = CreateSolidBrush(app::wrong_letter_color);

    
    const RECT rects[4] =
    {
        {position.left, position.top, position.right - tile_size / 2, position.bottom - tile_size / 2},
        {position.left + tile_size / 2, position.top, position.right, position.bottom - tile_size / 2},
        {position.left, position.top + tile_size / 2, position.right - tile_size / 2, position.bottom},
        {position.left + tile_size / 2, position.top + tile_size / 2, position.right, position.bottom},
    };

    if (this->states[0] == letter_state::none)
    {
        SelectObject(hdc, frame_pen);
        SelectObject(hdc, brush_none);

        RoundRect(hdc,
            position.left,
            position.top,
            position.right,
            position.bottom,
            7,
            7);
    }
    else
    {
        SelectObject(hdc, ghost_pen);

        for (int i = 0; i < 4; i++)
        {
	        if (states[i] == letter_state::correct)
                SelectObject(hdc, brush_correct);
            else if (states[i] == letter_state::wrong_position)
                SelectObject(hdc, brush_wrong_position);
            else if (states[i] == letter_state::wrong_letter)
                SelectObject(hdc, brush_wrong_letter);

            RoundRect(hdc,
                rects[i].left,
                rects[i].top,
                rects[i].right,
                rects[i].bottom,
                3,
                3);
        }
    }
    SetBkMode(hdc, TRANSPARENT);
    DrawTextA(hdc, &letter, 1, &position, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
}


kbrd_win::kbrd_win(std::shared_ptr<win_class> wclass) : win(std::move(wclass)),
m_current_row(0),
m_current_column(0)
{
    // Prepare keyboard tiles
    init_kbrd_tiles();

    // Set create options
    m_wnd_caption = L"WORDLE - KEYBOARD";
    m_style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_BORDER | WS_MINIMIZEBOX;
    m_extended_style = WS_EX_LAYERED;

    // Create window
    m_wnd_handler = m_wnd_class->create_window(
        *this, 
        CW_USEDEFAULT, 
        0, 
        11 * kbrd_tile::margin + 10 * kbrd_tile::tile_size + kbrd_tile::tile_size / 2,
        5 * kbrd_tile::margin + 4 * kbrd_tile::tile_size,
        nullptr, 
        nullptr);
}

void kbrd_win::init()
{
    // Set alpha 
    SetLayeredWindowAttributes(m_wnd_handler, 0, 255 * 0.8f, LWA_ALPHA);
    m_menu_handler = GetMenu(m_wnd_handler);

    // Init difficulty
	init_difficulty();

    // Center the window
    const int sizex = GetSystemMetrics(SM_CXSCREEN);
    const int sizey = GetSystemMetrics(SM_CYSCREEN);
    RECT win_rect;
    GetWindowRect(m_wnd_handler, &win_rect);
    MoveWindow(
        m_wnd_handler,
        sizex / 2 - (win_rect.right - win_rect.left) / 2,
        (sizey * 3) / 4 - (win_rect.bottom - win_rect.top) / 2,
        (win_rect.right - win_rect.left),
        (win_rect.bottom - win_rect.top),
        TRUE);

    // Show window
    this->show();
}

void kbrd_win::reset()
{
    m_current_row = 0;
    m_current_column  = 0;

    for (auto& tile : m_kbrd_tiles)
    {
        tile.states[0] = letter_state::none;
        tile.states[1] = letter_state::none;
        tile.states[2] = letter_state::none;
        tile.states[3] = letter_state::none;
    }
}

void kbrd_win::enter_word_easy(char* word, int row)
{
    auto result = app::get_game_windows().at(0)->check_word(&app::get_input()[row * Board::cols]);
    for (int j = 0; j < Board::cols; j++)
    {
        app::get_game_windows().at(0)->get_board().Tiles().at(row * Board::cols + j).state = result[j];

    	auto it = std::find_if(m_kbrd_tiles.begin(), m_kbrd_tiles.end(),
            [&](kbrd_tile& t) { return t.letter == word[j]; });

        if(static_cast<int>(it->states[0]) < static_cast<int>(result[j]))
        {
            it->states[0] = result[j];
        }

        it->states[1] = it->states[0];
        it->states[2] = it->states[0];
        it->states[3] = it->states[0];
    }
	this->send_draw_message();
}

void kbrd_win::enter_word_medium(char* word, int row)
{
    for (int i = 0; i < 2; i++)
    {
        auto result = app::get_game_windows().at(i)->check_word(&app::get_input()[row * Board::cols]);
        for (int j = 0; j < Board::cols; j++)
        {
            app::get_game_windows().at(i)->get_board().Tiles().at(row * Board::cols + j).state = result[j];

            auto it = std::find_if(m_kbrd_tiles.begin(), m_kbrd_tiles.end(),
                [&](kbrd_tile& t) { return t.letter == word[j]; });

            if (static_cast<int>(it->states[i]) < static_cast<int>(result[j]))
            {
                it->states[i] = result[j];
            }

            it->states[2 + i] = it->states[i];
        }
    }
	this->send_draw_message();
}

void kbrd_win::enter_word_hard(char* word, int row)
{
    for (int i = 0; i < 4; i++)
    {
        auto result = app::get_game_windows().at(i)->check_word(&app::get_input()[row * Board::cols]);
        for (int j = 0; j < Board::cols; j++)
        {
            app::get_game_windows().at(i)->get_board().Tiles().at(row * Board::cols + j).state = result[j];

            auto it = std::find_if(m_kbrd_tiles.begin(), m_kbrd_tiles.end(),
                [&](kbrd_tile& t) { return t.letter == word[j]; });

            if (static_cast<int>(it->states[i]) < static_cast<int>(result[j]))
            {
                it->states[i] = result[j];
            }
        }
    }
	this->send_draw_message();
}

LRESULT kbrd_win::window_proc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    int sizex = GetSystemMetrics(SM_CXSCREEN);
    int sizey = GetSystemMetrics(SM_CYSCREEN);
    static std::wstring value;

    switch (message)
    {
    case WM_CREATE:
    {
        wchar_t result[16];
        GetPrivateProfileString(L"WORDLE", L"DIFFICULTY", L"1", result, 16, L".\\Wordle.ini");

        if (result[0] == L'0')
            app::get_difficulty() = difficulty::easy;
        else if (result[0] == L'1')
            app::get_difficulty() = difficulty::medium;
        else
            app::get_difficulty() = difficulty::hard;

        m_timer_start = GetTickCount64();
    }
    break;
    case WM_ERASEBKGND:
        return 1;
    case WM_TIMER:
    {
    }
    break;
    
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDM_EXIT:
            DestroyWindow(window);
            break;
        case ID_DIFFICULTY_EASY:
            app::get_difficulty() = difficulty::easy;
            init_difficulty();
            value = std::to_wstring(static_cast<int>(app::get_difficulty()));
            WritePrivateProfileString(L"WORDLE", L"DIFFICULTY", value.c_str(), L".\\Wordle.ini");
            break;
        case ID_DIFFICULTY_MEDIUM:
            app::get_difficulty() = difficulty::medium;
            init_difficulty();
            value = std::to_wstring(static_cast<int>(app::get_difficulty()));
            WritePrivateProfileString(L"WORDLE", L"DIFFICULTY", value.c_str(), L".\\Wordle.ini");
            break;
        case ID_DIFFICULTY_HARD:
            app::get_difficulty() = difficulty::hard;
            init_difficulty();
            value = std::to_wstring(static_cast<int>(app::get_difficulty()));
            WritePrivateProfileString(L"WORDLE", L"DIFFICULTY", value.c_str(), L".\\Wordle.ini");
            break;
        default:
            return DefWindowProc(window, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(window, &ps);
        
        HPEN oldpen = static_cast<HPEN>(SelectObject(hdc, nullptr));
        HBRUSH oldbrush = static_cast<HBRUSH>(SelectObject(hdc, nullptr));
        HFONT oldfont = static_cast<HFONT>(SelectObject(hdc, app::get_font_handler()));

		for (auto& tile : m_kbrd_tiles)
		{
			tile.draw(hdc, 0, 0);
		}

        SelectObject(hdc, oldfont);
        SelectObject(hdc, oldpen);
        SelectObject(hdc, oldbrush);
        EndPaint(window, &ps);
    }
    break;
    case WM_CHAR:
    {
            if (m_current_row == app::get_rows_count() || app::is_game_won())
            {
                break;
            }
    		// handle backspace
		  if (wParam == 8 && m_current_column > 0)
		  {
            --m_current_column;
		  	app::get_input()[m_current_row * Board::cols + m_current_column] = ' ';
		  }
	        // handle enter
	      if (wParam == 13 && m_current_column == Board::cols)
	      {
              std::string word(&app::get_input()[m_current_row * Board::cols], Board::cols);

              if (!app::is_word_valid(word))
              {
                for (int i = m_current_row * Board::cols; i < m_current_row * Board::cols + Board::cols; i++)
                {
                    app::get_input()[i] = ' ';
                }
                m_current_column = 0;
              }
              else
              {
                  if (app::get_difficulty() == difficulty::easy)
                      enter_word_easy(&app::get_input()[m_current_row * Board::cols], m_current_row);
                  else if (app::get_difficulty() == difficulty::medium)
                      enter_word_medium(&app::get_input()[m_current_row * Board::cols], m_current_row);
                  else
                      enter_word_hard(&app::get_input()[m_current_row * Board::cols], m_current_row);

                  if (app::is_game_won())
                  {
                      app::end_game();
                      SetWindowTextA(m_wnd_handler, "WYGRANA");
                  }

                  m_current_column = 0;
                  ++m_current_row;



              }
	      }
	        // handle lower case and upper case
	      if ((wParam >= 'A' && wParam <= 'Z' || wParam >= 'a' && wParam <= 'z') 
              && m_current_column < Board::cols)
	      {
              if (wParam >= 'a' && wParam <= 'z') wParam += ('A' - 'a');

              app::get_input()[m_current_row * Board::cols + m_current_column] = static_cast<char>(wParam);
              ++m_current_column;
	      }
         
    		for (int i  = 0; i < 4; i++)
    		{
	            app::get_game_windows()[i]->send_draw_message();
    		}
            if (m_current_row == app::get_rows_count())
            {
                app::end_game();
            }
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_WINDOWPOSCHANGED:
        break;
    case WM_DISPLAYCHANGE:
;
        break;
    default:
        return DefWindowProc(window, message, wParam, lParam);
    }
    return 0;
}

void kbrd_win::init_difficulty()
{
    const int sizex = GetSystemMetrics(SM_CXSCREEN);
    const int sizey = GetSystemMetrics(SM_CYSCREEN);


    if (app::get_difficulty() == difficulty::easy)
    {
        CheckMenuItem(m_menu_handler, ID_DIFFICULTY_EASY, MF_CHECKED);
        CheckMenuItem(m_menu_handler, ID_DIFFICULTY_MEDIUM, MF_UNCHECKED);
        CheckMenuItem(m_menu_handler, ID_DIFFICULTY_HARD, MF_UNCHECKED);
        app::get_game_windows()[0]->set_difficulty_overlay(difficulty::easy);

        RECT win_rect;
        GetWindowRect(app::get_game_windows()[0]->get_wnd_handler(), &win_rect);
        MoveWindow(
            app::get_game_windows()[0]->get_wnd_handler(),
            sizex / 2 - (win_rect.right - win_rect.left) / 2,
            sizey / 2 - (win_rect.bottom - win_rect.top) / 2,
            (win_rect.right - win_rect.left),
            (win_rect.bottom - win_rect.top),
            TRUE);

        app::get_game_windows()[0]->show();
        app::get_game_windows()[1]->hide();
        app::get_game_windows()[2]->hide();
        app::get_game_windows()[3]->hide();
    }
    else if (app::get_difficulty() == difficulty::medium)
    {
        CheckMenuItem(m_menu_handler, ID_DIFFICULTY_EASY, MF_UNCHECKED);
        CheckMenuItem(m_menu_handler, ID_DIFFICULTY_MEDIUM, MF_CHECKED);
        CheckMenuItem(m_menu_handler, ID_DIFFICULTY_HARD, MF_UNCHECKED);

        app::get_game_windows()[0]->set_difficulty_overlay(difficulty::medium);
        app::get_game_windows()[1]->set_difficulty_overlay(difficulty::medium);

        RECT win_rect;
        GetWindowRect(app::get_game_windows()[0]->get_wnd_handler(), &win_rect);
        MoveWindow(
            app::get_game_windows()[0]->get_wnd_handler(),
            sizex / 4 - (win_rect.right - win_rect.left) / 2,
            sizey / 2 - (win_rect.bottom - win_rect.top) / 2,
            (win_rect.right - win_rect.left),
            (win_rect.bottom - win_rect.top),
            TRUE);

        GetWindowRect(app::get_game_windows()[1]->get_wnd_handler(), &win_rect);
        MoveWindow(
            app::get_game_windows()[1]->get_wnd_handler(),
            sizex * 3 / 4 - (win_rect.right - win_rect.left) / 2,
            sizey / 2 - (win_rect.bottom - win_rect.top) / 2,
            (win_rect.right - win_rect.left),
            (win_rect.bottom - win_rect.top),
            TRUE);

        app::get_game_windows()[0]->show();
        app::get_game_windows()[1]->show();
        app::get_game_windows()[2]->hide();
        app::get_game_windows()[3]->hide();
    }
    else
    {
        CheckMenuItem(m_menu_handler, ID_DIFFICULTY_EASY, MF_UNCHECKED);
        CheckMenuItem(m_menu_handler, ID_DIFFICULTY_MEDIUM, MF_UNCHECKED);
        CheckMenuItem(m_menu_handler, ID_DIFFICULTY_HARD, MF_CHECKED);
        app::get_game_windows()[0]->set_difficulty_overlay(difficulty::hard);
        app::get_game_windows()[1]->set_difficulty_overlay(difficulty::hard);
        app::get_game_windows()[2]->set_difficulty_overlay(difficulty::hard);
        app::get_game_windows()[3]->set_difficulty_overlay(difficulty::hard);

        RECT win_rect;
        GetWindowRect(app::get_game_windows()[0]->get_wnd_handler(), &win_rect);
        MoveWindow(
            app::get_game_windows()[0]->get_wnd_handler(),
            sizex / 4 - (win_rect.right - win_rect.left) / 2,
            sizey / 4 - (win_rect.bottom - win_rect.top) / 2,
            (win_rect.right - win_rect.left),
            (win_rect.bottom - win_rect.top),
            TRUE);

        GetWindowRect(app::get_game_windows()[2]->get_wnd_handler(), &win_rect);
        MoveWindow(
            app::get_game_windows()[2]->get_wnd_handler(),
            sizex / 4 - (win_rect.right - win_rect.left) / 2,
            sizey * 3 / 4 - (win_rect.bottom - win_rect.top) / 2,
            (win_rect.right - win_rect.left),
            (win_rect.bottom - win_rect.top),
            TRUE);

        GetWindowRect(app::get_game_windows()[1]->get_wnd_handler(), &win_rect);
        MoveWindow(
            app::get_game_windows()[1]->get_wnd_handler(),
            sizex * 3 / 4 - (win_rect.right - win_rect.left) / 2,
            sizey / 4 - (win_rect.bottom - win_rect.top) / 2,
            (win_rect.right - win_rect.left),
            (win_rect.bottom - win_rect.top),
            TRUE);

        GetWindowRect(app::get_game_windows()[3]->get_wnd_handler(), &win_rect);
        MoveWindow(
            app::get_game_windows()[3]->get_wnd_handler(),
            sizex * 3 / 4 - (win_rect.right - win_rect.left) / 2,
            sizey * 3/ 4 - (win_rect.bottom - win_rect.top) / 2,
            (win_rect.right - win_rect.left),
            (win_rect.bottom - win_rect.top),
            TRUE);

        app::get_game_windows()[0]->show();
        app::get_game_windows()[1]->show();
        app::get_game_windows()[2]->show();
        app::get_game_windows()[3]->show();
    }
    app::reset();

    this->reset();
    this->send_draw_message();

    app::clear_input();
    app::get_game_windows()[0]->reset();
    app::get_game_windows()[1]->reset();
    app::get_game_windows()[2]->reset();
    app::get_game_windows()[3]->reset();

    app::get_game_windows()[0]->send_draw_message();
    app::get_game_windows()[1]->send_draw_message();
    app::get_game_windows()[2]->send_draw_message();
    app::get_game_windows()[3]->send_draw_message();
}

void kbrd_win::init_kbrd_tiles()
{
    for (int i = 0; i < 10; i++)
    {
        m_kbrd_tiles[i].letter = kbrd_tile::letters[i];
        m_kbrd_tiles[i].position.top = kbrd_tile::margin;
        m_kbrd_tiles[i].position.bottom = m_kbrd_tiles[i].position.top + kbrd_tile::tile_size;
        m_kbrd_tiles[i].position.left = kbrd_tile::tile_size * i + kbrd_tile::margin * (i + 1);
        m_kbrd_tiles[i].position.right = m_kbrd_tiles[i].position.left + kbrd_tile::tile_size;
    }

    for (int i = 0; i < 9; i++)
    {
        int index = i + 10;
        m_kbrd_tiles[index].letter = kbrd_tile::letters[index];

        m_kbrd_tiles[index].position.top = 2 * kbrd_tile::margin + kbrd_tile::tile_size;
        m_kbrd_tiles[index].position.bottom = m_kbrd_tiles[index].position.top + kbrd_tile::tile_size;
        m_kbrd_tiles[index].position.left = kbrd_tile::tile_size * i + kbrd_tile::margin * (i + 1) + kbrd_tile::tile_size / 2;
        m_kbrd_tiles[index].position.right = m_kbrd_tiles[index].position.left + kbrd_tile::tile_size;
    }

    for (int i = 0; i < 7; i++)
    {
        int index = i + 19;
        m_kbrd_tiles[index].letter = kbrd_tile::letters[index];
        m_kbrd_tiles[index].position.top = 3 * kbrd_tile::margin + 2 * kbrd_tile::tile_size;
        m_kbrd_tiles[index].position.bottom = m_kbrd_tiles[index].position.top + kbrd_tile::tile_size;
        m_kbrd_tiles[index].position.left = kbrd_tile::tile_size * i + kbrd_tile::margin * (i + 1) + kbrd_tile::tile_size * 3 / 2 + kbrd_tile::margin;
        m_kbrd_tiles[index].position.right = m_kbrd_tiles[index].position.left + kbrd_tile::tile_size;
    }
}


