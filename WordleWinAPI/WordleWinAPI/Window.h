#pragma once
#include "windows.h"
#include <memory>
#include <string>

class win_class;

class win
{
public:
    friend win_class;

    win(std::shared_ptr<win_class> wclass);
    ~win() = default;

    LRESULT static CALLBACK s_window_proc(
        HWND window,
        UINT message,
        WPARAM wparam,
        LPARAM lparam);

    virtual LRESULT window_proc(
        HWND window,
        UINT message, 
        WPARAM wParam, 
        LPARAM lParam) = 0;

    HWND& get_wnd_handler() { return m_wnd_handler; }

    void show();
    void hide();
    void activate_show_command(int showCommand);
    void send_draw_message();
    virtual void init() = 0;
protected:
    std::shared_ptr<win_class> m_wnd_class;
    HWND m_wnd_handler;
    DWORD m_extended_style;
    DWORD m_style;
    HMENU m_menu_handler;
    std::wstring m_wnd_caption;
};

class win_class
{
public:
    win_class(const wchar_t* name);
    bool register_class();
    bool is_registered() const { return m_is_registered; }
    WNDCLASSEX& get_wnd_classex() { return m_wnd_class; }
    HWND create_window(win& window, INT posX, INT posY, INT sizeX, INT sizeY, HWND parent, HMENU hmenu);

private:
    const wchar_t* m_name;
    bool m_is_registered;
    WNDCLASSEX m_wnd_class;
};

