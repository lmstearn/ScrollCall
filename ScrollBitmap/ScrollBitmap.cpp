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

HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // Title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
wchar_t* szFile = nullptr;
float scrAspect = 0, scaleX = 1, scaleY = 1, resX = 0, resY = 0;
SIZE scrDims = {0}, scrEdge = {0};
// wd, ht: button dims
int tmp = 0, wd = 0, ht = 0, capCallFrmResize = 0, xCurrentScroll, yCurrentScroll, scrShtOrBmpLoad;
HWND hWndGroupBox = 0, hWndButton = 0, hWndOpt1 = 0, hWndOpt2 = 0, hWndChk = 0;
BOOL optChk = TRUE, chkChk = FALSE, groupboxFlag = FALSE,  procEndWMSIZE = TRUE;
BOOL isLoading = TRUE, windowMoved,  isSizing = FALSE, restoreFromMax = FALSE;; // False on PrintWindow

// Timer
int timDragWindow = 0, timPaintBitmap = 0;


// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    MyBitmapWindowProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK staticSubClass(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
LRESULT CALLBACK staticSubClassButton(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
wchar_t* FileOpener(HWND hWnd);
void ReportErr(const wchar_t* format, ...);
void PrintWindow(HWND hWnd, HBITMAP hBmp, HDC hdcMemDefault);
//Functions for later use
BOOL BitmapFromPixels(Bitmap& myBitmap, const std::vector<std::vector<unsigned>>resultPixels, int width, int height);
void DoMonInfo();
char* VecToArr(std::vector<std::vector<unsigned>> vec);
BOOL AdjustImage(HWND hWnd, BOOL isScreenshot, HBITMAP hBitmap, BITMAP bmp, GpStatus gps, HDC& hdcMem, HDC& hdcMemIn, HDC hdcScreen, HDC hdcScreenCompat, HDC hdcWinCl, UINT& bmpWidth, UINT& bmpHeight, int xNewSize, int yNewSize, int updatedxCurrentScroll, int updatedyCurrentScroll, int resizePic = 0, int minMaxRestore = 0, BOOL newPic = FALSE);
void GetDims(HWND hWnd, int resizeType = 0, int oldResizeType = 0);
void SizeControls(BITMAP bmp, HWND hWnd, int& updatedxCurrentScroll, int& updatedyCurrentScroll, int resizeType = -1, int curFmWd = 0, int curFmHt = 0);
int ScrollInfo(HWND hWnd, int scrollXorY, int scrollType, int scrollDrag, int xNewSize = 0, int yNewSize = 0, int bmpWidth = 0, int bmpHeight = 0);
BOOL Kleenup(HWND hWnd, HBITMAP& hBitmap, HBITMAP& hbmpCompat, GpBitmap*& pgpbm, HDC hdcMemDefault, HDC& hdcMem, HDC& hdcMemIn, HDC& hdcWinCl, BOOL noExit = FALSE);
int delegateSizeControl(RECT rectOpt, HWND hWndOpt, int oldOptTop, int resizeType, int oldResizeType, int defOptTop, int updatedxCurrentScroll, int updatedyCurrentScroll, int newCtrlSizeTriggerHt, int newEdgeWd, int newWd, int minHt);
BOOL CreateToolTipForRect(HWND hwndParent, int toolType = 0);

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
    if (msgboxID == IDCANCEL)
        return 0;
    else
    {
        if (msgboxID == IDYES)
            groupboxFlag = TRUE;
        DoMonInfo();
    }

    HWND m_hWnd = CreateWindowW(szWindowClass, szTitle, WS_HSCROLL | WS_VSCROLL | WS_OVERLAPPEDWINDOW | (groupboxFlag ? NULL : WS_CLIPCHILDREN),
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
    // Old style
    //SetWindowTheme(m_hWnd, L" ", L" ");
    //Or SetThemeAppProperties(STAP_ALLOW_CONTROLS | STAP_ALLOW_WEBCONTENT);
    //SetThemeAppProperties(0);
    
    if (!m_hWnd)
    {
        ReportErr(L"Cannot create window! Quitting...");
        return FALSE;
    }
    // Old style theme
    // SetWindowTheme(m_hWnd, L" ", L" ");

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

    HDC hdc; // for WM_PAINT
    // These variables are required by BitBlt. 

    static HBITMAP hBitmap = { 0 };
    static GpStatus gps = { };

    static int fScroll;             // 1 if horz scrolling, -1 vert scrolling, 0 for WM_SIZE
    static BOOL fSize;          // TRUE if WM_SIZE 

    static HDC hdcWinCl;            // client area of DC for window
    static HDC hdcMemDefault;            // Default dummy Mem DC
    static HDC hdcMem;            // Mem DC
    static HDC hdcMemIn;            // Mem DC
    static HDC hdcScreen;           // DC for entire screen
    static HDC hdcScreenCompat;     // memory DC for screen
    static HBITMAP hbmpCompat;      // bitmap handle to old DC
    static GpBitmap* pgpbm;          // bitmap data structure
    static BITMAP bmp;          // bitmap data structure
    static UINT bmpWidth = 0;
    static UINT bmpHeight = 0;
    static BOOL isScreenshot;
    static int updatedxCurrentScroll, updatedyCurrentScroll; // updates on WM_SIZE
    static int xNewSize; // updates on WM_SIZE
    static int yNewSize;
    static int scrollStat;      // 0: None, 1: SB_HORZ, 2: SB_VERT, 3: SB_BOTH


    static BOOL toolTipOn;

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

        SMOOTHSCROLL_SPEED = 0X00000002;
        ulScrollLines = 0;

        GetDims(hWnd);

        RECT recthWndtmp = RectCl().RectCl(0, hWnd, 1); //must be initialised as can be used before WM_SIZE.
        xNewSize = RectCl().width(1);
        yNewSize = RectCl().height(1);
        if (groupboxFlag)
        {

            hWndGroupBox = CreateWindowExW(0, L"BUTTON",
                L"",
                WS_VISIBLE | WS_CHILD | BS_GROUPBOX | WS_CLIPSIBLINGS, //consider  WS_CLIPCHILDREN
                // also MSDN: Do not combine the BS_OWNERDRAW style with any other button styles!
                0, 0, wd, RectCl().height(1),
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

        hWndOpt1 = CreateWindowExW(WS_EX_WINDOWEDGE,
            L"BUTTON",
            L"Scroll",
            WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP | BS_CENTER,  // <---- WS_GROUP group the 1st and 2nd radio buttons.
            2, ht + OPT_HEIGHT,
            wd - 2, ht / 2,
            hWnd, //<----- Use main window handle
            (HMENU)IDC_OPT1,
            (HINSTANCE)NULL, NULL);
        hWndOpt2 = CreateWindowExW(WS_EX_WINDOWEDGE,
            L"BUTTON",
            L"ScrollEx",
            WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | BS_CENTER,  // Styles 
            2, 2 * ht,
            wd - 2, ht / 2,
            hWnd,
            (HMENU)IDC_OPT2,
            (HINSTANCE)NULL, NULL);
        SendMessageW(hWndOpt1, BM_SETCHECK, BST_CHECKED, 0);

        hWndChk = CreateWindowExW(WS_EX_WINDOWEDGE,
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
        scrShtOrBmpLoad = 0;
        toolTipOn = FALSE;
        windowMoved = FALSE;
        return 0;
        break;
    }
    case WM_SIZING:
    {
        fScroll = 0;
        fSize = TRUE;
        isSizing = TRUE;
        return TRUE;
        // Remove the above "return" to find that:
        // Custom painting during the sizing might have been a good idea,
        // except that the system invalidates the window with COLOR_WINDOW + 1
        //after each bitblt, which causes flicker.
        // Thus capCallFrmResize remains zero throughout.
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
                    Kleenup(hWnd, hBitmap, hbmpCompat, pgpbm, hdcMemDefault, hdcMem, hdcMemIn, hdcWinCl);
                    PostQuitMessage(0);
                }
        }
        return TRUE;
    }
    break;
    case WM_TIMER:
    {
        switch (wParam)
        {
        case IDT_DRAGWINDOW:
        {
            tmp = KillTimer(hWnd, IDT_DRAGWINDOW);
            if (tmp)
            {
                /*
                // The following causes excess drawing + painting && thus unnecessary
                SizeControls(bmp, hWnd, defFmWd, defFmHt, 0);
                hdcWin = GetWindowDC(hWnd);
                if (!AdjustImage(hWnd, isScreenshot, hBitmap, bmp, gps, hdcMem, hdcMemIn, hdcScreen, hdcScreenCompat, hdcWinCl, bmpWidth, bmpHeight, xCurrentScroll, 1 + chkChk, 0))
                    ReportErr(L"AdjustImage detected a problem with the image!");
               //ReportErr(L"AdjustImage detected a problem with the image!");
                ReleaseDC(hWnd, hdcWin);
                */
                capCallFrmResize = 0;
            }
        }
        break;
        case IDT_PAINTBITMAP:
        {
            tmp = KillTimer(hWnd, IDT_PAINTBITMAP);
            timPaintBitmap = 0;
            if (!AdjustImage(hWnd, isScreenshot, hBitmap, bmp, gps, hdcMem, hdcMemIn, hdcScreen, hdcScreenCompat, hdcWinCl, bmpWidth, bmpHeight, xNewSize, yNewSize, updatedxCurrentScroll, updatedyCurrentScroll, (chkChk) ? 2 : 0, SIZE_MAXIMIZED))
                ReportErr(L"AdjustImage detected a problem with the image!");

            //mPaintBitmap
        }
        break;
        }

        if (!tmp)
            ReportErr(L"Problem with timer termination.");
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
        if (!scrShtOrBmpLoad)
            return 0;
        KillTimer(hWnd, IDT_DRAGWINDOW);
        SizeControls(bmp, hWnd, updatedxCurrentScroll, updatedyCurrentScroll, START_SIZE_MOVE, xNewSize, yNewSize);
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
        if (!scrShtOrBmpLoad)
            return 0;
        fSize = TRUE;
        isSizing = FALSE;
        //InvalidateRect(hWnd, 0, TRUE);
        KillTimer(hWnd, IDT_DRAGWINDOW);
        if (windowMoved)
        {
            windowMoved = FALSE;
            RECT recthWndtmp = RectCl().RectCl(0, hWnd, 0);
            if (recthWndtmp.left < scrEdge.cx || recthWndtmp.right >(scrEdge.cx + scrDims.cx) || recthWndtmp.top < scrEdge.cy || recthWndtmp.bottom >(scrEdge.cy + scrDims.cy))
            {
                if (!toolTipOn)
                    toolTipOn = CreateToolTipForRect(hWnd, 1);
            }
            else
            {
                if (toolTipOn)
                    toolTipOn = CreateToolTipForRect(hWnd);
            }

        }
        else
        {
            timDragWindow = 0;
            capCallFrmResize = 0;
            scrollStat = ScrollInfo(hWnd, 0, 0, 0, xNewSize, yNewSize, bmpWidth, bmpHeight);
            SizeControls(bmp, hWnd, updatedxCurrentScroll, updatedyCurrentScroll, END_SIZE_MOVE, xNewSize, yNewSize);
            UpdateWindow(hWnd);
            // The following causes flicker but may clip controls in certain circumstances
            if (!scrollStat && !AdjustImage(hWnd, isScreenshot, hBitmap, bmp, gps, hdcMem, hdcMemIn, hdcScreen, hdcScreenCompat, hdcWinCl, bmpWidth, bmpHeight, xNewSize, yNewSize, updatedxCurrentScroll, updatedyCurrentScroll, 2, FALSE, SIZE_MAXIMIZED))
                ReportErr(L"AdjustImage detected a problem with the image!");
        }
        return 0;
    }
    break;

    case WM_SIZE:
    {
        if (!procEndWMSIZE)
            return 0;

        BOOL maxMinSize = (wParam == SIZE_MINIMIZED || wParam == SIZE_MAXIMIZED);
        // WM_SIZE called for each child control (no subclass)

        if (!(szFile && (szFile[0] == L'*')) && (!capCallFrmResize || !timDragWindow || maxMinSize))
        {

            xNewSize = LOWORD(lParam);
            yNewSize = HIWORD(lParam);
            
            SizeControls(bmp, hWnd, updatedxCurrentScroll, updatedyCurrentScroll, wParam, xNewSize, yNewSize);

            if (scrShtOrBmpLoad > 1)
            {
                if (!isLoading)
                {
                    if (!AdjustImage(hWnd, isScreenshot, hBitmap, bmp, gps, hdcMem, hdcMemIn, hdcScreen, hdcScreenCompat, hdcWinCl, bmpWidth, bmpHeight, xNewSize, yNewSize, updatedxCurrentScroll, updatedyCurrentScroll, (chkChk) ? 2 : 0, wParam))
                        ReportErr(L"AdjustImage detected a problem with the image!");

                    if (fSize && (wParam == SIZE_RESTORED))
                        timDragWindow = 0;
                    fSize = TRUE;
                    fScroll = 0;
                    // The horizontal scrolling range is defined by 
                // (bitmap_width) - (client_width). The current horizontal 
                // scroll value remains within the horizontal scrolling range. 
                }
                if ((scrollStat && !isSizing) || restoreFromMax || (wParam == SIZE_MAXIMIZED))
                {
                    restoreFromMax = (wParam == SIZE_MAXIMIZED);
                    scrollStat = ScrollInfo(hWnd, 0, 0, 0, xNewSize, yNewSize, bmpWidth, bmpHeight);
                }
            }
        }
        windowMoved = FALSE;
        return 0;
    }
    break;
    case WM_PAINT:
    {
        if (wParam == 0)
        {
            PAINTSTRUCT ps;
            if (!(hdc = BeginPaint(hWnd, &ps)))
                return 0;


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
                if (xCurrentScroll < wd)
                {
                    BitBlt(ps.hdc,
                        //prect->left + (isScreenshot ? 0 : ((fScroll == 1) ? 0 : -wd)),
                        // Blt at wd method
                        //prect->left + (isScreenshot ? (fScroll == 1 ? 0 : wd) : (((fScroll == 1) && (xCurrentScroll > wd)) ? 0 : wd)),
                        prect->left,
                        prect->top,
                        //prect->left + ( (isScreenshot) ? 0 : wd * scaleX), prect->top,
                        (prect->right - prect->left + wd - xCurrentScroll),
                        (prect->bottom - prect->top),
                        isScreenshot ? hdcScreenCompat : hdcMem,
                        prect->left + xCurrentScroll,
                        prect->top + yCurrentScroll,
                        SRCCOPY);

                    if (!groupboxFlag)
                    {
                        // Paint sections
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
                else
                    BitBlt(ps.hdc,
                        //prect->left + (isScreenshot ? 0 : ((fScroll == 1) ? 0 : -wd)),
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
                fScroll = 0;
            }
            else
            {
                if (fSize)
                {
                    if (toolTipOn)
                        toolTipOn = CreateToolTipForRect(hWnd);
                    if (!capCallFrmResize)
                        fSize = FALSE;
                    if (!isSizing)
                    {
                        if (!BitBlt(ps.hdc,
                            //0, 0,
                            -xCurrentScroll, -yCurrentScroll,
                            bmpWidth, bmpHeight,
                            hdcWinCl,
                            0, 0,
                            //xCurrentScroll, yCurrentScroll,
                            SRCCOPY))
                            ReportErr(L"BitBlt failed!");
                    }
                    if (!groupboxFlag)
                    {
                    // Paint sections
                        if (xCurrentScroll < wd)
                        {
                            PRECT prect;
                            prect = &ps.rcPaint;
                            if (prect->left < wd - 1) // Issue when form is sized small horizontally
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
                        }
                        //UpdateWindow(hWnd);
                    }

                }
                else
                {
                    // Gets here on !scrShtOrBmpLoad, or on a successive WM_MOVE
                    // when a paint is required when form is moved either off screen
                    // or behind another shown HWND_TOPMOST form.
                    if (scrShtOrBmpLoad && !isLoading)
                    {
                        RECT recthWndtmp = RectCl().RectCl(0, hWnd, 0);
                        if (recthWndtmp.left < scrEdge.cx || recthWndtmp.right >(scrEdge.cx + scrDims.cx) || recthWndtmp.top < scrEdge.cy || recthWndtmp.bottom >(scrEdge.cy + scrDims.cy))
                        {
                            if (!toolTipOn)
                                toolTipOn = CreateToolTipForRect(hWnd, 1);
                        }
                    }
                }
            }

            EndPaint(hWnd, &ps);
        }
        else
            RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);
        return 0;
    }
    break;
    case WM_GETMINMAXINFO:
    {        // prevent the window from becoming too small
        ((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
        ((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
    }
    break;
    case WM_HSCROLL:
    {
 
        // Scroll the window. (The system repaints most of the 
        // client area when ScrollWindow(Ex) is called; however, it is 
        // necessary to call UpdateWindow in order to repaint the 
        // rectangle of pixels that were invalidated.) 

        int xDelta = ScrollInfo(hWnd, HORZ_SCROLL, LOWORD(wParam), HIWORD(wParam), xNewSize, yNewSize, bmpWidth, bmpHeight);
        // If the current position does not change, do not scroll.
        if (!xDelta)
            return 0;
        fScroll = 1;
        fSize = FALSE;
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
            if (!ScrollWindow(hWnd, -xDelta, 0, (CONST RECT*) NULL, (CONST RECT*) NULL))
                ReportErr(L"ScrollWindow Failed!");
        }
        else
        {
            ScrollWindowEx(hWnd, -xDelta, 0, (CONST RECT*) NULL,
                (CONST RECT*) NULL, (HRGN)NULL, (PRECT)NULL, SW_SCROLLCHILDREN | SW_INVALIDATE); // SMOOTHSCROLL_FLAG fails
            UpdateWindow(hWnd);
        }
        // Reset the scroll bar. 
        ScrollInfo(hWnd, UPDATE_HORZSCROLLSIZE, 0, 0);
        return 0;

    }
    break;
    case WM_VSCROLL:
    {
        int yDelta = ScrollInfo(hWnd, VERT_SCROLL, LOWORD(wParam), HIWORD(wParam), xNewSize, yNewSize, bmpWidth, bmpHeight);

        // Scroll the window. (The system repaints most of the
        // client area when ScrollWindow(Ex) is called; however, it is
        // necessary to call UpdateWindow in order to repaint the
        // rectangle of pixels that were invalidated.)
        if (!yDelta)
            return 0;
        fScroll = -1;
        fSize = FALSE;
        if (optChk)
        {
            if (!ScrollWindow(hWnd, 0, -yDelta, (CONST RECT*) NULL, (CONST RECT*) NULL))
                ReportErr(L"ScrollWindow Failed!");
        }
        else
        {
            ScrollWindowEx(hWnd, 0, -yDelta, (CONST RECT*) NULL,
                (CONST RECT*) NULL, (HRGN)NULL, (PRECT)NULL, SW_SCROLLCHILDREN | SW_INVALIDATE); // SMOOTHSCROLL_FLAG fails
            UpdateWindow(hWnd);
        }

        // Reset scroll bar. 
        ScrollInfo(hWnd, UPDATE_VERTSCROLLSIZE, 0, 0);
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
            SendMessageW(hWnd, WM_VSCROLL, SB_LINEUP, 0);
            iAccumDelta -= iDeltaPerLine;
        }

        while (iAccumDelta <= -iDeltaPerLine)
        {
            SendMessageW(hWnd, WM_VSCROLL, SB_LINEDOWN, 0);
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
                optChk = (SendMessageW((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED);
        }
        break;
        case IDC_OPT2:
        {
            if (wmEvent == BN_CLICKED)
                optChk = (SendMessageW((HWND)lParam, BM_GETCHECK, 0, 0) != BST_CHECKED);
            // Also BST_INDETERMINATE, BST_UNCHECKED
        }
        break;
        case IDC_CHK:
        {
            if (wmEvent == BN_CLICKED)
                chkChk = (SendMessageW((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED);
            // Also BST_INDETERMINATE, BST_UNCHECKED
        }
        break;
        case ID_OPENBITMAP:
        {
            //https://social.msdn.microsoft.com/Forums/sqlserver/en-US/76441f64-a7f2-4483-ad6d-f51b40464d6b/how-to-get-both-width-and-height-of-bitmap-in-gdiplus-flat-api?forum=windowsgeneraldevelopmentissues

            szFile = (wchar_t*)calloc(MAX_LOADSTRING, sizeof(wchar_t));
            wcscpy_s(szFile, MAX_LOADSTRING, FileOpener(hWnd));


            //CLSID pngClsid;
           // GetEncoderClsid(L"image/png", &pngClsid);
            if (szFile[0] != L'*')
            {
                if (hdcMemDefault)
                    Kleenup(hWnd, hBitmap, hbmpCompat, pgpbm, hdcMemDefault, hdcMem, hdcMemIn, hdcWinCl, TRUE);
                else
                    hdcMemDefault = CreateCompatibleDC(hdcWinCl);

                Color clr;
                //wd = RectCl().width(2);
                //ht = RectCl().height(1);
                const std::vector<std::vector<unsigned>>resultPixels;

                gps = GdipCreateBitmapFromFile(szFile, &pgpbm);
                if (gps == Ok)
                {

                    hdcWinCl = GetDCEx(hWnd, (HRGN)NULL, DCX_CACHE | DCX_CLIPCHILDREN);


                    hBitmap = NULL;
                    gps = GdipCreateHBITMAPFromBitmap(pgpbm, &hBitmap, clr.GetValue());
                    // black = (bits == 0);   alpha=255 => 0xFF000000
                    if (hBitmap)
                    {
                        gps = GdipGetImageWidth(pgpbm, &bmpWidth);
                        gps = GdipGetImageHeight(pgpbm, &bmpHeight);
                        //HBITMAP hBmpCopy = (HBITMAP)CopyImage(hBitmap, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
                        //GpStatus WINGDIPAPI GdipDrawImageI(GpGraphics* graphics, GpImage* image, INT x, INT y);

                        hdcMemIn = CreateCompatibleDC(hdcWinCl);
                        hdcMem = CreateCompatibleDC(hdcWinCl);
                        isScreenshot = FALSE;
                        if (!AdjustImage(hWnd, isScreenshot, hBitmap, bmp, gps, hdcMem, hdcMemIn, hdcScreen, hdcScreenCompat, hdcWinCl, bmpWidth, bmpHeight, xNewSize, yNewSize, xCurrentScroll, yCurrentScroll, 2, FALSE, SIZE_MAXIMIZED))
                            ReportErr(L"AdjustImage detected a problem with the image!");
                        //SizeControls(bmp, hWnd, defFmWd, defFmHt, -1, xCurrentScroll, yCurrentScroll);
                        xCurrentScroll ? fScroll = 1 : fScroll = -1; //initialise for ScrollInfo below
                        szFile[0] = L'*'; // Set to deal with WM_SIZE issues
                        scrollStat = ScrollInfo(hWnd, 0, 0, 0, xNewSize, yNewSize, bmpWidth, bmpHeight);
                        szFile[0] = L'X';

                        scrShtOrBmpLoad = 3;
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
                        UpdateWindow(hWnd);
                        EnableWindow(hWndChk, TRUE);
                    }
                    else
                        ReportErr(L"hBitmap: Cannot create bitmap!!");
                }
                else
                    ReportErr(L"GPS: Cannot open bitmap!");
            }
            free(szFile);
            isSizing = FALSE;
            return (INT_PTR)TRUE;
        }
        break;
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
        break;
        case IDM_EXIT:
            Kleenup(hWnd, hBitmap, hbmpCompat, pgpbm, hdcMemDefault, hdcMem, hdcMemIn, hdcWinCl);
        break;
        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        break;
        }
    break;
    }
    break;
    case WM_LBUTTONDBLCLK:
    {
        if (hdcMemDefault)
            Kleenup(hWnd, hBitmap, hbmpCompat, pgpbm, hdcMemDefault, hdcMem, hdcMemIn, hdcWinCl, TRUE);


        if (hdcWinCl = GetDCEx(hWnd, (HRGN)NULL, DCX_CACHE | DCX_CLIPCHILDREN))
        {
            if (!hdcMemDefault)
                hdcMemDefault = CreateCompatibleDC(hdcWinCl);
            hdcMem = CreateCompatibleDC(hdcWinCl);
            scrShtOrBmpLoad = 1;
            bmpWidth = (scaleX * RectCl().width(1)) - wd;
            bmpHeight = scaleY * (RectCl().height(1));


            hBitmap = CreateCompatibleBitmap(hdcWinCl, bmpWidth, bmpHeight);
            if (hBitmap)
            {
                PrintWindow(hWnd, hBitmap, hdcMemDefault);
                SelectObject(hdcMem, hBitmap);


                if (!BitBlt(hdcWinCl, wd, RectCl().ClMenuandTitle(hWnd), bmpWidth, bmpHeight, hdcMem, 0, RectCl().ClMenuandTitle(hWnd), SRCCOPY))
                    ReportErr(L"Bad BitBlt from hdcMem!");

                scrollStat = ScrollInfo(hWnd, 0, 0, 0, 0, 0, bmpWidth, bmpHeight);
                EnableWindow(hWndChk, TRUE);
            }
            else
                ReportErr(L"CreateCompatibleBitmap: Unable to create bitmap!");
        }
        else
            ReportErr(L"GetDCEx: Failed to get DC!");
            isScreenshot = FALSE;

        return 0;
    }
    break;
    case WM_RBUTTONDOWN:
    {
        if (hdcMemDefault)
            Kleenup(hWnd, hBitmap, hbmpCompat, pgpbm, hdcMemDefault, hdcMem, hdcMemIn, hdcWinCl, TRUE);


        isScreenshot = TRUE;
        // Get the compatible DC of the client area. 
        if (hdcWinCl = GetDCEx(hWnd, (HRGN)NULL, DCX_CACHE | DCX_CLIPCHILDREN))
        {
        if (!hdcMemDefault)
        hdcMemDefault = CreateCompatibleDC(hdcWinCl);
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
        scrShtOrBmpLoad = 2;

        if (!AdjustImage(hWnd, isScreenshot, hBitmap, bmp, gps, hdcMem, hdcMemIn, hdcScreen, hdcScreenCompat, hdcWinCl, bmpWidth, bmpHeight, xNewSize, yNewSize, xCurrentScroll, yCurrentScroll, 2, FALSE, SIZE_MAXIMIZED))
            ReportErr(L"AdjustImage detected a problem with the image!");
        xCurrentScroll ? fScroll = 1 : fScroll = -1;
        if (!SetWindowOrgEx(hdcWinCl, -xCurrentScroll, yCurrentScroll, NULL))
            ReportErr(L"SetWindowOrgEx failed!");
        //xCurrentScroll = 0;
        //yCurrentScroll = 0;
        scrollStat = ScrollInfo(hWnd, 0, 0, 0, xNewSize, yNewSize, bmpWidth, bmpHeight);


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
            fScroll = 0;
        break;
        }

        EnableWindow(hWndChk, FALSE);
        //SetWindowOrgEx(hdcWinCl, xCurrentScroll, yCurrentScroll, NULL);
        }
        else
            ReportErr(L"GetDCEx: Failed to get DC!");

        isSizing = FALSE;
        return 0;
    }
    break;
    case WM_KEYDOWN:
    {
        switch (wParam)
        {
        case VK_ESCAPE:
        {
            Kleenup(hWnd, hBitmap, hbmpCompat, pgpbm, hdcMemDefault, hdcMem, hdcMemIn, hdcWinCl);
            PostQuitMessage(0);
        }
        break;
        default:
        break;
        }
    }
    break;
    case WM_DESTROY:
    {
        Kleenup(hWnd, hBitmap, hbmpCompat, pgpbm, hdcMemDefault, hdcMem, hdcMemIn, hdcWinCl);
        PostQuitMessage(0);
    }
    break;
    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    break;
    }
return 0;
}

LRESULT CALLBACK staticSubClass(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    UNUSED(dwRefData);
    static int savefScroll = 0;

    switch (uMsg) // instead of LPNMHDR  lpnmh = (LPNMHDR) lParam above;
    {
    case WM_PAINT:
    {
        //"If wParam is non-NULL, the common control may assume the value is an HDC and paints using that device context."
        if (wParam == 0)
        {
            //if (isSizing)
            /*// Supposed to "halve" the flickering in scrolling. No perceivable difference.

            if (fScroll)
            {
                savefScroll = fScroll;
                fScroll = 0;
            }
            else
            */
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hWnd, &ps);
                PRECT prect;
                prect = &ps.rcPaint;
                FillRect(hdc, prect, (HBRUSH)(COLOR_WINDOW + 1));
                EndPaint(hWnd, &ps);
                //fScroll = savefScroll;
            }
        }
        else
            RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);
        return TRUE;
    }
    break;
    case WM_NCDESTROY:
    {
        // NOTE: this requirement is NOT stated in the documentation, but it is stated in Raymond Chen's blog article...
        RemoveWindowSubclass(hWnd, staticSubClass, uIdSubclass);
        return 0;
    }
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
    {
        // NOTE: this requirement is NOT stated in the documentation, but it is stated in Raymond Chen's blog article...
        RemoveWindowSubclass(hWnd, staticSubClassButton, uIdSubclass);
        return 0;
    }
    break;
    default:
        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    break;
    }

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
        {
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            else
                return (INT_PTR)FALSE;
        }
        break;
        default:
            return (INT_PTR)FALSE;
        break;
        }
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
    if (buf)
    {
        vswprintf_s(buf, len, format, args);

        MessageBoxW(0, buf, L"Error", 0);
        //OutputDebugStringA(buf);
        free(buf);
    }
    else
        MessageBoxW(0, L"MessageBox: Out of memory!", L"Error", 0);
}

void PrintWindow(HWND hWnd, HBITMAP hBitmap, HDC hdcMemDefault)
{
    HDC hDCMem = CreateCompatibleDC(NULL);
    //HGDIOBJ hOld = GetCurrentObject(hDCMem, OBJ_BITMAP); //hBmpObj =>handle to bitmap (HBITMAP)

    SelectObject(hDCMem, hBitmap);
    SendMessageW(hWnd, WM_PRINT, (WPARAM)hDCMem,  PRF_CLIENT | PRF_ERASEBKGND | PRF_NONCLIENT);
    //Sleep(50);

    SelectObject(hdcMemDefault, hBitmap);
    DeleteDC(hDCMem);
}
void DoMonInfo()
{

    HMONITOR hMon = MonitorFromWindow(GetDesktopWindow(), MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO monInfo = {};
    monInfo.cbSize = sizeof(MONITORINFO);
    if (GetMonitorInfoW(hMon, &monInfo))
    {
        if (monInfo.dwFlags != MONITORINFOF_PRIMARY)
            ReportErr(L"ScrollCall has bot been fully tested on a secondary monitor!.");
    }
    else
        ReportErr(L"GetMonitorInfo: Cannot get info.");
    scrEdge.cx = monInfo.rcWork.left;
    scrEdge.cy = monInfo.rcWork.top;
    scrDims.cx = monInfo.rcWork.right - monInfo.rcWork.left;
    scrDims.cy = monInfo.rcWork.bottom - monInfo.rcWork.top;
}
BOOL AdjustImage(HWND hWnd, BOOL isScreenshot, HBITMAP hBitmap, BITMAP bmp, GpStatus gps, HDC& hdcMem, HDC& hdcMemIn, HDC hdcScreen, HDC hdcScreenCompat, HDC hdcWinCl, UINT& bmpWidth, UINT& bmpHeight, int curFmWd, int curFmHt, int updatedxCurrentScroll, int updatedyCurrentScroll, int resizePic, int minMaxRestore, BOOL newPic)
{
    static RECT rect;
    static int oldWd = 0, startFmWd = 0;

    BOOL retVal = FALSE;

    if (newPic)
    {
        HDC hdcWin;            // DC for window
        hdcWin = GetWindowDC(hWnd); // This is a shared windows resource
        if (!hdcWin)
        {
            ReportErr(L"hdcWin: Not available!");
            bmpWidth = 0;
            bmpHeight = 0;
            return retVal;
        }
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

       /*
       // If for some reason the bmp dims want not to exceed screen dims:
       int maxBmpWd = scrDims.cx - wd;
        int maxBmpHt = scrDims.cy - RectCl().ClMenuandTitle(hWnd);
        if (bmpWidth > maxBmpWd)
            bmpWidth = maxBmpWd;
        if (bmpHeight > maxBmpHt)
            bmpHeight = maxBmpHt;
        */
        if (!ReleaseDC(hWnd, hdcWin))
            ReportErr(L"hdcWin: Not released!");

        rect.left = 0;
        rect.top = 0;
        rect.bottom = bmpHeight;
        rect.right = bmpWidth;
    }

        if (isScreenshot)
        {
            if (resizePic)
            {
                retVal = (BOOL)FillRect(hdcWinCl, &rect, (HBRUSH)(COLOR_WINDOW + 1)); //SetBkColor(hdcWinCl, COLOR_WINDOW + 1) causes flickering in the scrolling

                //if (resizePic == 2) // redundant when sizing
                    retVal = StretchBlt(hdcScreenCompat, wd, 0, bmpWidth - wd,
                        bmpHeight, hdcScreen, 0, 0, bmpWidth, bmpHeight, SRCCOPY);

                if (retVal)
                    retVal = (BOOL)BitBlt(hdcWinCl, wd - updatedxCurrentScroll, -updatedyCurrentScroll, bmpWidth, bmpHeight, hdcScreenCompat, wd, 0, SRCCOPY); //Blt at wd method
                oldWd = wd;
                startFmWd = curFmWd; //Experimental
            }
            else
            {
                if (minMaxRestore == SIZE_MAXIMIZED)
                    retVal = StretchBlt(hdcWinCl, wd - updatedxCurrentScroll, -updatedyCurrentScroll, bmpWidth,
                        bmpHeight, hdcScreenCompat, oldWd, 0, bmpWidth, bmpHeight, SRCCOPY);
                else
                    retVal = (BOOL)BitBlt(hdcWinCl, wd - updatedxCurrentScroll, -updatedyCurrentScroll, bmpWidth, bmpHeight, hdcScreenCompat, oldWd, 0, SRCCOPY); //Blt at wd method
            }
        }
        else
        {
            if (resizePic || minMaxRestore == SIZE_MAXIMIZED || restoreFromMax)
            {
                retVal = (BOOL)FillRect(hdcWinCl, &rect, (HBRUSH)(COLOR_WINDOW + 1)); //SetBkColor(hdcWinCl, COLOR_WINDOW + 1) causes flickering in the scrolling

                //retVal = (int)SelectObject(hdcMem, hBitmap);
                HBITMAP hBmp = CreateCompatibleBitmap(hdcWinCl, bmpWidth + wd, bmpHeight);
                //retVal = (int)SelectObject(hdcMem, hBmpObj);
                retVal = (UINT64)SelectObject(hdcMem, hBmp);
                if (!retVal || (retVal == (BOOL)HGDI_ERROR))
                    ReportErr(L"hdcMem: Cannot use bitmap!");
                if (hBitmap)
                {
                    retVal = (UINT64)SelectObject(hdcMemIn, hBitmap);
                    if (!retVal || (retVal == (BOOL)HGDI_ERROR))
                        ReportErr(L"hdcMemIn: Cannot use bitmap!");

                    if (resizePic == 2)
                        retVal = StretchBlt(hdcMem, wd, 0, bmpWidth, bmpHeight, hdcMemIn, 0, 0, bmpWidth, bmpHeight, SRCCOPY);
                    else
                        retVal = (BOOL)BitBlt(hdcMem, wd, 0, bmpWidth, bmpHeight, hdcMemIn, 0, 0, SRCCOPY); //Blt at wd method

                    if (retVal)
                    {
                    retVal = (BOOL)BitBlt(hdcWinCl, -updatedxCurrentScroll, -updatedyCurrentScroll, bmpWidth + wd, bmpHeight, hdcMem, 0, 0, SRCCOPY); //Blt at wd method
                    if (!retVal)
                        ReportErr(L"BitBlt to client failed!");
                    RECT rectTmp = rect;
                    (updatedxCurrentScroll > wd)? rectTmp.right = 0: rectTmp.right = wd - updatedxCurrentScroll;
                    retVal = (BOOL)FillRect(hdcWinCl, &rectTmp, (HBRUSH)(COLOR_WINDOW + 1)); //SetBkColor(hdcWinCl, COLOR_WINDOW + 1) causes flickering in the scrolling
                    }
                }
                else
                    ReportErr(L"hBitmap: Not valid!");

                if(minMaxRestore != SIZE_MAXIMIZED)
                    oldWd = wd;
                if (!DeleteObject(hBmp))
                    ReportErr(L"AdjustImage: Cannot delete bitmap object!!");
            }
            else
            {
                if (minMaxRestore == SIZE_RESTORED)
                    retVal = (BOOL)BitBlt(hdcWinCl, -updatedxCurrentScroll, -updatedyCurrentScroll, bmpWidth, bmpHeight, hdcMem, oldWd, 0, SRCCOPY); //Blt at wd method
                else // SIZE_MINIMIZED or not used
                    retVal = StretchBlt(hdcWinCl, wd - updatedxCurrentScroll, -updatedyCurrentScroll, bmpWidth,
                        bmpHeight, hdcMem, oldWd, 0, bmpWidth, bmpHeight, SRCCOPY);

            }

        }

    return retVal;
}
void GetDims(HWND hWnd, int resizeType, int oldResizeType)
{
    static float firstWd = 0, firstHt = 0, wdBefMax = 0, htBefMax = 0, savedWd = 0, savedHt = 0, savedScaleX = 1, savedScaleY = 1, oldWd = 0, oldHt = 0;
    // int will not compute
    static int startCtrlHt = 0;
    static BOOL firstSizeAfterSTART_SIZE_MOVE = FALSE;


    if (oldWd)
    {

        if (resizeType != SIZE_MINIMIZED && resizeType != MAX_TO_MIN)
        {
            if ((resizeType != START_SIZE_MOVE && !firstSizeAfterSTART_SIZE_MOVE) || (firstSizeAfterSTART_SIZE_MOVE && windowMoved))
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
        {
            if (windowMoved)
            {
                scaleX = wd / oldWd;
                scaleY = ht / oldHt;
                wdBefMax = 0;
                htBefMax = 0;
            }
            else
            {
                if (wdBefMax)
                {
                    scaleX = wd / wdBefMax;
                    scaleY = ht / htBefMax;
                    wdBefMax = 0;
                    htBefMax = 0;
                }
                else
                {
                    scaleX = 1;
                    scaleY = 1;
                }
            }

            firstSizeAfterSTART_SIZE_MOVE = FALSE;
        }
        else
        {
            if (oldResizeType == SIZE_MAXIMIZED)
            {
                scaleX = wd / oldWd;
                scaleY = ht / oldHt;
                wdBefMax = 0;
                htBefMax = 0;
            }
            else
            {
                oldWd ? (scaleX = wd / oldWd) : scaleX = 1;
                oldHt ? (scaleY = ht / oldHt) : scaleY = 1;
            }
        }

    }
    break;
    case START_SIZE_MOVE:
    {
        //excludes case of oldResizeType = START_SIZE_MOVE on a border click
        if (oldResizeType != START_SIZE_MOVE)
        {
            RECT rectBtn = RectCl().RectCl(hWndButton, hWnd, 2);
            RECT rectOpt1 = RectCl().RectCl(hWndOpt1, hWnd, 3);
            startCtrlHt = rectOpt1.top - rectBtn.top;
            savedWd = wd;
            savedHt = ht;
            //don't care about scale, but without it, controls revert to default sizes
            scaleX = wd / oldWd;
            scaleY = ht / oldHt;
            (wd == oldWd) ? savedScaleX = 1 : savedScaleX = scaleX;
            (ht == oldHt) ? savedScaleY = 1 : savedScaleY = scaleY;
            firstSizeAfterSTART_SIZE_MOVE = TRUE;
        }
    }
    break;
    case END_SIZE_MOVE:
    {
        scaleX = wd / savedWd;
        scaleY = ht / savedHt;
    }
    break;
    case SIZE_MAXIMIZED:
    {
        if (oldResizeType == SIZE_MAXIMIZED) //SIZE_MINIMIZED step skipped if MAX_TO_MIN
        {
            scaleX = 1;
            scaleY = 1;
        }
        else
        {
            scaleX = wd / oldWd;
            scaleY = ht / oldHt;
            wdBefMax = oldWd;
            htBefMax = oldHt;
        }
    }
    break;
    default: // SIZE_MINIMIZED
    {
        scaleX = oldWd / wd;
        scaleY = oldHt / ht;
        wd = oldWd;
        ht = oldHt;
    }
    break;
    }

    /*For screen
    sizefactorX = 1, sizefactorY = 1
    sizefactorX = GetSystemMetrics(0) / (3 * wd);
    sizefactorY = GetSystemMetrics(1) / (2 * ht);
    */
}

void SizeControls(BITMAP bmp, HWND hWnd, int& updatedxCurrentScroll, int& updatedyCurrentScroll, int resizeType, int curFmWd, int curFmHt)
{
    int opt1Ht = 0, opt2Ht = 0, chkHt = 0, btnWd = 0, btnHt = 0;
    int newHt = 0, newEdgeWd = 0, newWd = 0, newEdgeHt = 0, newCtrlSizeTriggerWd = 0, newCtrlSizeTriggerHt = 0;

    
    static UINT defFmHt = 0;
    static UINT defFmWd = 0;
    static int oldResizeType = 0, startFmWd = 0, startFmHt = 0, oldFmWd = 0, oldFmHt = 0;
    static int ctrlSizeTriggerWd = 100, ctrlSizeTriggerHt = 100;
    static int defOpt1Top = 0, defOpt2Top = 0, defChkTop = 0;
    static int oldOpt1Top = 0, oldOpt2Top = 0, oldChkTop = 0, oldxScroll = 0, oldyScroll = 0;
    static int minWd = 0, minHt = 0, startSizeBtnLeft = 0, startSizeBtnTop = 0, startSizeBtnRight = 0, startSizeBtnBottom = 0, startSizeOpt1Top = 0, startSizeOpt2Top = 0, startSizeChkTop = 0;
    if (isLoading)
    {
            RECT recthWndtmp = RectCl().RectCl(0, hWnd, 1); //required for wd, ht, below
            curFmWd = RectCl().width(1);
            curFmHt = RectCl().height(1);
    }
    else
    {
        if (procEndWMSIZE)
            procEndWMSIZE = FALSE;
        else
            return;
    }

    if (!minWd)
    {
        minWd = RectCl().width(0);
        minHt = RectCl().height(0);
    }


    if (oldResizeType == SIZE_MAXIMIZED && resizeType == SIZE_MINIMIZED)
    {
        procEndWMSIZE = TRUE;
        return;
    }
    //Get updated rect for the form

    RECT rectBtn = {}, rectOpt1 = {}, rectOpt2 = {}, rectChk = {};

    GetDims(hWnd, resizeType, oldResizeType);
    
    if (capCallFrmResize && oldFmHt && (curFmHt - startFmHt != 0) && (curFmWd - startFmWd != 0))
    {
        procEndWMSIZE = TRUE;
        return;
    }


    rectBtn = RectCl().RectCl(hWndButton, hWnd, 2);
    rectOpt1 = RectCl().RectCl(hWndOpt1, hWnd, 3);
    rectOpt2 = RectCl().RectCl(hWndOpt2, hWnd, 4);
    rectChk = RectCl().RectCl(hWndChk, hWnd, 5);

    if ((ht > OPT_HEIGHT) && !defOpt1Top) // At Init
    {
        defOpt1Top = ht + OPT_HEIGHT;
        defOpt2Top = 2 * ht;
        defChkTop = 3 * ht;
        defFmWd = RectCl().width(1);
        defFmHt = RectCl().height(1);
    }

    if (resizeType == END_SIZE_MOVE)
    {

        btnHt = startSizeBtnBottom - startSizeBtnTop;
        btnWd = startSizeBtnRight - startSizeBtnLeft;
        rectBtn.left = startSizeBtnLeft + oldxScroll - xCurrentScroll;
        rectOpt1.left = startSizeBtnLeft + oldxScroll - xCurrentScroll;
        rectOpt2.left = startSizeBtnLeft + oldxScroll - xCurrentScroll;
        rectChk.left = startSizeBtnLeft + oldxScroll - xCurrentScroll;



        if (startFmHt != curFmHt)
        {
        rectBtn.top = startSizeBtnTop + oldyScroll - yCurrentScroll;
        rectOpt1.top = ((float)curFmHt/ (float)startFmHt) * (startSizeOpt1Top + oldyScroll) - yCurrentScroll;
        rectOpt2.top = ((float)curFmHt/ (float)startFmHt) * (startSizeOpt2Top + oldyScroll) - yCurrentScroll;
        rectChk.top =((float)curFmHt/ (float)startFmHt) * (startSizeChkTop + oldyScroll) - yCurrentScroll;
        }

    }
    else
    {
        btnWd = rectBtn.right - rectBtn.left;
        btnHt = rectBtn.bottom - rectBtn.top;

        if (resizeType == START_SIZE_MOVE)
        {
            if (!startSizeOpt1Top) //initialise once
            {
                oldOpt1Top = rectOpt1.top;
                oldOpt2Top = rectOpt2.top;
                oldChkTop = rectChk.top;
            }

            startSizeBtnTop = -yCurrentScroll;
            startSizeBtnBottom = startSizeBtnTop + (rectBtn.bottom - rectBtn.top);
            startSizeOpt1Top = ((float)curFmHt / (float)defFmHt) * defOpt1Top - yCurrentScroll;
            startSizeOpt2Top = ((float)curFmHt / (float)defFmHt) * defOpt2Top - yCurrentScroll;
            startSizeChkTop = ((float)curFmHt / (float)defFmHt) * defChkTop - yCurrentScroll;
            startSizeBtnLeft = rectBtn.left;
            startSizeBtnRight = rectBtn.right;
            (curFmWd < defFmWd) ? startFmWd = defFmWd : startFmWd = curFmWd;
            (curFmHt < defFmHt) ? startFmHt = defFmHt : startFmHt = curFmHt;
            oldxScroll = xCurrentScroll;
            oldyScroll = yCurrentScroll;


            oldResizeType = resizeType;
            // Unfortunately, applying SetWindowPos here causes more sizing loops,
            // and the size and position of the controls is even worse than current.
            procEndWMSIZE = TRUE;
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
                    if (resizeType != SIZE_MINIMIZED)
                    {
                        newEdgeWd = minWd - 2;
                        newWd = minWd;
                    }
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
                    if (resizeType != SIZE_MINIMIZED)
                    {
                        newEdgeHt = minHt - 2;
                        newHt = minHt;
                    }
                }
            }
        }

        if (rectBtn.top < -yCurrentScroll)
            rectBtn.top = -yCurrentScroll;



        updatedxCurrentScroll = ScrollInfo(hWnd, UPDATE_HORZSCROLLSIZE_CONTROL, 0, 0, curFmWd, curFmHt);
        updatedyCurrentScroll = ScrollInfo(hWnd, UPDATE_VERTSCROLLSIZE_CONTROL, 0, 0, curFmWd, curFmHt);


        if (resizeType == END_SIZE_MOVE)
        SetWindowPos(hWndButton, NULL, rectBtn.left, rectBtn.top, newWd, newHt, SWP_NOREDRAW | SWP_NOACTIVATE);
        else
        {
            // If maximised, scroll value may change- left/top of control may move below zero
            // so its right side wants to be at wd, and bottom at ht.
            if (resizeType == SIZE_MAXIMIZED)
            {
               SetWindowPos(hWndButton, NULL, -updatedxCurrentScroll, -updatedyCurrentScroll,
                    newWd, newHt, NULL);
            }
            else
            {
                if (oldResizeType == SIZE_MAXIMIZED)
                    SetWindowPos(hWndButton, NULL, rectBtn.left, -updatedyCurrentScroll,
                        newWd, newHt, NULL);
                else
                {
                    SetWindowPos(hWndButton, NULL, -updatedxCurrentScroll, -updatedyCurrentScroll,
                        newWd, newHt, NULL);
                    if (!isLoading && isSizing)
                    {
                        (oldFmWd == curFmWd) ? (newCtrlSizeTriggerWd = 0) : ((oldFmWd < curFmWd) ? newCtrlSizeTriggerWd = 1 : newCtrlSizeTriggerWd = -1);
                        (oldFmHt == curFmHt) ? (newCtrlSizeTriggerHt = 0) : ((oldFmHt < curFmHt) ? newCtrlSizeTriggerHt = 1 : newCtrlSizeTriggerHt = -1);
                        if (newCtrlSizeTriggerWd == ctrlSizeTriggerWd)
                            newCtrlSizeTriggerWd = 0;
                        else
                        {
                            if (newCtrlSizeTriggerWd ) //reverse direction
                            {
                                ctrlSizeTriggerWd = newCtrlSizeTriggerWd;
                            }
                        }
                        if (newCtrlSizeTriggerHt == ctrlSizeTriggerHt)
                        {
                            if (newCtrlSizeTriggerHt)
                                if ((float)(curFmHt / oldFmHt) > scaleY)
                                    newCtrlSizeTriggerHt = 0;
                        }
                        else
                        {
                            if (newCtrlSizeTriggerHt) //reverse direction
                            {
                                ctrlSizeTriggerHt = newCtrlSizeTriggerHt;
                            }
                        }
                    }
                }
            }
        }




        if (!isSizing && groupboxFlag)
        {
            // Too much flicker due to the repaint in sizing
            SetWindowPos(hWndGroupBox, HWND_BOTTOM, -xCurrentScroll, -yCurrentScroll,
                newWd, bmp.bmHeight, SWP_DEFERERASE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOACTIVATE );
        }

        if (resizeType != SIZE_MINIMIZED)
        {
            oldOpt1Top = delegateSizeControl(rectOpt1, hWndOpt1, oldOpt1Top, resizeType, oldResizeType, defOpt1Top, updatedxCurrentScroll, updatedyCurrentScroll, newCtrlSizeTriggerHt, newEdgeWd, newWd, minHt);
            //_RPTF4(_CRT_WARN, "yCurrentScroll = %d,  newCtrlSizeTriggerHt = %d,  rectOpt1.top= %d, oldOpt1Top= %d\n", yCurrentScroll, newCtrlSizeTriggerHt, rectOpt1.top, oldOpt1Top);
            // No full path :https://stackoverflow.com/questions/8487986/file-macro-shows-full-path/54335644#54335644
            oldOpt2Top = delegateSizeControl(rectOpt2, hWndOpt2, oldOpt2Top, resizeType, oldResizeType, defOpt2Top, updatedxCurrentScroll, updatedyCurrentScroll, newCtrlSizeTriggerHt, newEdgeWd, newWd, minHt);
            oldChkTop = delegateSizeControl(rectChk, hWndChk, oldChkTop, resizeType, oldResizeType, defChkTop, updatedxCurrentScroll, updatedyCurrentScroll, newCtrlSizeTriggerHt, newEdgeWd, newWd, minHt);
        }

    }
    else // Redundant on !isLoading as btnWd is always > minWd
    {
        if (btnWd < minWd)
        {
            if (resizeType == END_SIZE_MOVE)
                SetWindowPos(hWndButton, NULL, rectBtn.left, rectBtn.top, minWd, rectBtn.bottom - rectBtn.top, NULL);
            else
                SetWindowPos(hWndButton, NULL, scaleX * rectBtn.left + xCurrentScroll, scaleY * rectBtn.top + yCurrentScroll,
                    minWd, rectBtn.bottom - rectBtn.top, NULL);
        }
        if (btnHt < minHt)
        {
            if (resizeType == END_SIZE_MOVE)
                SetWindowPos(hWndButton, NULL, rectBtn.left, rectBtn.top, rectBtn.right - rectBtn.left, minHt, NULL);
            else
                SetWindowPos(hWndButton, NULL, scaleX * rectBtn.left + xCurrentScroll, scaleY * rectBtn.top + yCurrentScroll,
                    rectBtn.right - rectBtn.left, minHt, NULL);
        }
    }

    if (resizeType == SIZE_RESTORED && !isLoading)
    {
        oldFmWd = curFmWd;
        oldFmHt = curFmHt;
    }

    oldResizeType = resizeType;
    procEndWMSIZE = TRUE;
}
int delegateSizeControl(RECT rectOpt, HWND hWndOpt, int oldOptTop, int resizeType, int oldResizeType, int defOptTop, int updatedxCurrentScroll, int updatedyCurrentScroll, int newCtrlSizeTriggerHt, int newEdgeWd, int newWd, int minHt)
{
    int optHt = 0;
    static int yOldScroll = 0;
    if (resizeType == SIZE_RESTORED)
    {
        optHt = scaleY * (rectOpt.bottom - rectOpt.top);
        // Must be adjusted with scroll offsets
        if (oldOptTop)
        {
            if (oldResizeType == SIZE_MAXIMIZED)
                rectOpt.top = oldOptTop + yOldScroll - updatedyCurrentScroll;
            else
            {
                if (oldResizeType != START_SIZE_MOVE) // sizing
                    rectOpt.top += scaleY * (rectOpt.top - oldOptTop + newCtrlSizeTriggerHt);

                if (!rectOpt.top)
                {
                    if (oldOptTop < 0)
                        rectOpt.top = 1;
                    else
                        rectOpt.top = -1;
                }
            }
        }
        //if (curFmHt >= defFmHt)
        //     rectOpt.top += ((oldResizeType == SIZE_RESTORED)? scaleY: 1) * (rectOpt.top - oldOptTop);
        //else
        //    rectOpt.top += ((oldResizeType == SIZE_RESTORED) ? scaleY : 1) * (rectOpt.top - oldOptTop);
    }
    else
    {
        if (resizeType != END_SIZE_MOVE)
        {
            optHt = scaleY * (rectOpt.bottom - rectOpt.top);

            if (resizeType == SIZE_MAXIMIZED)
            {
                yOldScroll = yCurrentScroll;
                if (oldResizeType != SIZE_MAXIMIZED) //SIZE_MINIMIZED not stored
                    oldOptTop = rectOpt.top;
            }
            //rectOpt.top = scaleY * (rectOpt.top - oldOptTop + yCurrentScroll);
            //The following does not preserve vertical scale for rectOpt.top afte SIZE_MAXIMIZED, so want update
            rectOpt.top = scaleY * (rectOpt.top + yCurrentScroll) - updatedyCurrentScroll;


        }
    }

    if (optHt < minHt / 2)
        optHt = minHt / 2;

    if (rectOpt.top < defOptTop - yCurrentScroll)
        rectOpt.top = defOptTop - yCurrentScroll;

    if (resizeType == END_SIZE_MOVE)
        SetWindowPos(hWndOpt, NULL, rectOpt.left, rectOpt.top, newEdgeWd, optHt, NULL);
    else
    {
        if (resizeType == SIZE_MAXIMIZED)
        {
            SetWindowPos(hWndOpt, NULL, -updatedxCurrentScroll, rectOpt.top,
                newWd, optHt, NULL);
        }
        else
        {
            if (oldResizeType == SIZE_MAXIMIZED)
                SetWindowPos(hWndOpt, NULL, rectOpt.left, rectOpt.top,
                    newWd, optHt, NULL);
            else
                SetWindowPos(hWndOpt, NULL, -updatedxCurrentScroll, rectOpt.top,
                    newEdgeWd, optHt, NULL);
        }
    }
    if (resizeType == SIZE_RESTORED || (resizeType == END_SIZE_MOVE) && (oldResizeType != START_SIZE_MOVE)) // curious case of oldResizeType as  START_SIZE_MOVE on a border click
        return rectOpt.top;
    else
        return oldOptTop;
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
    break;
    }

    // New position must be between 0 and screen bounds. 
    newPos = max(0, newPos);
    newPos = min(maxScroll, newPos);

    return newPos;
}
int ScrollInfo(HWND hWnd, int scrollXorY, int scrollType, int scrollDrag, int xNewSize, int yNewSize, int bmpWidth, int bmpHeight)
{

    int newPos, retVal = 0;
    BOOL setWindowThemeRunFlag = FALSE, scrollChanged = FALSE;

    // int bmpWidth, int bmpHeight UINT defFmWd, UINT defFmHt as property or global
    
    static int scrollStat = 0;
    static int oldScrollStat;

        // These variables are required for horizontal scrolling.
    static int xMinScroll = 0;      // minimum horizontal scroll value (starts at 0 so rarely adjusted)
    static int xMaxScroll = 0;      // maximum HORZ scroll value
    static int xTrackPos = 0;   // current scroll drag value

    // These variables are required for vertical scrolling.
    static int yMinScroll = 0;      // minimum vertical scroll value
    static int yMaxScroll = 0;      // maximum VERT scroll value
    static int yTrackPos = 0;

    static SCROLLINFO siHORZ = { 0 };
    static SCROLLINFO siVERT = { 0 };
    procEndWMSIZE = FALSE;

switch (scrollXorY)
    {
    case UPDATE_HORZSCROLLSIZE_CONTROL:
    {
        if (isLoading || scrollStat)
        {
            siHORZ.cbSize = sizeof(siHORZ);
            //ShowScrollBar(hWnd, SB_HORZ, TRUE);
            siHORZ.fMask = SIF_PAGE | SIF_POS;
            siHORZ.nPage = xNewSize;
            SetScrollInfo(hWnd, SB_HORZ, &siHORZ, TRUE);
            GetScrollInfo(hWnd, SB_HORZ, &siHORZ);
            retVal = siHORZ.nPos;
        }
    }
    break;
    case UPDATE_VERTSCROLLSIZE_CONTROL:
    {
        if (isLoading || scrollStat)
        {
            siVERT.cbSize = sizeof(siVERT);
            siVERT.fMask = SIF_PAGE | SIF_POS;
            siVERT.nPage = yNewSize;
            SetScrollInfo(hWnd, SB_VERT, &siVERT, TRUE);
            GetScrollInfo(hWnd, SB_VERT, &siVERT);
            retVal = siVERT.nPos;
        }
    }
    break;
    case HORZ_SCROLL: // scrollXorY > 0 HORZ scroll, < 0 VERT scroll
    {
        if (scrollStat)
        {
            siHORZ.cbSize = sizeof(siHORZ);
            siHORZ.fMask = SIF_TRACKPOS;
            GetScrollInfo(hWnd, SB_HORZ, &siHORZ);
            xTrackPos = siHORZ.nTrackPos;

            // Reset the current scroll position
            newPos = ScrollIt(hWnd, scrollType, scrollDrag, xCurrentScroll, xMinScroll, xMaxScroll, xTrackPos);
            if (newPos == xCurrentScroll)
                retVal = 0;
            else
            {
                // Determine the amount scrolled (in pixels). 
                retVal = newPos - xCurrentScroll; // delta = new_pos - current_pos  
                    // Reset the current scroll position. 

                xCurrentScroll = newPos;
            }
        }
    }
    break;
    case VERT_SCROLL:
    {
        if (scrollStat)
        {
            siVERT.cbSize = sizeof(siVERT);
            siVERT.fMask = SIF_TRACKPOS;
            GetScrollInfo(hWnd, SB_VERT, &siVERT);
            yTrackPos = siVERT.nTrackPos;

            newPos = ScrollIt(hWnd, scrollType, scrollDrag, yCurrentScroll, yMinScroll, yMaxScroll, yTrackPos);
            // If the current position does not change, return 0 and do not scroll.
            if (newPos == yCurrentScroll)
                retVal = 0;
            else
            {
                retVal = newPos - yCurrentScroll;
                yCurrentScroll = newPos;
            }
        }
    }
    break;
    default:
    {

        siHORZ.cbSize = sizeof(siHORZ);
        siHORZ.nPos = xCurrentScroll;

        if (scrollXorY == UPDATE_HORZSCROLLSIZE)
        {
            siHORZ.fMask = SIF_POS;
            SetScrollInfo(hWnd, SB_HORZ, &siHORZ, TRUE);
            retVal = scrollStat;
        }
        else
        {
            if (!scrollXorY)
            {
                if (!isLoading && (bmpWidth + wd > xNewSize))
                {
                    xMaxScroll = max(bmpWidth - xNewSize, 0);
                    xCurrentScroll = min(xCurrentScroll, xMaxScroll);
                    siHORZ.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
                    siHORZ.nMin = xMinScroll;
                    siHORZ.nMax = bmpWidth;
                    siHORZ.nPage = xNewSize;
                    // Not dealing with xTrackPos.
                    SetScrollInfo(hWnd, SB_HORZ, &siHORZ, TRUE);
                    ShowScrollBar(hWnd, SB_HORZ, TRUE);
                    scrollStat = 1;
                    if (scrollStat != oldScrollStat)
                        scrollChanged = TRUE;
                }
                else
                {
                    if (isLoading)
                        siHORZ.cbSize = 0;
                    siHORZ = { 0 };
                    xCurrentScroll = 0;
                    if (scrollStat == 1 || scrollStat == 3) // else not shown
                    {
                        scrollChanged = TRUE;
                        //si.fMask = SIF_DISABLENOSCROLL;
                        if (IsThemeActive()) //SetWindowTheme reveals the hidden HORZ scrollbar if called twice.
                            setWindowThemeRunFlag = (SetWindowTheme(hWnd, NULL, _T("Scrollbar")) == S_OK);
                        // For some reason yet to be determined, SetWindowTheme
                        // also removes the painted bitmap on SIZE_MAXIMIZED
                        // thus a timer is required to repaint it.
                        // Curious, as after this function returns, WM_PAINT is actually
                        // sent by the system to repaint, with no effect!
                        // Also, SetWindowPos in SizeControls occasionally blanks
                        // the image when the scroll changes.
                        if (!ShowScrollBar(hWnd, SB_HORZ, FALSE))
                            ReportErr(L"SB_HORZ: ShowScrollBar failed!");
                        if (scrollStat == 1) // the other scrollbar re-appears!
                            ShowScrollBar(hWnd, SB_VERT, FALSE);

                        (scrollStat == 3) ? scrollStat = 2 : scrollStat = 0;
                    }
                }
            }
        }
        // The vertical scrolling range is defined by 
        // (bitmap_height) - (client_height). The current vertical 
        // scroll value remains within the vertical scrolling range. 

        siVERT.cbSize = sizeof(siVERT);
        siVERT.nPos = yCurrentScroll;

        if (scrollXorY == UPDATE_VERTSCROLLSIZE)
        {
            siVERT.fMask = SIF_POS;

            siVERT.nPos = yCurrentScroll;
            SetScrollInfo(hWnd, SB_VERT, &siVERT, TRUE);
            retVal = scrollStat;
        }
        else
        {
            if (!scrollXorY)
            {
                if (!isLoading && (bmpHeight > yNewSize))
                {
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
                    ShowScrollBar(hWnd, SB_VERT, TRUE);
                    (scrollStat) ? (scrollStat = 3) : (scrollStat = 2);
                    if (scrollStat != oldScrollStat)
                        scrollChanged = TRUE;
                }
                else
                {
                    if (isLoading)
                        siVERT.cbSize = 0;
                    siVERT = { 0 };
                    yCurrentScroll = 0;
                    if (scrollStat == 2 || scrollStat == 3)
                    {
                        scrollChanged = TRUE;
                        if (!setWindowThemeRunFlag && IsThemeActive())
                            SetWindowTheme(hWnd, NULL, _T("Scrollbar"));

                        if (!ShowScrollBar(hWnd, SB_VERT, FALSE))
                            ReportErr(L"SB_VERT: ShowScrollBar failed!");
                        (scrollStat == 3) ? scrollStat = 1 : scrollStat = 0;
                    }

                    //RedrawWindow(hWnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);
                }
            }
            oldScrollStat = scrollStat;
            retVal = scrollStat;
        }
        if (scrollChanged == TRUE)
        {
            if (!timPaintBitmap && !(timPaintBitmap = SetTimer(hWnd,
                IDT_PAINTBITMAP,
                10,
                (TIMERPROC)NULL)))
                ReportErr(L"No timer is available.");
        }
    }
    break;
    }

    procEndWMSIZE = TRUE;
    return retVal;


}
BOOL Kleenup(HWND hWnd, HBITMAP& hBitmap, HBITMAP& hbmpCompat, GpBitmap*& pgpbm, HDC hdcMemDefault, HDC& hdcMem, HDC& hdcMemIn, HDC& hdcWinCl, BOOL noExit)
{
    if (hdcWinCl && !ReleaseDC(hWnd, hdcWinCl) && noExit)
        ReportErr(L"hdcWinCl: Not released!");
    if (pgpbm)
        GpStatus gps = GdipDisposeImage(pgpbm); //No return value
    if (hdcMemDefault)
        SelectObject(hdcMemDefault, hBitmap);
    if (hdcMem && !DeleteDC(hdcMem) && noExit)
        ReportErr(L"hdcMem: DeleteDC failed!");
    if (hdcMemIn && !DeleteDC(hdcMemIn) && noExit)
        ReportErr(L"hdcMemIn: DeleteDC failed!");
    if (hbmpCompat && !DeleteObject(hbmpCompat) && noExit)
        ReportErr(L"hbmpCompat: Not deleted!");
    if (hBitmap && !DeleteObject(hBitmap) && noExit)
        ReportErr(L"hBitmap: Cannot delete bitmap object!");
    if (!noExit)
    DestroyWindow(hWnd);
    return true;
}

