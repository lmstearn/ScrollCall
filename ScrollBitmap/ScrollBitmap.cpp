#include "ScrollBitmap.h"
RECT RectCl::rectOut1 = {};
RECT RectCl::rectOut2 = {};
RECT RectCl::rectOut3 = {};
HWND RectCl::ownerHwnd = 0;
RECT RectCl::rectOwnerHwnd = {};


#define MAX_LOADSTRING 255
#define FOURTHREEVID					12
#define WIDESCREENVID				16
#define UNIVISIUM						18 // e.g. Galaxy Tabs
#define ULTRAWIDEVID					21
#define DESIGNSCRX						1360
#define DESIGNSCRY						768



// Global Variables: static can be used for these
RECT rcWindow;
SCROLLINFO si = {0};
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // Title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
wchar_t* szFile = nullptr;
float scrAspect = 0, scaleX = 1, scaleY = 1,  resX = 0, resY = 0;
// wd, ht: button dims
int tmp = 0, wd = 0, ht = 0, capCallFrmResize = 0;
HWND hWndGroupBox = 0, hWndButton = 0, hWndOpt1 =0, hWndOpt2 =0;
BOOL optChk = TRUE, groupboxFlag = FALSE, isLoading = TRUE;
RECT rectB, rectO1, rectO2;


// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    MyBitmapWindowProc(HWND, UINT, WPARAM, LPARAM);
void GetDims(HWND hWnd);
LRESULT CALLBACK staticSubClass(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
LRESULT CALLBACK staticSubClassButton(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
wchar_t* FileOpener(HWND hWnd);
void ReportErr(const wchar_t* format, ...);
void PrintWindow(HWND hWnd, HBITMAP& hBmp);
//Functions for later use
BOOL bitmapFromPixels(Bitmap& myBitmap, const std::vector<std::vector<unsigned>>resultPixels, int width, int height);
float DoSysInfo(HWND hWnd, bool progLoad);
char* VecToArr(std::vector<std::vector<unsigned>> vec);
BOOL AdjustImage(BOOL isScreenshot, HBITMAP hBitmap, BITMAP bmp, GpStatus gps, HDC hdcMem, HDC hdcScreen, HDC hdcScreenCompat, HDC hdcWin, HDC hdcWinCl, UINT& bmpWidth, UINT& bmpHeight, BOOL newScrShot = FALSE);
void SizeControls(BITMAP bmp, HWND hWnd, int offsetx = 0, int offsetY = 0);

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

wcex.style          = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
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

int msgboxID = MessageBoxW(NULL, L"Use GroupBox?", L"GroupBox", MB_YESNOCANCEL | MB_ICONQUESTION);
    if (msgboxID == IDYES)
    groupboxFlag = TRUE;
    else
    {
        if (msgboxID == IDCANCEL)
        return 0;
    }

HWND m_hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | (groupboxFlag? NULL: WS_CLIPCHILDREN),
CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!m_hWnd)
    return FALSE;


ShowWindow(m_hWnd, nCmdShow);
UpdateWindow(m_hWnd);
isLoading = FALSE;
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


static HBITMAP bm = { 0 };
static GpStatus gps = { };


static HDC hdcWin;            // DC for window
static HDC hdcWinCl;            // client area of DC for window
static HDC hdcMem;            // Mem DC 
static HDC hdcScreen;           // DC for entire screen 
static HDC hdcScreenCompat;     // memory DC for screen 
static HBITMAP hbmpCompat;      // bitmap handle to old DC 
static BITMAP bmp;          // bitmap data structure 
static UINT bmpWidth = 0;
static UINT bmpHeight = 0;
static UINT fmHt = 0;
static UINT fmWd = 0;
static BOOL fBlt;           // TRUE if BitBlt occurred 
static int fScroll;             // 1 if horz scrolling, -1 vert scrolling, 0 for WM_SIZE
static BOOL fSize;          // TRUE if fBlt & WM_SIZE 
static BOOL isScreenshot;

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
        SMOOTHSCROLL_SPEED = 0X00000002;
        ulScrollLines = 0;
        RECT recthWndtmp = RectCl().RectCl(0, hWnd, 0);
        fmHt = recthWndtmp.bottom - recthWndtmp.top;
        fmWd = recthWndtmp.right - recthWndtmp.left;

        GetDims(hWnd);
        GdiplusInit gdiplusinit;

        if (groupboxFlag)
        {

                hWndGroupBox = CreateWindowEx(0, TEXT("BUTTON"),
                TEXT(""),
                WS_VISIBLE | WS_CHILD | BS_GROUPBOX | WS_CLIPSIBLINGS, //consider  WS_CLIPCHILDREN
                // also MSDN: Do not combine the BS_OWNERDRAW style with any other button styles!
                0, 0, wd, fmHt,
                hWnd,
                (HMENU)IDC_GROUPBOX,
                GetModuleHandle(NULL),
                NULL);
        }
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

        hWndOpt1 = CreateWindowEx(WS_EX_WINDOWEDGE,
            L"BUTTON",
            L"Scroll",
            WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP,  // <---- WS_GROUP group the following radio buttons 1st,2nd button 
            2, ht + 10,
            wd - 2, ht / 2,
            hWnd, //<----- Use main window handle
            (HMENU)IDC_OPT1,
            (HINSTANCE)NULL, NULL);
        hWndOpt2 = CreateWindowEx(WS_EX_WINDOWEDGE,
            L"BUTTON",
            L"ScrollEx",
            WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,  // Styles 
            2, 2 * ht,
            wd - 2, ht / 2,
            hWnd,
            (HMENU)IDC_OPT2,
            (HINSTANCE)NULL, NULL);
        SendMessage(hWndOpt1, BM_SETCHECK, BST_CHECKED, 0);

    


    
    
    // Create a normal DC and a memory DC for the entire 
    // screen. The normal DC provides a snapshot of the 
    // screen contents. The memory DC keeps a copy of this 
    // snapshot in the associated bitmap. 

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
    ReportErr(L"SPI_GETWHEELSCROLLLINES: Cannot get info.");

    if (groupboxFlag)
    {
        if (!SetWindowSubclass(hWndGroupBox, staticSubClass, 1, 0))
            // uIdSubclass is 1 and incremented for each new subclass implemented
            // dwRefData is 0 and has no explicit use
        {
            //std::cerr << "Failed to subclass list\n";
            ReportErr(L"Cannot subclass control! Quitting...");
            DestroyWindow(hWndGroupBox);
            return NULL;

        }
    }
    /*
    if (!SetWindowSubclass(hWndButton, staticSubClassButton, 2, 0))
    {
        ReportErr(L"Cannot subclass control! Quitting...");
        DestroyWindow(hWndButton);
        return NULL;

    }
    */
    break;
    }
    	case WM_SIZING:
        {
            if (capCallFrmResize > capCallFrmResize)
            {
                tmp = SetTimer(hWnd,             // handle to main window 
                    IDT_DRAGWINDOW,                   // timer identifier 
                    10,                           // millisecond interval 
                    (TIMERPROC)NULL);               // no timer callback 

                if (tmp == 0)
                {
                    ReportErr(L"No timer is available.");
                }
            }
            else
            {
                capCallFrmResize++;
                fScroll = 0;
                fSize = TRUE;
            }
            return TRUE;
        }
        case WM_TIMER:
        {
            if (wParam == IDT_DRAGWINDOW)
            {
                    KillTimer(hWnd, IDT_DRAGWINDOW);
                    SizeControls(bmp, hWnd);
                    hdcWin = GetWindowDC(hWnd);
                    AdjustImage(isScreenshot, bm, bmp, gps, hdcMem, hdcScreen, hdcScreenCompat, hdcWin, hdcWinCl, bmpWidth, bmpHeight);
                        //ReportErr(L"AdjustImage detected a problem with the image!");
                    ReleaseDC(hWnd, hdcWin);
                    capCallFrmResize = 0;
            }
        }
        break;
        case WM_GETMINMAXINFO:
            // prevent the window from becoming too small
            ((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
            ((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
            return 0;
       break;
        case WM_ENTERSIZEMOVE:
        {
            //InvalidateRect(hWnd, 0, TRUE);
        }
        case WM_EXITSIZEMOVE:
        {
            fSize = TRUE;
            InvalidateRect(hWnd, 0, TRUE);
            return 0;
            KillTimer(hWnd, IDT_DRAGWINDOW);
            tmp = SetTimer(hWnd,             // handle to main window 
                IDT_DRAGWINDOW,                   // timer identifier 
                2,                           // millisecond interval 
                (TIMERPROC)NULL);               // no timer callback 

            if (tmp == 0)
            {
                ReportErr(L"No timer is available.");
            }
        }
        return 0;
        break;

    case WM_SIZE:
    {

    // WM_SIZE called for each child control (no subclass)
    if (!capCallFrmResize)
    {
        int xNewSize;
        int yNewSize;

        xNewSize = LOWORD(lParam);
        yNewSize = HIWORD(lParam);
        SizeControls(bmp, hWnd);


        if (!isLoading)
        {
            hdcWin = GetWindowDC(hWnd);
                if (!AdjustImage(isScreenshot, bm, bmp, gps, hdcMem, hdcScreen, hdcScreenCompat, hdcWin, hdcWinCl, bmpWidth, bmpHeight))
                ReportErr(L"AdjustImage detected a problem with the image!");
            ReleaseDC(hWnd, hdcWin);
        }

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
    }
    break;
    }

    case WM_PAINT:
    {
        if (wParam == 0)
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
                    //prect->left + (isScreenshot ? 0 : (((fScroll == 1) && (xCurrentScroll > wd)) ? 0 : wd)),
                    // Blt at wd method
                    //prect->left + (isScreenshot ? (fScroll == 1 ? 0 : wd) : (((fScroll == 1) && (xCurrentScroll > wd)) ? 0 : wd)),
                    prect->left,
                    prect->top,
                    //prect->left + ( (isScreenshot) ? 0 : wd * scaleX), prect->top,
                    (prect->right - prect->left),
                    (prect->bottom - prect->top),
                    isScreenshot ? hdcScreenCompat : hdcMem,
                    prect->left + xCurrentScroll,
                    prect->top + yCurrentScroll,
                    SRCCOPY);

                if (!groupboxFlag) // Paint sections
                {
                         if (xCurrentScroll < wd)
                        {
                            RECT rect;
                            rect.top = prect->top;
                            rect.bottom = prect->bottom;
                            rect.left = prect->left;
                            rect.right = prect->left + wd - xCurrentScroll;
                            FillRect(ps.hdc, &rect, (HBRUSH)(COLOR_WINDOW + 1));
                            //UpdateWindow(hWnd);
                        }
                         /*
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

                        }
*/
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
                }
                fScroll = 0;
            }
            else
            {
            if (fSize)
            {
                //if (!capCallFrmResize)
                BitBlt(ps.hdc,
                    //0, 0,
                    -xCurrentScroll, -yCurrentScroll,
                    bmp.bmWidth, bmp.bmHeight,
                    isScreenshot ? hdcScreenCompat : hdcMem,
                   0, 0,
                    //xCurrentScroll, yCurrentScroll,
                    SRCCOPY);
                //UpdateWindow(hWnd);
                    if (!capCallFrmResize)
                    fSize = FALSE;
                    if (!groupboxFlag) // Paint sections
                    {
                        if (xCurrentScroll < wd)
                        {
                            PRECT prect;
                            prect = &ps.rcPaint;
                            RECT rect;
                            rect.top = prect->top;
                            rect.bottom = prect->bottom;
                            rect.left = prect->left;
                            rect.right = prect->left + wd - xCurrentScroll;
                            FillRect(ps.hdc, &rect, (HBRUSH)(COLOR_WINDOW + 1));
                            //UpdateWindow(hWnd);
                        }
                    }
            }
            else
                if (!isLoading)
                ReportErr(L"Cannot get here, fSize should be set...");
            }

            EndPaint(hWnd, &ps);
        }
        else
        RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);

    break;
    }

    case WM_HSCROLL:
    {
    si.cbSize = sizeof(si);
    si.fMask = SIF_TRACKPOS;
    GetScrollInfo(hWnd, SB_HORZ, &si);
    xTrackPos = si.nTrackPos;
    int xDelta;     // xDelta = new_pos - current_pos  
    int xNewPos;    // new position 
    int yDelta = 0;
    SetWindowOrgEx(hdcMem, wd * scaleX, 0, NULL);
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
    // client area when ScrollWindow(Ex) is called; however, it is 
    // necessary to call UpdateWindow in order to repaint the 
    // rectangle of pixels that were invalidated.) 


    if (optChk)
    {
        /*
        The following produces flicker
        RECT rect;
        rect.left = wd;
        rect.top = 0;
        rect.bottom = fmHt * scaleY;
        rect.right = bmp.bmWidth;
        ScrollWindow(hWnd, -xDelta, -yDelta, (CONST RECT*) NULL, &rect);
        */
        ScrollWindow(hWnd, -xDelta, -yDelta, (CONST RECT*) NULL, (CONST RECT*) NULL);
    }
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
    SetWindowOrgEx(hdcMem, 0, 0, NULL);

    switch (LOWORD(wParam))
    {
    case SB_BOTTOM:         //Scrolls to the lower right.
    yNewPos = si.nMax;
    break;
    case SB_TOP:         //Scrolls to the lower right.
        yNewPos = si.nMin;
        break;
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
    // client area when ScrollWindow(Ex) is called; however, it is 
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

    // Reset scroll bar. 
    si.fMask = SIF_POS;
    si.nPos = yCurrentScroll;
    SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
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
        break;
    }
    case WM_COMMAND: // WM_PARENTNOTIFY if hWndGroupBox is parent
    {
        USHORT wmId = LOWORD(wParam);
        USHORT wmEvent = HIWORD(wParam);

        // Commented code is for hWndGroupbox as parent
        // POINT pt; // Position of cursor when button was pressed
       //  HWND hwndButton; // Button at position of cursor

        // pt.x = LOWORD(lParam);
        // pt.y = HIWORD(lParam);
        // hwndButton = ChildWindowFromPoint(hWndGroupBox, pt);

        switch (wmId) //(GetWindowLong(hwndButton, GWL_ID))
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
            break;
        }
        case ID_OPENBITMAP:
        {
            //https://social.msdn.microsoft.com/Forums/sqlserver/en-US/76441f64-a7f2-4483-ad6d-f51b40464d6b/how-to-get-both-width-and-height-of-bitmap-in-gdiplus-flat-api?forum=windowsgeneraldevelopmentissues
            GpBitmap* pgpbm = nullptr;
            Color clr;
            szFile = (wchar_t*)calloc(MAX_LOADSTRING, sizeof(wchar_t));
            wcscpy_s(szFile, MAX_LOADSTRING, FileOpener(hWnd));


                //CLSID pngClsid;
               // GetEncoderClsid(L"image/png", &pngClsid);
            if (szFile[0] != L'*')
            {
                ReleaseDC(hWnd, hdcWinCl);
                hdcWinCl = GetDCEx(hWnd, (HRGN)NULL, DCX_CACHE | DCX_CLIPCHILDREN);
                wd = RectCl().RectCl(1).right - RectCl().RectCl(1).left;
                ht = RectCl().RectCl(0).bottom - RectCl().RectCl(0).top;
                const std::vector<std::vector<unsigned>>resultPixels;
               
                gps = GdipCreateBitmapFromFile(szFile, &pgpbm);
                if (gps == Ok)
                {
                    HBITMAP hBitmap = NULL;
                    gps = GdipCreateHBITMAPFromBitmap(pgpbm, &hBitmap, clr.GetValue());

                    gps = GdipGetImageWidth(pgpbm, &bmpWidth);
                    gps = GdipGetImageHeight(pgpbm, &bmpHeight);
                    //HBITMAP hBmpCopy = (HBITMAP)CopyImage(hBitmap, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
                    //GpStatus WINGDIPAPI GdipDrawImageI(GpGraphics* graphics, GpImage* image, INT x, INT y);
                    hdcMem = CreateCompatibleDC(hdcWinCl);
                    hdcWin = GetWindowDC(hWnd);

                        if (!AdjustImage(isScreenshot, hBitmap, bmp, gps, hdcMem, hdcScreen, hdcScreenCompat, hdcWin, hdcWinCl, bmpWidth, bmpHeight))
                        ReportErr(L"AdjustImage detected a problem with the image!");
                    ReleaseDC(hWnd, hdcWin);
                    if (hBitmap)
                        DeleteObject(hBitmap);

                    if (pgpbm)
                        gps = GdipDisposeImage(pgpbm);
                    isScreenshot = FALSE;
                }
                else
                    ReportErr(L"Cannot open bitmap!");
            }
            free(szFile);
            fBlt = TRUE;
            return (INT_PTR)TRUE;
        }
        case IDM_ABOUT:
        {
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        }
        case IDM_EXIT:
        {
            si.cbSize = 0;
            DeleteDC(hdcMem);
            DeleteObject(hbmpCompat);
            DestroyWindow(hWnd);
            break;
        }
        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
        break;
    }
    case WM_LBUTTONDBLCLK:
    {
        ReleaseDC(hWnd, hdcWinCl);
        hdcWinCl = GetDCEx(hWnd, (HRGN)NULL, DCX_CACHE | DCX_CLIPCHILDREN);
        hdcMem = CreateCompatibleDC(hdcWinCl);
        HBITMAP hBmp = NULL;
        bmpWidth = (scaleX * RectCl().width(0)) - wd;
        bmpHeight = scaleY * (RectCl().height(0));


        hBmp = CreateCompatibleBitmap(hdcWinCl, bmpWidth, bmpHeight);

        PrintWindow(hWnd, hBmp);
        SelectObject(hdcMem, hBmp);


        if (!BitBlt(hdcWinCl, wd, RectCl().ClMenuandTitle(hWnd), bmpWidth, bmpHeight, hdcMem, 0, RectCl().ClMenuandTitle(hWnd), SRCCOPY))
            ReportErr(L"Bad BitBlt from hdcMem!");


        if (hBmp)
            DeleteObject(hBmp);


        isScreenshot = FALSE;
        break;
    }
    case WM_RBUTTONDOWN:
    {
        // Get the compatible DC of the client area. 
        ReleaseDC(hWnd, hdcWinCl);
        hdcWinCl = GetDCEx(hWnd, (HRGN)NULL, DCX_CACHE | DCX_CLIPCHILDREN);

        SizeControls(bmp, hWnd, xCurrentScroll, yCurrentScroll);
        xCurrentScroll ? fScroll = 1 : fScroll = -1;
        xCurrentScroll = 0;
        yCurrentScroll = 0;
        //SetWindowOrgEx(hdcWinCl, xCurrentScroll, yCurrentScroll, NULL);


        SetScrollPos(hWnd, SB_VERT, 0, TRUE);
        SetScrollPos(hWnd, SB_HORZ, 0, TRUE);
        /*
        ScrollWindowEx(hWnd, -xCurrentScroll, -yCurrentScroll, (CONST RECT*) NULL,
            (CONST RECT*) NULL, (HRGN)NULL, (PRECT)NULL, SW_SCROLLCHILDREN | SW_INVALIDATE); // SMOOTHSCROLL_FLAG fails
        tmp = GetLastError();
        SetViewportOrgEx(hdcScreenCompat, xCurrentScroll, yCurrentScroll, NULL);
        SetWindowOrgEx(hdcScreenCompat, xCurrentScroll, yCurrentScroll, NULL);

        fSize = TRUE;
        si.cbSize = sizeof(si);
        si.nPos = xCurrentScroll;
        tmp = SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);

        // The vertical scrolling range is defined by 
        // (bitmap_height) - (client_height). The current vertical 
        // scroll value remains within the vertical scrolling range. 
        yCurrentScroll = 0;
        si.cbSize = sizeof(si);
        si.nPos = yCurrentScroll;
        tmp = SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
        */


    isScreenshot = TRUE;
    hdcWin = GetWindowDC(hWnd);
    
        if (!AdjustImage(isScreenshot, bm, bmp, gps, hdcMem, hdcScreen, hdcScreenCompat, hdcWin, hdcWinCl, bmpWidth, bmpHeight, 1))
        ReportErr(L"AdjustImage detected a problem with the image!");

    ReleaseDC(hWnd, hdcWin);
  
    fBlt = TRUE;

    break;
    }
    case WM_KEYDOWN:
    {
        switch (wParam)
        case VK_ESCAPE:
        {
            si.cbSize = 0;
                if (hdcWinCl)
                ReleaseDC(hWnd, hdcWinCl);
            DeleteDC(hdcMem);
            DeleteObject(hbmpCompat);
            PostQuitMessage(0);
            break;
        }
    break;
    }
    case WM_DESTROY:
    {
        si.cbSize = 0;
            if (hdcWinCl)
            ReleaseDC(hWnd, hdcWinCl);
        DeleteDC(hdcMem);
        DeleteObject(hbmpCompat);
        PostQuitMessage(0);
        break;
    }
    default:
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
return 0;
}
void GetDims(HWND hWnd)
{
static float firstWd = 0, firstHt = 0, oldWd = 0, oldHt = 0;
GetWindowRect(hWnd, &rcWindow);
    if (oldWd)
    {
    oldWd = wd;
    oldHt = ht;
    //For button

        if (!isLoading)
        {
            wd = (int)(rcWindow.right - rcWindow.left) / 9;
            ht = (int)((rcWindow.bottom - rcWindow.top)/ 9);
        }
    }
    else
    {
        if (firstWd)
        {
            oldWd = wd = (int)(rcWindow.right - rcWindow.left) / 9;
            oldHt = ht = (int)((rcWindow.bottom - rcWindow.top) / 9);

        }
        else
        {
            firstWd = wd = (int)(rcWindow.right - rcWindow.left) / 9;
            firstHt = ht = (int)(rcWindow.bottom - rcWindow.top / 9);
            if (rcWindow.left < GetSystemMetrics(0) && rcWindow.bottom < GetSystemMetrics(1))
                MoveWindow(hWnd, GetSystemMetrics(0) / 4, GetSystemMetrics(1) / 4, GetSystemMetrics(0) / 2, GetSystemMetrics(1) / 2, 1);
            else
                ReportErr(L"Not a primary monitor: Resize unavailable.");
        }
    }

scaleX = wd/oldWd;
scaleY = ht/oldHt;

/*For screen
sizefactorX = 1, sizefactorY = 1
sizefactorX = GetSystemMetrics(0) / (3 * wd);
sizefactorY = GetSystemMetrics(1) / (2 * ht);
*/
}

