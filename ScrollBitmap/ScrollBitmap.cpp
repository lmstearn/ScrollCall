// Gdiplus 
#include "ScrollBitmap.h"
RECT RectCl::rectOut1 = {};
RECT RectCl::rectOut2 = {};
RECT RectCl::rectOut3 = {};
HWND RectCl::ownerHwnd = 0;

#define MAX_LOADSTRING 255
#define FOURTHREEVID					12
#define WIDESCREENVID				16
#define UNIVISIUM						18 // e.g. Galaxy Tabs
#define ULTRAWIDEVID					21
#define DESIGNSCRX						1360
#define DESIGNSCRY						768
using namespace Gdiplus;
using namespace Gdiplus::DllExports;

// Global Variables:
RECT rcWindow;
SCROLLINFO si = {0};
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
wchar_t* szFile = nullptr;
float scrAspect = 0, scaleX = 1, scaleY = 1,  resX = 0, resY = 0;
int tmp = 0, wd = 0, ht = 0;
HWND hWndButton = 0, hWndOpt1 =0, hWndOpt2 =0;
ULONG_PTR gdiplusToken;
BOOL optChk = TRUE;
RECT rectB, rectO1, rectO2;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    MyBitmapWindowProc(HWND, UINT, WPARAM, LPARAM);
void GetWdHt(HWND hWnd);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
float DoSysInfo(HWND hWnd, bool progLoad);
wchar_t* FileOpener(HWND hWnd);
void message(const wchar_t* format, ...);



int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
_In_opt_ HINSTANCE hPrevInstance,
_In_ LPWSTR    lpCmdLine,
_In_ int       nCmdShow)
{
UNREFERENCED_PARAMETER(hPrevInstance);
UNREFERENCED_PARAMETER(lpCmdLine);
//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
// TODO: Place code here.
// Initialize global strings
LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
LoadStringW(hInstance, IDC_SCROLLBITMAP, szWindowClass, MAX_LOADSTRING);
MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    return FALSE;


HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SCROLLBITMAP));

MSG msg;

// Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        }
    }

return (int) msg.wParam;
}

  

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
WNDCLASSEXW wcex;

wcex.cbSize = sizeof(WNDCLASSEX);

wcex.style          = CS_HREDRAW | CS_VREDRAW;
wcex.lpfnWndProc    = MyBitmapWindowProc;
wcex.cbClsExtra     = 0;
wcex.cbWndExtra     = 0;
wcex.hInstance      = hInstance;
wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SCROLLBITMAP));
wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SCROLLBITMAP);
wcex.lpszClassName  = szWindowClass;
wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
hInst = hInstance; // Store instance handle in our global variable

HWND m_hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW
| WS_CLIPCHILDREN,
CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!m_hWnd)
    return FALSE;


ShowWindow(m_hWnd, nCmdShow);
UpdateWindow(m_hWnd);

return TRUE;
}

//
//  FUNCTION: MyBitmapWindowProc(HWND, UINT, WPARAM, LPARAM)
//  This function is called by the Windows function DispatchMessage( )
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
LRESULT CALLBACK MyBitmapWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
HDC hdc;
PAINTSTRUCT ps;

// These variables are required by BitBlt. 
static HDC hdcWinCl;            // window DC 
static  HDC hdcMem;            // Mem DC 
static HDC hdcScreen;           // DC for entire screen 
static HDC hdcScreenCompat;     // memory DC for screen 
static HBITMAP hbmpCompat;      // bitmap handle to old DC 
static BITMAP bmp;          // bitmap data structure 
static UINT bmpWidth = 0;
static UINT bmpHeight = 0;
static BOOL fBlt;           // TRUE if BitBlt occurred 
static int fScroll;             // 1 if horz scrolling, -1 vert scrolling, 0 for WM_SIZE
static BOOL fSize;          // TRUE if fBlt & WM_SIZE 
static BOOL isScreenshot;   // TRUE if fBlt & WM_SIZE 

// These variables are required for horizontal scrolling.
static int xMinScroll;      // minimum horizontal scroll value
static int xCurrentScroll;      // current horizontal scroll value
static int  xOldScroll;     // last horizontal scroll value
static int xMaxScroll;      // maximum horizontal scroll value

// These variables are required for vertical scrolling.
static int yMinScroll;      // minimum vertical scroll value
static int yCurrentScroll;      // current vertical scroll value
static int yOldScroll;      // last vertical scroll value 
static int yMaxScroll;      // maximum vertical scroll value
static int xTrackPos;   // current scroll drag value
static int yTrackPos;
   
