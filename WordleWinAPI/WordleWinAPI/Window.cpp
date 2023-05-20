#include "Window.h"
#include "Application.h"

win::win(std::shared_ptr<win_class> wclass) :
	m_wnd_class(std::move(wclass))
{
    m_wnd_class->register_class();
}

LRESULT win::s_window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    win* w = nullptr;
    if (message == WM_NCCREATE)
    {
        w = static_cast<win*>(
            reinterpret_cast<LPCREATESTRUCTW>(lparam)->lpCreateParams);
        SetWindowLongPtrW(window, GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(w));

        LRESULT res = w ?
            w->window_proc(window, message, wparam, lparam) :
            DefWindowProcW(window, message, wparam, lparam);
    }
    else
    {
        w = reinterpret_cast<win*>(
            GetWindowLongPtrW(window, GWLP_USERDATA));
    }

    LRESULT res = w ?
        w->window_proc(window, message, wparam, lparam) :
        DefWindowProcW(window, message, wparam, lparam);
    if (message == WM_NCDESTROY)
        SetWindowLongPtrW(window, GWLP_USERDATA, 0);
    return res;

}

void win::show()
{
    ShowWindow(m_wnd_handler, SW_SHOW);
	UpdateWindow(m_wnd_handler);
}

void win::hide()
{
    ShowWindow(m_wnd_handler, SW_HIDE);
    UpdateWindow(m_wnd_handler);
}

void win::activate_show_command(int showCommand)
{
    ShowWindow(m_wnd_handler, showCommand);
    UpdateWindow(m_wnd_handler);
}

void win::send_draw_message()
{
    InvalidateRect(m_wnd_handler, NULL, TRUE);
    UpdateWindow(m_wnd_handler);
}

win_class::win_class(const wchar_t* name) :
	m_name(name),
	m_is_registered(false),
	m_wnd_class()
{
	
}

bool win_class::register_class()
{
    if (m_is_registered) return true;

    m_wnd_class.cbSize = sizeof(WNDCLASSEX);
    m_wnd_class.lpfnWndProc = win::s_window_proc;
    m_wnd_class.hInstance = app::get_instance_handler();
    m_wnd_class.lpszClassName = m_name;

    m_is_registered = RegisterClassEx(&m_wnd_class) != 0;

    return m_is_registered;
}

HWND win_class::create_window(win& window, INT posX, INT posY, INT sizeX, INT sizeY, HWND parent, HMENU hmenu)
{
    HWND requestedWindow = CreateWindowEx(
        window.m_extended_style,
        m_name,
        window.m_wnd_caption.c_str(),
        window.m_style,
        posX,
        posY,
        sizeX,
        sizeY,
        parent,
        hmenu,
        app::get_instance_handler(),
        &window);

    return requestedWindow;
}