LRESULT CALLBACK staticSubClass(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    UNUSED(dwRefData);


    switch (uMsg) // instead of LPNMHDR  lpnmh = (LPNMHDR) lParam above;
    {
    case WM_PAINT:
    {
        if (wParam == 0)
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            PRECT prect;
            prect = &ps.rcPaint;
            FillRect(hdc, prect, (HBRUSH)(COLOR_WINDOW + 1));
            EndPaint(hWnd, &ps);
        }
        else
            RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);
        return TRUE;
        break;
    }
    case WM_NCDESTROY:
        // NOTE: this requirement is NOT stated in the documentation, but it is stated in Raymond Chen's blog article...
        RemoveWindowSubclass(hWnd, staticSubClass, uIdSubclass);
        break;
    default:
        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
        break;
    }

    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}
LRESULT CALLBACK staticSubClassButton(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    UNUSED(dwRefData);

    switch (uMsg) // instead of LPNMHDR  lpnmh = (LPNMHDR) lParam above;
    {

    case WM_NCDESTROY:
        // NOTE: this requirement is NOT stated in the documentation, but it is stated in Raymond Chen's blog article...
        RemoveWindowSubclass(hWnd, staticSubClassButton, uIdSubclass);
        break;
    default:
        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
        break;
    }

    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
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

//example to use: message("test %s %d %d %d", "str", 1, 2, 3);
void ReportErr(const wchar_t* format, ...)
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

MessageBoxW(0, buf, L"Error", 0);
//OutputDebugStringA(buf);
free(buf);
}

