#include "ScrollBitmap.h"
RECT RectCl::rectOut1 = {};
RECT RectCl::rectOut2 = {};
RECT RectCl::rectOut3 = {};
RECT RectCl::rectOut4 = {};
HWND RectCl::ownerHwnd = 0;
RECT RectCl::initRectOwnerHwnd = {};
RECT RectCl::rectOwnerHwnd = {};


#define MAX_LOADSTRING 255
#define FOURTHREEVID					12
#define WIDESCREENVID				16
#define UNIVISIUM						18 // e.g. Galaxy Tabs
#define ULTRAWIDEVID					21
#define DESIGNSCRX						1360
#define DESIGNSCRY						768

using namespace Gdiplus;
using namespace Gdiplus::DllExports;


// Global Variables: static can be also applied to these, although not generally recommended
SCROLLINFO siHORZ = { 0 };
SCROLLINFO siVERT = { 0 };
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // Title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
wchar_t* szFile = nullptr;
float scrAspect = 0, scaleX = 1, scaleY = 1, resX = 0, resY = 0;
// wd, ht: button dims
int tmp = 0, wd = 0, ht = 0, capCallFrmResize = 0;
UINT xCurrentScroll, yCurrentScroll;
HWND hWndGroupBox = 0, hWndButton = 0, hWndOpt1 = 0, hWndOpt2 = 0, hWndChk = 0;
BOOL optChk = TRUE, chkChk = FALSE, groupboxFlag = FALSE, isLoading = TRUE;

// Timer
int timDragWindow;


// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    MyBitmapWindowProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK staticSubClass(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
LRESULT CALLBACK staticSubClassButton(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
wchar_t* FileOpener(HWND hWnd);
void ReportErr(const wchar_t* format, ...);
void PrintWindow(HWND hWnd, HBITMAP& hBmp);
//Functions for later use
BOOL BitmapFromPixels(Bitmap& myBitmap, const std::vector<std::vector<unsigned>>resultPixels, int width, int height);
void DoSysInfo();
char* VecToArr(std::vector<std::vector<unsigned>> vec);
BOOL AdjustImage(BOOL isScreenshot, HBITMAP hBitmap, BITMAP bmp, GpStatus gps, HDC hdcMem, HDC hdcMemIn, HDC hdcScreen, HDC hdcScreenCompat, HDC hdcWin, HDC hdcWinCl, UINT& bmpWidth, UINT& bmpHeight, int xCurrentScroll, int resizePic = 0, BOOL maxMin = FALSE);
void GetDims(HWND hWnd, int resizeType = 0);
void SizeControls(BITMAP bmp, HWND hWnd, UINT defFmWd, UINT defFmHt, int resizeType = -1, int xNewSize = 0, int yNewSize = 0);
int ScrollInfo(HWND hWnd, int scrollXorY, int scrollType, int scrollDrag, int xNewSize = 0, int yNewSize = 0, int bmpWidth = 0, int bmpHeight = 0, UINT defFmWd = 0, UINT defFmHt = 0);
BOOL Kleenup(HWND hWnd, HBITMAP& hBitmap, HBITMAP& hbmpCompat, GpBitmap*& pgpbm, HDC& hdcMem, HDC& hdcMemIn, HDC& hdcWinCl);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    //_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    // TODO: Place code here.
    GdiplusInit gdiplusinit;
    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SCROLLBITMAP, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow))
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

    return (int)msg.wParam;
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

    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcex.lpfnWndProc = MyBitmapWindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SCROLLBITMAP));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_SCROLLBITMAP);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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
    {
        groupboxFlag = TRUE;
        DoSysInfo();
    }
    else
    {
        if (msgboxID == IDCANCEL)
            return 0;
    }

    HWND m_hWnd = CreateWindowW(szWindowClass, szTitle, WS_HSCROLL | WS_VSCROLL | WS_OVERLAPPEDWINDOW | (groupboxFlag ? NULL : WS_CLIPCHILDREN),
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
    // Old style
    //SetWindowTheme(m_hWnd, L" ", L" ");
    //Or SetThemeAppProperties(STAP_ALLOW_CONTROLS | STAP_ALLOW_WEBCONTENT);
    //SetThemeAppProperties(0);
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


    // These variables are required by BitBlt. 

    static HBITMAP hBitmap = { 0 };
    static GpStatus gps = { };


    static HDC hdc; // for WM_PAINT
    static HDC hdcWin;            // DC for window
    static HDC hdcWinCl;            // client area of DC for window
    static HDC hdcMem;            // Mem DC
    static HDC hdcMemIn;            // Mem DC
    static HDC hdcScreen;           // DC for entire screen
    static HDC hdcScreenCompat;     // memory DC for screen
    static HBITMAP hbmpCompat;      // bitmap handle to old DC
    static GpBitmap* pgpbm;          // bitmap data structure
    static BITMAP bmp;          // bitmap data structure
    static UINT bmpWidth = 0;
    static UINT bmpHeight = 0;
    static UINT defFmHt = 0;
    static UINT defFmWd = 0;
    static int fScroll;             // 1 if horz scrolling, -1 vert scrolling, 0 for WM_SIZE
    static BOOL fSize;          // TRUE if WM_SIZE 
    static BOOL windowMoved;
    static BOOL isScreenshot;
    static int xNewSize;
    static int yNewSize;
    static int scrollStat;      // 0: None, 1: SB_HORZ, 2: SB_VERT, 3: SB_BOTH




    static int  iDeltaPerLine;      // for mouse wheel logic
    static int iAccumDelta;
    static ULONG ulScrollLines;

    static UINT SMOOTHSCROLL_FLAG;
    static UINT SMOOTHSCROLL_SPEED;

    switch (uMsg)
    {
    case WM_CREATE:
    {
        timDragWindow = 1;
        // Start Gdiplus
        siHORZ.cbSize = 0;
        siVERT.cbSize = 0;
        SMOOTHSCROLL_SPEED = 0X00000002;
        ulScrollLines = 0;

        GetDims(hWnd);
        RECT recthWndtmp = RectCl().RectCl(0, hWnd, 1);
        defFmWd = RectCl().width(1);
        defFmHt = RectCl().height(1);
        if (groupboxFlag)
        {

            hWndGroupBox = CreateWindowEx(0, TEXT("BUTTON"),
                TEXT(""),
                WS_VISIBLE | WS_CHILD | BS_GROUPBOX | WS_CLIPSIBLINGS, //consider  WS_CLIPCHILDREN
                // also MSDN: Do not combine the BS_OWNERDRAW style with any other button styles!
                0, 0, wd, defFmHt,
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
            WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP | BS_CENTER,  // <---- WS_GROUP group the following radio buttons 1st,2nd button 
            2, ht + 10,
            wd - 2, ht / 2,
            hWnd, //<----- Use main window handle
            (HMENU)IDC_OPT1,
            (HINSTANCE)NULL, NULL);
        hWndOpt2 = CreateWindowEx(WS_EX_WINDOWEDGE,
            L"BUTTON",
            L"ScrollEx",
            WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | BS_CENTER,  // Styles 
            2, 2 * ht,
            wd - 2, ht / 2,
            hWnd,
            (HMENU)IDC_OPT2,
            (HINSTANCE)NULL, NULL);
        SendMessage(hWndOpt1, BM_SETCHECK, BST_CHECKED, 0);


        hWndChk = CreateWindowEx(WS_EX_WINDOWEDGE,
            L"BUTTON",
            L"Stretch",
            WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX | BS_CENTER,  // Styles 
            2, 3 * ht,
            wd - 2, ht / 2,
            hWnd,
            (HMENU)IDC_CHK,
            (HINSTANCE)NULL, NULL);



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
        fScroll = 0;
        fSize = FALSE;

        // Initialize the horizontal scrolling variables. 
        xCurrentScroll = 0;
        // Initialize the vertical scrolling variables. 
        yCurrentScroll = 0;


        //SMOOTHSCROLL_FLAG = MAKELRESULT((USHORT)SW_SMOOTHSCROLL, SMOOTHSCROLL_SPEED);
        SMOOTHSCROLL_FLAG = SW_SMOOTHSCROLL | SMOOTHSCROLL_SPEED;


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
        windowMoved = FALSE;
        return 0;
        break;
    }
    case WM_SIZING:
    {
        fScroll = 0;
        fSize = TRUE;
        return TRUE;
        // Remove the above return to find that:
        // Custom painting during the sizing might have been a good idea,
        // except that the system invalidates the window with COLOR_WINDOW + 1
        //after each bitblt, which causes flicker.
        if (!timDragWindow || (capCallFrmResize == 5))
        {
            timDragWindow = SetTimer(hWnd,             // handle to main window 
                IDT_DRAGWINDOW,                   // timer identifier 
                10,                           // millisecond interval 
                (TIMERPROC)NULL);               // no timer callback 

            if (timDragWindow == 0)
            {
                ReportErr(L"No timer is available.");
                capCallFrmResize++;
            }
            else
            capCallFrmResize = 0;
        }
        else
        {
                capCallFrmResize++;
                fScroll = 0;
                fSize = TRUE;
                if (capCallFrmResize > 5)
                {
                    ReportErr(L"Timer var incorrect!");
                    Kleenup(hWnd, hBitmap, hbmpCompat, pgpbm, hdcMem, hdcMemIn, hdcWinCl);
                    PostQuitMessage(0);
                }
        }
        return TRUE;
    }
    break;
    case WM_TIMER:
    {
        if (wParam == IDT_DRAGWINDOW)
        {
            tmp = KillTimer(hWnd, IDT_DRAGWINDOW);
            if (tmp)
            {
                /*
                // The following causes excess drawing + painting && thus unecessary
                SizeControls(bmp, hWnd, defFmWd, defFmHt, 0);
                hdcWin = GetWindowDC(hWnd);
                AdjustImage(isScreenshot, bm, bmp, gps, hdcMem, hdcScreen, hdcScreenCompat, hdcWin, hdcWinCl, bmpWidth, bmpHeight);
                //ReportErr(L"AdjustImage detected a problem with the image!");
                ReleaseDC(hWnd, hdcWin);
                */
                capCallFrmResize = 0;
            }
            else
                ReportErr(L"Problem with timer termination.");
        }
        return 0;
    }
    break;
    case WM_MOVE:
    {
        if (!windowMoved)
        {
            if (!isLoading)
                windowMoved = TRUE;
            KillTimer(hWnd, IDT_DRAGWINDOW);
            timDragWindow = 0;
        }
        return 0;
    }
    break;
    case WM_ENTERSIZEMOVE:
    {
        KillTimer(hWnd, IDT_DRAGWINDOW);
        SizeControls(bmp, hWnd, defFmWd, defFmHt, START_SIZE_MOVE);
        //InvalidateRect(hWnd, 0, TRUE);
        timDragWindow = SetTimer(hWnd,             // handle to main window 
            IDT_DRAGWINDOW,                   // timer identifier 
            10,                           // millisecond interval 
            (TIMERPROC)NULL);               // no timer callback 
        return 0;
    }
    break;
    case WM_EXITSIZEMOVE:
    {
        fSize = TRUE;
        //InvalidateRect(hWnd, 0, TRUE);
        KillTimer(hWnd, IDT_DRAGWINDOW);
        if (windowMoved)
        windowMoved = FALSE;
        else
        {
            timDragWindow = 0;
            capCallFrmResize = 0;
            SizeControls(bmp, hWnd, defFmWd, defFmHt, END_SIZE_MOVE);
            hdcWin = GetWindowDC(hWnd);
            if (!AdjustImage(isScreenshot, hBitmap, bmp, gps, hdcMem, hdcMemIn, hdcScreen, hdcScreenCompat, hdcWin, hdcWinCl, bmpWidth, bmpHeight, xCurrentScroll, 1 + chkChk, 0))
                ReportErr(L"AdjustImage detected a problem with the image!");
            ReleaseDC(hWnd, hdcWin);
        }
        return 0;
    }
    break;

    case WM_SIZE:
    {
        BOOL maxMinSize = (wParam == SIZE_MINIMIZED || wParam == SIZE_MAXIMIZED);
        // WM_SIZE called for each child control (no subclass)
        if (!(szFile && (szFile[0] == L'*')) && (!capCallFrmResize || !timDragWindow || maxMinSize))
        {

            xNewSize = LOWORD(lParam);
            yNewSize = HIWORD(lParam);
            SizeControls(bmp, hWnd, defFmWd, defFmHt, wParam, xNewSize, yNewSize);


            if (!isLoading)
            {
                hdcWin = GetWindowDC(hWnd);
                if (!AdjustImage(isScreenshot, hBitmap, bmp, gps, hdcMem, hdcMemIn, hdcScreen, hdcScreenCompat, hdcWin, hdcWinCl, bmpWidth, bmpHeight, 0, (chkChk)? 2: 0, (wParam != SIZE_RESTORED)))
                    ReportErr(L"AdjustImage detected a problem with the image!");
                ReleaseDC(hWnd, hdcWin);


                    if (fSize && (wParam == SIZE_RESTORED))
                        timDragWindow = 0;
                    fSize = TRUE;
                    // The horizontal scrolling range is defined by 
                // (bitmap_width) - (client_width). The current horizontal 
                // scroll value remains within the horizontal scrolling range. 
            }
            ScrollInfo(hWnd, xCurrentScroll, yCurrentScroll, 0, xMaxScroll, yMaxScroll, bmpWidth, bmpHeight, defFmWd, defFmHt, yTrackPos, xMinScroll, yMinScroll, xNewSize, yNewSize);
        }
        return 0;
    }
    break;
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
                        //prect->left + (isScreenshot ? 0 : ((fScroll == 1) ? 0 : -wd)),
                        // Blt at wd method
                        //prect->left + (isScreenshot ? (fScroll == 1 ? 0 : wd) : (((fScroll == 1) && (xCurrentScroll > wd)) ? 0 : wd)),
                        prect->left,
                        prect->top,
                        //prect->left + ( (isScreenshot) ? 0 : wd * scaleX), prect->top,
                        (prect->right - prect->left + ((xCurrentScroll < wd)? wd: 0)),
                        (prect->bottom - prect->top),
                        isScreenshot ? hdcScreenCompat : hdcMem,
                        prect->left + xCurrentScroll,
                        prect->top + yCurrentScroll,
                        SRCCOPY);

                    if (!groupboxFlag) // Paint sections
                {
                        if (xCurrentScroll < wd)
                        {
                            // consider test like RectCl().width(1) - prect->left > wd
                                RECT rect;
                                rect.left = prect->left;
                                if (!rect.left)
                                {
                                    rect.right = prect->left + wd - xCurrentScroll;
                                    rect.top = prect->top;
                                    rect.bottom = prect->bottom;
                                    FillRect(ps.hdc, &rect, (HBRUSH)(COLOR_WINDOW + 1));
                                }
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
                        bmpWidth, bmpHeight,
                        hdcWinCl,
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
                            if (prect->left <  wd - 1) // Issue when form is sized small horizontally
                            {
                                RECT rect;
                                rect.top = prect->top;
                                rect.bottom = prect->bottom;
                                rect.left = prect->left;
                                tmp = RectCl().width(0);
                                if (tmp < wd)
                                    tmp = wd;
                                rect.right = prect->left + tmp - xCurrentScroll;
                                FillRect(ps.hdc, &rect, (HBRUSH)(COLOR_WINDOW + 1));
                            }

                            //UpdateWindow(hWnd);
                        }
                    }
                }
                else
                    if (!isLoading && scrollStat)
                        ReportErr(L"Cannot get here, fSize should be set...");
            }

            EndPaint(hWnd, &ps);
        }
        else
            RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);
        return 0;
    }
    break;
    case WM_GETMINMAXINFO:
        // prevent the window from becoming too small
        ((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
        ((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
        return 0;
        break;
    case WM_HSCROLL:
    {
 
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
            rect.bottom = defFmHt * scaleY;
            rect.right = bmp.bmWidth;
            ScrollWindow(hWnd, -xDelta, -yDelta, (CONST RECT*) NULL, &rect);
            */
            ScrollWindow(hWnd, -xDelta, 0, (CONST RECT*) NULL, (CONST RECT*) NULL);
        }
        else
        {
            ScrollWindowEx(hWnd, -xDelta, 0, (CONST RECT*) NULL,
                (CONST RECT*) NULL, (HRGN)NULL, (PRECT)NULL, SW_SCROLLCHILDREN | SW_INVALIDATE); // SMOOTHSCROLL_FLAG fails
            UpdateWindow(hWnd);
        }
        // Reset the scroll bar. 
        ScrollInfo(hWnd, xCurrentScroll, yCurrentScroll, 1, xMaxScroll, yMaxScroll, bmpWidth, bmpHeight);
        fScroll = 1;
        return 0;

    }
    break;
    case WM_VSCROLL:
    {

        // Scroll the window. (The system repaints most of the 
        // client area when ScrollWindow(Ex) is called; however, it is 
        // necessary to call UpdateWindow in order to repaint the 
        // rectangle of pixels that were invalidated.) 
        if (optChk)
            ScrollWindow(hWnd, 0, -yDelta, (CONST RECT*) NULL, (CONST RECT*) NULL);
        else
        {
            ScrollWindowEx(hWnd, 0, -yDelta, (CONST RECT*) NULL,
                (CONST RECT*) NULL, (HRGN)NULL, (PRECT)NULL, SW_SCROLLCHILDREN | SW_INVALIDATE); // SMOOTHSCROLL_FLAG fails
            UpdateWindow(hWnd);
        }

        // Reset scroll bar. 
        ScrollInfo(hWnd, xCurrentScroll, yCurrentScroll, -1, xMaxScroll, yMaxScroll, bmpWidth, bmpHeight);
        fScroll = -1;
        return 0;
    }
    break;
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
    }
    break;
    case WM_COMMAND: // WM_PARENTNOTIFY if hWndGroupBox is parent to controls (not the case here)
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
        }
        break;
        case IDC_OPT2:
        {
            if (wmEvent == BN_CLICKED)
                optChk = (SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) != BST_CHECKED);
            // Also BST_INDETERMINATE, BST_UNCHECKED
        }
        break;
        case IDC_CHK:
        {
            if (wmEvent == BN_CLICKED)
                chkChk = (SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED);
            // Also BST_INDETERMINATE, BST_UNCHECKED
        }
        break;
        case ID_OPENBITMAP:
        {
            //https://social.msdn.microsoft.com/Forums/sqlserver/en-US/76441f64-a7f2-4483-ad6d-f51b40464d6b/how-to-get-both-width-and-height-of-bitmap-in-gdiplus-flat-api?forum=windowsgeneraldevelopmentissues
            if (hBitmap)
                DeleteObject(hBitmap);

            if (pgpbm)
                gps = GdipDisposeImage(pgpbm);

            Color clr;

            szFile = (wchar_t*)calloc(MAX_LOADSTRING, sizeof(wchar_t));
            wcscpy_s(szFile, MAX_LOADSTRING, FileOpener(hWnd));


            //CLSID pngClsid;
           // GetEncoderClsid(L"image/png", &pngClsid);
            if (szFile[0] != L'*')
            {

                //wd = RectCl().width(2);
                //ht = RectCl().height(1);
                const std::vector<std::vector<unsigned>>resultPixels;

                gps = GdipCreateBitmapFromFile(szFile, &pgpbm);
                if (gps == Ok)
                {
                    ReleaseDC(hWnd, hdcWinCl);
                    hdcWinCl = GetDCEx(hWnd, (HRGN)NULL, DCX_CACHE | DCX_CLIPCHILDREN);


                    HBITMAP hBitmap = NULL;
                    gps = GdipCreateHBITMAPFromBitmap(pgpbm, &hBitmap, clr.GetValue());
                    // black = (bits == 0);   alpha=255 => 0xFF000000
                    gps = GdipGetImageWidth(pgpbm, &bmpWidth);
                    gps = GdipGetImageHeight(pgpbm, &bmpHeight);
                    //HBITMAP hBmpCopy = (HBITMAP)CopyImage(hBitmap, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
                    //GpStatus WINGDIPAPI GdipDrawImageI(GpGraphics* graphics, GpImage* image, INT x, INT y);

  
                    hdcMemIn = CreateCompatibleDC(hdcWinCl);
                    hdcMem = CreateCompatibleDC(hdcWinCl);
                    hdcWin = GetWindowDC(hWnd);
                    isScreenshot = FALSE;
                    if (!AdjustImage(isScreenshot, hBitmap, bmp, gps, hdcMem, hdcMemIn, hdcScreen, hdcScreenCompat, hdcWin, hdcWinCl, bmpWidth, bmpHeight, xCurrentScroll, 2, 0))
                        ReportErr(L"AdjustImage detected a problem with the image!");
                    //SizeControls(bmp, hWnd, defFmWd, defFmHt, -1, xCurrentScroll, yCurrentScroll);
                    xCurrentScroll ? fScroll = 1 : fScroll = -1;
                    szFile[0] = L'*'; // Set to deal with WM_SIZE issues
                    scrollStat = ScrollInfo(hWnd, xCurrentScroll, yCurrentScroll, 0, xMaxScroll, yMaxScroll, bmpWidth, bmpHeight, defFmWd, defFmHt, yTrackPos, xMinScroll, yMinScroll, xNewSize, yNewSize);
                    szFile[0] = L'X';
                    xCurrentScroll = 0;
                    yCurrentScroll = 0;
                    //SetWindowOrgEx(hdcWinCl, xCurrentScroll, yCurrentScroll, NULL);

                    switch (scrollStat)
                    {
                    case 1:
                        SetScrollPos(hWnd, SB_HORZ, 0, TRUE);
                        break;
                    case 2:
                        SetScrollPos(hWnd, SB_VERT, 0, TRUE);
                        break;
                    case 3:
                        SetScrollPos(hWnd, SB_BOTH, 0, TRUE);
                        break;
                    default:
                        fScroll = 0;
                    break;
                    }
                    if (!AdjustImage(isScreenshot, hBitmap, bmp, gps, hdcMem, hdcMemIn, hdcScreen, hdcScreenCompat, hdcWin, hdcWinCl, bmpWidth, bmpHeight, xCurrentScroll))
                        ReportErr(L"AdjustImage detected a problem with the image!");

                    

                    ReleaseDC(hWnd, hdcWin);

                }
                else
                    ReportErr(L"Cannot open bitmap!");
            }
            free(szFile);
            fSize = FALSE;
            return (INT_PTR)TRUE;
        }
        break;
        case IDM_ABOUT:
        {
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        }
        case IDM_EXIT:
        {
            Kleenup(hWnd, hBitmap, hbmpCompat, pgpbm, hdcMem, hdcMemIn, hdcWinCl);
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
        bmpWidth = (scaleX * RectCl().width(1)) - wd;
        bmpHeight = scaleY * (RectCl().height(1));


        hBmp = CreateCompatibleBitmap(hdcWinCl, bmpWidth, bmpHeight);

        PrintWindow(hWnd, hBmp);
        SelectObject(hdcMem, hBmp);


        if (!BitBlt(hdcWinCl, wd, RectCl().ClMenuandTitle(hWnd), bmpWidth, bmpHeight, hdcMem, 0, RectCl().ClMenuandTitle(hWnd), SRCCOPY))
            ReportErr(L"Bad BitBlt from hdcMem!");


        if (hBmp)
            DeleteObject(hBmp);


        isScreenshot = FALSE;
        return 0;
    }
    break;
    case WM_RBUTTONDOWN:
    {
        if (hBitmap)
            DeleteObject(hBitmap);
        if (pgpbm)
            gps = GdipDisposeImage(pgpbm);
        isScreenshot = TRUE;
        // Get the compatible DC of the client area. 
        ReleaseDC(hWnd, hdcWinCl);
        hdcWinCl = GetDCEx(hWnd, (HRGN)NULL, DCX_CACHE | DCX_CLIPCHILDREN);

        //SizeControls(bmp, hWnd, defFmWd, defFmHt, -1, xCurrentScroll, yCurrentScroll);

        /*
        ScrollWindowEx(hWnd, -xCurrentScroll, -yCurrentScroll, (CONST RECT*) NULL,
            (CONST RECT*) NULL, (HRGN)NULL, (PRECT)NULL, SW_SCROLLCHILDREN | SW_INVALIDATE); // SMOOTHSCROLL_FLAG fails
        tmp = GetLastError();
        SetViewportOrgEx(hdcScreenCompat, xCurrentScroll, yCurrentScroll, NULL);
        SetWindowOrgEx(hdcScreenCompat, xCurrentScroll, yCurrentScroll, NULL);

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

        hdcWin = GetWindowDC(hWnd);

        if (!AdjustImage(isScreenshot, hBitmap, bmp, gps, hdcMem, hdcMemIn, hdcScreen, hdcScreenCompat, hdcWin, hdcWinCl, bmpWidth, bmpHeight, xCurrentScroll, 2, 0))
            ReportErr(L"AdjustImage detected a problem with the image!");
        xCurrentScroll ? fScroll = 1 : fScroll = -1;
        scrollStat = SetWindowOrgEx(hdcWinCl, -xCurrentScroll, yCurrentScroll, NULL);
        //xCurrentScroll = 0;
        //yCurrentScroll = 0;
        ScrollInfo(hWnd, xCurrentScroll, yCurrentScroll, 0, xMaxScroll, yMaxScroll, bmpWidth, bmpHeight, defFmWd, defFmHt, yTrackPos, xMinScroll, yMinScroll, xNewSize, yNewSize);


        switch (scrollStat)
        {
        case 1:
            SetScrollPos(hWnd, SB_HORZ, xCurrentScroll, TRUE);
            break;
        case 2:
            SetScrollPos(hWnd, SB_VERT, yCurrentScroll, TRUE);
            break;
        case 3:
        {
            SetScrollPos(hWnd, SB_HORZ, xCurrentScroll, TRUE);
            SetScrollPos(hWnd, SB_VERT, yCurrentScroll, TRUE);
        }
        break;
        default:
        break;
        }

        //SetWindowOrgEx(hdcWinCl, xCurrentScroll, yCurrentScroll, NULL);


        ReleaseDC(hWnd, hdcWin);
        fSize = FALSE;
        return 0;
        break;
    }
    case WM_KEYDOWN:
    {
        switch (wParam)
        {
        case VK_ESCAPE:
        {
            Kleenup(hWnd, hBitmap, hbmpCompat, pgpbm, hdcMem, hdcMemIn, hdcWinCl);
            PostQuitMessage(0);
            break;
        }
        }
        return 0;
        break;
    }
    case WM_DESTROY:
    {
        Kleenup(hWnd, hBitmap, hbmpCompat, pgpbm, hdcMem, hdcMemIn, hdcWinCl);
        PostQuitMessage(0);
        return 0;
        break;
    }
    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
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
    }
    break;
    case WM_NCDESTROY:
        // NOTE: this requirement is NOT stated in the documentation, but it is stated in Raymond Chen's blog article...
        RemoveWindowSubclass(hWnd, staticSubClass, uIdSubclass);
        return 0;
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
        return 0;
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
    break;
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

wchar_t* FileOpener(HWND hWnd)
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
void DoSysInfo()
{

    HMONITOR hMon = MonitorFromWindow(GetDesktopWindow(), MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO monInfo;
    monInfo.cbSize = sizeof(MONITORINFO);


    if (GetMonitorInfoW(hMon, &monInfo))
    {
        if (monInfo.dwFlags != MONITORINFOF_PRIMARY)
            ReportErr(L"ScrollCall has bot been fully tested on a secondary monitor!.");
    }
    else
        ReportErr(L"GetMonitorInfo: Cannot get info.");
}
BOOL AdjustImage(BOOL isScreenshot, HBITMAP hBitmap, BITMAP bmp, GpStatus gps, HDC hdcMem, HDC hdcMemIn, HDC hdcScreen, HDC hdcScreenCompat, HDC hdcWin, HDC hdcWinCl, UINT& bmpWidth, UINT& bmpHeight, int xCurrentScroll, int resizePic, BOOL maxMin)
{
    static int oldWd = 0;

    BOOL retVal = FALSE;

    BITMAP bm = { 0 };
    HGDIOBJ hBmpObj = GetCurrentObject(hdcWin, OBJ_BITMAP); //hBmpObj =>handle to bitmap (HBITMAP)
    if (hBmpObj && GetObject(hBmpObj, sizeof(BITMAP), &bm)) //bm => BITMAP structure
    {
        //bm.biBitCount = 32;
        bmpWidth = (isScreenshot) ? bmp.bmWidth : bmpWidth;
        bmpHeight = (isScreenshot) ? bmp.bmHeight : bmpHeight;
    }
    else
        ReportErr(L"AdjustImage: Unable to size bitmap!");
    RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.bottom = bmpHeight;
    rect.right = bmpWidth;


    if (isScreenshot)
    {
        if (resizePic)
        {
            retVal = (BOOL)FillRect(hdcWinCl, &rect, (HBRUSH)(COLOR_WINDOW + 1)); //SetBkColor(hdcWinCl, COLOR_WINDOW + 1) causes flickering in the scrolling

            if (resizePic == 2)
            retVal = StretchBlt(hdcScreenCompat, wd, 0, bmpWidth - wd,
                bmpHeight, hdcScreen, 0, 0, bmpWidth, bmpHeight, SRCCOPY);

            if (retVal)
                retVal = (BOOL)BitBlt(hdcWinCl, (xCurrentScroll)? wd - xCurrentScroll: wd, 0, bmpWidth, bmpHeight, hdcScreenCompat, wd, 0, SRCCOPY); //Blt at wd method
            oldWd = wd;
        }
        else
        {
            if (maxMin)
                retVal = StretchBlt(hdcWinCl, wd, 0, bmpWidth,
                    bmpHeight, hdcScreenCompat, oldWd, 0, bmpWidth, bmpHeight, SRCCOPY);
            else
                retVal = (BOOL)BitBlt(hdcWinCl, wd, 0, bmpWidth, bmpHeight, hdcScreenCompat, oldWd, 0, SRCCOPY); //Blt at wd method
        }
            //retVal = (BOOL)BitBlt(hdcScreenCompat, wd, 0, bmp.bmWidth,
                //bmp.bmHeight, hdcScreen, 0, 0, SRCCOPY);
            // Copy the compatible DC to the client area.
            //retVal = (BOOL)BitBlt(hdcWinCl, wd, 0, bmp.bmWidth, bmp.bmHeight, hdcScreenCompat, 0, 0, SRCCOPY); //Blt at zero method causes problems with horz scrolling

    }
    else
    {
        if (resizePic)
        {
            retVal = (BOOL)FillRect(hdcWinCl, &rect, (HBRUSH)(COLOR_WINDOW + 1)); //SetBkColor(hdcWinCl, COLOR_WINDOW + 1) causes flickering in the scrolling

            //retVal = (int)SelectObject(hdcMem, hBitmap);
            HBITMAP hBmp = CreateCompatibleBitmap(hdcWinCl, bmpWidth + wd, bmpHeight);
            //retVal = (int)SelectObject(hdcMem, hBmpObj);
            retVal = (int)SelectObject(hdcMem, hBmp);

            // In later calls, hBitmap goes out of scope, so better handling is required
            if (hBitmap)
                retVal = (UINT64)SelectObject(hdcMemIn, hBitmap);
            if (!retVal || (retVal == (BOOL)HGDI_ERROR))
                ReportErr(L"AdjustImage: Cannot use bitmap!");
            retVal = (BOOL)BitBlt(hdcMem, wd, 0, bmpWidth, bmpHeight, hdcMemIn, 0, 0, SRCCOPY); //Blt at wd method

            //retVal = StretchBlt(hdcMem, wd, 0, bmpWidth - wd, bmpHeight, hdcMemIn, 0, 0, bmpWidth, bmpHeight, SRCCOPY);
            if (retVal)
                retVal = (BOOL)BitBlt(hdcWinCl, 0, 0, bmpWidth + wd, bmpHeight, hdcMem, 0, 0, SRCCOPY); //Blt at wd method
            oldWd = wd;
            if (!DeleteObject(hBmp))
                ReportErr(L"AdjustImage: Cannot delete bitmap object!!");
        }
        else
        {
            if (maxMin)
                retVal = StretchBlt(hdcWinCl, wd, 0, bmpWidth,
                    bmpHeight, hdcMem, oldWd, 0, bmpWidth, bmpHeight, SRCCOPY);
            else
                retVal = (BOOL)BitBlt(hdcWinCl, wd, 0, bmpWidth, bmpHeight, hdcMem, oldWd, 0, SRCCOPY); //Blt at wd method
        }

    }

    return retVal;
}
void GetDims(HWND hWnd, int resizeType)
{
    static float firstWd = 0, firstHt = 0, savedWd = 0, savedHt = 0, savedScaleX = 1, savedScaleY = 1, oldWd = 0, oldHt = 0;
    // int will not compute
    static int startCtrlHt = 0;
    static BOOL firstSizeAfterSTART_SIZE_MOVE = FALSE;


    if (oldWd)
    {

        if (resizeType != SIZE_MINIMIZED && resizeType != MAX_TO_MIN)
        {
            if (resizeType != START_SIZE_MOVE && !firstSizeAfterSTART_SIZE_MOVE)
            {
                oldWd = wd;
                oldHt = ht;
            }
            //For button

            if (!isLoading)
            {
                RECT recthWndtmp;
                GetWindowRect(hWnd, &recthWndtmp);
                wd = (int)(recthWndtmp.right - recthWndtmp.left) / 9;
                ht = (int)(recthWndtmp.bottom - recthWndtmp.top) / 9;
            }
        }


    }
    else
    {
        //Init class: Must be done twice because of default window positions before WM_SIZE.
        RECT recthWndtmp = RectCl().RectCl(0, hWnd, 0);
        if (firstWd)
        {
            firstWd = oldWd = wd = RectCl().width(0);
            firstHt = oldHt = ht = RectCl().height(0);
        }
        else
        {
            firstWd = wd = RectCl().width(0);
            firstHt = ht = RectCl().height(0);
            if (recthWndtmp.left < GetSystemMetrics(0) && recthWndtmp.bottom < GetSystemMetrics(1))
                MoveWindow(hWnd, GetSystemMetrics(0) / 4, GetSystemMetrics(1) / 4, GetSystemMetrics(0) / 2, GetSystemMetrics(1) / 2, 1);
            else
                ReportErr(L"Not a primary monitor: Resize unavailable.");
        }
    }

    if (wd < firstWd)
        wd = firstWd;
    if (ht < firstHt)
        ht = firstHt;

    switch (resizeType)
    {
    case SIZE_RESTORED: // 0
    {
        if (firstSizeAfterSTART_SIZE_MOVE)
            oldWd = oldWd;
        oldWd ? (scaleX = wd / oldWd) : scaleX = 1;
        oldHt ? (scaleY = ht / oldHt) : scaleY = 1;
        firstSizeAfterSTART_SIZE_MOVE = FALSE;
    }
    break;
    case START_SIZE_MOVE:
    {
        RECT rectBtmp = RectCl().RectCl(hWndButton, hWnd, 2);
        RECT rectOpt1tmp = RectCl().RectCl(hWndOpt1, hWnd, 3);
        startCtrlHt = rectOpt1tmp.top - rectBtmp.top;
        savedWd = wd;
        savedHt = ht;
        //don't care about scale, but without it, controls revert to default sizes
        scaleX = wd / oldWd;
        scaleY = ht / oldHt;
        if (wd != oldWd)
        savedScaleX = scaleX;
        if (ht != oldHt)
        savedScaleY = scaleY;
        firstSizeAfterSTART_SIZE_MOVE = TRUE;

    }
    break;
    case END_SIZE_MOVE:
    {
        if (wd == savedWd)
            scaleX = savedScaleX;
        else
        {
            //oldWd = savedWd;
            scaleX = wd / savedWd;
        }

        if (oldHt == savedHt)
            scaleY = savedScaleY;
        else
        {
            //oldHt = savedHt;
            scaleY = ht / savedHt;
            /*
            RECT rectBtmp = RectCl().RectCl(hWndButton, hWnd, 2);
            RECT rectOpt1tmp = RectCl().RectCl(hWndOpt1, hWnd, 3);
            GetWindowRect(hWndButton, &rectBtmp); //get window rect of control relative to screen
            SetLastError(0);
            if (!MapWindowPoints(NULL, hWnd, (LPPOINT)&rectBtmp, 2))
                if (GetLastError())
                    ReportErr(L"MapWindowPoints: Fail.");;
            float curScaleY = (float)(rectOpt1tmp.top - rectBtmp.top) / startCtrlHt;
            if (scaleY > 1)
            {
                if (scaleY > curScaleY)
                    scaleY = curScaleY;
            }
            else
            {
                if (scaleY < curScaleY)
                    scaleY = curScaleY;
            }
            */
        }
    }
    break;
    case SIZE_MAXIMIZED:
    {
        scaleX = wd / oldWd;
        scaleY = ht / oldHt;
    }
    break;
    default: // SIZE_MINIMIZED
    {
        scaleX = oldWd / wd;
        scaleY = oldHt / ht;
        wd = oldWd;
        ht = oldHt;
    }


    }
    /*For screen
    sizefactorX = 1, sizefactorY = 1
    sizefactorX = GetSystemMetrics(0) / (3 * wd);
    sizefactorY = GetSystemMetrics(1) / (2 * ht);
    */
}

