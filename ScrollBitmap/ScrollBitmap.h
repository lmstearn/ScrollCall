#pragma once
#define _WIN32_WINNT 0x0600 // (Post Vista)
// C4005: The SDK uses the latest (included in framework.h) and redefines it anyhow

#include "framework.h"
#include <cmath>
#include <vector>
#include <fstream>
//Common etc.
#include <CommCtrl.h>
#include <Commdlg.h>
#include <uxTheme.h>
#include <stdexcept>


#define GDIPVER 0x110
#include <objidl.h> //https://stackoverflow.com/questions/7305614/include-gdiplus-h-causes-error
#include <gdiplus.h>
#include <gdiplusflat.h>

#include "resource.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "gdiplus.lib" )
#pragma comment(lib, "UxTheme.lib")
#pragma warning( disable : 4018 )
#pragma warning( disable : 4996 )

//#pragma comment(lib, "C:\\Program Files\\Microsoft SDKs\\Windows\\vX.Y\\Lib\\ComCtl32.Lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

//Suppress warning on unused variables https://stackoverflow.com/a/1486931/2128797
#define UNUSED(expr) do { (void)(expr); } while (0)

#define MAX_ARRAY_LENGTH 4000000

//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>

/*For reference
typedef struct BitmapData {
    UINT width; //number of pixels in one scan line
    UINT height; // number of scan lines
    INT Stride; // stride or scan width
    PixelFormat PixelFormat;
    void* Scan0; //address of the first pixel data (first scan line)
    UINT_PTR Reserved;
};
*/
// Gdiplus

using std::runtime_error;
ULONG_PTR gdiplusToken;

struct GdiplusInit
{
    GdiplusInit()
    {
        Gdiplus::GdiplusStartupInput inp;
        Gdiplus::GdiplusStartupOutput outp;
        if (Gdiplus::Ok != Gdiplus::GdiplusStartup(&token_, &inp, &outp))
            throw runtime_error("GdiplusStartup");
    }
    ~GdiplusInit() // Destructor
    {
        Gdiplus::GdiplusShutdown(token_);
    }
private:
    ULONG_PTR token_;
}gdiplusInit;

typedef struct tagRectCl
{
    // long left;
    // long top;
    // long right;
    // long bottom;
    static RECT rectOut1;
    static RECT rectOut2;
    static RECT rectOut3;
    static RECT rectOut4;
    static RECT rectOut5;
    static RECT rectOut6;
    static HWND ownerHwnd;
    static RECT initRectOwnerHwnd;
    static RECT rectOwnerHwnd;
    //RECT RectCl() {};
    int ClMenuandTitle(HWND ownHwnd)
    {
        // Get usable client height
        RECT rectIn = {};
        RECT rectInCl = {};
        GetWindowRect(ownerHwnd, &rectIn);
        GetClientRect(ownerHwnd, &rectInCl);
        // Also menu bar height
        return (rectIn.bottom - rectIn.top) - (rectInCl.bottom - rectInCl.top) - GetSystemMetrics(SM_CYMENU);
    }

    RECT RectCl(int ctrlIndex)
    {
        switch (ctrlIndex)
        {
        case 0:
            return initRectOwnerHwnd;
        case 1:
            return rectOwnerHwnd;
        case 2:
            return rectOut1;
        case 3:
            return rectOut2;
        case 4:
            return rectOut3;
        case 5:
            return rectOut4;
        case 6:
            return rectOut5;
        case 7:
            return rectOut5;
        default:
            return {};
        }
    };
    RECT RectCl(HWND hwndCtrl, HWND& ownHwnd, int ctrlIndex)
    {
        RECT rectIn = {};
        if (ctrlIndex > 1)
        {
            GetWindowRect(hwndCtrl, &rectIn); //get window rect of control relative to screen
            MapWindowPoints(NULL, ownHwnd, (LPPOINT)&rectIn, 2);
        }
        switch (ctrlIndex)
        {
        case 0:
        {
            //if (initRectOwnerHwnd.right - initRectOwnerHwnd.left == 0)
            GetWindowRect(ownerHwnd, &initRectOwnerHwnd);
            rectIn = initRectOwnerHwnd;
            break;
        }
        case 1:
        {
            // init ownerHwnd
            ownerHwnd = ownHwnd;
            GetClientRect(ownerHwnd, &rectOwnerHwnd);
            rectIn = rectOwnerHwnd;
            break;
        }
        case 2:
        {
            rectOut1 = rectIn;
            break;
        }
        case 3:
        {
            rectOut2 = rectIn;
            break;
        }
        case 4:
        {
            rectOut3 = rectIn;
            break;
        }
        case 5:
        {
            rectOut4 = rectIn;
            break;
        }
        case 6:
        {
            rectOut5 = rectIn;
            break;
        }
        case 7:
        {
            rectOut5 = rectIn;
            break;
        }
        }
        return rectIn;
    }

    int width(int ctrlIndex)
    {
        if (ctrlIndex)
            return RectCl(ctrlIndex).right - RectCl(ctrlIndex).left;
        else
            return (int)(RectCl(ctrlIndex).right - RectCl(ctrlIndex).left) / CTRL_PROPORTION_OF_FORM;
    }
    int height(int ctrlIndex)
    {
        if (ctrlIndex)
            return RectCl(ctrlIndex).bottom - RectCl(ctrlIndex).top;
        else
            return (int)(RectCl(ctrlIndex).bottom - RectCl(ctrlIndex).top) / CTRL_PROPORTION_OF_FORM;
    }

} RectCl;