void PrintWindow(HWND hWnd, HBITMAP& hBmp)
{
    HDC hDCMem = CreateCompatibleDC(NULL);

    HGDIOBJ hOld = SelectObject(hDCMem, hBmp);
    SendMessage(hWnd, WM_PRINT, (WPARAM)hDCMem, PRF_CHILDREN | PRF_CLIENT | PRF_ERASEBKGND | PRF_NONCLIENT | PRF_OWNED);
    Sleep(50);
    SelectObject(hDCMem, hOld);
    DeleteObject(hDCMem);
}
//**************************************************************
// Functions for possible later use
//**************************************************************
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
        ReportErr(L"GetMonitorInfo: Cannot get info.");
    }
    return 0;
}
//https://stackoverflow.com/a/39654760/2128797
std::vector<std::vector<unsigned>> getPixels(Gdiplus::Bitmap bitmap, int& width, int& height) {


    //Pass up the width and height, as these are useful for accessing pixels in the vector o' vectors.
    width = bitmap.GetWidth();
    height = bitmap.GetHeight();

    auto* bitmapData = new Gdiplus::BitmapData;

    //Lock the whole bitmap so we can read pixel data easily.
    Gdiplus::Rect rect(0, 0, width, height);
    bitmap.LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, bitmapData);

    //Get the individual pixels from the locked area.
    auto* pixels = static_cast<unsigned*>(bitmapData->Scan0);

    //Vector of vectors; each vector is a column.
    std::vector<std::vector<unsigned>> resultPixels(width, std::vector<unsigned>(height));

    const int stride = abs(bitmapData->Stride);
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            //Get the pixel colour from the pixels array which we got earlier.
            const unsigned pxColor = pixels[y * stride / 4 + x];

            //Get each individual colour component. Bitmap colours are in reverse order.
            const unsigned red = (pxColor & 0xFF0000) >> 16;
            const unsigned green = (pxColor & 0xFF00) >> 8;
            const unsigned blue = pxColor & 0xFF;

            //Combine the values in a more typical RGB format (as opposed to the bitmap way).
            const int rgbValue = RGB(red, green, blue);

            //Assign this RGB value to the pixel location in the vector o' vectors.
            resultPixels[x][y] = rgbValue;
        }
    }

    //Unlock the bits that we locked before.
    bitmap.UnlockBits(bitmapData);
    return resultPixels;
}
BOOL bitmapFromPixels(Bitmap& myBitmap, const std::vector<std::vector<unsigned>>resultPixels, int width, int height)
{
// Possible usage:
//HPALETTE hpal;
//GdipCreateBitmapFromHBITMAP(hBmp, hpal, &pgpbm);
//Bitmap ctlBitmap(wd,  ht, PixelFormat32bppARGB);
//bitmapFromPixels(ctlBitmap, resultPixels, wd, ht);

    //consider MAX_ARRAY_LENGTH;



   // auto* bitmapData = new Gdiplus::BitmapData;


    BitmapData bitmapData;
    bitmapData.Width = width,
        bitmapData.Height = height,
        bitmapData.Stride = 4 * bitmapData.Width;
    bitmapData.PixelFormat = PixelFormat32bppARGB;
    bitmapData.Scan0 = (VOID*)VecToArr(resultPixels);
    bitmapData.Reserved = NULL;


    Gdiplus::Rect rect(0, 0, width, height);
    myBitmap.LockBits(&rect, Gdiplus::ImageLockModeWrite | ImageLockModeUserInputBuf, PixelFormat32bppARGB, &bitmapData);
    myBitmap.UnlockBits(&bitmapData);

    return TRUE;
}