static int  iDeltaPerLine;      // for mouse wheel logic
static int iAccumDelta;
static ULONG ulScrollLines;

static UINT SMOOTHSCROLL_FLAG;
static UINT SMOOTHSCROLL_SPEED;

    switch (uMsg)
    {
    case WM_CREATE:
    {
    // Start Gdiplus
    si.cbSize = 0;
    si.cbSize = 0;
    SMOOTHSCROLL_SPEED = 0X00000002;
    ulScrollLines = 0;
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    GetWdHt(hWnd);

    hWndButton = CreateWindowW(
    L"BUTTON",  // Predefined class; Unicode assumed 
    L"Open\nImage",      // Button text 
    WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_MULTILINE,  // Styles 
    2,         // x position 
    2,         // y position 
    wd,        // Button width
    ht,        // Button height
    hWnd,     // Parent window
    (HMENU)ID_OPENBITMAP,       // CTRL ID.
    (HINSTANCE)NULL,
    NULL);      // Pointer not needed.

    // Radio Option
 
    hWndOpt1= CreateWindowEx(WS_EX_WINDOWEDGE,
        L"BUTTON",
        L"Scroll",
        WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP,  // <---- WS_GROUP group the following radio buttons 1st,2nd button 
        2, ht + 10,
        wd-2, ht/2,
        hWnd, //<----- Use main window handle
        (HMENU)IDC_OPT1,
        (HINSTANCE)NULL, NULL);
    hWndOpt2 = CreateWindowEx(WS_EX_WINDOWEDGE,
        L"BUTTON",
        L"ScrollEx",
        WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,  // Styles 
        2, 2 * ht,
        wd-2, ht/2,
        hWnd,
        (HMENU)IDC_OPT2,
        (HINSTANCE)NULL, NULL);
        SendMessage(hWndOpt1, BM_SETCHECK, BST_CHECKED, 0);
 
 
    // Create a normal DC and a memory DC for the entire 
    // screen. The normal DC provides a snapshot of the 
    // screen contents. The memory DC keeps a copy of this 
    // snapshot in the associated bitmap. 
    // wd += 10;
    isScreenshot = FALSE;
    hdcScreen = CreateDC(L"DISPLAY", (PCTSTR)NULL,
    (PCTSTR)NULL, (CONST DEVMODE*) NULL);
    hdcScreenCompat = CreateCompatibleDC(hdcScreen);

    // Retrieve the metrics for the bitmap associated with the 
    // regular device context. 
    bmp.bmBitsPixel = (BYTE)GetDeviceCaps(hdcScreen, BITSPIXEL);
    bmp.bmPlanes = (BYTE)GetDeviceCaps(hdcScreen, PLANES);
    bmp.bmWidth = GetDeviceCaps(hdcScreen, HORZRES);
    bmp.bmHeight = GetDeviceCaps(hdcScreen, VERTRES);

    // The width must be byte-aligned. 
    bmp.bmWidthBytes = ((bmp.bmWidth + 15) & ~15) / 8;

    // Create a bitmap for the compatible DC. 
    hbmpCompat = CreateBitmap(bmp.bmWidth, bmp.bmHeight,
    bmp.bmPlanes, bmp.bmBitsPixel, (CONST VOID*) NULL);

    // Select the bitmap for the compatible DC. 
    SelectObject(hdcScreenCompat, hbmpCompat);

    // Initialize the flags. 
    fBlt = FALSE;
    fScroll = 0;
    fSize = FALSE;

    // Initialize the horizontal scrolling variables. 
    xMinScroll = 0;
    xCurrentScroll = 0;
    xOldScroll = 0;
    xMaxScroll = 0;

    // Initialize the vertical scrolling variables. 
    yMinScroll = 0;
    yCurrentScroll = 0;
    yMaxScroll = 0;

    //SMOOTHSCROLL_FLAG = MAKELRESULT((USHORT)SW_SMOOTHSCROLL, SMOOTHSCROLL_SPEED);
    SMOOTHSCROLL_FLAG = SW_SMOOTHSCROLL  | SMOOTHSCROLL_SPEED;

        
        if (SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &ulScrollLines, 0))
        {
        // ulScrollLines usually equals 3 or 0 (for no scrolling)
        // WHEEL_DELTA equals 120, so iDeltaPerLine will be 40
            if (ulScrollLines)
            iDeltaPerLine = WHEEL_DELTA / ulScrollLines;
            else
            iDeltaPerLine = 0;
        }
        else
        MessageBoxExW(hWnd, L"SPI_GETWHEELSCROLLLINES: Cannot get info.", 0, 0, 0);
        return 0;
        break;
    }
    case WM_SIZE:
    {
    int xNewSize;
    int yNewSize;

    xNewSize = LOWORD(lParam);
    yNewSize = HIWORD(lParam);


    GetWdHt(hWnd);
    RECT rectBtmp = RectCl().RectCl(hWndButton, hWnd, 1);
    GetClientRect(hWndButton, &rectB);
    //rectB.top += BOX_HEIGHT;
    SetWindowPos(hWndButton, NULL, scaleX * (rectBtmp.left), scaleY * rectBtmp.top,
    scaleX * (rectB.right - rectB.left), scaleY * (rectB.bottom - rectB.top), NULL);
    
    RECT rectOpt1tmp = RectCl().RectCl(hWndOpt1, hWnd, 2);
    GetClientRect(hWndOpt1, &rectO1);
    //Extra edging for the wd - 2
    SetWindowPos(hWndOpt1, NULL, scaleX * (rectOpt1tmp.left), scaleY * (rectOpt1tmp.top),
    scaleX * (rectB.right - rectB.left -2), scaleY * (rectO1.bottom - rectO1.top), NULL);
    RECT rectOpt2tmp = RectCl().RectCl(hWndOpt2, hWnd, 3);
    GetClientRect(hWndOpt2, &rectO2);
    SetWindowPos(hWndOpt2, NULL, scaleX * (rectOpt2tmp.left), scaleY * (rectOpt2tmp.top),
    scaleX * (rectB.right - rectB.left - 2), scaleY * (rectO2.bottom - rectO2.top), NULL);


    if (fBlt)
    fSize = TRUE;
    // The horizontal scrolling range is defined by 
    // (bitmap_width) - (client_width). The current horizontal 
    // scroll value remains within the horizontal scrolling range. 
    xMaxScroll = max(bmp.bmWidth - xNewSize, 0);
    xCurrentScroll = min(xCurrentScroll, xMaxScroll);
    si.cbSize = sizeof(si);
    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    si.nMin = xMinScroll;
    si.nMax = bmp.bmWidth;
    si.nPage = xNewSize;
    si.nPos = xCurrentScroll;
    SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);

    // The vertical scrolling range is defined by 
    // (bitmap_height) - (client_height). The current vertical 
    // scroll value remains within the vertical scrolling range. 
    yMaxScroll = max(bmp.bmHeight - yNewSize, 0);
    yCurrentScroll = min(yCurrentScroll, yMaxScroll);
    si.cbSize = sizeof(si);
    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    si.nMin = yMinScroll;
    si.nMax = bmp.bmHeight;
    si.nPage = yNewSize;
    si.nPos = yCurrentScroll;
    si.nTrackPos = yTrackPos;
    SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
    return 0;
    break;
    }
    case WM_PAINT:
    {
    PAINTSTRUCT ps;
    hdc = BeginPaint(hWnd, &ps);


    // If scrolling has occurred, use the following call to 
    // BitBlt to paint the invalid rectangle. 
    // 
    // The coordinates of this rectangle are specified in the 
    // RECT structure to which prect points. 
    // 
    // Note that it is necessary to increment the seventh 
    // argument (prect->left) by xCurrentScroll and the 
    // eighth argument (prect->top) by yCurrentScroll in 
    // order to map the correct pixels from the source bitmap. 
    if (fScroll)
    {

    PRECT prect;
    prect = &ps.rcPaint;
    BitBlt(ps.hdc,
        prect->left + (isScreenshot ? 0 : (((fScroll == 1) && (xCurrentScroll > wd)) ? 0: wd * scaleX)),
        prect->top,
        //prect->left + ( (isScreenshot) ? 0 : wd * scaleX), prect->top,
        (prect->right - prect->left),
        (prect->bottom - prect->top),
        isScreenshot ? hdcScreenCompat: hdcMem,
        prect->left + xCurrentScroll,
        prect->top + yCurrentScroll,
        SRCCOPY);

    if (fScroll == -1 && xCurrentScroll < scaleX * wd)
    {

            if (yCurrentScroll > RectCl().RectCl(3).bottom)
            {
            RECT rect;
            rect.top = (yCurrentScroll - yOldScroll > 0)? RectCl().RectCl(3).bottom: 0;
            rect.bottom = RectCl().RectCl(0).bottom;
            rect.left = prect->left;
            rect.right = prect->left + (scaleX * wd) - xCurrentScroll;
            FillRect(ps.hdc, &rect, (HBRUSH)(COLOR_WINDOW + 1));
            //UpdateWindow(hWnd);
            }
            else
            {
                int y = RectCl().RectCl(3).top;
                int y3T = max(RectCl().RectCl(3).top, yCurrentScroll);
                int y2B = max(RectCl().RectCl(2).bottom, yCurrentScroll);
                int y2T = max(RectCl().RectCl(2).top, yCurrentScroll);
                int y1B = max(RectCl().RectCl(1).bottom, yCurrentScroll);
                if (yCurrentScroll > RectCl().RectCl(3).top)
                {
                    if (yOldScroll < y3T)
                    {
                        RECT rect;
                        rect.top = RectCl().RectCl(3).bottom;
                        rect.bottom = RectCl().RectCl(0).bottom;
                        rect.left = prect->left;
                        rect.right = prect->left + (scaleX * wd) - xCurrentScroll;
                        FillRect(ps.hdc, &rect, (HBRUSH)(COLOR_WINDOW + 1));
                        if (yOldScroll < y2T)
                        {
                            RECT rect;
                            rect.top = RectCl().RectCl(2).bottom;
                            rect.bottom = RectCl().RectCl(3).top;
                            rect.left = prect->left;
                            rect.right = prect->left + (scaleX * wd) - xCurrentScroll;
                            FillRect(ps.hdc, &rect, (HBRUSH)(COLOR_WINDOW + 1));
                        }
                    }
                }
                else // yCurrentScroll <= RectCl().RectCl(3).top)
                {
                    if (yOldScroll < y3T)
                    {
                        RECT rect;
                        rect.top = RectCl().RectCl(3).bottom;
                        rect.bottom = RectCl().RectCl(0).bottom;
                        rect.left = prect->left;
                        rect.right = prect->left + (scaleX * wd) - xCurrentScroll;
                        FillRect(ps.hdc, &rect, (HBRUSH)(COLOR_WINDOW + 1));
                        if (yOldScroll < y2T)
                        {
                            RECT rect;
                            rect.top = RectCl().RectCl(2).bottom;
                            rect.bottom = RectCl().RectCl(3).top;
                            rect.left = prect->left;
                            rect.right = prect->left + (scaleX * wd) - xCurrentScroll;
                            FillRect(ps.hdc, &rect, (HBRUSH)(COLOR_WINDOW + 1));
                        }
                    }
                    else
                    {
                        RECT rect;
                        rect.top = yOldScroll;
                        rect.bottom = y3T;
                        rect.left = prect->left;
                        rect.right = prect->left + (scaleX * wd) - xCurrentScroll;
                        FillRect(ps.hdc, &rect, (HBRUSH)(COLOR_WINDOW + 1));
                    }
                }

            }
    }

    /*


    if (fScroll == 1)
        {
            if (xOldScroll - xCurrentScroll > 0)
            {
                if (xCurrentScroll > wd)
                BitBlt(ps.hdc,
                    prect->left, prect->top,
                    (prect->right - prect->left),
                    (prect->bottom - prect->top),
                    hdcScreenCompat,
                    prect->left + xCurrentScroll,
                    prect->top + yCurrentScroll,
                    SRCCOPY);
                else
                BitBlt(ps.hdc,
                    wd - xCurrentScroll + prect->left, prect->top,
                    (wd - xCurrentScroll + prect->right - prect->left),
                    (prect->bottom - prect->top),
                    hdcScreenCompat,
                    prect->left + xCurrentScroll,
                    prect->top + yCurrentScroll,
                    SRCCOPY);
            }
            else
            {
                BitBlt(ps.hdc,
                    prect->left, prect->top,
                    (prect->right - prect->left),
                    (prect->bottom - prect->top),
                    hdcScreenCompat,
                    prect->left + xCurrentScroll,
                    prect->top + yCurrentScroll,
                    SRCCOPY);
            }
        }
        else
        {
            if (xCurrentScroll > wd)
            BitBlt(ps.hdc,
            prect->left, prect->top,
            (prect->right - prect->left),
            (prect->bottom - prect->top),
            hdcScreenCompat,
            prect->left + xCurrentScroll,
            prect->top + yCurrentScroll,
            SRCCOPY);
        else
        {
            BitBlt(ps.hdc,
                wd - xCurrentScroll + prect->left, prect->top,
                (xCurrentScroll - wd + prect->right - prect->left),
                (prect->bottom - prect->top),
                hdcScreenCompat,
                prect->left + xCurrentScroll,
                prect->top + yCurrentScroll,
                SRCCOPY);
        }

        }*/
        /* If the window has been resized and the user has
       // captured the screen, use the following call to
       // BitBlt to paint the window's client area.
           x
           The x - coordinate, in logical units, of the upper - left corner of the destination rectangle.
           y
           The y - coordinate, in logical units, of the upper - left corner of the destination rectangle.
           cx
           The width, in logical units, of the source and destination rectangles.
           cy
           The height, in logical units, of the source and the destination rectangles.
           hdcSrc
           A handle to the source device context.
           x1
           The x - coordinate, in logical units, of the upper - left corner of the source rectangle.
           y1
           The y - coordinate, in logical units, of the upper - left corner of the source rectangle.
           */

    if (fSize)
    {
        BitBlt(ps.hdc,
            0, 0,
            bmp.bmWidth, bmp.bmHeight,
            hdcScreenCompat,
            xCurrentScroll, yCurrentScroll,
            SRCCOPY);

        fSize = FALSE;
    }

    fScroll = 0;
    }

    EndPaint(hWnd, &ps);
    return 0;
    break;
    }

    case WM_HSCROLL:
    {
    int xDelta;     // xDelta = new_pos - current_pos  
    int xNewPos;    // new position 
    int yDelta = 0;
    si.cbSize = sizeof(si);
    si.fMask = SIF_TRACKPOS;
    GetScrollInfo(hWnd, SB_HORZ, &si);
    xTrackPos = si.nTrackPos;

        switch (LOWORD(wParam))
        {
        // User clicked the scroll bar shaft left of the scroll box. 
        case SB_PAGEUP:
        xNewPos = xCurrentScroll - 50;
        break;

        // User clicked the scroll bar shaft right of the scroll box. 
        case SB_PAGEDOWN:
        xNewPos = xCurrentScroll + 50;
        break;

        // User clicked the left arrow. 
        case SB_LINEUP:
        xNewPos = xCurrentScroll - 5;
        break;

        // User clicked the right arrow. 
        case SB_LINEDOWN:
        xNewPos = xCurrentScroll + 5;
        break;
        case SB_THUMBTRACK:
        xNewPos = xTrackPos;
        break;
         // User dragged the scroll box. 
        case SB_THUMBPOSITION:
        xNewPos = HIWORD(wParam);
        break;
        default:
        xNewPos = xCurrentScroll;
        }

    // New position must be between 0 and the screen width. 
    xNewPos = max(0, xNewPos);
    xNewPos = min(xMaxScroll, xNewPos);

    // If the current position does not change, do not scroll.
    if (xNewPos == xCurrentScroll)
        return 0;


    fScroll = 1;

    // Determine the amount scrolled (in pixels). 
    xDelta = xNewPos - xCurrentScroll;

    // Reset the current scroll position. 
    xOldScroll = xCurrentScroll;
    xCurrentScroll = xNewPos;

    // Scroll the window. (The system repaints most of the 
    // client area when ScrollWindowEx is called; however, it is 
    // necessary to call UpdateWindow in order to repaint the 
    // rectangle of pixels that were invalidated.) 
    if (optChk)
        ScrollWindow(hWnd, -xDelta, -yDelta, (CONST RECT*) NULL, (CONST RECT*) NULL);
    else
    {
        ScrollWindowEx(hWnd, -xDelta, -yDelta, (CONST RECT*) NULL,
            (CONST RECT*) NULL, (HRGN)NULL, (PRECT)NULL, SW_SCROLLCHILDREN | SW_INVALIDATE); // SMOOTHSCROLL_FLAG fails
        UpdateWindow(hWnd);
    }
    // Reset the scroll bar. 
    si.cbSize = sizeof(si);
    si.fMask = SIF_POS;
    si.nPos = xCurrentScroll;
    SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);
    return 0;
    break;
    }

    case WM_VSCROLL:
    {
    si.cbSize = sizeof(si);
    si.fMask = SIF_TRACKPOS;
    GetScrollInfo(hWnd, SB_VERT, &si);
    yTrackPos = si.nTrackPos;
    int xDelta = 0;
    int yDelta;     // yDelta = new_pos - current_pos 
    int yNewPos;    // new position 

    switch (LOWORD(wParam))
    {
    // User clicked the scroll bar shaft above the scroll box. 
    case SB_PAGEUP:
    yNewPos = yCurrentScroll - 50;
    break;

    // User clicked the scroll bar shaft below the scroll box. 
    case SB_PAGEDOWN:
    yNewPos = yCurrentScroll + 50;
    break;

    // User clicked the top arrow. 
    case SB_LINEUP:
    yNewPos = yCurrentScroll - 5;
    break;

    // User clicked the bottom arrow. 
    case SB_LINEDOWN:
    yNewPos = yCurrentScroll + 5;
    break;
    case SB_THUMBTRACK:
    yNewPos = yTrackPos;
    break;
    // User dragged the scroll box. 
    case SB_THUMBPOSITION:
    yNewPos = HIWORD(wParam);
    break;

    default:
    yNewPos = yCurrentScroll;
    }

    // New position must be between 0 and the screen height. 
    yNewPos = max(0, yNewPos);
    yNewPos = min(yMaxScroll, yNewPos);

    // If the current position does not change, do not scroll.
    if (yNewPos == yCurrentScroll)
        return 0;
    // Set the scroll flag to TRUE. 
    fScroll = -1;

    // Determine the amount scrolled (in pixels). 
    yDelta = yNewPos - yCurrentScroll;

    // Reset the current scroll position. 
    yOldScroll = yCurrentScroll;
    yCurrentScroll = yNewPos;
    
    // Scroll the window. (The system repaints most of the 
    // client area when ScrollWindowEx is called; however, it is 
    // necessary to call UpdateWindow in order to repaint the 
    // rectangle of pixels that were invalidated.) 
    if (optChk)
        ScrollWindow(hWnd, -xDelta, -yDelta, (CONST RECT*) NULL, (CONST RECT*) NULL);
    else
    {
        ScrollWindowEx(hWnd, -xDelta, -yDelta, (CONST RECT*) NULL,
            (CONST RECT*) NULL, (HRGN)NULL, (PRECT)NULL, SW_SCROLLCHILDREN | SW_INVALIDATE); // SMOOTHSCROLL_FLAG fails
        UpdateWindow(hWnd);
    }

    // Reset the scroll bar. 
    si.fMask = SIF_POS;
    si.nPos = yCurrentScroll;
    SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
    return 0;
    break;
    }
    case WM_MOUSEWHEEL:
    {
        if (iDeltaPerLine == 0)
            return 0;

        iAccumDelta += (short)HIWORD(wParam);     // 120 or -120
        while (iAccumDelta >= iDeltaPerLine)
        {
            SendMessage(hWnd, WM_VSCROLL, SB_LINEUP, 0);
            iAccumDelta -= iDeltaPerLine;
        }

        while (iAccumDelta <= -iDeltaPerLine)
        {
            SendMessage(hWnd, WM_VSCROLL, SB_LINEDOWN, 0);
            iAccumDelta += iDeltaPerLine;
        }
        return 0;
        break;
    }
    case WM_COMMAND:
    {
        USHORT wmId = LOWORD(wParam);
        USHORT wmEvent = HIWORD(wParam);
        switch (wmId)
        {
        case IDC_OPT1:
        {
            if (wmEvent == BN_CLICKED)
                optChk = (SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED);
            break;
        }
        case IDC_OPT2:
        {
            if (wmEvent == BN_CLICKED)
                optChk = (SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) != BST_CHECKED);
            // Also BST_INDETERMINATE, BST_UNCHECKED
            return 0;
            break;
        }
        case ID_OPENBITMAP:
        {
            //https://social.msdn.microsoft.com/Forums/sqlserver/en-US/76441f64-a7f2-4483-ad6d-f51b40464d6b/how-to-get-both-width-and-height-of-bitmap-in-gdiplus-flat-api?forum=windowsgeneraldevelopmentissues
            GpBitmap* pgpbm = nullptr;
            Color clr;
            szFile = (wchar_t*)calloc(MAX_LOADSTRING, sizeof(wchar_t));
            wcscpy_s(szFile, MAX_LOADSTRING, FileOpener(hWnd));
            if (szFile[0] != L'*')
            {
                hdcWinCl = GetDC(hWnd);
                GpStatus gps = GdipCreateBitmapFromFile(szFile, &pgpbm);
                if (gps == Ok)
                {
                    HBITMAP hBitmap = NULL;
                    gps = GdipCreateHBITMAPFromBitmap(pgpbm, &hBitmap, clr.GetValue());

                    gps = GdipGetImageWidth(pgpbm, &bmpWidth);
                    gps = GdipGetImageHeight(pgpbm, &bmpHeight);
 
                    hdcMem = CreateCompatibleDC(hdcWinCl);

                    SelectObject(hdcMem, hBitmap);
                    if (!BitBlt(hdcWinCl, wd* scaleX, 0, bmpWidth, bmpHeight, hdcMem, 0, 0, SRCCOPY))
                    MessageBoxExW(hWnd, L"Bad BitBlt from hdcMem!", 0, 0, 0);
                    // Now to "translate" the memory DC co-ords
                    HDC hdcMemtmp = CreateCompatibleDC(hdcWinCl);
                    SelectObject(hdcMemtmp, hBitmap);

                    if (!BitBlt(hdcMem, wd * scaleX, 0, bmpWidth, bmpHeight, hdcMemtmp, 0, 0, SRCCOPY))
                    MessageBoxExW(hWnd, L"Bad BitBlt from hdcMemtmp!", 0, 0, 0);
                    DeleteDC(hdcMemtmp);
                    //SelectObject(hdcMem, hBitmap);
  
                    ReleaseDC(hWnd, hdcWinCl);

                    if (hBitmap)
                        DeleteObject(hBitmap);

                    if (pgpbm)
                        gps = GdipDisposeImage(pgpbm);
                    isScreenshot = FALSE;
                }
                else
                    MessageBoxExW(hWnd, L"Cannot open bitmap!", 0, 0, 0);
            }
            free(szFile);
            fBlt = TRUE;
            return (INT_PTR)TRUE;
        }
        case IDM_ABOUT:
        {
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            return 0;
            break;
        }
        case IDM_EXIT:
        {
            si.cbSize = 0;
            DeleteDC(hdcMem);
            GdiplusShutdown(gdiplusToken);
            DestroyWindow(hWnd);
            return 0;
            break;
        }
        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
        return 0;
        break;
    }
    case WM_RBUTTONDOWN:
    {
    // Get the compatible DC of the client area. 
    hdcWinCl = GetDC(hWnd);

    // Fill the client area to remove any existing contents. 

    // Copy the contents of the current screen 
    // into the compatible DC. 
    BitBlt(hdcScreenCompat, wd * scaleX, 0, bmp.bmWidth * scaleX,
    bmp.bmHeight * scaleY, hdcScreen, 0, 0, SRCCOPY);
 

 
    HDC hdcWin = GetWindowDC(hWnd);
    BITMAP bm = { 0 };
    HGDIOBJ hBmp = GetCurrentObject(hdcWin, OBJ_BITMAP);
        if (hBmp && GetObject(hBmp, sizeof(BITMAP), &bm))
        {
        bmpHeight = bm.bmHeight;
        bmpWidth = bm.bmWidth;
        RECT rect;
        rect.left = 0;
        rect.top = 0;
        rect.bottom = bmpHeight;
        rect.right = bmpWidth;
        FillRect(hdcWinCl, &rect, (HBRUSH)(COLOR_WINDOW + 1));

        // Copy the compatible DC to the client area.
         BitBlt(hdcWinCl, wd * scaleX, 0, bmp.bmWidth, bmp.bmHeight, hdcScreenCompat, wd * scaleX, 0, SRCCOPY);
        ReleaseDC(hWnd, hdcWin);
        isScreenshot = TRUE;
        }
        ReleaseDC(hWnd, hdcWinCl);
        fBlt = TRUE;

    return 0;
    break;
    }
    case WM_KEYDOWN:
    {
    switch (wParam)
    case VK_ESCAPE:
    {
        si.cbSize = 0;
        DeleteDC(hdcMem);
        GdiplusShutdown(gdiplusToken);
        PostQuitMessage(0);
        return 0;
        break;
    }
    return 0;
    break;
    }
    case WM_DESTROY:
    {
        si.cbSize = 0;
        DeleteDC(hdcMem);
        GdiplusShutdown(gdiplusToken);
        PostQuitMessage(0);
        return 0;
        break;
    }
    default:
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

}
void GetWdHt(HWND hWnd)
{
static float oldWd = 0, oldHt = 0;
GetWindowRect(hWnd, &rcWindow);
    if (oldWd)
    {
    oldWd = wd;
    oldHt = ht;
    //For button
    wd = (int)(rcWindow.right - rcWindow.left) / 9;
    ht = (int)(rcWindow.bottom - rcWindow.top) / 9;
    }
    else
    {
        wd = (int)(rcWindow.right - rcWindow.left) / 9;
        ht = (int)(rcWindow.bottom - rcWindow.top) / 9;
        oldWd = wd;
        oldHt = ht;
        if (rcWindow.left < GetSystemMetrics(0) && rcWindow.bottom < GetSystemMetrics(1))
        MoveWindow(hWnd, GetSystemMetrics(0) / 4, GetSystemMetrics(1) / 4, GetSystemMetrics(0) / 2, GetSystemMetrics(1) / 2, 1);
        else
        MessageBoxExW(hWnd, L"Not a primary monitor: Resize unavailable.", 0, 0, 0);
    }

scaleX = wd/oldWd;
scaleY = ht/oldHt;

/*For screen
sizefactorX = 1, sizefactorY = 1
sizefactorX = GetSystemMetrics(0) / (3 * wd);
sizefactorY = GetSystemMetrics(1) / (2 * ht);
*/
}
// Message handler for about box.

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
    return (INT_PTR)TRUE;

    case WM_COMMAND:
    if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
    {
    EndDialog(hDlg, LOWORD(wParam));
    return (INT_PTR)TRUE;
    }
    break;
    }
