#pragma once
#include "framework.h"
#include <cmath>
//Common etc.
#include <CommCtrl.h>
#include <Commdlg.h>

#pragma comment( lib, "gdiplus.lib" )
#pragma warning( disable : 4018 )
#pragma warning( disable : 4996 )
#define GDIPVER 0x110
#include <gdiplus.h>
#include <gdiplusflat.h>
#include "resource.h"

//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>

typedef struct tagRectCl;
typedef struct tagRectCl {
    // long left;
    // long top;
    // long right;
    // long bottom;
    static RECT rectOut1;
    static  RECT rectOut2;
    static RECT rectOut3;
    static HWND ownerHwnd;
    //RECT RectCl() {};
    RECT RectCl(int ctrlIndex)
    {
        switch (ctrlIndex)
        {
        case 0:
        {
            RECT rect;
            GetClientRect(ownerHwnd, &rect);
            return rect;
        }
        case 1:
            return rectOut1;
        case 2:
            return rectOut2;
        case 3:
            return rectOut3;
        default:
            return {};
        }
    };
    RECT RectCl(HWND& hwndCtrl, HWND& ownHwnd, int ctrlIndex)
    {
        RECT rectIn;
        GetWindowRect(hwndCtrl, &rectIn); //get window rect of control relative to screen
        MapWindowPoints(NULL, ownHwnd, (LPPOINT)&rectIn, 2);
        switch (ctrlIndex)
        {
        case 1:
        {
            // init ownerHwnd
            ownerHwnd = ownHwnd;
            rectOut1 = rectIn;
            break;
        }
        case 2:
            rectOut2 = rectIn;
        case 3:
            rectOut3 = rectIn;
        }
        return rectIn;
    }

    int width(int ctrlIndex)
    {
        return RectCl(ctrlIndex).right - RectCl(ctrlIndex).left;
    }
    int height(int ctrlIndex)
    {
        return RectCl(ctrlIndex).bottom - RectCl(ctrlIndex).top;
    }

} RectCl;