char* VecToArr(std::vector<std::vector<unsigned>> vec)
{
    std::size_t totalsize = 0;
    // if totalsize > MAX_ARRAY_LENGTH

    for (int i = 0; i < vec.size(); i++)
        totalsize += vec[i].size();

    int* newarr = new int[totalsize];
    char* bytes = new char[totalsize];
    int* newarr_ptr_copy = newarr;

    for (int i = 0; i < vec.size(); i++)
    {
        std::copy(vec[i].begin(), vec[i].end(), newarr_ptr_copy);
        newarr_ptr_copy += vec[i].size();

    }
    for (int i = 0; i < totalsize; i++)
        //std::copy(static_cast<const char*>(static_cast<const void*>(&newarr[i])),
        //static_cast<const char*>(static_cast<const void*>(&newarr[i])) + sizeof newarr[i],
        //bytes);
        bytes[i] = (char)newarr[i];


    return bytes;
}



BOOL AdjustImage(BOOL isScreenshot, HBITMAP hBitmap, BITMAP bmp, GpStatus gps, HDC hdcMem, HDC hdcScreen,HDC hdcScreenCompat, HDC hdcWin, HDC hdcWinCl, UINT& bmpWidth, UINT& bmpHeight, BOOL newScrShot)
{

    BOOL retVal = FALSE;

    BITMAP bm = { 0 };
    HGDIOBJ hBmpObj = GetCurrentObject(hdcWin, OBJ_BITMAP); //hBmp =>handle to bitmap (HBITMAP)
    if (hBmpObj && GetObject(hBmpObj, sizeof(BITMAP), &bm)) //bm => BITMAP structure
    {
        //bm.biBitCount = 32;
        bmpWidth = bm.bmWidth *( (isScreenshot) ? 1 : scaleX);
        bmpHeight = bm.bmHeight *( (isScreenshot) ? 1 : scaleY);
    }
    else
        ReportErr(L"Unable to size bitmap!");

    if (isScreenshot)
    {
        RECT rect;
        rect.left = 0;
        rect.top = 0;
        rect.bottom = bmpHeight;
        rect.right = bmpWidth;
        //if (!capCallFrmResize)
        retVal = (BOOL)FillRect(hdcWinCl, &rect, (HBRUSH)(COLOR_WINDOW + 1)); //SetBkColor(hdcWinCl, COLOR_WINDOW + 1) causes flickering in the scrolling
        if (retVal)
        {
            if (newScrShot)
            retVal = (BOOL)BitBlt(hdcScreenCompat, wd, 0, bmp.bmWidth,
                bmp.bmHeight, hdcScreen, 0, 0, SRCCOPY);
            // Copy the compatible DC to the client area.
            //retVal = (BOOL)BitBlt(hdcWinCl, wd, 0, bmp.bmWidth, bmp.bmHeight, hdcScreenCompat, 0, 0, SRCCOPY); //Blt at zero method causes problems with horz scrolling
            if (retVal)
            retVal = (BOOL)BitBlt(hdcWinCl, wd, 0, bmp.bmWidth, bmp.bmHeight, hdcScreenCompat, wd, 0, SRCCOPY); //Blt at wd method
        }
    }
    else
    {

        GpBitmap* pgpbm = nullptr;
        Color clr;
        // black = (bits == 0);   alpha=255 => 0xFF000000
        SelectObject(hdcMem, hBitmap);

        retVal = (BOOL)BitBlt(hdcWinCl, wd, 0, bmpWidth, bmpHeight, hdcMem, 0, 0, SRCCOPY);

        if (pgpbm)
            gps = GdipDisposeImage(pgpbm);
    }

    return retVal;
}

