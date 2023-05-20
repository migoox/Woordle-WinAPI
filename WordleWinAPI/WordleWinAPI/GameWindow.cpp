#include "GameWindow.h"
#include "resource.h"
#include "Application.h"

game_win::game_win(std::shared_ptr<win_class> wclass) : win(std::move(wclass))
{
    m_wnd_caption = L"WORDLE - PUZZLE";
    m_style = WS_OVERLAPPED | WS_CAPTION;
    m_extended_style = 0;

    RECT size{ 0, 0, Board::width, Board::max_height };
    AdjustWindowRectEx(&size, m_style, false, 0);

    m_wnd_handler = m_wnd_class->create_window(
        *this,
        CW_USEDEFAULT,
        0,
        size.right - size.left,
        size.bottom - size.top,
        app::get_keyboard_window()->get_wnd_handler(),
        nullptr);
}


void game_win::init()
{
    this->hide();
    m_word_guessed = false;
}

void game_win::reset()
{
    m_board = Board();
    m_word_guessed = false;
}

void game_win::set_word(const std::string& word)
{
    m_word = word;
}

std::array<letter_state, Board::cols> game_win::check_word(char* guess)
{
    std::array<letter_state, Board::cols> result;

    for (int i = 0; i < Board::cols; i++)
    {
        result[i] = letter_state::wrong_letter;
    }
    for (int i = 0; i < Board::cols; i++)
    {
        if (guess[i] == m_word[i])
        {
            result[i] = letter_state::correct;
            continue;
        }
        for (int j = 0; j < Board::cols; j++)
        {
            if (j == i) continue;

            if (guess[i] == m_word[j])
            {
                result[i] = letter_state::wrong_position;
            }
        }
    }

    bool guessed = true;
    for (auto& r : result)
    {
	    if (r != letter_state::correct)
	    {
            guessed = false;
            break;
	    }
    }
    if (guessed) m_word_guessed = true;

    return result;
}