return (INT_PTR)FALSE;
}

wchar_t * FileOpener(HWND hWnd)
{

    static const INITCOMMONCONTROLSEX commonCtrls =
    {
    sizeof(INITCOMMONCONTROLSEX),
    ICC_STANDARD_CLASSES | ICC_BAR_CLASSES
    };

InitCommonControlsEx(&commonCtrls);

OPENFILENAMEW ofn;       // common dialog box structure
wchar_t szFile[255] = { 0 };       // https://stackoverflow.com/questions/7769998/how-to-return-local-array-in-c#comment107762728_7769998
//wchar_t* szFile = new wchar_t [255]; // not working with ofn.lpstrFile

// Initialize OPENFILENAME
ZeroMemory(&ofn, sizeof(OPENFILENAMEW));
ofn.lStructSize = sizeof(OPENFILENAMEW);
ofn.hwndOwner = hWnd;
ofn.lpstrFile = szFile;
ofn.nMaxFile = sizeof(szFile);
ofn.lpstrFilter = L"All Files (*.*)\0*.*\0JPEG\0*.jpg\0\0PNG\0*.png\0\0BMP\0*.bmp\0\0GIF\0*.gif\0";
ofn.nFilterIndex = 1;
ofn.lpstrFileTitle = NULL;
ofn.nMaxFileTitle = 0;
ofn.lpstrInitialDir = NULL;
ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

if (GetOpenFileNameW(&ofn) == TRUE)
return ofn.lpstrFile;
else
szFile[0] = L'*';
szFile[1] = L'\0';
return szFile;
    
}
float DoSysInfo(HWND hWnd, bool progLoad)
{

HMONITOR hMon = MonitorFromWindow(GetDesktopWindow(), MONITOR_DEFAULTTOPRIMARY);
MONITORINFO monInfo;
monInfo.cbSize = sizeof(MONITORINFO);

    if (GetMonitorInfoW(hMon, &monInfo))
    {
    resX = (float)abs(monInfo.rcMonitor.right - monInfo.rcMonitor.left);
    resY = (float)abs(monInfo.rcMonitor.top - monInfo.rcMonitor.bottom);
    tmp = (int)(9 * resX / resY);
    //scale factors
    resX = resX / DESIGNSCRX;
    resY = resY / DESIGNSCRY;
        if (abs(1 - resX) < 0.1)
        resX = 1;
        if (abs(1 - resY) < 0.1)
        resY = 1;

        if (tmp <= FOURTHREEVID)
        tmp = FOURTHREEVID;
        else
        {
            if (tmp <= WIDESCREENVID)
            tmp = WIDESCREENVID;
            else
            {
                if (tmp <= UNIVISIUM)
                tmp = UNIVISIUM;
                else
                tmp = ULTRAWIDEVID;
            }
        }
    return (float)tmp / 9;
    }
    else
    {
    MessageBoxExW(hWnd, L"GetMonitorInfo: Cannot get info.", 0, 0, 0);
    }
return 0;
}
//example to use: message("test %s %d %d %d", "str", 1, 2, 3);
void message(const wchar_t* format, ...)
{
    if (!format)
    return;
int len;
wchar_t* buf;
va_list args;
va_start(args, format);

len = _vscwprintf(format, args) + 1;  //add room for terminating '\0'
buf = (wchar_t*)malloc(len * sizeof(wchar_t));
vswprintf_s(buf, len, format, args);

MessageBoxW(0, buf, L"debug", 0);
//OutputDebugStringA(buf);
free(buf);
}