BOOL CreateToolTipForRect(HWND hwndParent, int toolType)
{
    static HWND hwndTT = 0; 
    wchar_t off[1] = { L'\0' };
    wchar_t offScreen[55] = {L'f', L'S', L'i', L'z', L'e', L' ', L'n', L'o', L't', L' ', L's', L'e', L't', L':', L' ', L'P',  L'a',  L'r',  L't',  L' ',  L'o',  L'f',  L' ',  L'w',  L'i',  L'n',  L'd',  L'o',  L'w',  L' ', L'n', L'o',  L't', L' ', L'p', L'a', L'i', L'n', L't', L'e', L'd', L'.', L' ', L'O', L'f', L'f', L' ', L'S', L'c', L'r', L'e', L'e', L'n', L'?', L'\0' };
    // Create a tooltip.
    if (!hwndTT)
    hwndTT = CreateWindowExW(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
        WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        hwndParent, NULL, hInst, NULL);

    SetWindowPos(hwndTT, HWND_TOPMOST, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    // Set up "tool" information. In this case, the "tool" is the entire parent window.

    TOOLINFOW ti = { 0 };
    ti.cbSize = sizeof(TOOLINFOW);
    ti.uFlags = TTF_SUBCLASS;
    ti.hwnd = hwndParent;
    ti.hinst = hInst;
    ti.lpszText = (LPWSTR)((toolType) ? offScreen: off);

    GetClientRect(hwndParent, &ti.rect);

    SendMessageW(hwndTT, TTM_SETMAXTIPWIDTH, 0, 150);

    // Associate the tooltip with the "tool" window.
    SendMessageW(hwndTT, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

    return (BOOL)toolType;
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