void SizeControls(BITMAP bmp, HWND hWnd, UINT defFmWd, UINT defFmHt, int resizeType, int xNewSize, int yNewSize)
{
    int opt1Ht = 0, opt2Ht = 0, chkHt = 0, btnWd = 0, btnHt = 0, curFmWd = 0, curFmHt = 0;
    int newHt = 0, newEdgeWd = 0, newWd = 0, newEdgeHt = 0;
    int updatedxCurrentScroll = 0, updatedyCurrentScroll = 0;

    //static BOOL 
    static int oldResizeType = 0, oldFmWd = 0, oldFmHt = 0;
    static int defOpt1Top = 0, defOpt2Top = 0, defChkTop = 0, scrollDefBtnTop = 0, scrollDefOpt1Top = 0, scrollDefOpt2Top = 0, scrolldefChkTop = 0;
    static int oldBtnLeft = 0, oldBtnTop  = 0, oldOpt1Top = 0, oldOpt2Top = 0, oldChkTop = 0;
    static int minWd = 0, minHt = 0, startSizeBtmpLeft = 0, startSizeBtmpTop = 0, startSizeBtmpRight = 0, startSizeBtmpBottom = 0, startSizeOpt1Top = 0, startSizeOpt2Top = 0, startSizeChkTop = 0;

    if (!minWd)
    {
        minWd = RectCl().width(0);
        minHt = RectCl().height(0);
    }


    if (oldResizeType == SIZE_MAXIMIZED && resizeType == SIZE_MINIMIZED)
        resizeType = MAX_TO_MIN;

    //Get updated rect for the form
    RECT rectB, rectOpt, rectChk;
    RECT rectOpt1tmp = {}, rectOpt2tmp = {}, rectChktmp = {}, rectBtmp = {};

    RECT recthWndtmp = RectCl().RectCl(0, hWnd, 1);
    curFmWd = RectCl().width(1);
    curFmHt = RectCl().height(1);
    GetDims(hWnd, resizeType);

    if (capCallFrmResize && oldFmHt && (curFmHt - oldFmHt != 0) && (curFmWd - oldFmWd != 0))
        return;

    if ((ht > 10) && !defOpt1Top)
    {
        defOpt1Top = ht + 10;
        defOpt2Top = 2 * ht;
        defChkTop = 3 * ht;
    }

    rectBtmp = RectCl().RectCl(hWndButton, hWnd, 2);
    rectOpt1tmp = RectCl().RectCl(hWndOpt1, hWnd, 3);
    rectOpt2tmp = RectCl().RectCl(hWndOpt2, hWnd, 4);
    rectChktmp = RectCl().RectCl(hWndChk, hWnd, 5);




    if (resizeType == END_SIZE_MOVE)
    {

        if (yCurrentScroll < 0)
        {
            xCurrentScroll = 0;
            yCurrentScroll = 0;
        }

        if (oldFmHt != curFmHt)
        {
        //rectOpt1tmp.top = startSizeOpt1Top;
        //rectOpt2tmp.top = startSizeOpt2Top;
        //rectChktmp.top = startSizeChkTop;
        btnHt = startSizeBtmpBottom - startSizeBtmpTop;
        //rectBtmp.top = -yCurrentScroll;
        rectOpt1tmp.top += scaleY * (rectOpt1tmp.top - startSizeOpt1Top);
        rectOpt2tmp.top += scaleY * (rectOpt2tmp.top - startSizeOpt2Top);
        rectChktmp.top += scaleY * (rectChktmp.top - startSizeChkTop);
        //rectOpt1tmp.top = scaleY * defOpt1Top - yCurrentScroll;
        //rectOpt2tmp.top = scaleY * defOpt2Top - yCurrentScroll;
        //rectChktmp.top = scaleY * defChkTop - yCurrentScroll;
            if (oldFmWd == curFmWd)
                btnWd = minWd;
            else
                btnWd = startSizeBtmpRight - startSizeBtmpLeft;
        }
        else
        {
            btnWd = startSizeBtmpRight - startSizeBtmpLeft;
            btnHt = minHt;
        }

        rectBtmp.left = startSizeBtmpLeft;
        rectOpt1tmp.left = startSizeBtmpLeft;
        rectOpt2tmp.left = startSizeBtmpLeft;
        rectChktmp.left = startSizeBtmpLeft;


    }
    else
    {
        GetClientRect(hWndButton, &rectB);
        btnWd = rectB.right - rectB.left;
        btnHt = rectB.bottom - rectB.top;

        scrollDefBtnTop =  -yCurrentScroll;
        scrollDefOpt1Top = defOpt1Top - yCurrentScroll;
        scrollDefOpt2Top = defOpt2Top - yCurrentScroll;
        scrolldefChkTop = defChkTop - yCurrentScroll;



        if (resizeType == START_SIZE_MOVE)
        {
            startSizeBtmpTop = max(rectBtmp.top, scrollDefBtnTop);
            startSizeOpt1Top = max(rectOpt1tmp.top, scrollDefOpt1Top);
            startSizeOpt2Top = max(rectOpt2tmp.top, scrollDefOpt2Top);
            startSizeChkTop = max(rectChktmp.top, scrolldefChkTop);
            startSizeBtmpLeft = rectBtmp.left;
            startSizeBtmpRight = rectBtmp.right;
            startSizeBtmpBottom = rectBtmp.bottom;
            oldFmWd = curFmWd;
            oldFmHt = curFmHt;
            // Unfortunately, applying SetWindowPos here causes more sizing loops,
            // and the size and position of the controls is even worse than current.
            return;
        }

    }
    //rectB.top += BOX_HEIGHT;
    if (btnWd >= minWd && btnHt >= minHt) // button size determines scale
    {

        newWd = scaleX * btnWd;
        newHt = scaleY * btnHt;
        //Extra edging for the wd - 2
        newEdgeWd = scaleX * (btnWd - 2);
        newEdgeHt = scaleY * (btnHt - 2);

        //if (resizeType == START_SIZE_MOVE || resizeType == SIZE_RESTORED)
        {
            if (newWd < minWd)
            {
                newEdgeWd = minWd - 2;
                newWd = minWd;
            }
            else
            {
                if (curFmWd < defFmWd)
                {
                    newEdgeWd = minWd - 2;
                    newWd = minWd;
                }
            }

            if (newHt < minHt)
            {
                newEdgeHt = btnHt - 2;
                newHt = minHt;
            }
            else
            {
                if (curFmHt < defFmHt)
                {
                    newEdgeHt = minHt - 2;
                    newHt = minHt;
                }
            }
        }

        if (rectBtmp.top < scrollDefBtnTop)
            rectBtmp.top = scrollDefBtnTop;

        if (resizeType == END_SIZE_MOVE)
        SetWindowPos(hWndButton, NULL, rectBtmp.left, rectBtmp.top, newWd, newHt, NULL);
        else
        {
            // If maximised, scroll value may change- left/top of control may move below zero
            // so its right side wants to be at wd, and bottom at ht.
            if (resizeType == SIZE_MAXIMIZED)
            {
                oldBtnLeft = rectBtmp.left + xCurrentScroll;
                oldBtnTop = rectBtmp.top + yCurrentScroll;
                updatedxCurrentScroll = ScrollInfo(hWnd, UPDATE_HORZSCROLLSIZE_CONTROL, 0, 0, xNewSize, yNewSize);
                updatedyCurrentScroll = ScrollInfo(hWnd, UPDATE_VERTSCROLLSIZE_CONTROL, 0, 0, xNewSize, yNewSize);
                SetWindowPos(hWndButton, NULL, oldBtnLeft - updatedxCurrentScroll, oldBtnTop - updatedyCurrentScroll,
                    newWd, newHt, NULL);
            }
            else
            {
                if (oldResizeType == SIZE_MAXIMIZED)
                    SetWindowPos(hWndButton, NULL, oldBtnLeft - xCurrentScroll, oldBtnTop - yCurrentScroll,
                        newWd, newHt, NULL);
                else
                    SetWindowPos(hWndButton, NULL, scaleX * rectBtmp.left + xCurrentScroll, scaleY * rectBtmp.top + yCurrentScroll,
                        newWd, newHt, NULL);
            }
        }

        if (groupboxFlag)
        {
            //if ((rectB.bottom - rectB.top) > defFmHt)
            SetWindowPos(hWndGroupBox, NULL, xCurrentScroll, yCurrentScroll,
                newWd, bmp.bmHeight, NULL);
        }

        GetClientRect(hWndOpt1, &rectOpt);
 
        if (resizeType == SIZE_RESTORED)
        {
            opt1Ht = scaleY * (rectOpt1tmp.bottom - rectOpt1tmp.top);
            // Must be adjusted with scroll offsets
            if (curFmHt >= defFmHt)
                (oldOpt1Top) ? (rectOpt1tmp.top = oldOpt1Top * ((oldResizeType == SIZE_RESTORED) ? scaleY : 1)) : (rectOpt1tmp.top *= scaleY);
            else
                rectOpt1tmp.top = oldOpt1Top;
        }
        else
        {
            if (resizeType != END_SIZE_MOVE)
            {
                opt1Ht = scaleY * (rectOpt.bottom - rectOpt.top);
                rectOpt1tmp.top *= scaleY;
            }
        }
        if (opt1Ht < minHt / 2)
            opt1Ht = minHt / 2;

        if (rectOpt1tmp.top < scrollDefOpt1Top)
            rectOpt1tmp.top = scrollDefOpt1Top;

        if (resizeType == END_SIZE_MOVE)
        SetWindowPos(hWndOpt1, NULL, rectOpt1tmp.left, rectOpt1tmp.top, newEdgeWd, opt1Ht, NULL);
        else
            SetWindowPos(hWndOpt1, NULL, scaleX* (rectOpt1tmp.left) + xCurrentScroll, rectOpt1tmp.top + yCurrentScroll,
                newEdgeWd, opt1Ht, NULL);




        if (resizeType == SIZE_RESTORED)
        {
            opt2Ht = scaleY * (rectOpt2tmp.bottom - rectOpt2tmp.top);
            if (curFmHt >= defFmHt)
                (oldOpt2Top) ? (rectOpt2tmp.top = oldOpt2Top * ((oldResizeType == SIZE_RESTORED) ? scaleY : 1)) : (rectOpt2tmp.top *= scaleY);
            else
                rectOpt2tmp.top = oldOpt2Top;
        }
        else
        {
            if (resizeType != END_SIZE_MOVE)
            {
                opt2Ht = scaleY * (rectOpt.bottom - rectOpt.top);
                rectOpt2tmp.top *= scaleY;
            }
        }


        if (opt2Ht < minHt / 2)
            opt2Ht = minHt / 2;

        if (rectOpt2tmp.top < scrollDefOpt2Top)
            rectOpt2tmp.top = scrollDefOpt2Top;
        if (resizeType == END_SIZE_MOVE)
        SetWindowPos(hWndOpt2, NULL, rectOpt2tmp.left, rectOpt2tmp.top, newEdgeWd, opt2Ht, NULL);
        else
        SetWindowPos(hWndOpt2, NULL, scaleX * (rectOpt2tmp.left) + xCurrentScroll, rectOpt2tmp.top + yCurrentScroll,
            newEdgeWd, opt2Ht, NULL);




            GetClientRect(hWndChk, &rectChk);

        if (resizeType == SIZE_RESTORED)
        {
            chkHt = scaleY * (rectChktmp.bottom - rectChktmp.top);
            if (curFmHt >= defFmHt)
                (oldChkTop) ? (rectChktmp.top = oldChkTop * ((oldResizeType == SIZE_RESTORED) ? scaleY : 1)) : (rectChktmp.top *= scaleY);
            else
                rectChktmp.top = oldChkTop;
        }
        else
        {
            
            if (resizeType != END_SIZE_MOVE)
            {
                chkHt = scaleY * (rectChk.bottom - rectChk.top);
                rectChktmp.top *= scaleY;
            }
        }


        if (chkHt < minHt / 2)
            chkHt = minHt / 2;

        if (rectChktmp.top < scrolldefChkTop)
            rectChktmp.top = scrolldefChkTop;
        if (resizeType == END_SIZE_MOVE)
        SetWindowPos(hWndChk, NULL, rectChktmp.left, rectChktmp.top, newEdgeWd, chkHt, NULL);
        else
        SetWindowPos(hWndChk, NULL, scaleX * (rectChktmp.left) + xCurrentScroll, rectChktmp.top + yCurrentScroll,
            newEdgeWd, chkHt, NULL);









    }
    else
    {
        if (btnWd < minWd)
        if (resizeType == END_SIZE_MOVE)
            SetWindowPos(hWndButton, NULL, rectBtmp.left, rectBtmp.top, minWd, rectBtmp.bottom - rectBtmp.top, NULL);
        else
            SetWindowPos(hWndButton, NULL, scaleX * rectBtmp.left + xCurrentScroll, scaleY * rectBtmp.top + yCurrentScroll,
                minWd, rectBtmp.bottom - rectBtmp.top, NULL);
        if (btnHt < minHt)
        {
            if (resizeType == END_SIZE_MOVE)
                SetWindowPos(hWndButton, NULL, rectBtmp.left, rectBtmp.top, rectBtmp.right - rectBtmp.left, minHt, NULL);
            else
                SetWindowPos(hWndButton, NULL, scaleX * rectBtmp.left + xCurrentScroll, scaleY * rectBtmp.top + yCurrentScroll,
                    rectBtmp.right - rectBtmp.left, minHt, NULL);
        }
    }

    if (resizeType == SIZE_RESTORED)
    {
        oldOpt1Top = rectOpt1tmp.top;
        oldOpt2Top = rectOpt2tmp.top;
        oldChkTop = rectChktmp.top;
    }

    oldResizeType = resizeType;
}
int ScrollIt(HWND hWnd, int scrollType, int scrollDrag, int currentScroll, int minScroll, int maxScroll, int trackPos)
{

    int newPos;    // new position 

        //hdcMem already adjusted for control
        //if (isScreenshot)
        //SetWindowOrgEx(hdcScreenCompat, wd * scaleX, 0, NULL);
    switch (scrollType) //(LOWORD(wParam))
    {
        // User clicked the scroll bar shaft left of the scroll box. 
    case SB_BOTTOM:         //Scrolls to lower right/bottom.
        newPos = minScroll;
        break;
    case SB_TOP:
        newPos = minScroll;
        break;
    case SB_PAGEUP:
        newPos = currentScroll - SCROLL_PAGESIZE;
        break;

        // User clicked the scroll bar shaft right/bottom of the scroll box. 
    case SB_PAGEDOWN:
        newPos = currentScroll + SCROLL_PAGESIZE;
        break;

        // User clicked left/top arrow. 
    case SB_LINEUP:
        newPos = currentScroll - SCROLL_SIZE;
        break;

        // User clicked right/bottom arrow. 
    case SB_LINEDOWN:
        newPos = currentScroll + SCROLL_SIZE;
        break;
    case SB_THUMBTRACK:
        newPos = trackPos;
        break;
        // User dragged the scroll box. 
    case SB_THUMBPOSITION:
        newPos = scrollDrag; //HIWORD(wParam);
        break;
    default:
        newPos = currentScroll;
    }

    // New position must be between 0 and screen bounds. 
    newPos = max(0, newPos);
    newPos = min(maxScroll, newPos);

    // If the current position does not change, do not scroll.
    if (newPos == currentScroll)
        return 0;




    

    return newPos;
}
int ScrollInfo(HWND hWnd, int scrollXorY, int scrollType, int scrollDrag, int xNewSize, int yNewSize, int bmpWidth, int bmpHeight, UINT defFmWd, UINT defFmHt)
{
    int newPos, retVal = 0;

    // int bmpWidth, int bmpHeight UINT defFmWd, UINT defFmHt as property or global
        // These variables are required for horizontal scrolling.
    static int xMinScroll = 0;      // minimum horizontal scroll value (starts at 0 so rarely adjusted)

    // These variables are required for vertical scrolling.
    static int yMinScroll = 0;      // minimum vertical scroll value

    static int xMaxScroll = 0;      // maximum HORZ scroll value
    static int yMaxScroll = 0;      // maximum VERT scroll value
    static int xTrackPos = 0;   // current scroll drag value
    static int yTrackPos = 0;
    siHORZ.cbSize = sizeof(siHORZ);
    siVERT.cbSize = sizeof(siVERT);
#define UPDATE_HORZSCROLLSIZE_CONTROL	93
#define UPDATE_VERTSCROLLSIZE_CONTROL	94

    if (scrollXorY == UPDATE_HORZSCROLLSIZE_CONTROL) // called from SizeControls
    {
        //ShowScrollBar(hWnd, SB_HORZ, TRUE);
        siHORZ.fMask = SIF_PAGE | SIF_POS;
        siHORZ.nPage = xNewSize;
        SetScrollInfo(hWnd, SB_HORZ, &siHORZ, TRUE);
        GetScrollInfo(hWnd, SB_HORZ, &siHORZ);
        retVal = siHORZ.nPos;
        return retVal;

    }
    else
    {
        if (scrollXorY == UPDATE_VERTSCROLLSIZE_CONTROL)
        {
            siVERT.fMask = SIF_PAGE | SIF_POS;
            siVERT.nPage = yNewSize;
            SetScrollInfo(hWnd, SB_VERT, &siVERT, TRUE);
            GetScrollInfo(hWnd, SB_VERT, &siVERT);
            retVal = siVERT.nPos;
            return retVal;
        }
    }

    if (scrollXorY == SCROLL_RIGHTDOWN) // scrollXorY > 0 HORZ scroll, < 0 VERT scroll
    {

        siHORZ.fMask = SIF_TRACKPOS;
        GetScrollInfo(hWnd, SB_HORZ, &siHORZ);
        xTrackPos = siHORZ.nTrackPos;

        // Reset the current scroll position
        newPos = ScrollIt(hWnd, scrollType, scrollDrag, xCurrentScroll, xMinScroll,  xMaxScroll, xTrackPos);

        // Determine the amount scrolled (in pixels). 
        retVal = newPos - xCurrentScroll; // delta = new_pos - current_pos  
            // Reset the current scroll position. 

        xCurrentScroll = newPos;
        return retVal;
    }
    else
    {
        if (scrollXorY == SCROLL_LEFTUP)
        {

            siVERT.fMask = SIF_TRACKPOS;
            GetScrollInfo(hWnd, SB_VERT, &siVERT);
            yTrackPos = siVERT.nTrackPos;

            newPos = ScrollIt(hWnd, scrollType, scrollDrag, yCurrentScroll, yMinScroll, yMaxScroll, yTrackPos);
            retVal = newPos - yCurrentScroll;
            yCurrentScroll = newPos;
            return retVal;
        }
    }






siHORZ.nPos = xCurrentScroll;

    if (scrollXorY == 1)
    {
        siHORZ.fMask = SIF_POS;
        SetScrollInfo(hWnd, SB_HORZ, &siHORZ, TRUE);
        return retVal;
    }
    else
    {
        if (!scrollXorY)
        {
            if (!isLoading && bmpWidth > defFmWd)
            {
                ShowScrollBar(hWnd, SB_HORZ, TRUE);
                xMaxScroll = max(bmpWidth - xNewSize, 0);
                xCurrentScroll = min(xCurrentScroll, xMaxScroll);
                siHORZ.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
                siHORZ.nMin = xMinScroll;
                siHORZ.nMax = bmpWidth;
                siHORZ.nPage = xNewSize;
                // Not dealing with xTrackPos.
                SetScrollInfo(hWnd, SB_HORZ, &siHORZ, TRUE);
                retVal = 1;
            }
            else
            {
                xCurrentScroll = 0;
                //si.fMask = SIF_DISABLENOSCROLL;
                if (IsThemeActive())
                    SetWindowTheme(hWnd, NULL, _T("Scrollbar"));
                ShowScrollBar(hWnd, SB_HORZ, FALSE);

            }
        }
    }
    // The vertical scrolling range is defined by 
    // (bitmap_height) - (client_height). The current vertical 
    // scroll value remains within the vertical scrolling range. 


    siVERT.nPos = yCurrentScroll;

    if (scrollXorY == -1)
    {
        siVERT.fMask = SIF_POS;
        SetScrollInfo(hWnd, SB_VERT, &siVERT, TRUE);
        return retVal;
    }
    else
    {
        if (!scrollXorY)
        {
            if (!isLoading && bmpHeight > defFmHt)
            {
                ShowScrollBar(hWnd, SB_VERT, TRUE);
                yMaxScroll = max(bmpHeight - yNewSize, 0);
                yCurrentScroll = min(yCurrentScroll, yMaxScroll);
                siVERT.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
                siVERT.nMin = yMinScroll;
                int x = GetSystemMetrics(21);
                siVERT.nMax = bmpHeight;
                //si.nMax = bmp.bmHeight + ((bmp.bmWidth > defFmWd)? GetSystemMetrics(21) : 0);
                siVERT.nPage = yNewSize;
                siVERT.nPos = yCurrentScroll;
                siVERT.nTrackPos = yTrackPos;
                SetScrollInfo(hWnd, SB_VERT, &siVERT, TRUE);
                return ((retVal) ? 3 : 2);
            }
            else
            {
                yCurrentScroll = 0;
                if (IsThemeActive())
                    SetWindowTheme(hWnd, NULL, _T("Scrollbar"));
                if (retVal)
                {
                    ShowScrollBar(hWnd, SB_VERT, FALSE);
                }
                else
                {
                    ShowScrollBar(hWnd, SB_BOTH, FALSE);
                }
                return retVal;
                //RedrawWindow(hWnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);
            }
        }
        return retVal;
    }

}
BOOL Kleenup(HWND hWnd, HBITMAP& hBitmap, HBITMAP& hbmpCompat, GpBitmap*& pgpbm, HDC& hdcMem, HDC& hdcMemIn, HDC& hdcWinCl)
{
    if (hdcWinCl)
        ReleaseDC(hWnd, hdcWinCl);

    if (hBitmap)
        DeleteObject(hBitmap);

    if (pgpbm)
        GpStatus gps = GdipDisposeImage(pgpbm);
    siHORZ.cbSize = 0;
    siVERT.cbSize = 0;
    DeleteDC(hdcMem);
    DeleteDC(hdcMemIn);
    DeleteObject(hbmpCompat);
    DestroyWindow(hWnd);
    return true;
}
//**************************************************************
// Functions for possible later use
//**************************************************************
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
BOOL BitmapFromPixels(Bitmap& myBitmap, const std::vector<std::vector<unsigned>>resultPixels, int width, int height)
{
    // Possible usage:
    //HPALETTE hpal;
    //GdipCreateBitmapFromHBITMAP(hBmp, hpal, &pgpbm);
    //Bitmap ctlBitmap(wd,  ht, PixelFormat32bppARGB);
    //BitmapFromPixels(ctlBitmap, resultPixels, wd, ht);

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
BOOL ChangeWindowMsgFilterEx(HWND hWnd, UINT uMsg)
{
/*    BOOL ChangeWindowMsgFilterEx(HWND hwnd, UINT Msg);
    if (!(ChangeWindowMsgFilterEx(hwnd, WM_DROPFILES) && ChangeWindowMsgFilterEx(hwnd, WM_COPYDATA) && ChangeWindowMsgFilterEx(hwnd, WM_COPYGLOBALDATA)))
    {
        DisplayError(hwnd, L"ChangeWindowMsgFilterEx: Could not allow message", errCode, 0);
    }
*/
    typedef BOOL(WINAPI* fnChangeWindowMessageFilterEx)(HWND, UINT, DWORD, PCHANGEFILTERSTRUCT);
    fnChangeWindowMessageFilterEx pfn =
        reinterpret_cast<fnChangeWindowMessageFilterEx>(
            reinterpret_cast<void*>(
                GetProcAddress(GetModuleHandle(L"user32"),
                    "ChangeWindowMessageFilterEx")));

    if (!(pfn))
        //use the old function
    {

        typedef BOOL(WINAPI* fnChangeWindowMessageFilter)(UINT, DWORD);
        fnChangeWindowMessageFilter pfn =
            reinterpret_cast<fnChangeWindowMessageFilter>(
                reinterpret_cast<void*>(
                    GetProcAddress(GetModuleHandle(L"user32"),
                        "ChangeWindowMessageFilter")));
        return pfn(uMsg, MSGFLT_ADD);
    }
    return pfn(hWnd, uMsg, MSGFLT_ALLOW, NULL);
}


