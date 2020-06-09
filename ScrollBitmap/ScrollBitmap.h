#pragma once
#include "framework.h"
#include <cmath>
#include <vector>
#include <fstream>
//Common etc.
#include <CommCtrl.h>
#include <Commdlg.h>
#include <stdexcept>


#define GDIPVER 0x110
#include <objidl.h> //https://stackoverflow.com/questions/7305614/include-gdiplus-h-causes-error
#include <gdiplus.h>
#include <gdiplusflat.h>

#include "resource.h"
#pragma comment(lib, "comctl32.lib")
#pragma comment( lib, "gdiplus.lib" )
#pragma warning( disable : 4018 )
#pragma warning( disable : 4996 )

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
using namespace Gdiplus;
using namespace Gdiplus::DllExports;
using std::runtime_error;
ULONG_PTR gdiplusToken;

struct GdiplusInit
{
    GdiplusInit()
    {
        GdiplusStartupInput inp;
        GdiplusStartupOutput outp;
        if (Ok != GdiplusStartup(&token_, &inp, &outp))
            throw runtime_error("GdiplusStartup");
    }
    ~GdiplusInit() // Destructor
    {
        GdiplusShutdown(token_);
    }
private:
    ULONG_PTR token_;
}gdiplusInit;

typedef struct tagRectCl {
    // long left;
    // long top;
    // long right;
    // long bottom;
    static RECT rectOut1;
    static RECT rectOut2;
    static RECT rectOut3;
    static HWND ownerHwnd;
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
                return rectOwnerHwnd;
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
    RECT RectCl(HWND hwndCtrl, HWND& ownHwnd, int ctrlIndex)
    {
        RECT rectIn = {};
        if (ctrlIndex)
        {
            GetWindowRect(hwndCtrl, &rectIn); //get window rect of control relative to screen
            MapWindowPoints(NULL, ownHwnd, (LPPOINT)&rectIn, 2);
        }
        switch (ctrlIndex)
        {
        case 0:
        {
            // init ownerHwnd
            ownerHwnd = ownHwnd;
            GetClientRect(ownerHwnd, &rectOwnerHwnd);
        }
        case 1:
        {
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