void game_win::draw(HDC hdc)
{
    static HPEN ghost_pen = CreatePen(PS_SOLID, 0, app::frame_letter_color);

    static HPEN frame_pen = CreatePen(PS_SOLID, 2, app::frame_letter_color);
    static HBRUSH brush_none = CreateSolidBrush(app::blank_letter_color);

    static HBRUSH brush_correct = CreateSolidBrush(app::correct_letter_color);
    static HBRUSH brush_wrong_position = CreateSolidBrush(app::wrong_position_letter_color);
    static HBRUSH brush_wrong_letter = CreateSolidBrush(app::wrong_letter_color);

    int i = 0;
    for (auto& tile : m_board.Tiles())
    {
        if (tile.state == letter_state::none)
        {
			SelectObject(hdc, brush_none);
		    SelectObject(hdc, frame_pen);
        }
        else if (tile.state == letter_state::correct)
        {
            SelectObject(hdc, brush_correct);
            SelectObject(hdc, ghost_pen);
        }
        else if (tile.state == letter_state::wrong_position)
        {
            SelectObject(hdc, brush_wrong_position);
            SelectObject(hdc, ghost_pen);
        }
        else
        {
            SelectObject(hdc, brush_wrong_letter);
            SelectObject(hdc, ghost_pen);
        }
        

        RoundRect(hdc,
            tile.position.left,
            tile.position.top,
            tile.position.right,
            tile.position.bottom,
            7,
            7);
        SetBkMode(hdc, TRANSPARENT);
        //SetTextColor(hdc, RGB(255, 255, 255));
        DrawTextA(hdc, &app::get_input()[i++], 1, &tile.position, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
        //SetTextColor(hdc, RGB(0, 0, 0));
    }
}

void game_win::draw_overlay(HDC hdc)
{
    static const int cx = 4000;
    static const int cy = 4000;

    if (!app::is_ended()) return;

    static HDC overlayDC = CreateCompatibleDC(hdc);
    static HBITMAP mp = CreateCompatibleBitmap(hdc, cx, cy);
    static HBRUSH win_brush = CreateSolidBrush(RGB(0, 255, 0));
    static HBRUSH lose_brush = CreateSolidBrush(RGB(255, 0, 0));


    SelectObject(overlayDC, mp);

    BLENDFUNCTION bf;
    bf.BlendOp = AC_SRC_OVER;
    bf.SourceConstantAlpha = 160;
    bf.BlendFlags = 0;
    bf.AlphaFormat = 0;


    if (m_word_guessed)
		SelectObject(overlayDC, win_brush);
    else
        SelectObject(overlayDC, lose_brush);

    Rectangle(overlayDC, 0, 0, cx, cy);
    AlphaBlend(hdc, 
        0, 0, 
        cx, cy, 
        overlayDC, 
        0, 0, 
        cx, cy, bf);

    RECT rect;
    GetClientRect(m_wnd_handler, &rect);
    SetTextColor(hdc, RGB(255, 255, 255));
    DrawTextA(hdc, this->m_word.c_str(), -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
    SetTextColor(hdc, RGB(0, 0, 0));

}


LRESULT game_win::window_proc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    static int sizex = GetSystemMetrics(SM_CXSCREEN);
    static int sizey = GetSystemMetrics(SM_CYSCREEN);

    // TUTORIAL
    static HDC offDC = nullptr;
    static HBITMAP offOldBitmap = nullptr;
    static HBITMAP offBitmap = nullptr;
    switch (message)
    {
    case WM_CREATE:
    {
        HDC hdc = GetDC(m_wnd_handler);
        offDC = CreateCompatibleDC(hdc);
        ReleaseDC(m_wnd_handler, hdc);

        SetTimer(window, 7, 50, NULL);
        m_timer_start = GetTickCount64();
    }
    break;
    case WM_ERASEBKGND:
        return 1;
    case WM_TIMER:
    {

    }
    break;
    case WM_NCHITTEST:
    {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        ScreenToClient(m_wnd_handler, &pt);

        RECT rcClient;
        GetClientRect(m_wnd_handler, &rcClient);

        if (PtInRect(&rcClient, pt))
        {
            return HTCAPTION;
        }
    }
    break;
    case WM_SIZE:
    {// TUTORIAL
        int clientWidth = LOWORD(lParam);
        int clientHeight = HIWORD(lParam);
        HDC hdc = GetDC(m_wnd_handler);
        if (offOldBitmap != nullptr) 
        {
        	SelectObject(offDC, offOldBitmap);
        }
        if (offBitmap != nullptr) 
        {
            DeleteObject(offBitmap);
        }
        offBitmap = CreateCompatibleBitmap(hdc, clientWidth, clientHeight);
        offOldBitmap = (HBITMAP)SelectObject(offDC, offBitmap);
        ReleaseDC(m_wnd_handler, hdc);
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(window, &ps);
        RECT rc;
        GetClientRect(m_wnd_handler, &rc);

        HPEN oldpen = static_cast<HPEN>(SelectObject(offDC, nullptr));
        HBRUSH oldbrush = static_cast<HBRUSH>(SelectObject(offDC, nullptr));
        HFONT oldfont = static_cast<HFONT>(SelectObject(offDC, app::get_font_handler()));

        HBRUSH bckg_brush = CreateSolidBrush(app::background_color);
        SelectObject(offDC, bckg_brush);
    		Rectangle(offDC, -10, -10, rc.right + 10, rc.bottom + 10);


        this->draw(offDC);
        this->draw_overlay(offDC);

        SelectObject(offDC, oldfont);
        SelectObject(offDC, oldpen);
        SelectObject(offDC, oldbrush);

        BitBlt(hdc, 0, 0, rc.right, rc.bottom, offDC, 0, 0, SRCCOPY);
        EndPaint(window, &ps);
    }
    break;
    case WM_DESTROY:
        if (offOldBitmap != nullptr)
        {
            SelectObject(offDC, offOldBitmap);  
        }
        if (offDC != nullptr)
        {
            DeleteDC(offDC);
        }
        if (offBitmap != nullptr)
        {
            DeleteObject(offBitmap);
        }

        PostQuitMessage(0);
        break;
    case WM_WINDOWPOSCHANGED:
        break;
    case WM_DISPLAYCHANGE:
        //mScreenCentre = { GetSystemMetrics(SM_CXSCREEN) / 2, GetSystemMetrics(SM_CYSCREEN) / 2 };
        break;
    default:
        return DefWindowProc(window, message, wParam, lParam);
    }
    return 0;
}

void game_win::set_difficulty_overlay(difficulty d)
{
    if (d == difficulty::easy)
    {
        RECT size{ 0, 0, Board::width, Board::easy_height };
        AdjustWindowRectEx(&size, m_style, false, 0);

        RECT w;
        GetWindowRect(m_wnd_handler, &w);
        MoveWindow(m_wnd_handler, w.left, w.top, size.right - size.left, size.bottom - size.top, TRUE);

    }
    else if (d == difficulty::medium)
    {
        RECT size{ 0, 0, Board::width, Board::medium_height };
        AdjustWindowRectEx(&size, m_style, false, 0);

        RECT w;
        GetWindowRect(m_wnd_handler, &w);
        MoveWindow(m_wnd_handler, w.left, w.top, size.right - size.left, size.bottom - size.top, TRUE);
    }
    else
    {
        RECT size{ 0, 0, Board::width, Board::hard_height };
        AdjustWindowRectEx(&size, m_style, false, 0);

        RECT w;
        GetWindowRect(m_wnd_handler, &w);
        MoveWindow(m_wnd_handler, w.left, w.top, size.right - size.left, size.bottom - size.top, TRUE);
    }
}