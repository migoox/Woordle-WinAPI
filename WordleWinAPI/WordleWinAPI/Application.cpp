#include "Application.h"
#include "Resource.h"
#include <stdexcept>
#include <string.h>
#include <stdio.h>
#include <tchar.h>
#include <fstream>
#include <random>
#include <algorithm>
#include <ctype.h>
#include<stdio.h>
#include<string.h>

HINSTANCE app::s_instance{};
std::shared_ptr<kbrd_win> app::m_keyboard_win{};
std::array<std::shared_ptr<game_win>, app::game_window_count> app::m_game_wins{};
difficulty app::m_current_difficulty{ difficulty::hard };
char app::input[Board::max_tile_count]{};
HFONT app::font_handler{ nullptr };
std::unordered_set<std::string> app::words;
bool app::game_ended{ false };

app::app(HINSTANCE instance, int showCommand)
{
    // Save the instance
    s_instance = instance;

    // Init font
    init_font();

    // Load words
    load_words();

    // Create keyboard window
    m_keyboard_win_class = std::make_shared<win_class>(L"KEYBOARD_CLASS");
	m_keyboard_win_class->get_wnd_classex().style = CS_HREDRAW | CS_VREDRAW;
    m_keyboard_win_class->get_wnd_classex().cbClsExtra = 0;
    m_keyboard_win_class->get_wnd_classex().cbWndExtra = 0;
    m_keyboard_win_class->get_wnd_classex().hInstance = app::get_instance_handler();
    m_keyboard_win_class->get_wnd_classex().hIcon = LoadIcon(app::get_instance_handler(), MAKEINTRESOURCE(IDI_ICON1));
    m_keyboard_win_class->get_wnd_classex().hCursor = LoadCursor(nullptr, IDC_ARROW);
    m_keyboard_win_class->get_wnd_classex().hbrBackground = CreateSolidBrush(RGB(255, 255, 255));
    m_keyboard_win_class->get_wnd_classex().lpszMenuName = MAKEINTRESOURCE(IDC_MENU);
    m_keyboard_win_class->get_wnd_classex().hIconSm = LoadIcon(app::get_instance_handler(), MAKEINTRESOURCE(IDI_ICON1));

    m_keyboard_win = std::make_shared<kbrd_win>(m_keyboard_win_class);

    // Create game windows
    m_game_win_class = std::make_shared<win_class>(L"GAME_CLASS");
    m_game_win_class->get_wnd_classex().style = CS_HREDRAW | CS_VREDRAW;
    m_game_win_class->get_wnd_classex().cbClsExtra = 0;
    m_game_win_class->get_wnd_classex().cbWndExtra = 0;
    m_game_win_class->get_wnd_classex().hInstance = app::get_instance_handler();
    m_game_win_class->get_wnd_classex().hIcon = LoadIcon(app::get_instance_handler(), MAKEINTRESOURCE(IDI_ICON1));
    m_game_win_class->get_wnd_classex().hCursor = LoadCursor(nullptr, IDC_ARROW);
    m_game_win_class->get_wnd_classex().hbrBackground = CreateSolidBrush(RGB(255, 255, 255));
    m_game_win_class->get_wnd_classex().lpszMenuName = nullptr;
    m_game_win_class->get_wnd_classex().hIconSm = LoadIcon(app::get_instance_handler(), MAKEINTRESOURCE(IDI_ICON1));


    for (auto& w : m_game_wins)
    {
        w = std::make_shared<game_win>(m_game_win_class);
    }

    // Initialize windows
    for (auto& w : m_game_wins)
    {
        w->init();
    }
    m_keyboard_win->init();


    // set input to blank as deafult
    for (int i = 0; i < Board::max_tile_count; ++i)
    {
        input[i] = ' ';
    }


}

app::~app()
{
}

int app::run()
{
    MSG msg{};
    BOOL msgResult = TRUE;
    HACCEL hAccelTable = LoadAccelerators(app::get_instance_handler(), MAKEINTRESOURCE(IDACC_LABTEMPLATE));
    while ((msgResult = GetMessage(&msg, nullptr, 0, 0)) != 0)
    {
        if (msgResult == -1)
            return EXIT_FAILURE;
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return EXIT_SUCCESS;
}

void app::init_font()
{
    if (AddFontResource(L".\\arial.ttf") == 0)
    {
        return;
    }

    LOGFONT lf;
    ZeroMemory(&lf, sizeof(lf));
    lf.lfHeight = 28;
    lf.lfWeight = FW_BOLD;
    _tcscpy_s(lf.lfFaceName, _T("Arial"));

    font_handler = CreateFontIndirect(&lf);

}

void app::load_words()
{
    std::ifstream file("Wordle.txt");
    if (!file.is_open())
    {
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (line.length() == Board::cols)
        {
            char word[Board::cols + 1];
            strncpy_s(word, line.c_str(), Board::cols);
            word[Board::cols] = '\0';
            for (int i = 0; i < Board::cols; i++)
                word[i] = toupper(word[i]);
            words.insert(word);
        }
    }

    file.close();
}

std::string app::random_word()
{
    std::mt19937 gen(std::random_device{}());
    std::string result;
    std::sample(words.begin(), words.end(), &result, 1, gen);
    return result;
}

bool app::is_word_valid(std::string word)
{
    if (words.find(word) == words.end())
    {
        return false;
    }
    return true;
}

void app::reset()
{
    m_game_wins[0]->set_word(random_word());
    m_game_wins[1]->set_word(random_word());
    m_game_wins[2]->set_word(random_word());
    m_game_wins[3]->set_word(random_word());
    game_ended = false;
}

void app::clear_input()
{
    for (int i = 0; i < Board::max_tile_count; ++i)
    {
        input[i] = ' ';
    }
}
