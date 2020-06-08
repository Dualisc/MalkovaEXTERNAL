#pragma once
#include "../../commons.h"
#include <DirectOverlay.h>
#define MAX_ITEMS 25

#define T_FOLDER 1
#define T_OPTION 2
#define T_INT 3
#define T_BOOL 4

#define LineH 15

struct Options {
    LPCWSTR Name;
    int* Function;
    BYTE Type;
    int limit;
};


struct Menu {
    LPCWSTR Title;
    int x;
    int y;
    int w;
};



class JBMenu
{
public:
    JBMenu(void);
    void Init_Menu(LPCWSTR Title, int x, int y);

    void Draw_Menu();
    void Navigation();
    void AddFolder(LPCWSTR Name, int* Pointer, int limit);
    void AddOption(LPCWSTR Name, int* Pointer, int* Folder, int limit, int type);

    bool IsReady();

    DWORD Color_Font;
    DWORD Color_On;
    DWORD Color_Off;
    DWORD Color_Folder;
    DWORD Color_Current;

public:
    bool Is_Ready, Visible;
    int Items, Cur_Pos;


    Options sOptions[MAX_ITEMS];
    Menu sMenu;

};