void SizeControls(BITMAP bmp, HWND hWnd, int offsetX, int offsetY)
{
    //Get updated rect for the form
    RECT recthWndtmp = RectCl().RectCl(0, hWnd, 0);

    GetDims(hWnd);

    RECT rectBtmp = RectCl().RectCl(hWndButton, hWnd, 1);
    GetClientRect(hWndButton, &rectB);
    //rectB.top += BOX_HEIGHT;
    SetWindowPos(hWndButton, NULL, scaleX * (rectBtmp.left) + offsetX, scaleY * rectBtmp.top + offsetY,
        scaleX * (rectB.right - rectB.left), scaleY * (rectB.bottom - rectB.top), NULL);

    if (groupboxFlag)
    {
        SetWindowPos(hWndGroupBox, NULL, offsetX, offsetY,
            scaleX * (rectB.right - rectB.left), scaleY * (bmp.bmHeight), NULL);
    }

    RECT rectOpt1tmp = RectCl().RectCl(hWndOpt1, hWnd, 2);
    GetClientRect(hWndOpt1, &rectO1);
    //Extra edging for the wd - 2
    SetWindowPos(hWndOpt1, NULL, scaleX * (rectOpt1tmp.left) + offsetX, scaleY * (rectOpt1tmp.top) + offsetY,
        scaleX * (rectB.right - rectB.left - 2), scaleY * (rectO1.bottom - rectO1.top), NULL);
    RECT rectOpt2tmp = RectCl().RectCl(hWndOpt2, hWnd, 3);
    GetClientRect(hWndOpt2, &rectO2);
    SetWindowPos(hWndOpt2, NULL, scaleX * (rectOpt2tmp.left) + offsetX, scaleY * (rectOpt2tmp.top) + offsetY,
        scaleX * (rectB.right - rectB.left - 2), scaleY * (rectO2.bottom - rectO2.top), NULL);
}