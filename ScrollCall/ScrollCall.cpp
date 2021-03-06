#include "Scrollcall.h"
RECT RectCl::rectOut1 = {};
RECT RectCl::rectOut2 = {};
RECT RectCl::rectOut3 = {};
RECT RectCl::rectOut4 = {};
RECT RectCl::rectOut5 = {};
RECT RectCl::rectOut6 = {};
HWND RectCl::ownerHwnd = 0;
RECT RectCl::initRectOwnerHwnd = {};
RECT RectCl::rectOwnerHwnd = {};

using namespace Gdiplus;
using namespace Gdiplus::DllExports;


// Global Variables: static can be also applied to these, although not generally recommended

HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // Title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

wchar_t* szFile = nullptr; // Image file name

// Scaling parms
float scrAspect = 0, scaleX = 1, scaleY = 1, resX = 0, resY = 0;
SIZE scrDims = { 0 }, scrEdge = { 0 };
// const UINT WM_USERACTION = WM_APP + 1; //originally considered for UpDown 

int wd = 0, ht = 0; // Button dims
int tmp = 0;    // Temp variable
RECT rectTmp; // Well used temp variable for co-ords
int scrShtOrBmpLoad;    // Type of image loaded: 1: PrintWindow, 2: Screenshot, 3: Image
int xCurrentScroll, yCurrentScroll; // Scroll parms, updated in ScrollInfo and SizeControls

// Window handles
HWND m_hWnd = NULL, hAboutDlg = NULL, hWndGroupBox = 0;
HWND hWndButton = 0, hWndOpt1 = 0, hWndOpt2 = 0, hWndChk = 0;

BOOL procEndWMSIZE = TRUE;  // paranoia variable to prevent sizing recursion 
BOOL scrollChanged = FALSE; // True when any of the scrollbars change visibility 
BOOL scrollChk = TRUE; //   ScrollWindow or ScrollWindowEx
BOOL stretchChk = FALSE;    // Stretch the image
BOOL groupboxFlag = FALSE;  // Use groupbox
BOOL isLoading = TRUE; // Are we loaded yet?
BOOL windowMoved; // Main window moved?
BOOL isSizing = FALSE; // True when sizing
BOOL lastSizeMax = FALSE; // last size was maximised: N/A for PrintTheWindow

// Timer & paint factors
int timDragWindow = 1, timPaintBitmap = 0; // positive value indicates timer running
// timDragWindow initialised for dragging without image loaded
int timPaintDelay = 0, capCallFrmResize = 0;
int timTracker = 0;

//  UpDown variables
//------------------------------------
// Range of values for controls.
const UINT valMin = 0, valMax = 50;
// Handles to the controls.
HWND hLblUpDown = NULL, hwndUpDnEdtBdy = NULL, hwndUpDnCtl = NULL;
HWND ctrlArray[9] = { 0 };



// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK MyBitmapWindowProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK staticSubClass(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
wchar_t* FileOpener(HWND hWnd);
void ReportErr(const wchar_t* szTypes, ...);
void PrintTheWindow(HWND hWnd, HBITMAP hBmp, HBITMAP hBitmapScroll);
void DoMonInfo(HWND hWnd);
BOOL AdjustImage(HWND hWnd, HBITMAP hBitmap, HBITMAP &hBitmapScroll, HGDIOBJ &hdefBitmap, HGDIOBJ &hdefBitmapScroll, BITMAP bmp, HDC& hdcMem, HDC& hdcMemIn, HDC& hdcMemScroll, HDC hdcScreen, HDC hdcScreenCompat, HDC hdcWinCl, UINT& bmpWidth, UINT& bmpHeight, int xNewSize, int yNewSize, int resizePic = 0, int minMaxRestore = 0, BOOL newPic = FALSE);
void PaintScrolledorPrinted(PAINTSTRUCT *ps, HDC hdcWinCl, HDC hdcMem, int xNewSize, int yNewSize, UINT bmpWidth, UINT  bmpHeight, BOOL printCl = FALSE, BOOL noFill = FALSE);
void GetDims(HWND hWnd, int resizeType = 0, int oldResizeType = 0);
void SizeControls(int bmpHeight, HWND hWnd, int &yOldScroll, int resizeType = -1, int curFmWd = 0, int curFmHt = 0, BOOL newPic = FALSE);
int ScrollIt(HWND hWnd, int scrollType, int scrollDrag, int currentScroll, int minScroll, int maxScroll, int trackPos);
int ScrollInfo(HWND hWnd, int scrollXorY, int scrollType, int scrollDrag, int xNewSize = 0, int yNewSize = 0, int bmpWidth = 0, int bmpHeight = 0, BOOL newPic = FALSE);
void ScaleFont(HWND hWnd, int contSize, BOOL isUpDown = FALSE);
BOOL Kleenup(HWND hWnd, HBITMAP& hBitmap, HBITMAP& hBitmapScroll, HGDIOBJ hdefBitmap, HGDIOBJ hdefBitmapScroll, HBITMAP& hbmpCompat, GpBitmap*& pgpbm, HDC& hdcMem, HDC& hdcMemIn, HDC& hdcMemScroll, HDC& hdcWinCl, int typeOfDC = 0, BOOL noExit = FALSE);
int delegateSizeControl(RECT rectOpt, HWND hWndOpt, int oldOptTop, int resizeType, int oldResizeType, int defOptTop, int yScrollBefNew, int newCtrlSizeTriggerHt, int newWd, int newHt, int minHt, BOOL newPic =FALSE, HWND buddyHWnd = 0);
BOOL CreateToolTipForRect(HWND hwndParent, int toolType = 0);
BOOL IsAllFormInWindow(HWND hWnd, BOOL toolTipOn, BOOL isMaximized = FALSE);
HWND UpDownCreate(HWND hWndParent, BOOL ctrlType = 0);
BOOL SetDragFullWindow(BOOL dragFullWindow = FALSE, BOOL restoreDef = FALSE);
void ResetControlPos(HWND hWnd, BOOL setToEdgeX = FALSE, BOOL setToEdgeY = FALSE);
void InitWindowDims(HWND hWnd, int scrollStat, int& xNewSize, int& yNewSize, UINT bmpWidth = 0, UINT bmpHeight = 0);
void ChangeRedrawStyle(HWND hWnd, BOOL removeStyle = FALSE);
wchar_t* ReallocateMem(wchar_t* aSource, int Size);
//This space for "Functions for later use"
//
char* VecToArr(std::vector<std::vector<unsigned>> vec);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    // For debug
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    GdiplusInit gdiplusinit;
    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SCROLLCALL, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow))
        return (INT_PTR)FALSE;



    HACCEL hAccelTable = LoadAcceleratorsW(hInstance, MAKEINTRESOURCEW(IDC_SCROLLCALL));

    MSG msg;
    BOOL a;
    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        //Intent is to filter "About" dialog messages
        a = (hAboutDlg == NULL) ? 0 : IsDialogMessageW(hAboutDlg, &msg);
        if (!a && !TranslateAcceleratorW(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
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

    // Double click for Print Window
    wcex.style = CS_DBLCLKS;
    // CS_HREDRAW | CS_VREDRAW are nice as defaults, but cause invalidation
    // with positive values of timPaintDelay (Paint Mult), so use SetWindowLongPtr
    wcex.lpfnWndProc = MyBitmapWindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCEW(IDI_SCROLLCALL));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_SCROLLCALL);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCEW(IDI_SMALL));

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
        return (INT_PTR)FALSE;
    else
    {
        if (msgboxID == IDYES)
            groupboxFlag = TRUE;
        DoMonInfo(GetDesktopWindow());
    }

    m_hWnd = CreateWindowW(szWindowClass, szTitle, WS_HSCROLL | WS_VSCROLL | WS_OVERLAPPEDWINDOW | (groupboxFlag ? NULL : WS_CLIPCHILDREN),
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
    // For Ye Olde style themes:
    // SetWindowTheme(m_hWnd, L" ", L" ");
    // Or SetThemeAppProperties(STAP_ALLOW_CONTROLS | STAP_ALLOW_WEBCONTENT);
    // SetThemeAppProperties(0);

    if (!m_hWnd)
    {
        ReportErr(L"Cannot create window! Quitting...");
        return (INT_PTR)FALSE;
    }

    ShowWindow(m_hWnd, nCmdShow);
    UpdateWindow(m_hWnd);
    isLoading = FALSE;
    return (INT_PTR)TRUE;
}

//
//  FUNCTION: MyBitmapWindowProc(HWND, UINT, WPARAM, LPARAM)
//  This function is called by the Windows function DispatchMessage( )
//
//  PURPOSE: Processes messages for the main window.
// e.g.
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
LRESULT CALLBACK MyBitmapWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
// note UxTheme messages WM_UAH*

    HDC hDCPaint; // for WM_PAINT

    // static HDC hDCPrint = 0; // for WM_PRINTCLIENT


    // For Aerosnap
    int winArrangeParms[9] = {};
    static BOOL snapHORZ = FALSE;
    static BOOL snapVERT = FALSE;

    // These variables are required by BitBlt. 
    static HBITMAP hBitmap = { 0 };
    static HBITMAP hBitmapScroll = { 0 };
    HGDIOBJ hdefBitmap = { 0 };
    HGDIOBJ hdefBitmapScroll = { 0 };
    static GpStatus gps = { };

    // Scroll & size toggles
    static BOOL fScroll;             // TRUE  if horz, vert scrolling, FALSE for WM_SIZE
    static BOOL fSize;          // TRUE if WM_SIZE 
    static int scrollStat;      // 0: None, 1: SB_HORZ, 2: SB_VERT, 3: SB_BOTH
    static BOOL isMaximized;
    static int maxHorzSize = FALSE; // temporary for minmaxinfo


    // DC's & handles
    static HDC hdcWinCl;            // client area of DC for window
    static HDC hdcMem;            // Mem DC
    static HDC hdcMemIn;            // Mem DC
    static HDC hdcMemScroll;            // Mem DC for scrolling
    static HDC hdcScreen;           // DC for entire screen
    static HDC hdcScreenCompat;     // memory DC for screen
    static HBITMAP hbmpCompat;      // bitmap handle to old DC
    static GpBitmap* pgpbm;          // bitmap data structure
    static BITMAP bmp;          // bitmap data structure

    // Dimension variables
    static UINT bmpWidth = 0;
    static UINT bmpHeight = 0;
    static int yOldScroll; // required to update control positions
    static int xNewSize, xSnapSize, xSnapPos; // updates on WM_SIZE
    static int yNewSize, ySnapSize, ySnapPos;


    //For UpDown
    static const wchar_t* labDesc = L"Paint Mult\0";
    static const int ctlUpDownIncrement = 5;
    UINT nCode;
    int upOrDown = 0;
    LPNMUPDOWN lpnmud = {};


    // More scroll variables & flags
    static int  iDeltaPerLine;      // for mouse wheel logic
    static int iAccumDelta;
    static ULONG ulScrollLines = 0;
    static UINT SMOOTHSCROLL_FLAG;
    static const UINT SMOOTHSCROLL_SPEED = 0X00000002;

    // Toggle for off-monitor tooltip 
    static BOOL toolTipOn;

    // Variables for debug output: 
    static int sizeCount;      //Number of WM_SIZE processed in drag
    static int paintCount;      //Number of WM_PAINT processed in drag
    static int dragTickInit;
    static int dragTick = 0;
    static int paintTick = 0;

    //Main msg loop
    switch (uMsg)
    {
    case WM_CREATE:
    {

        // Init window dimensions
        GetDims(hWnd);
        InitWindowDims(hWnd, scrollStat, xNewSize, yNewSize);

        // Init controls: all control dimensions based on wd & ht
        if (groupboxFlag)
        {
            hWndGroupBox = CreateWindowExW(0, L"BUTTON",
                L"",
                WS_VISIBLE | WS_CHILD | BS_GROUPBOX | WS_CLIPSIBLINGS, //consider  WS_CLIPCHILDREN
                // also MSDN: Do not combine the BS_OWNERDRAW style with any other button styles!
                0, 0, wd, RectCl().height(1),
                hWnd,
                (HMENU)IDC_GROUPBOX,
                hInst,
                NULL);
        }
        ctrlArray[2] = hWndButton = CreateWindowW(
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

        // Radio button Options
        ctrlArray[3] = hWndOpt1 = CreateWindowExW(WS_EX_WINDOWEDGE,
            L"BUTTON",
            L"Scroll",
            // <---- WS_GROUP for grouping the 2 radio buttons.
            WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP | BS_CENTER,
            2, ht + OPT_HEIGHT,
            wd - 2, ht / 2,
            hWnd, //<----- Use main window handle
            (HMENU)IDC_OPT1,
            (HINSTANCE)NULL, NULL);
        ctrlArray[4] = hWndOpt2 = CreateWindowExW(WS_EX_WINDOWEDGE,
            L"BUTTON",
            L"ScrollEx",
            WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | BS_CENTER,
            2, 2 * ht,
            wd - 2, ht / 2,
            hWnd,
            (HMENU)IDC_OPT2,
            (HINSTANCE)NULL, NULL);
        SendMessageW(hWndOpt1, BM_SETCHECK, BST_CHECKED, 0);

        ctrlArray[5] = hWndChk = CreateWindowExW(WS_EX_WINDOWEDGE,
            L"BUTTON",
            L"Stretch",
            WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX | BS_CENTER,
            2, 3 * ht,
            wd - 2, ht / 2,
            hWnd,
            (HMENU)IDC_CHK,
            (HINSTANCE)NULL, NULL);

        EnableWindow(hWndChk, FALSE);

        // UpDown label
        if (ctrlArray[6] = hLblUpDown = CreateWindowExW(WS_EX_LEFT,    //Extended window styles.
            WC_STATIC,
            NULL,
            WS_CHILD | WS_VISIBLE   // Regular window styles.
            | SS_EDITCONTROL | SS_CENTER,           // Typical label control styles.
            0, 0,
            wd, ht / 2,
            hWnd,
            NULL,
            hInst,
            NULL))
            SendMessageW(hLblUpDown, WM_SETTEXT, 0, (LPARAM)labDesc);


        // UpDownCreate contains call to INITCOMMONCONTROLSEX
        ctrlArray[7] = hwndUpDnEdtBdy = UpDownCreate(hWnd);
        ctrlArray[8] = hwndUpDnCtl = UpDownCreate(hWnd, 1);

        // From MSDN:
        // Initialise normal DC (hdcScreen) and a memory DC
        // (hdcScreenCompat)for entire screen. The normal DC
        // provides a snapshot of screen contents. The memory DC
        // keeps a copy of this snapshot in the associated bitmap.

        hdcScreen = CreateDC(L"DISPLAY", (PCTSTR)NULL,
            (PCTSTR)NULL, (CONST DEVMODE*) NULL);
        hdcScreenCompat = CreateCompatibleDC(hdcScreen);

        // Retrieve the metrics for the hdcScreen bitmap
        bmp.bmBitsPixel = (BYTE)GetDeviceCaps(hdcScreen, BITSPIXEL);
        bmp.bmPlanes = (BYTE)GetDeviceCaps(hdcScreen, PLANES);
        bmp.bmWidth = GetDeviceCaps(hdcScreen, HORZRES);
        bmp.bmHeight = GetDeviceCaps(hdcScreen, VERTRES);

        // The width must be byte-aligned. 
        bmp.bmWidthBytes = ((bmp.bmWidth + 15) & ~15) / 8;

        // Create bitmap for the compatible DC. 
        hbmpCompat = CreateBitmap(bmp.bmWidth, bmp.bmHeight,
            bmp.bmPlanes, bmp.bmBitsPixel, (CONST VOID*) NULL);

        // Select the bitmap for the compatible DC. 
        SelectObject(hdcScreenCompat, hbmpCompat);

        // Initialize scrolling flags. 
        fScroll = FALSE;
        fSize = FALSE;
        // Initialize horizontal scrolling variables. 
        xCurrentScroll = 0;
        // Initialize vertical scrolling variables. 
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


        // Subclass groupbox
        if (groupboxFlag)
        {
            if (!SetWindowSubclass(hWndGroupBox, staticSubClass, IDS_GRPSUBCLASS, 0))
                // uIdSubclass is 1 and incremented for each new subclass implemented
                // dwRefData is 0 and has no explicit use
            {
                ReportErr(L"Cannot subclass Groupbox control! Quitting...");
                DestroyWindow(hWndGroupBox);
                return (LRESULT)FALSE;

            }
        }

        /*
        // Possibility of subclassing another control
        if (!SetWindowSubclass(hWndButton, staticSubClassButton, 2, 0))
        {
            ReportErr(L"Cannot subclass control! Quitting...");
            DestroyWindow(hWndButton);
            return NULL;

        }
        */

        // Final initialisations
        SetDragFullWindow();  //store default
        SetDragFullWindow(TRUE);

        scrShtOrBmpLoad = 0;
        toolTipOn = FALSE;
        windowMoved = FALSE;
        return (LRESULT)FALSE;
    }
    break;
    // This not used
    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* mmi = reinterpret_cast<MINMAXINFO*>(lParam);
        static POINT defDims;
        if (!maxHorzSize && !isLoading)
        {
            // Code does not listen for resolution or monitor changes
            maxHorzSize = mmi->ptMaxSize.x;
            if (scrDims.cx > maxHorzSize)
                ReportErr(L"Work width should not exceed monitor width!");
            if (scrDims.cy > mmi->ptMaxSize.y)
                ReportErr(L"Work height should not exceed monitor height!");
            defDims.x = 5 * wd;
            defDims.y = 5 * ht;
        }
        // The structure gets continually pumped with the same values.
        // This is dynamic, another option is to handle it manually.
        mmi->ptMinTrackSize = defDims;
        return (LRESULT)FALSE;
    }
    break;
    case WM_SIZING:
    {
        isSizing = TRUE;
        if (timPaintDelay)
        {
            // Spin timPaintDelay > 0: custom painting during sizing reduces calls to WM_SIZE
            // which then reduces WM_PAINT calls. If not altogether turned off in WM_PAINT
            // there will be automatic window invalidations with COLOR_WINDOW + 1
            // after each bitblt in WM_PAINT, which causes some flicker. Better handling of the timer
            // in this case than current also required to avoid other unwanted visual effects,
            // although the one or two initial flickers while sizing with, say, timPaintDelay = 10
            // seems unavoidable.
            // Otherwise with timPaintDelay = 0, capCallFrmResize remains zero throughout.
            if (!timDragWindow || (capCallFrmResize == timPaintDelay))
            {
                timDragWindow = (int)SetTimer(hWnd,             // handle to main window 
                    IDT_DRAGWINDOW,                   // timer identifier 
                    timPaintDelay,                           // millisecond interval 
                    // Reduced WM_SIZE processing for larger values of timPaintDelay
                    (TIMERPROC)NULL);               // no timer callback 

                if (timDragWindow == 0)
                {
                    ReportErr(L"No timer is available.");
                    capCallFrmResize++;
                }
                else
                {
                    timTracker = 0;
                    capCallFrmResize = 0;
                }
            }
            else
            {
                capCallFrmResize++;
                fSize = TRUE;
                if (capCallFrmResize > timPaintDelay) // Should never get here
                {
                    ReportErr(L"Timer var incorrect!");
                    Kleenup(hWnd, hBitmap, hBitmapScroll, hdefBitmap, hdefBitmapScroll, hbmpCompat, pgpbm, hdcMem, hdcMemIn, hdcMemScroll, hdcWinCl);
                    PostQuitMessage(0);
                }
            }
        }
        return (LRESULT)TRUE;
    }
    break;
    case WM_TIMER:
    {
        switch (wParam)
        {
        case IDT_DRAGWINDOW:
        {
            if (tmp = KillTimer(hWnd, IDT_DRAGWINDOW))
            {
                /*
                // The following is duplicated in WM_SIZE, thus not used:
                SizeControls(bmp, hWnd, xCurrentScroll, yCurrentScroll, START_SIZE_MOVE, xNewSize, yNewSize);
                SizeControls(bmpHeight, hWnd, yOldScroll, START_SIZE_MOVE, xNewSize, yNewSize);
                if (!(AdjustImage(hWnd, hBitmap, hBitmapScroll, hdefBitmap, hdefBitmapScroll, bmp, hdcMem, hdcMemIn, hdcMemScroll, hdcScreen, hdcScreenCompat, hdcWinCl, bmpWidth, bmpHeight, xNewSize, yNewSize, ((stretchChk) ? 2 : 1)), SIZE_MAXIMIZED))
                    ReportErr(L"AdjustImage detected a problem with the image!");
                */
                capCallFrmResize = 0;
            }
            else
                ReportErr(L"IDT_DRAGWINDOW: Problem with timer termination.");
        }
        break;
        case IDT_PAINTBITMAP:
        {

            if (tmp = KillTimer(hWnd, IDT_PAINTBITMAP))
            {
            timPaintBitmap = 0;
            if (!fScroll) // For any value of scrollStat
                // The paint while scrolling is handled elsewhere
            {
                if (scrShtOrBmpLoad == 1)
                {
                    PaintScrolledorPrinted(NULL, hdcWinCl, hdcMem, xNewSize, yNewSize, bmpWidth, bmpHeight, TRUE);
                }
                else
                {
                    // If isMaximized allowed, all the window shows up blank
                    // as the groupbox (and controls) is painted after the snapshot.
                    // As it is, the paint is "messed up."
                    if (groupboxFlag && !isMaximized)
                        InvalidateRect(hWnd, NULL, TRUE);
                    // Else if called in ExitSizeMove, the controls will not always show
                    if ((snapHORZ || snapVERT) || (groupboxFlag && !((scrShtOrBmpLoad == 2) && isMaximized)))
                        SizeControls(bmpHeight, hWnd, yOldScroll, END_SIZE_MOVE, xNewSize, yNewSize);

                    if (((snapHORZ || snapVERT) || groupboxFlag || scrollChanged) && !AdjustImage(hWnd, hBitmap, hBitmapScroll, hdefBitmap, hdefBitmapScroll, bmp, hdcMem, hdcMemIn, hdcMemScroll, hdcScreen, hdcScreenCompat, hdcWinCl, bmpWidth, bmpHeight, xNewSize, yNewSize, ((stretchChk) ? 2 : ((scrShtOrBmpLoad == 2) ? 1 : 0)), SIZE_MAXIMIZED, TRUE))
                        ReportErr(L"AdjustImage detected a problem with the image!");
                    snapHORZ = FALSE;
                    snapVERT = FALSE;

                }
            }
            }
            else
                ReportErr(L"IDT_PAINTBITMAP: Problem with timer termination.");
        }
            break;
        default:
            break;
        }
        return (LRESULT)FALSE;
    }
    break;
    case WM_MOVE:
    {
        // Not concerned with the effects of WM_MOVE
        // (other than it being triggered along with WM_SIZE
        // in Max/Min actions.
        if (!windowMoved)
        {
            if (!isLoading)
                windowMoved = TRUE;
            KillTimer(hWnd, IDT_DRAGWINDOW);
            timDragWindow = 0;
        }
        return (LRESULT)FALSE;
    }
    break;
    case WM_ENTERSIZEMOVE:
    {
        KillTimer(hWnd, IDT_DRAGWINDOW);
        SizeControls(bmpHeight, hWnd, yOldScroll, START_SIZE_MOVE, xNewSize, yNewSize);
        //InvalidateRect(hWnd, 0, TRUE);
        if (!scrShtOrBmpLoad)
            return (LRESULT)FALSE;
        fScroll = FALSE;
        dragTickInit = (int)GetTickCount();
        sizeCount = 0;
        paintCount = 0;

        if (timDragWindow = (int)SetTimer(hWnd,
            IDT_DRAGWINDOW,
            ((timPaintDelay) ? timPaintDelay : 10),
            (TIMERPROC)NULL))
            timTracker = 0;
        else
            ReportErr(L"No timer is available.");
        return (LRESULT)FALSE;
    }
    break;
    case WM_EXITSIZEMOVE:
    {
        fSize = TRUE;
        //InvalidateRect(hWnd, 0, TRUE);
        KillTimer(hWnd, IDT_DRAGWINDOW);
        toolTipOn = IsAllFormInWindow(hWnd, toolTipOn);

        if (windowMoved && !lastSizeMax && !isMaximized)
        {
            windowMoved = FALSE;
            //PostMessageW(hWnd, WM_PAINT, wParam, lParam); //Taken care of by system
        }
        else
        {
        timDragWindow = 0;
        capCallFrmResize = 0;
        isSizing = FALSE;

        if (!stretchChk)
            scrollStat = ScrollInfo(hWnd, 0, 0, 0, xNewSize, yNewSize, bmpWidth, bmpHeight);
        if (!groupboxFlag || scrShtOrBmpLoad < 2)
            SizeControls(bmpHeight, hWnd, yOldScroll, END_SIZE_MOVE, xNewSize, yNewSize);
        //UpdateWindow(hWnd);

        if (!scrShtOrBmpLoad)
        {
            fSize = FALSE;
            return (LRESULT)FALSE;
        }

        if (groupboxFlag)
        {
            // SetScrollInfo is likely to "invalidate" the controls (but NOT for scrShtOrBmpLoad == 1)
            // Unable to "double buffer" the groupbox here so call SizeControls from the timer
            if (scrShtOrBmpLoad)
            {
                if (timPaintBitmap = (int)SetTimer(hWnd,
                    IDT_PAINTBITMAP,
                    IDT_TIMER_SMALL,
                    (TIMERPROC)NULL))
                    timTracker = IDT_TIMER_SMALL;
                else
                    ReportErr(L"No timer is available.");
            }
        }
        else
        {
            // The following may cause flicker, (and clip controls?) in certain circumstances
            if (scrShtOrBmpLoad == 1)
                PaintScrolledorPrinted(NULL, hdcWinCl, hdcMem, xNewSize, yNewSize, bmpWidth, bmpHeight, TRUE);
            else
            {
                if (!(AdjustImage(hWnd, hBitmap, hBitmapScroll, hdefBitmap, hdefBitmapScroll, bmp, hdcMem, hdcMemIn, hdcMemScroll, hdcScreen, hdcScreenCompat, hdcWinCl, bmpWidth, bmpHeight, xNewSize, yNewSize, ((stretchChk) ? 2 : 1), END_SIZE_MOVE)))
                    ReportErr(L"AdjustImage detected a problem with the image!");
            }
        }
        if (timPaintDelay)
        {
            dragTick = (int)GetTickCount() - dragTickInit;
            ReportErr(L"sisi", L"Number of paints: ", paintCount, L"Number of sizes: ", sizeCount);
        }
        }
        return (LRESULT)FALSE;
    }
    break;

    case WM_SIZE:
    {
        if (!procEndWMSIZE)
            return (LRESULT)FALSE;
        lastSizeMax = isMaximized;
        isMaximized = (wParam == SIZE_MAXIMIZED);

        // WM_SIZE called by system for each child control pos change (no subclass)
        if (!(szFile && (szFile[0] == L'*')) && (!capCallFrmResize || !timDragWindow || isMaximized || (wParam == SIZE_MINIMIZED)))
        {

            xNewSize = LOWORD(lParam);
            yNewSize = HIWORD(lParam);
            if (lastSizeMax || isMaximized) // lastSizeMax on the rare occasion the Maximize broaches screen boundaries.
                toolTipOn = IsAllFormInWindow(hWnd, toolTipOn, isMaximized);

            SizeControls(bmpHeight, hWnd, yOldScroll, (int)wParam, xNewSize, yNewSize);

            if (scrShtOrBmpLoad)
            {
                //  Move image along with the size a bit.
                if (scrShtOrBmpLoad == 1)
                    //hDCPrint can lose scope fairly quickly:
                    PaintScrolledorPrinted(NULL, hdcWinCl, hdcMem, xNewSize, yNewSize, bmpWidth, bmpHeight, TRUE, TRUE);
                else
                {
                    if (!AdjustImage(hWnd, hBitmap, hBitmapScroll, hdefBitmap, hdefBitmapScroll, bmp, hdcMem, hdcMemIn, hdcMemScroll, hdcScreen, hdcScreenCompat, hdcWinCl, bmpWidth, bmpHeight, xNewSize, yNewSize, ((stretchChk) ? 2 : 0), (int)wParam))
                        ReportErr(L"AdjustImage detected a problem with the image!");
                }

                if (wParam != SIZE_RESTORED)    // Case of Max/Min after scrolling.
                    fScroll = FALSE;

                fSize = TRUE;

                if (!stretchChk)
                {
                    // The horizontal scrolling range is defined by 
                    // (bitmap_width) - (client_width). The current horizontal 
                    // scroll value remains within the horizontal scrolling range. 
                    if (isSizing)
                    {
                        if (timPaintDelay)
                        {
                            dragTick = (int)GetTickCount() - dragTickInit;
                            sizeCount += 1;
                            ReportErr(L"si", L"In Size: Time elapsed: ", dragTick);
                        }
                    }
                    else
                    {
                        if (scrollStat || lastSizeMax)
                            scrollStat = ScrollInfo(hWnd, 0, 0, 0, xNewSize, yNewSize, bmpWidth, bmpHeight);
                    }
                }
            }
        }
        windowMoved = FALSE;
        return (LRESULT)FALSE;
    }
    break;
    case WM_PAINT:
    {
        if (wParam == 0)
        {
            PAINTSTRUCT ps;
            if (!(hDCPaint = BeginPaint(hWnd, &ps)))
                return (LRESULT)FALSE;

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
                if (scrShtOrBmpLoad == 1)
                    PaintScrolledorPrinted(&ps, NULL, hdcMemScroll, xNewSize, yNewSize, bmpWidth, bmpHeight, TRUE, TRUE);
                else
                    PaintScrolledorPrinted(&ps, NULL, (scrShtOrBmpLoad == 2) ? hdcScreenCompat : hdcMemScroll, xNewSize, yNewSize, bmpWidth, bmpHeight);
            }   // fScroll = 0;
            else
            {

                if (timPaintDelay)
                {
                    /*
                        if (fSize)
                        {
                            if (!capCallFrmResize)
                                fSize = FALSE;
                        }
                        else
                      */
                    if (isSizing)
                    {
                        paintTick = (int)GetTickCount() - dragTickInit;
                        paintCount += 1;
                        ReportErr(L"si", L"In Paint: Time elapsed: ", paintTick);
                    }
                    if (!scrShtOrBmpLoad)
                        toolTipOn = IsAllFormInWindow(hWnd, toolTipOn, isMaximized);
                }
                else
                {
                    if (fSize)
                    {
                        // Getting here just validates the window
                        // All the painting is done performed at the size.
                        if (!capCallFrmResize)
                            fSize = FALSE;
                                //UpdateWindow(hWnd);
                    }
                    else
                    {
                        // Gets here on !scrShtOrBmpLoad, or on a successive WM_MOVE
                        // when a paint is required when form is moved either off screen
                        // or (partly) behind another shown HWND_TOPMOST form.
                        if (!scrShtOrBmpLoad && !isLoading)
                            toolTipOn = IsAllFormInWindow(hWnd, toolTipOn, isMaximized);
                    }
                }
            }
            EndPaint(hWnd, &ps);
        }
        else
            RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);
        return (LRESULT)FALSE;
    }
    break;

    case WM_PRINTCLIENT:
    {
        if (scrShtOrBmpLoad == 1 && !isLoading)
        {
            // hDCPrint = (HDC)wParam;
            // PostMessage causes BitBlt to fail at the hDC
            // SendMessageW(hWnd, WM_PAINT, wParam, lParam);
            // Any sizing code will not work from here in any case
        }
        return 0;
    }
    break;

    case WM_NOTIFY:
    {
        nCode = ((LPNMHDR)lParam)->code;

        switch (nCode)
        {
        case UDN_DELTAPOS:
        {
            lpnmud = (LPNMUPDOWN)lParam;
            upOrDown = lpnmud->iDelta;
            //  lpnmud->iDelta < 0 is up, else down;
            if (upOrDown)
            {
                wchar_t iPosStr[4];
                SendMessageW(hwndUpDnEdtBdy, WM_GETTEXT, (WPARAM)4, (LPARAM)iPosStr);
                timPaintDelay = _wtoi(iPosStr);
                if (upOrDown < 0)
                {
                    if (timPaintDelay > valMin)
                    {
                        timPaintDelay -= ctlUpDownIncrement;
                        tmp = swprintf(NULL, 0, L"%d", timPaintDelay) + 1;
                        swprintf_s(iPosStr, tmp, L"%d", timPaintDelay);
                        if (!SendMessageW(hwndUpDnEdtBdy, WM_SETTEXT, NULL, (LPARAM)iPosStr))
                            ReportErr(L"Spin text failed!");
                    }
                }
                else
                {
                    if (timPaintDelay < valMax)
                    {
                        timPaintDelay += ctlUpDownIncrement;
                        tmp = swprintf(NULL, 0, L"%d", timPaintDelay) + 1;
                        swprintf_s(iPosStr, tmp, L"%d", timPaintDelay);
                        if (!SendMessageW(hwndUpDnEdtBdy, WM_SETTEXT, NULL, (LPARAM)iPosStr))
                            ReportErr(L"Spin text failed!");
                    }
                }
                upOrDown = 0;
            }

        }
        break;
        default:
            break;
        }
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
            return (LRESULT)FALSE;
        fScroll = TRUE;
        fSize = FALSE;
        if (scrShtOrBmpLoad == 1)
        {
            RECT rectControls = {wd + xCurrentScroll, yCurrentScroll, xNewSize + xCurrentScroll, yNewSize + yCurrentScroll};
            if (!ScrollDC(hdcWinCl, -xDelta, 0, (CONST RECT*) &rectControls, (CONST RECT*) &rectControls, (HRGN)NULL, (RECT*) &rectControls))
                ReportErr(L"HORZ_SCROLL: ScrollWindow Failed!");
            // WM_PAINT not automatically sent
            RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
            //InvalidateRect(hWnd, NULL, FALSE);
        }
        else
        {
            if (scrollChk)
            {
                // Another interesting alternative: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-scrolldc
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
                    ReportErr(L"HORZ_SCROLL: ScrollWindow Failed!");
            }
            else
            {
                ScrollWindowEx(hWnd, -xDelta, 0, (CONST RECT*) NULL,
                    (CONST RECT*) NULL, (HRGN)NULL, (PRECT)NULL, SW_SCROLLCHILDREN | SW_INVALIDATE); // SMOOTHSCROLL_FLAG fails
                //UpdateWindow(hWnd);
            }
        }
        // Reset the scroll bar. 
        ScrollInfo(hWnd, UPDATE_HORZSCROLLSIZE, 0, 0);
        return (LRESULT)FALSE;

    }
    break;
    case WM_VSCROLL:
    {
        if (!lParam) //lParam can be the updown control hwnd!
        {
            int yDelta = ScrollInfo(hWnd, VERT_SCROLL, LOWORD(wParam), HIWORD(wParam), xNewSize, yNewSize, bmpWidth, bmpHeight);

            if (!yDelta)
                return (LRESULT)FALSE;
            fScroll = TRUE;
            fSize = FALSE;
            if (scrShtOrBmpLoad == 1)
            {
             RECT rectControls = {wd + xCurrentScroll, yCurrentScroll, xNewSize + xCurrentScroll, yNewSize + yCurrentScroll};

            if (!ScrollDC(hdcWinCl, 0, -yDelta, (CONST RECT*) &rectControls, (CONST RECT*) &rectControls, (HRGN)NULL, (RECT*)NULL))
                ReportErr(L"VERT_SCROLL: ScrollWindow Failed!");

            RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
            }
            else
            {
                if (scrollChk)
                {
                    if (!ScrollWindow(hWnd, 0, -yDelta, (CONST RECT*) NULL, (CONST RECT*) NULL))
                        ReportErr(L"VERT_SCROLL: ScrollWindow Failed!");
                }
                else
                {
                    ScrollWindowEx(hWnd, 0, -yDelta, (CONST RECT*) NULL,
                        (CONST RECT*) NULL, (HRGN)NULL, (PRECT)NULL, SW_SCROLLCHILDREN | SW_INVALIDATE); // SMOOTHSCROLL_FLAG fails
                    //UpdateWindow(hWnd);
                }
            }

            // Reset scroll bar. 
            ScrollInfo(hWnd, UPDATE_VERTSCROLLSIZE, 0, 0);
            yOldScroll = yCurrentScroll;
            return (LRESULT)FALSE;
        }
    }
    break;
    case WM_MOUSEWHEEL:
    {
        if (iDeltaPerLine == 0)
            return (LRESULT)FALSE;

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
        return (LRESULT)FALSE;
    }
    break;
    case WM_NCLBUTTONDBLCLK:
        // The posted notification of the snap is too slow,
        // so implement our own "boiled down" aerosnap.
    {
        // Aerosnap sizing turned on?
        BOOL snapSizingEnabled = FALSE;
        if (SystemParametersInfoW(SPI_GETWINARRANGING, 0, (PVOID)&winArrangeParms, 0))
            snapSizingEnabled = winArrangeParms[0]; // Not exactly in the current documentation
        else
            ReportErr(L"SPI_GETWINARRANGING: Cannot get info.");

        if (snapSizingEnabled)
        {
            if (wParam == HTLEFT || wParam == HTRIGHT)
            {
                snapHORZ = TRUE;
                rectTmp = RectCl().RectCl(0, hWnd, 0);
                if (rectTmp.right - rectTmp.left == scrDims.cx)
                {
                    rectTmp.left = xSnapPos;
                    xNewSize = xSnapSize;
                }
                else
                {
                    xSnapPos = rectTmp.left;
                    xSnapSize = rectTmp.right - rectTmp.left;
                    rectTmp.left = scrEdge.cx;
                    xNewSize = scrDims.cx;
                }

                MoveWindow(hWnd, rectTmp.left, rectTmp.top, xNewSize, RectCl().height(1), FALSE);
                if (timPaintBitmap = (int)SetTimer(hWnd,
                    IDT_PAINTBITMAP,
                    IDT_TIMER_SMALL,
                    (TIMERPROC)NULL))
                    timTracker = IDT_TIMER_SMALL;
                else
                    ReportErr(L"No timer is available.");
            }
            else
            {
                if (wParam == HTBOTTOM)
                {
                    snapVERT = TRUE;
                    rectTmp = RectCl().RectCl(0, hWnd, 0);
                    // Don't trust yNewSize!
                    if (rectTmp.bottom - rectTmp.top == scrDims.cy)
                    {
                        rectTmp.top = ySnapPos;
                        yNewSize = ySnapSize;
                    }
                    else
                    {
                        ySnapPos = rectTmp.top;
                        ySnapSize = rectTmp.bottom - rectTmp.top;
                        rectTmp.top = scrEdge.cy;
                        yNewSize = scrDims.cy;
                    }

                    MoveWindow(hWnd, rectTmp.left, rectTmp.top, RectCl().width(1), yNewSize, FALSE);
                    if (timPaintBitmap = (int)SetTimer(hWnd,
                        IDT_PAINTBITMAP,
                        IDT_TIMER_SMALL,
                        (TIMERPROC)NULL))
                        timTracker = IDT_TIMER_SMALL;
                    else
                        ReportErr(L"No timer is available.");
                }

            }
        }
        return (LRESULT)FALSE;
    }
    break;
    case WM_COMMAND: // WM_PARENTNOTIFY if hWndGroupBox is parent to controls (not the case here)
    {
        USHORT wmId = LOWORD(wParam);
        USHORT wmEvent = HIWORD(wParam);

        // Commented code is for hWndGroupbox as *parent*
        // POINT pt; // Position of cursor when button was pressed
       //  HWND hwndButton; // Button at position of cursor

        // pt.x = LOWORD(lParam);
        // pt.y = HIWORD(lParam);
        // hwndButton = ChildWindowFromPoint(hWndGroupBox, pt);

        switch (wmId) //(GetWindowLong(hwndButton, GWL_ID))
        {
        case IDD_UPDOWN:
        {
            // Interesting link on key commands- not dealt with here in any case.
            //https://social.msdn.microsoft.com/Forums/vstudio/en-US/d02e2e3c-8a47-4812-8475-99cd3309ba7d/possible-bug-in-updown-control-found?forum=vcgeneral
                //Note IDM_RUN_UPDOWN (is no longer?) a Win32 predefined symbol. Perhaps Win16?
        }
        break;
        //Nothing here for case IDD_UPDOWN_BUDDY 

        case IDC_OPT1:
        {
            if (wmEvent == BN_CLICKED)
            {
                scrollChk = (SendMessageW((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED);
                SendMessageW(hWndOpt2, BM_SETCHECK, BST_UNCHECKED, 0);
            }
        }
        break;
        case IDC_OPT2:
        {
            if (wmEvent == BN_CLICKED)
            {
                scrollChk = (SendMessageW((HWND)lParam, BM_GETCHECK, 0, 0) != BST_CHECKED);
                SendMessageW(hWndOpt1, BM_SETCHECK, BST_UNCHECKED, 0);

            }
            // Also BST_INDETERMINATE, BST_UNCHECKED
        }
        break;
        case IDC_CHK:
        {
            if (wmEvent == BN_CLICKED)
                stretchChk = (SendMessageW((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED);

            rectTmp.left = 0;
            rectTmp.top = 0;
            rectTmp.right = max(bmpWidth, xNewSize);
            rectTmp.bottom = max(bmpHeight, yNewSize);
            if (!(tmp = (BOOL)FillRect(hdcWinCl, &rectTmp, (HBRUSH)(COLOR_WINDOW + 1))))
                ReportErr(L"FillRect failed to clear old image!");
            capCallFrmResize = 0;
            if (stretchChk)
            {
                ResetControlPos(hWnd);
                xCurrentScroll = 0;
                yCurrentScroll = 0;
                yOldScroll = yCurrentScroll;
                if (scrollStat = ScrollInfo(hWnd, 0, 0, 0, xNewSize, yNewSize, xNewSize - wd, yNewSize))
                    ReportErr(L"ScrollStat: No scrollbars with stretch.");
            }
            else
            {
                scrollStat = ScrollInfo(hWnd, 0, 0, 0, xNewSize, yNewSize, bmpWidth, bmpHeight, TRUE);
                SizeControls(bmpHeight, hWnd, yOldScroll, ((isMaximized) ? SIZE_MAXIMIZED : SIZE_RESTORED), xNewSize, yNewSize);
            }

            if (!AdjustImage(hWnd, hBitmap, hBitmapScroll, hdefBitmap, hdefBitmapScroll, bmp, hdcMem, hdcMemIn, hdcMemScroll, hdcScreen, hdcScreenCompat, hdcWinCl, bmpWidth, bmpHeight, xNewSize, yNewSize, ((stretchChk) ? 2 : 1), 0, TRUE))
                ReportErr(L"AdjustImage detected a problem with the image!");
            isSizing = FALSE;
            fScroll = FALSE;
            timDragWindow = 0;
            toolTipOn = IsAllFormInWindow(hWnd, toolTipOn);
        }
        break;
        case ID_OPENBITMAP:
        {

            szFile = (wchar_t*)calloc(MAX_LOADSTRING, sizeof(wchar_t));
            wcscpy_s(szFile, MAX_LOADSTRING, FileOpener(hWnd));


            //CLSID pngClsid;
           // GetEncoderClsid(L"image/png", &pngClsid);
            if (szFile[0] != L'*')
            {
                Kleenup(hWnd, hBitmap, hBitmapScroll, hdefBitmap, hdefBitmapScroll, hbmpCompat, pgpbm, hdcMem, hdcMemIn, hdcMemScroll, hdcWinCl, 3, TRUE);

                Color clr;

                // const std::vector<std::vector<unsigned>>resultPixels; // for GetPixels

                gps = GdipCreateBitmapFromFile(szFile, &pgpbm);
                if (gps == Ok)
                {
                    hdcWinCl = GetDCEx(hWnd, (HRGN)NULL, DCX_CACHE | DCX_CLIPCHILDREN);


                    hBitmap = NULL;
                    gps = GdipCreateHBITMAPFromBitmap(pgpbm, &hBitmap, clr.GetValue());
                    // black = (bits == 0);   alpha=255 => 0xFF000000
                    if (hBitmap)
                    {
                        // "White over" existing DC first
                        rectTmp.left = 0;
                        rectTmp.top = 0;
                        rectTmp.right = bmpWidth + wd;
                        rectTmp.bottom = bmpHeight;
                        if (!((BOOL)FillRect(hdcWinCl, &rectTmp, (HBRUSH)(COLOR_WINDOW + 1))))
                            ReportErr(L"FillRect failed to clear old image!");

                        gps = GdipGetImageWidth(pgpbm, &bmpWidth);
                        gps = GdipGetImageHeight(pgpbm, &bmpHeight);
                        //HBITMAP hBmpCopy = (HBITMAP)CopyImage(hBitmap, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
                        //GpStatus WINGDIPAPI GdipDrawImageI(GpGraphics* graphics, GpImage* image, INT x, INT y);

                        hdcMemIn = CreateCompatibleDC(hdcWinCl);
                        hdcMem = CreateCompatibleDC(hdcWinCl);
                        hdcMemScroll = CreateCompatibleDC(hdcWinCl);

                        SendMessageW(hWndChk, BM_SETCHECK, BST_UNCHECKED, 0);
                        stretchChk = 0;

                        szFile[0] = L'*'; // Set to deal with WM_SIZE issues

                        if (scrShtOrBmpLoad == 1)
                        {
                            xCurrentScroll = 0;
                            yCurrentScroll = 0;
                        }

                        ResetControlPos(hWnd, !xCurrentScroll, !yCurrentScroll);
                        scrShtOrBmpLoad = 3;
                        // Get bmpWidth, bmpHeight only
                        AdjustImage(hWnd, hBitmap, hBitmapScroll, hdefBitmap, hdefBitmapScroll, bmp, hdcMem, hdcMemIn, hdcMemScroll, hdcScreen, hdcScreenCompat, hdcWinCl, bmpWidth, bmpHeight, 0, 0, 1, 0, TRUE);

                        yOldScroll = yCurrentScroll;

                        //SetWindowOrgEx(hdcWinCl, xCurrentScroll, yCurrentScroll, NULL);
                        if (bmpWidth < xNewSize)
                            xCurrentScroll = 0;
                        if (bmpHeight < yNewSize)
                            yCurrentScroll = 0;

                        SizeControls(bmpHeight, hWnd, yCurrentScroll, ((isMaximized) ? SIZE_MAXIMIZED : SIZE_RESTORED), xNewSize, yNewSize, TRUE);


                        // Update size- essential if just after loading
                        InitWindowDims(hWnd, scrollStat, xNewSize, yNewSize, bmpWidth, bmpHeight);
                        // Flag new pic
                        scrollStat = ScrollInfo(hWnd, 0, 0, 0, xNewSize, yNewSize, bmpWidth, bmpHeight, TRUE);

                        if (!AdjustImage(hWnd, hBitmap, hBitmapScroll, hdefBitmap, hdefBitmapScroll, bmp, hdcMem, hdcMemIn, hdcMemScroll, hdcScreen, hdcScreenCompat, hdcWinCl, bmpWidth, bmpHeight, xNewSize, yNewSize, 1, 0, TRUE))
                            ReportErr(L"AdjustImage detected a problem with the image!");


                        fScroll = FALSE;
                        szFile[0] = L'X';
                        timDragWindow = 0;

                        // CallDeprecatedSetScrollPos(hWnd, scrollStat); // not requ'd
                        // UpdateWindow(hWnd);
                        EnableWindow(hWndChk, TRUE);
                        // Remove invalidation, specifically on positive timPaintDelay

                        ChangeRedrawStyle(hWnd);
                        toolTipOn = IsAllFormInWindow(hWnd, toolTipOn);
                    }
                    else
                        ReportErr(L"hBitmap: Cannot create bitmap!!");
                }
                else
                    ReportErr(L"GPS: Cannot open bitmap!");
            }


            free(szFile);
            isSizing = FALSE;
            return (LRESULT)TRUE;
        }
        break;
        case IDM_ABOUT:
        {
            if (hAboutDlg)
                ShowWindow(hAboutDlg, SW_SHOWDEFAULT);
            else
            {
                //DialogBox(hInst, MAKEINTRESOURCEW(IDD_ABOUTBOX), hWnd, About);
                if (hAboutDlg = CreateDialogParamW(hInst, MAKEINTRESOURCEW(IDD_ABOUTBOX), hWnd, About, NULL))
                    ShowWindow(hAboutDlg, SW_SHOWDEFAULT);
                else
                    ReportErr(L"About: Cannot create!");
            }
        }

        break;
        case IDM_EXIT:
        {
            DestroyWindow(hWnd);
            // _CrtDumpMemoryLeaks();
        }
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

        int oldbmpWidth = bmpWidth;
        int oldbmpHeight = bmpHeight;
        // Experiment with WM_PRINT
        Kleenup(hWnd, hBitmap, hBitmapScroll, hdefBitmap, hdefBitmapScroll, hbmpCompat, pgpbm, hdcMem, hdcMemIn, hdcMemScroll, hdcWinCl, 1, TRUE);


        if (hdcWinCl = GetDCEx(hWnd, (HRGN)NULL, DCX_CACHE | DCX_CLIPCHILDREN))
        {
            hdcMem = CreateCompatibleDC(hdcWinCl);
            hdcMemScroll = CreateCompatibleDC(hdcWinCl);
            bmpWidth = RectCl().width(1) - wd;
            bmpHeight = RectCl().height(1);


            hBitmap = CreateCompatibleBitmap(hdcWinCl, bmpWidth, bmpHeight);
            hBitmapScroll = CreateCompatibleBitmap(hdcWinCl, bmpWidth, bmpHeight);
            if (hBitmap)
            {
                rectTmp.left = 0;
                rectTmp.top = 0;
                rectTmp.right = max(max(bmpWidth, oldbmpWidth), xNewSize);
                rectTmp.bottom = max(max(bmpHeight, oldbmpHeight), yNewSize);
                if (!(tmp = (BOOL)FillRect(hdcWinCl, &rectTmp, (HBRUSH)(COLOR_WINDOW + 1))))
                    ReportErr(L"FillRect: Paint failed!");

                PrintTheWindow(hWnd, hBitmap, hBitmapScroll);
                hdefBitmap = SelectObject(hdcMem, hBitmap);
                hdefBitmapScroll = SelectObject(hdcMemScroll, hBitmapScroll);

                yOldScroll = yCurrentScroll;

                if (xCurrentScroll || yCurrentScroll)
                {
                    //Controls must be returned to 0
                    xCurrentScroll = 0;
                    yCurrentScroll = 0;

                    //SetWindowOrgEx(hdcWinCl, xCurrentScroll, yCurrentScroll, NULL);
                    ResetControlPos(hWnd, TRUE, TRUE);
                }

                scrollStat = ScrollInfo(hWnd, 0, 0, 0, xNewSize, yNewSize, bmpWidth, bmpHeight, TRUE);

                SizeControls(bmpHeight, hWnd, yOldScroll, ((isMaximized) ? SIZE_MAXIMIZED : END_SIZE_MOVE), xNewSize, yNewSize, TRUE);
                // can exclude RectCl().ClMenuandTitle(hWnd)

                InitWindowDims(hWnd, scrollStat, xNewSize, yNewSize, bmpWidth, bmpHeight);

                //BitBlt to wd from hdcMem
                if (!(BOOL)BitBlt(hdcMemScroll, wd, 0, bmpWidth, bmpHeight, hdcMem, 0, 0, SRCCOPY))
                    ReportErr(L"Bad BitBlt to hdcMemScroll!");


                EnableWindow(hWndChk, FALSE);
                SendMessageW(hWndChk, BM_SETCHECK, BST_UNCHECKED, 0);
                stretchChk = 0;

                // Prefer redraw on size
                ChangeRedrawStyle(hWnd, TRUE);
                toolTipOn = IsAllFormInWindow(hWnd, toolTipOn);
                scrShtOrBmpLoad = 1;
                if (timPaintBitmap = (int)SetTimer(hWnd,
                    IDT_PAINTBITMAP,
                    IDT_TIMER_LARGE,
                    (TIMERPROC)NULL))
                    timTracker = IDT_TIMER_LARGE;
                else
                    ReportErr(L"No timer is available.");
                timDragWindow = 0;
                isSizing = FALSE;
                fScroll = FALSE;

            }
            else
                ReportErr(L"CreateCompatibleBitmap: Unable to create bitmap!");
        }
        else
            ReportErr(L"GetDCEx: Failed to get DC!");

        return (LRESULT)FALSE;
    }
    break;
    case WM_RBUTTONDOWN:
    {
        // Kleenup rather pointless as hbmpCompat is already selected in hdcScreenCompat.
        Kleenup(hWnd, hBitmap, hBitmapScroll, hdefBitmap, hdefBitmapScroll, hbmpCompat, pgpbm, hdcMem, hdcMemIn, hdcMemScroll, hdcWinCl, 2, TRUE);


        // Get the compatible DC of the client area. 
        if (hdcWinCl = GetDCEx(hWnd, (HRGN)NULL, DCX_CACHE | DCX_CLIPCHILDREN))
        {

            // Adjust old scroll co-ords to new setting
            if (scrShtOrBmpLoad == 1)
            {
                xCurrentScroll = 0;
                yCurrentScroll = 0;
            }

            ResetControlPos(hWnd, !xCurrentScroll, !yCurrentScroll);

            scrShtOrBmpLoad = 2;

            yOldScroll = yCurrentScroll;

            // For the first load of image: Has to be invoked twice, even though GetDims() has it.
            InitWindowDims(hWnd, scrollStat, xNewSize, yNewSize, bmp.bmWidth - wd, bmp.bmHeight);

            scrollStat = ScrollInfo(hWnd, 0, 0, 0, xNewSize, yNewSize, bmp.bmWidth - wd, bmp.bmHeight, TRUE);
            SizeControls(bmp.bmHeight, hWnd, yCurrentScroll, ((isMaximized) ? SIZE_MAXIMIZED : SIZE_RESTORED), xNewSize, yNewSize, TRUE);

            if (!AdjustImage(hWnd, hBitmap, hBitmapScroll, hdefBitmap, hdefBitmapScroll, bmp, hdcMem, hdcMemIn, hdcMemScroll, hdcScreen, hdcScreenCompat, hdcWinCl, bmpWidth, bmpHeight, xNewSize, yNewSize, 1, 0, TRUE))
                ReportErr(L"AdjustImage detected a problem with the image!");



            fScroll = FALSE;
            timDragWindow = 0;
            // CallDeprecatedSetScrollPos(hWnd, scrollStat); // not requ'd
            EnableWindow(hWndChk, FALSE);
            SendMessageW(hWndChk, BM_SETCHECK, BST_UNCHECKED, 0);
            stretchChk = 0;

            //SetWindowOrgEx(hdcWinCl, xCurrentScroll, yCurrentScroll, NULL);
            ChangeRedrawStyle(hWnd, TRUE);

            toolTipOn = IsAllFormInWindow(hWnd, toolTipOn);
        }
        else
            ReportErr(L"GetDCEx: Failed to get DC!");

        isSizing = FALSE;
        return (LRESULT)FALSE;
    }
    break;
    case WM_KEYDOWN:
    {
        switch (wParam)
        {
        case VK_ESCAPE:
        {
            DestroyWindow(hWnd);
        }
        break;
        default:
            break;
        }
    }
    break;
    case WM_DESTROY:
    {
        // Invoked again from the internals- no big deal.
        Kleenup(hWnd, hBitmap, hBitmapScroll, hdefBitmap, hdefBitmapScroll, hbmpCompat, pgpbm, hdcMem, hdcMemIn, hdcMemScroll, hdcWinCl);
        PostQuitMessage(0);
    }
    break;
    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
        break;
    }
    return (LRESULT)FALSE;
}

LRESULT CALLBACK staticSubClass(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    UNUSED(dwRefData);
   // static int savefScroll = 0;

    switch (uMsg) // instead of LPNMHDR  lpnmh = (LPNMHDR) lParam above;
    {
    case WM_PAINT:
    {
        //"If wParam is non-NULL, the common control may assume the value is an HDC and paints using that device context."
        if (wParam == 0)
        {

            // Supposed to "halve" the flickering in scrolling. No perceivable difference.
            /*
            if (fScroll)
            {
                savefScroll = fScroll;
                fScroll = 0;
            }
            */
            if (!isSizing)
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hWnd, &ps);
                if (scrShtOrBmpLoad != 1)
                {
                    PRECT prect;
                    prect = &ps.rcPaint;
                    if (!(BOOL)FillRect(hdc, prect, (HBRUSH)(COLOR_WINDOW + 1)))
                        ReportErr(L"FillRect: Groupbox paint failed!");
                }
                EndPaint(hWnd, &ps);
                //fScroll = savefScroll;
            }
        }
        else
            RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);
        return (LRESULT)TRUE;
    }
    break;
    case WM_NCDESTROY:
    {
        // NOTE: this requirement is NOT stated in the documentation, but it is stated in Raymond Chen's blog article...
        RemoveWindowSubclass(hWnd, staticSubClass, uIdSubclass);
        return (LRESULT)FALSE;
    }
    break;
    default:
        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
        break;
    }

    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}


// Message handler for about box.
INT_PTR CALLBACK About(HWND hAboutDlg, UINT message, WPARAM wParam, LPARAM lParam)
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
            EndDialog(hAboutDlg, LOWORD(wParam));
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

HWND UpDownCreate(HWND hWndParent, BOOL ctrlType)
{
    HWND hControl = NULL;
    int classVar = 0;
    if (ctrlType)
        classVar = ICC_STANDARD_CLASSES;
    else
        classVar = ICC_UPDOWN_CLASS;
    static const INITCOMMONCONTROLSEX commonCtrls =
    {
    sizeof(INITCOMMONCONTROLSEX),
    (DWORD)classVar
    };

    if (ctrlType)
    {
        if (hControl = CreateWindowExW(WS_EX_LEFT | WS_EX_LTRREADING,
            UPDOWN_CLASS,
            NULL,
            WS_CHILDWINDOW | WS_VISIBLE
            | UDS_AUTOBUDDY | UDS_ALIGNLEFT | UDS_ARROWKEYS | UDS_HOTTRACK,
            0, 0,
            wd / 2, ht,          // Or set to zero to automatically size to fit the buddy window.
            hWndParent,
            (HMENU)IDD_UPDOWN,
            hInst,
            NULL))
            SendMessageW(hControl, UDM_SETRANGE, 0, MAKELPARAM(valMax, valMin));    // Sets controls' direction & range.
    }
    else
    {
        const wchar_t* initZero = L"0\0";
        if (hControl = CreateWindowExW(WS_EX_LEFT,    //Extended window styles.
            WC_STATIC,
            NULL,
            WS_CHILDWINDOW | WS_VISIBLE | WS_BORDER | WS_GROUP   // Window styles.
            | SS_EDITCONTROL | ES_CENTER,                     // Label control style.
            0, 0,
            wd / 2 - 1, ht,
            hWndParent,
            (HMENU)IDD_UPDOWN_BUDDY,
            hInst,
            NULL))
            SendMessageW(hControl, WM_SETTEXT, 0, (LPARAM)initZero);
    }

    if (!hControl)
        ReportErr(L"Problem with UpDown control creation.");
    return hControl;
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

//example to use: ReportErr(L"test %s \n %d %d %d %s %d %d %d", L"str", 1, 2, 3, L"\n", 1, 2, 3);
    /*
            RECT rectBtn = RectCl().RectCl(hWndButton, hWnd, 2);
            ReportErr(L"Values:  \n rectBtn.left: %d rectBtn.right: %d rectBtn.top: %d rectBtn.bottom: %d"
                "\n xCurrentScroll: %d yCurrentScroll: %d xCurrentScroll: %d xCurrentScroll: %d"
                "\n scrDims.cx: %d scrDims.cy: %d scrEdge.cx: %d scrEdge.cy: %d",
                rectBtn.left, rectBtn.right, rectBtn.top, rectBtn.bottom, xCurrentScroll, xCurrentScroll, xCurrentScroll, xCurrentScroll, scrDims.cx, scrDims.cy, scrEdge.cx, scrEdge.cy);
    */
void ReportErr(const wchar_t* szTypes, ...)
{
    int len, lenTotal, i;
    wchar_t* buf = 0, * outBuf = 0;
    va_list args;
    va_start(args, szTypes);
    len = lenTotal = 0;
    // Order of function parameters must match order in szTypes
    // or outBuf will print garbage or swprintf will seg fault.
    if (wcscmp(szTypes, L"wisi") && wcscmp(szTypes, L"si") && wcscmp(szTypes, L"sisi"))
    {
        if (szTypes)
        {
            // a bodgy format string here can produce ERROR_NOT_SUPPORTED exception
            len = _vscwprintf(szTypes, args) + 2;  //add room for terminating '\0'
            outBuf = (wchar_t*)malloc(len * sizeof(wchar_t));
            vswprintf_s(outBuf, len, szTypes, args);

            if (FALSE)
                OutputDebugStringW(outBuf);
            else
                MessageBoxW(m_hWnd, outBuf, L"Error", MB_ICONINFORMATION);

        }

    }
    else
    {
        outBuf = (wchar_t*)malloc(MAX_LOADSTRING * sizeof(wchar_t));
        memset(outBuf, 0, MAX_LOADSTRING * sizeof(wchar_t));
        outBuf[0] = L'\0';
        // https://docs.microsoft.com/en-us/cpp/cpp/functions-with-variable-argument-lists-cpp?
        // Step through the list.
        for (i = 0;  i < wcslen (szTypes); ++i)
        {
            union Printable_t
            {
                int i;
                float   f;
                wchar_t w;
                wchar_t* s;
            } Printable;
            buf = (wchar_t*)malloc(MAX_LOADSTRING * sizeof(wchar_t));
            memset(buf, 0, MAX_LOADSTRING * sizeof(wchar_t));

            if ((szTypes[i]))
            {
                switch (szTypes[i])
                {   // Type to expect.
                case L'i':
                {
                    Printable.i = va_arg(args, int);
                    len = 2 * swprintf(buf, MAX_LOADSTRING, L"%i", Printable.i) + 2;  //add room for terminating '\0'
                    buf = ReallocateMem(buf, len);
                    //wprintf_s(L"%i\n", Printable.i);
                    swprintf(buf, len, L"%i\0", Printable.i);
                }
                break;
                case L'f':
                {
                    Printable.f = va_arg(args, double);
                    len = 2 * swprintf(buf, MAX_LOADSTRING, L"%f", Printable.f) + 2;
                    buf = ReallocateMem(buf, len);
                    //wprintf_s(L"%i\n", Printable.f);
                    swprintf(buf, len, L"%f\0", Printable.f);
                }
                break;
                case L'w':
                {
                    Printable.w = va_arg(args, wchar_t);
                    len = 2 * swprintf(buf, MAX_LOADSTRING, L"%c", Printable.w) + 2;
                    buf = ReallocateMem(buf, len);
                    //wprintf_s(L"%c\n", Printable.w);
                    swprintf(buf, len, L"%c\0", Printable.w);
                }
                break;
                case L's':
                {
                    Printable.s = va_arg(args, wchar_t*);
                    len = 2 * swprintf(buf, MAX_LOADSTRING, L"%s", Printable.s) + 2;
                    buf = ReallocateMem(buf, len);
                    //wprintf_s(L"%s\n", Printable.s);
                    swprintf(buf, len, L"%s\0", Printable.s);
                }
                break;
                default:
                    break;
                }
                if (buf)
                {
                    lenTotal += len + 2;
                    wcscat_s(outBuf, lenTotal, L"\n");

                    wcscat_s(outBuf, lenTotal, buf);
                    free(buf);
                }
            }
        }

        if (TRUE)
        {
            OutputDebugStringW(outBuf);
            OutputDebugStringW(L"\n");
        }
        else
            MessageBoxW(m_hWnd, buf, L"Error", MB_ICONINFORMATION);
    }
    if (outBuf)
    free(outBuf);
    va_end(args);
}

void PrintTheWindow(HWND hWnd, HBITMAP hBitmap, HBITMAP hBitmapScroll)
{
    HDC hDCMem = CreateCompatibleDC(NULL);
    HDC hDCMemScroll = CreateCompatibleDC(NULL);

    if (hDCMem && hDCMemScroll)
    {
        SelectObject(hDCMem, hBitmap);
        SendMessageW(hWnd, WM_PRINT, (WPARAM)hDCMem, PRF_CLIENT | PRF_ERASEBKGND | PRF_NONCLIENT);
        SelectObject(hDCMemScroll, hBitmapScroll);
        SendMessageW(hWnd, WM_PRINT, (WPARAM)hDCMemScroll, PRF_CLIENT | PRF_ERASEBKGND | PRF_NONCLIENT);
        //Sleep(10);

        DeleteDC(hDCMem);
        DeleteDC(hDCMemScroll);
    }
    else
        ReportErr(L"Could CreateCompatibleDC for hDCMem/hdcMemScroll!");
}
void DoMonInfo(HWND hWnd)
{
    HMONITOR hMonPrimary = MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);
    HMONITOR hMonNearest = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);

    //if (hMonPrimary == hMonNearest)
    MONITORINFO monInfoPrimary = {};
    monInfoPrimary.cbSize = sizeof(MONITORINFO);
    MONITORINFO monInfoNearest = {};
    monInfoNearest.cbSize = sizeof(MONITORINFO);
    if (GetMonitorInfoW(hMonPrimary, &monInfoPrimary))
    {
        if (monInfoPrimary.dwFlags != MONITORINFOF_PRIMARY)
            ReportErr(L"ScrollCall has not been fully tested on a secondary monitor!.");
    }
    else
        ReportErr(L"GetMonitorInfo: Cannot get info.");
    if (GetMonitorInfoW(hMonNearest, &monInfoNearest))
    {
        if (monInfoNearest.dwFlags != MONITORINFOF_PRIMARY)
            ReportErr(L"ScrollCall has not been fully tested on a secondary monitor!.");
    }
    else
        ReportErr(L"GetMonitorInfo: Cannot get info.");

    scrEdge.cx = monInfoPrimary.rcWork.left;
    scrEdge.cy = monInfoPrimary.rcWork.top;
    scrDims.cx = monInfoPrimary.rcWork.right - monInfoPrimary.rcWork.left;
    scrDims.cy = monInfoPrimary.rcWork.bottom - monInfoPrimary.rcWork.top;

    if (monInfoPrimary.rcMonitor.right != monInfoNearest.rcMonitor.right)
        ReportErr(L"Driver Misconfiguration? Monitor Mismatch.");
    if (monInfoPrimary.rcMonitor.right != monInfoNearest.rcMonitor.right)
        ReportErr(L"Driver Misconfiguration? Monitor Mismatch.");
}
BOOL AdjustImage(HWND hWnd, HBITMAP hBitmap, HBITMAP &hBitmapScroll, HGDIOBJ &hdefBitmap, HGDIOBJ &hdefBitmapScroll, BITMAP bmp, HDC& hdcMem, HDC& hdcMemIn, HDC& hdcMemScroll, HDC hdcScreen, HDC hdcScreenCompat, HDC hdcWinCl, UINT& bmpWidth, UINT& bmpHeight, int curFmWd, int curFmHt, int resizePic, int minMaxRestore, BOOL newPic)
{
    // The names oldWdIn & oldWdOut may not be the best descriptors,-
    // they are used in mousedown screenshot drag-in and drag-out visuals,
    // all because the screenshot is stretched inward to fit in with the controls
    static int newPicWd = 0, oldWdIn = 0, oldWdOut = wd, oldCurFmWd = 0, defCurFmWd = 0, sizingChangeDir = 0;
    static RECT imgRect = {};
    static BOOL baseDCBltd = FALSE, firstDragAfternewPic = FALSE;
    
    BOOL retVal = FALSE;
    //if (timPaintDelay)
        //SendMessageW(hWnd, WM_SETREDRAW, TRUE, 0);
    if (newPic)
    {
        HDC hdcWin;            // DC for window
        hdcWin = GetWindowDC(hWnd); // Let's not forget it's a shared windows resource
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
            bmpWidth = (scrShtOrBmpLoad == 2) ? bmp.bmWidth - wd : bmpWidth;
            bmpHeight = (scrShtOrBmpLoad == 2) ? bmp.bmHeight : bmpHeight;
        }
        else
            ReportErr(L"AdjustImage: Unable to size bitmap!");

        /*
        // If for some reason the bmp dims are required not to exceed screen dims:
        int maxBmpWd = scrDims.cx - wd;
         int maxBmpHt = scrDims.cy - RectCl().ClMenuandTitle(hWnd);
         if (bmpWidth > maxBmpWd)
             bmpWidth = maxBmpWd;
         if (bmpHeight > maxBmpHt)
             bmpHeight = maxBmpHt;
         */
        if (!ReleaseDC(hWnd, hdcWin))
            ReportErr(L"hdcWin: Not released!");

        // Called for dimension only
        if (!curFmWd)
            return 0;
        imgRect.left = 0;
        imgRect.top = 0;
        imgRect.right = bmpWidth;
        imgRect.bottom = bmpHeight;

        if (minMaxRestore != SIZE_MAXIMIZED)
        newPicWd = wd;

        defCurFmWd = oldCurFmWd = curFmWd;
        firstDragAfternewPic = TRUE;
    }

    if (scrShtOrBmpLoad == 2)
    {
        tmp = bmpWidth + newPicWd - wd;
        if (resizePic || (minMaxRestore == SIZE_MAXIMIZED) || lastSizeMax)
        {
            //SetBkColor(hdcWinCl, COLOR_WINDOW + 1); // causes flickering in the scrolling
            if (!(retVal = (BOOL)FillRect(hdcScreenCompat, &imgRect, (HBRUSH)(COLOR_WINDOW + 1))))
                ReportErr(L"FillRect: Paint failed!");
            if (!(groupboxFlag && (minMaxRestore == SIZE_MAXIMIZED)))
            {
                if (!(retVal = (BOOL)FillRect(hdcWinCl, &imgRect, (HBRUSH)(COLOR_WINDOW + 1))))
                    ReportErr(L"FillRect: Paint failed!");
            }
            //HBITMAP hBmp = CreateCompatibleBitmap(hdcWinCl, bmpWidth + wd, bmpHeight);
            // Going this way can increase ScrollCall's memory usage considerably

            //if (resizePic == 2) // StretchChk matters little here

            retVal = StretchBlt(hdcScreenCompat, wd, 0, tmp, bmpHeight, hdcScreen, 0, 0, tmp + wd, bmpHeight, SRCCOPY);

            // Incorrect usage of wd & newPicWd makes the following clip the RHS by amount (wd).
            // The same sizing BitBlt further down displayed all of the (bmpWidth - wd) screen image.
            if (retVal)
            {
                if (!(retVal = (BOOL)BitBlt(hdcWinCl, wd - xCurrentScroll, -yCurrentScroll, tmp, bmpHeight, hdcScreenCompat, wd, 0, SRCCOPY)))
                    ReportErr(L"BitBlt from ScreenCompat failed!");
                baseDCBltd = TRUE;
                sizingChangeDir = 0;
               oldWdIn = wd;
          }
            else
                ReportErr(L"StretchBlt from ScreenCompat failed!");

        }
        else
        {
            // Incorrect usage of newPicWd and wd can make the blt'd bitmap
            // on the DC "jump" an unknown x in (wd + x) in WM_PAINT during the size
            // Tested that for a compatible hBmp & HDCMem used as an intermediary for hdcScreenCompat
            if (isSizing)
            {
                if (minMaxRestore == SIZE_RESTORED)
                {
                    if (curFmWd == oldCurFmWd) // reducing width: this avoids the image "jump" to the right
                    {
                        if (baseDCBltd)
                        {
                            if (firstDragAfternewPic)
                            {
                                firstDragAfternewPic = FALSE;
                                oldWdOut = wd;
                                retVal = 1;
                            }
                            else
                                retVal = (BOOL)BitBlt(hdcWinCl, wd - xCurrentScroll, -yCurrentScroll, bmpWidth, bmpHeight, hdcScreenCompat, wd, 0, SRCCOPY);
                        }
                        else // Avoids jitter on the diagonal drag
                            retVal = 1;
                    }
                    else
                    {
                        if (curFmWd < oldCurFmWd)
                        {
                            if (baseDCBltd)
                            {
                                sizingChangeDir = 1;
                                baseDCBltd = FALSE;
                            }
                            if (sizingChangeDir == 2)
                                retVal = (BOOL)BitBlt(hdcWinCl, wd - xCurrentScroll, -yCurrentScroll, bmpWidth, bmpHeight, hdcScreenCompat, newPicWd, 0, SRCCOPY);
                            else
                                retVal = (BOOL)BitBlt(hdcWinCl, wd - xCurrentScroll, -yCurrentScroll, bmpWidth, bmpHeight, hdcScreenCompat, oldWdOut, 0, SRCCOPY);
                        }
                        else
                        {
                            if (baseDCBltd)
                            {
                                sizingChangeDir = 2;
                                baseDCBltd = FALSE;
                            }
                            rectTmp = imgRect;
                            (xCurrentScroll > wd) ? rectTmp.right = 0 : rectTmp.right = wd - xCurrentScroll;
                            if (!(retVal = (BOOL)FillRect(hdcWinCl, &rectTmp, (HBRUSH)(COLOR_WINDOW + 1))))
                                ReportErr(L"FillRect: Paint failed!");
                            if (sizingChangeDir == 1)
                                retVal = (BOOL)BitBlt(hdcWinCl, wd - xCurrentScroll, -yCurrentScroll, bmpWidth, bmpHeight, hdcScreenCompat, oldWdIn, 0, SRCCOPY);
                            else
                                retVal = (BOOL)BitBlt(hdcWinCl, wd - xCurrentScroll, -yCurrentScroll, bmpWidth, bmpHeight, hdcScreenCompat, newPicWd, 0, SRCCOPY);

                            oldWdOut = wd;
                        }
                    }
                    oldCurFmWd = curFmWd;
                }
            }
            else
            {
                if (minMaxRestore == SIZE_RESTORED)
                    retVal = (BOOL)BitBlt(hdcWinCl, wd - xCurrentScroll, -yCurrentScroll, bmpWidth, bmpHeight, hdcScreenCompat, wd, 0, SRCCOPY); //Blt at wd method
                else
                // SIZE_MINIMIZED or not used
                    retVal = StretchBlt(hdcWinCl, wd - xCurrentScroll, -yCurrentScroll, tmp,
                    bmpHeight, hdcScreenCompat, newPicWd, 0, bmpWidth, bmpHeight, SRCCOPY);
            }
        }
    }
    else //Load bitmap
    {
        if (resizePic || (minMaxRestore == SIZE_MAXIMIZED) || lastSizeMax)
        {
            HBITMAP hBmp = 0;
            //SetBkColor(hdcWinCl, COLOR_WINDOW + 1); // causes flickering in the scrolling
            rectTmp = {};
            if (curFmWd > bmpWidth)
                rectTmp.right = curFmWd;
            else
                rectTmp.right = bmpWidth;
            if (curFmHt > bmpHeight)
                rectTmp.bottom = curFmHt;
            else
                rectTmp.bottom = bmpHeight;

            if ((!timPaintDelay && minMaxRestore != END_SIZE_MOVE) && !(retVal = (BOOL)FillRect(hdcWinCl, &rectTmp, (HBRUSH)(COLOR_WINDOW + 1))))
                ReportErr(L"FillRect: Paint failed!");
            hBmp = CreateCompatibleBitmap(hdcWinCl, bmpWidth + wd, bmpHeight);
            retVal = (UINT64)SelectObject(hdcMem, hBmp);
            if (!retVal || (retVal == (BOOL)HGDI_ERROR))
                ReportErr(L"hdcMem: Cannot use bitmap!");

            hBitmapScroll = CreateCompatibleBitmap(hdcWinCl, bmpWidth + wd, bmpHeight);
            hdefBitmapScroll = SelectObject(hdcMemScroll, hBitmapScroll);
            if (!hdefBitmapScroll || (hdefBitmapScroll == HGDI_ERROR))
                ReportErr(L"hdcMemScroll: Cannot use bitmap!");
            if (hBitmap)
            {
                hdefBitmap = SelectObject(hdcMemIn, hBitmap);
                if (!hdefBitmap || (hdefBitmap == HGDI_ERROR))
                    ReportErr(L"hdcMemIn: Cannot use bitmap!");

                retVal = (BOOL)BitBlt(hdcMem, wd, 0, bmpWidth, bmpHeight, hdcMemIn, 0, 0, SRCCOPY); //Blt at wd

                if (retVal)
                {
                    if (resizePic == 2)
                    {
                        retVal = (BOOL)StretchBlt(hdcWinCl, wd - xCurrentScroll, -yCurrentScroll, curFmWd - wd, curFmHt, hdcMem, wd, 0, bmpWidth, bmpHeight, SRCCOPY);
                        retVal = (BOOL)StretchBlt(hdcMemScroll, wd, 0, bmpWidth, bmpHeight, hdcMem, wd, 0, bmpWidth, bmpHeight, SRCCOPY);
                    }
                    else
                    {
                        retVal = (BOOL)BitBlt(hdcWinCl, wd - xCurrentScroll, -yCurrentScroll, bmpWidth, bmpHeight, hdcMem, wd, 0, SRCCOPY); //Blt at wd method
                        retVal = (BOOL)BitBlt(hdcMemScroll, wd, 0, bmpWidth, bmpHeight, hdcMem, wd, 0, SRCCOPY); //Blt at wd method
                    }
                    if (!retVal)
                        ReportErr(L"Blt to client failed!");

                    rectTmp = imgRect;
                    (xCurrentScroll > wd) ? rectTmp.right = 0 : rectTmp.right = wd - xCurrentScroll;
                    retVal = (BOOL)FillRect(hdcWinCl, &rectTmp, (HBRUSH)(COLOR_WINDOW + 1));
                    newPicWd = wd;
                    //retVal = (BOOL)FillRect(hdcMemScroll, &rectTmp, (HBRUSH)(COLOR_WINDOW + 1));
                }
            }
            else
                ReportErr(L"hBitmap: Not valid!");

            if (hBmp && !DeleteObject(hBmp))
                ReportErr(L"AdjustImage: Cannot delete bitmap object!!");

        }
        else
        {
            // Compare this with the section for the Screenshot:
            // oldWd & oldCurFmWd not required
            if (minMaxRestore == SIZE_RESTORED)
            {
                // The groupbox only covers some of the blank space during the sizing.
                if (xCurrentScroll <= wd)
                {
                    rectTmp = imgRect;
                    rectTmp.right = wd - xCurrentScroll;
                    if (!(retVal = (BOOL)FillRect(hdcWinCl, &rectTmp, (HBRUSH)(COLOR_WINDOW + 1))))
                        ReportErr(L"FillRect: Paint failed!");
                }
                retVal = (BOOL)BitBlt(hdcWinCl, wd - xCurrentScroll, -yCurrentScroll, bmpWidth + newPicWd, bmpHeight, hdcMem, newPicWd, 0, SRCCOPY);
            }
            else // SIZE_MINIMIZED or not used
                retVal = StretchBlt(hdcWinCl, wd - xCurrentScroll, -yCurrentScroll, bmpWidth,
                    bmpHeight, hdcMem, newPicWd, 0, bmpWidth, bmpHeight, SRCCOPY);
        }

    }

    return retVal;
}
void PaintScrolledorPrinted(PAINTSTRUCT *ps, HDC hdcWinCl, HDC hdcMem, int xNewSize, int yNewSize, UINT bmpWidth, UINT bmpHeight, BOOL printCl, BOOL noFill)
{
    static int oldWd = wd, startWd = 0;
    if (ps)
    {
        rectTmp = ps->rcPaint;
        if (scrShtOrBmpLoad == 1)
        {
            if (!BitBlt(ps->hdc,
                //prect->left + (isScreenshot ? (fScroll == 1 ? 0 : wd) : (((fScroll == 1) && (xCurrentScroll > wd)) ? 0 : wd)),
                wd,
                rectTmp.top,
                xNewSize,
                (rectTmp.bottom - rectTmp.top),
                hdcMem,
                oldWd + xCurrentScroll,
                yCurrentScroll,
                SRCCOPY))
                ReportErr(L"Bad BitBlt from hdcMem!");
        }
        else
        {
            if (!BitBlt(ps->hdc,
                //prect->left + (isScreenshot ? (fScroll == 1 ? 0 : wd) : (((fScroll == 1) && (xCurrentScroll > wd)) ? 0 : wd)),
                rectTmp.left,
                rectTmp.top,
                (rectTmp.right - rectTmp.left),
                (rectTmp.bottom - rectTmp.top),
                hdcMem,
                rectTmp.left + xCurrentScroll,
                rectTmp.top + yCurrentScroll,
                SRCCOPY))
                ReportErr(L"Bad BitBlt from hdcMem!");
        }
        if (!groupboxFlag && !printCl)
        {
            // Paint sections
            // consider test like RectCl().width(1) - prect->left > wd
            if (!rectTmp.left)
            {
                rectTmp.right = wd - ((scrShtOrBmpLoad == 1) ? 0: xCurrentScroll);
                if (!(tmp = (BOOL)FillRect(ps->hdc, &rectTmp, (HBRUSH)(COLOR_WINDOW + 1))))
                    ReportErr(L"FillRect for PaintScrolledorPrinted failed!");
            }
            //UpdateWindow(hWnd);
        }
    }
    else
    { // Only printCl here
        rectTmp.top = 0;

        if (groupboxFlag)
        {
            rectTmp.top = -yCurrentScroll;
            rectTmp.left = wd;
            
            // tmp is set so nothing is painted "behind" the groupbox.
            // The groupbox is not transparent, and the parent does not
            // have the WS_CLIPCHILDREN style, so the groupbox,
            // along with its sibling controls are readily painted over.

            // new image
            if (timTracker == IDT_TIMER_LARGE)
            {
                tmp = 0;
                oldWd = wd;
                timTracker = 0;
            }
            else
            {
                if (!isSizing)
                    startWd = wd;
                tmp = xCurrentScroll;
            }
        }
        else
        {
            if (!noFill)
            {
                // Client co-ords for HDC
                rectTmp.left = 0;
                rectTmp.right = xNewSize;
                rectTmp.bottom = yNewSize;
                if (!(tmp = (BOOL)FillRect(hdcWinCl, &rectTmp, (HBRUSH)(COLOR_WINDOW + 1))))
                    ReportErr(L"FillRect for PaintScrolledorPrinted failed!");
            }

            if (timTracker == IDT_TIMER_LARGE)
            {
                oldWd = wd;
                tmp = 0;
                rectTmp.left = wd;
                timTracker = 0;
            }
            else
            {
                tmp = xCurrentScroll;
                if (isSizing)
                {
                    rectTmp.top = -yCurrentScroll;
                    rectTmp.left = wd;
                }
                else
                {
                    startWd = wd;
                    rectTmp.left = wd;
                }
            }
        }

        if (!BitBlt(hdcWinCl,
            //prect->left + (isScreenshot ? (fScroll == 1 ? 0 : wd) : (((fScroll == 1) && (xCurrentScroll > wd)) ? 0 : wd)),
            rectTmp.left,
            rectTmp.top,
            bmpWidth,
            bmpHeight,
            hdcMem,
            tmp,
            0,
            SRCCOPY))
            ReportErr(L"Bad BitBlt from hdcMem!");

        if (!xCurrentScroll && isSizing)
        {
            // Prevents artifacts forming in the "growth" zone
            if (wd > startWd)
            {
                rectTmp.left = startWd;
                rectTmp.right = wd;
                if (!(tmp = (BOOL)FillRect(hdcWinCl, &rectTmp, (HBRUSH)(COLOR_WINDOW + 1))))
                    ReportErr(L"FillRect for PaintScrolledorPrinted failed!");
            }
            else
                startWd = 0;
        }
    }

}
void GetDims(HWND hWnd, int resizeType, int oldResizeType)
{
    // int for these will not compute
    static float firstWd = 0, firstHt = 0, wdBefMax = 0, htBefMax = 0, savedWd = 0, savedHt = 0, savedScaleX = 1, savedScaleY = 1, oldWd = 0, oldHt = 0;
    static BOOL firstSizeAfterSTART_SIZE_MOVE = FALSE;
    int xNewSize, yNewSize;


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
                InitWindowDims(hWnd, 3, xNewSize, yNewSize);
                wd = (int)(xNewSize) / CTRL_PROPORTION_OF_FORM;
                ht = (int)(yNewSize) / CTRL_PROPORTION_OF_FORM;
            }
        }


    }
    else
    {
        //Init class: Must be done twice because of default window positions before WM_SIZE.
        rectTmp = RectCl().RectCl(0, hWnd, 0);
        if (firstWd)
        {
            firstWd = oldWd = wd = RectCl().width(0);
            firstHt = oldHt = ht = RectCl().height(0);
        }
        else
        {
            firstWd = wd = RectCl().width(0);
            firstHt = ht = RectCl().height(0);
            if (rectTmp.left < GetSystemMetrics(0) && rectTmp.bottom < GetSystemMetrics(1))
                MoveWindow(hWnd, GetSystemMetrics(0) / 4, GetSystemMetrics(1) / 4, GetSystemMetrics(0) / 2, GetSystemMetrics(1) / 2, TRUE);
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
                if (oldResizeType != SIZE_MINIMIZED)
                {
                    oldWd ? (scaleX = wd / oldWd) : scaleX = 1;
                    oldHt ? (scaleY = ht / oldHt) : scaleY = 1;
                }
                // else use saved scale
            }
        }

    }
    break;
    case START_SIZE_MOVE:
    {
        //excludes case of oldResizeType = START_SIZE_MOVE on a border click
        if (oldResizeType != START_SIZE_MOVE)
        {
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
        scaleX = wd / firstWd;
        scaleY = ht / firstHt;
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

void SizeControls(int bmpHeight, HWND hWnd, int &yOldScroll, int resizeType, int curFmWd, int curFmHt, BOOL newPic)
{
    // Sizing controls, and thier vertical separation.
    int opt1Ht = 0, opt2Ht = 0, chkHt = 0, btnWd = 0, btnHt = 0;
    int newHt = 0, newWd = 0, newCtrlSizeTriggerWd = 0, newCtrlSizeTriggerHt = 0;


    static UINT defFmWd = 0, defFmHt = 0;

    static int startFmWd = 0, startFmHt = 0, oldFmWd = 0, oldFmHt = 0;
    static int ctrlSizeTriggerWd = 100, ctrlSizeTriggerHt = 100;
    static int defOpt1Top = 0, defOpt2Top = 0, defChkTop = 0, defLblTop = 0, defUpDnTop = 0;
    static int oldOpt1Top = 0, oldOpt2Top = 0, oldChkTop = 0, oldLblTop = 0, oldUpDnTop = 0;
    static int oldResizeType = 0, xScrollBefSizing = 0, yScrollBefSizing = 0;
    static int minWd = 0, minHt = 0, startSizeBtnLeft = 0, startSizeBtnTop = 0, startSizeBtnRight = 0, startSizeBtnBottom = 0, startSizeOpt1Top = 0, startSizeOpt2Top = 0, startSizeChkTop = 0, startSizeLblTop = 0, startSizeUpDnTop = 0;
    if (isLoading)
    {
        rectTmp = RectCl().RectCl(0, hWnd, 1); //required for wd, ht, in GetDims below
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
    RECT rectBtn = {}, rectOpt1 = {}, rectOpt2 = {}, rectChk = {}, rectLbl = {}, rectUpDn = {};

    GetDims(hWnd, resizeType, oldResizeType);

    if (capCallFrmResize && oldFmHt && (curFmHt - startFmHt != 0) && (curFmWd - startFmWd != 0))
    {
        // Should never get here
        procEndWMSIZE = TRUE;
        return;
    }

    rectBtn = RectCl().RectCl(hWndButton, hWnd, 2);
    rectOpt1 = RectCl().RectCl(hWndOpt1, hWnd, 3);
    rectOpt2 = RectCl().RectCl(hWndOpt2, hWnd, 4);
    rectChk = RectCl().RectCl(hWndChk, hWnd, 5);
    rectLbl = RectCl().RectCl(hLblUpDown, hWnd, 6);
    rectUpDn = RectCl().RectCl(hwndUpDnCtl, hWnd, 7);


    if ((ht > OPT_HEIGHT) && !defOpt1Top) // At Init
    {
        // Determines relative co-ords of controls
        defOpt1Top = ht + OPT_HEIGHT;
        defOpt2Top = 2 * ht;
        defChkTop = 3 * ht;
        defLblTop = 4 * ht;
        defUpDnTop = 4.6 * ht;
        defFmWd = RectCl().width(1);
        defFmHt = RectCl().height(1);
    }

    if (resizeType == END_SIZE_MOVE)
    {
        // Can change even after the last size
        xCurrentScroll = ScrollInfo(hWnd, UPDATE_HORZSCROLLSIZE_CONTROL, 0, 0, curFmWd, curFmHt);
        yCurrentScroll = ScrollInfo(hWnd, UPDATE_VERTSCROLLSIZE_CONTROL, 0, 0, curFmWd, curFmHt);
        if (scrShtOrBmpLoad == 1) // Buttons not moved
            rectBtn.left = rectOpt1.left = rectOpt2.left = rectChk.left = rectLbl.left = rectUpDn.left = 0;
        else
        {
            rectBtn.left = startSizeBtnLeft + xScrollBefSizing - xCurrentScroll;
            rectOpt1.left = startSizeBtnLeft + xScrollBefSizing - xCurrentScroll;
            rectOpt2.left = startSizeBtnLeft + xScrollBefSizing - xCurrentScroll;
            rectChk.left = startSizeBtnLeft + xScrollBefSizing - xCurrentScroll;
            rectLbl.left = startSizeBtnLeft + xScrollBefSizing - xCurrentScroll;
            rectUpDn.left = startSizeBtnLeft + xScrollBefSizing - xCurrentScroll;
        }
        btnHt = startSizeBtnBottom - startSizeBtnTop;
        btnWd = startSizeBtnRight - startSizeBtnLeft;


        if (startFmHt == defFmHt && curFmHt < startFmHt)
            curFmHt = startFmHt;
        if (startFmHt != curFmHt)
        {
            rectBtn.top = startSizeBtnTop + yScrollBefSizing - yCurrentScroll;
            // Sanity check for button, other controls dealt with in delegateSizeControl
            if (rectBtn.top < -yCurrentScroll)
                rectBtn.top = -yCurrentScroll;

            rectOpt1.top = ((float)curFmHt / (float)startFmHt) * (startSizeOpt1Top + yScrollBefSizing) - yCurrentScroll;
            rectOpt2.top = ((float)curFmHt / (float)startFmHt) * (startSizeOpt2Top + yScrollBefSizing) - yCurrentScroll;
            rectChk.top = ((float)curFmHt / (float)startFmHt) * (startSizeChkTop + yScrollBefSizing) - yCurrentScroll;
            rectLbl.top = ((float)curFmHt / (float)startFmHt) * (startSizeLblTop + yScrollBefSizing) - yCurrentScroll;
            rectUpDn.top = ((float)curFmHt / (float)startFmHt) * (startSizeUpDnTop + yScrollBefSizing) - yCurrentScroll;
        }

    }
    else
    {
        btnWd = rectBtn.right - rectBtn.left;
        btnHt = rectBtn.bottom - rectBtn.top;

        if (resizeType == START_SIZE_MOVE)
        {
            //snapshot everything and leave
            if (!startSizeOpt1Top) //initialise once
            {
                oldOpt1Top = rectOpt1.top;
                oldOpt2Top = rectOpt2.top;
                oldChkTop = rectChk.top;
                oldLblTop = rectLbl.top;
                oldUpDnTop = rectUpDn.top;
            }

            startSizeBtnTop = -yCurrentScroll;

            startSizeBtnBottom = startSizeBtnTop + (rectBtn.bottom - rectBtn.top);
            startSizeOpt1Top = ((float)curFmHt / (float)defFmHt) * defOpt1Top - yCurrentScroll;
            // If height of form was small, controls will not move
            // and may clip others, as the relevant condition in 
            // delegateSizeControl (called in the previous size)
            // limits minimal form height. Thus the following:
            if (startSizeOpt1Top < defOpt1Top - yCurrentScroll)
                startSizeOpt1Top = defOpt1Top - yCurrentScroll;
            startSizeOpt2Top = ((float)curFmHt / (float)defFmHt) * defOpt2Top - yCurrentScroll;
            if (startSizeOpt2Top < defOpt2Top - yCurrentScroll)
                startSizeOpt2Top = defOpt2Top - yCurrentScroll;
            startSizeChkTop = ((float)curFmHt / (float)defFmHt) * defChkTop - yCurrentScroll;
            if (startSizeChkTop < defChkTop - yCurrentScroll)
                startSizeChkTop = defChkTop - yCurrentScroll;
            startSizeLblTop = ((float)curFmHt / (float)defFmHt) * defLblTop - yCurrentScroll;
            if (startSizeLblTop < defLblTop - yCurrentScroll)
                startSizeLblTop = defLblTop - yCurrentScroll;
            startSizeUpDnTop = ((float)curFmHt / (float)defFmHt) * defUpDnTop - yCurrentScroll;
            if (startSizeUpDnTop < defUpDnTop - yCurrentScroll)
                startSizeUpDnTop = defUpDnTop - yCurrentScroll;
            // buddy position adjusted by system

            if (scrShtOrBmpLoad == 1)  // Buttons not moved
            {
                startSizeBtnLeft = rectBtn.left = xCurrentScroll;
                startSizeBtnRight = xCurrentScroll + wd;
            }
            else
            {
                startSizeBtnLeft = rectBtn.left;
                startSizeBtnRight = rectBtn.right;
            }
            


            // Establish absolute minimum size of controls
            (curFmWd < defFmWd) ? startFmWd = defFmWd : startFmWd = curFmWd;
            (curFmHt < defFmHt) ? startFmHt = defFmHt : startFmHt = curFmHt;
            xScrollBefSizing = xCurrentScroll;
            yScrollBefSizing = yCurrentScroll;


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
        //Extra edging for the wd - (2 * SM_CXEDGE)
        //newEdgeWd = scaleX * (btnWd - 2 * SM_CXEDGE);
        //newEdgeHt = scaleY * (btnHt - 2 * SM_CXEDGE);

        if (newWd < minWd)
            newWd = minWd;
        else
        {
            if (curFmWd < defFmWd)
                newWd = minWd;
        }

        if (newHt < minHt)
            newHt = minHt;
        else
        {
            if (curFmHt < defFmHt)
                newHt = minHt;
        }



        if (resizeType == END_SIZE_MOVE)
            // SWP_NOACTIVATE may cause occasional "paint fail" with this button and the UpDown controls 
            SetWindowPos(hWndButton, NULL, rectBtn.left, rectBtn.top, newWd, newHt, SWP_SHOWWINDOW);
        else
        {

            if (newPic)
            {
                if (rectBtn.top < -yCurrentScroll)
                    rectBtn.top = -yCurrentScroll;
            }
            else
            {
                xCurrentScroll = ScrollInfo(hWnd, UPDATE_HORZSCROLLSIZE_CONTROL, 0, 0, curFmWd, curFmHt);
                yCurrentScroll = ScrollInfo(hWnd, UPDATE_VERTSCROLLSIZE_CONTROL, 0, 0, curFmWd, curFmHt);
            }


            // If maximised, scroll value may change- left/top of control may move
            // below zero so its right side wants to be at wd, and bottom at ht.
            if (resizeType == SIZE_MAXIMIZED)
            {
                SetWindowPos(hWndButton, NULL, -xCurrentScroll, -yCurrentScroll,
                    newWd, newHt, SWP_NOSENDCHANGING);
            }
            else
            {
                if (oldResizeType == SIZE_MAXIMIZED)
                    SetWindowPos(hWndButton, NULL, rectBtn.left, -yCurrentScroll,
                        newWd, newHt, SWP_NOSENDCHANGING);
                else
                {
                    if (scrShtOrBmpLoad == 1)
                    SetWindowPos(hWndButton, NULL, 0, 0,
                        newWd, newHt, SWP_NOSENDCHANGING);
                    else
                    SetWindowPos(hWndButton, NULL, -xCurrentScroll, -yCurrentScroll,
                        newWd, newHt, SWP_NOSENDCHANGING);

                    if (!isLoading && isSizing)
                    {
                        // Useless linear method of propagating size increments
                        // A better method would consider dx/dt of drag speed. 
                        (oldFmWd == curFmWd) ? (newCtrlSizeTriggerWd = 0) : ((oldFmWd < curFmWd) ? newCtrlSizeTriggerWd = 1 : newCtrlSizeTriggerWd = -1);
                        (oldFmHt == curFmHt) ? (newCtrlSizeTriggerHt = 0) : ((oldFmHt < curFmHt) ? newCtrlSizeTriggerHt = 1 : newCtrlSizeTriggerHt = -1);
                        if (newCtrlSizeTriggerWd == ctrlSizeTriggerWd)
                            newCtrlSizeTriggerWd = 0;
                        else
                        {
                            if (newCtrlSizeTriggerWd) //reverse direction
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


        // Possible for following calls to be condensed viz the hWnds in ctrlArray.
        if (resizeType != SIZE_MINIMIZED)
        {

            oldOpt1Top = delegateSizeControl(rectOpt1, hWndOpt1, oldOpt1Top, resizeType, oldResizeType, defOpt1Top, yOldScroll, newCtrlSizeTriggerHt, newWd, newHt, minHt, newPic);

            // For debug
            //_RPTF4(_CRT_WARN, "yCurrentScroll = %d,  newCtrlSizeTriggerHt = %d,  rectOpt1.top= %d, oldOpt1Top= %d\n", yCurrentScroll, newCtrlSizeTriggerHt, rectOpt1.top, oldOpt1Top);
            // No full path :https://stackoverflow.com/questions/8487986/file-macro-shows-full-path/54335644#54335644

            oldOpt2Top = delegateSizeControl(rectOpt2, hWndOpt2, oldOpt2Top, resizeType, oldResizeType, defOpt2Top, yOldScroll, newCtrlSizeTriggerHt, newWd, newHt, minHt, newPic);
            oldChkTop = delegateSizeControl(rectChk, hWndChk, oldChkTop, resizeType, oldResizeType, defChkTop, yOldScroll, newCtrlSizeTriggerHt, newWd, newHt, minHt, newPic);
            oldLblTop = delegateSizeControl(rectLbl, hLblUpDown, oldLblTop, resizeType, oldResizeType, defLblTop, yOldScroll, newCtrlSizeTriggerHt, newWd, newHt, minHt, newPic);
            ScaleFont(hLblUpDown, floor(3 * newHt / 7));
            // Must set the buddy pos here as GetWindowRect(hwndUpDnCtl, &rectUpDn) in a WM_SIZE is not reliable
            oldUpDnTop = delegateSizeControl(rectUpDn, hwndUpDnCtl, oldUpDnTop, resizeType, oldResizeType, defUpDnTop, yOldScroll, newCtrlSizeTriggerHt, newWd / 2, newHt, minHt, newPic, hwndUpDnEdtBdy);
            ScaleFont(hwndUpDnEdtBdy, newHt, TRUE);
        }

        if (groupboxFlag)
        {
            // Too much flicker due to the repaint in sizing
            SetWindowPos(hWndGroupBox, NULL, -xCurrentScroll, -yCurrentScroll,
                newWd, max(curFmHt, bmpHeight), SWP_NOSENDCHANGING); //  SWP_SHOWWINDOW
        }
    }
    else // Following redundant on !isLoading as btnWd is always > minWd
    {
        if (btnWd < minWd)
        {
            if (resizeType == END_SIZE_MOVE)
                SetWindowPos(hWndButton, NULL, rectBtn.left, rectBtn.top, minWd, rectBtn.bottom - rectBtn.top, SWP_NOSENDCHANGING);
            else
                SetWindowPos(hWndButton, NULL, scaleX * rectBtn.left + xCurrentScroll, scaleY * rectBtn.top + yCurrentScroll,
                    minWd, rectBtn.bottom - rectBtn.top, SWP_NOSENDCHANGING);
        }
        if (btnHt < minHt)
        {
            if (resizeType == END_SIZE_MOVE)
                SetWindowPos(hWndButton, NULL, rectBtn.left, rectBtn.top, rectBtn.right - rectBtn.left, minHt, SWP_NOSENDCHANGING);
            else
                SetWindowPos(hWndButton, NULL, scaleX * rectBtn.left + xCurrentScroll, scaleY * rectBtn.top + yCurrentScroll,
                    rectBtn.right - rectBtn.left, minHt, SWP_NOSENDCHANGING);
        }
    }

    if (resizeType == SIZE_RESTORED && !isLoading)
    {
        oldFmWd = curFmWd;
        oldFmHt = curFmHt;
    }


    yOldScroll = yCurrentScroll;
    oldResizeType = resizeType;
    procEndWMSIZE = TRUE;
}
int delegateSizeControl(RECT rectOpt, HWND hWndOpt, int oldOptTop, int resizeType, int oldResizeType, int defOptTop, int yScrollBefNew, int newCtrlSizeTriggerHt, int newWd, int newHt, int minHt, BOOL newPic, HWND buddyHWnd)
{
    int optHt = newHt;
    optHt = (buddyHWnd) ? optHt : optHt / 2;
    static int yOldScroll = 0, ctrlCt = 0;
    static int restoreArray[6] = { 0 };
    if (newPic)
    {
        rectOpt.top -= yCurrentScroll;
    }
    else
    {
        if (scrShtOrBmpLoad == 1)
        {
            if (resizeType == SIZE_RESTORED)
            {
                if (oldOptTop && (oldResizeType != SIZE_MINIMIZED))
                {
                    if (oldResizeType == SIZE_MAXIMIZED)
                        rectOpt.top = restoreArray[ctrlCt] - yCurrentScroll;
                    else
                    rectOpt.top += scaleY * (rectOpt.top - oldOptTop + newCtrlSizeTriggerHt);
                }
            }
            else
            {
                if (resizeType != END_SIZE_MOVE)
                {
                    if (oldResizeType != SIZE_MAXIMIZED)
                        restoreArray[ctrlCt] = rectOpt.top;

                    rectOpt.top = scaleY * rectOpt.top;
                    oldOptTop = rectOpt.top;
                }
            }
        }
        else
        {
            if (resizeType == SIZE_RESTORED)
            {
                // Must be adjusted with scroll offsets
                if (oldOptTop && (oldResizeType != SIZE_MINIMIZED))
                {
                    if (oldResizeType == SIZE_MAXIMIZED)
                        rectOpt.top = restoreArray[ctrlCt] - yCurrentScroll;
                    else
                    {
                        if (oldResizeType != START_SIZE_MOVE) // sizing
                            rectOpt.top += scaleY * (rectOpt.top - oldOptTop + newCtrlSizeTriggerHt);

                        // Borderland
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
                    if (resizeType == SIZE_MAXIMIZED)
                    {
                        // restoreArray will store previous SIZE_RESTORED dims.
                        // SIZE_MINIMIZED not stored so oldResizeType can be SIZE_MAXIMIZED successively

                        if (oldResizeType == SIZE_MAXIMIZED)
                            oldOptTop += yOldScroll;
                        else
                        {
                            restoreArray[ctrlCt] = rectOpt.top + yScrollBefNew;
                            oldOptTop = rectOpt.top;
                        }
                        yOldScroll = yScrollBefNew;
                    }

                    //rectOpt.top = scaleY * (rectOpt.top - oldOptTop + yCurrentScroll);
                    //The following may not preserve vertical scale for rectOpt.top after SIZE_MAXIMIZED, so want update
                    rectOpt.top = scaleY * (rectOpt.top + yOldScroll) - yCurrentScroll;
                }
            }
        }
    }

    if (optHt < minHt / 2)
        optHt = minHt / 2;

    if (rectOpt.top < defOptTop - yCurrentScroll)
        rectOpt.top = defOptTop - yCurrentScroll;


    if (resizeType == END_SIZE_MOVE)
    {
        // using parms like newEdgeWd (see above) can invalidate the entire window in certain circumstances
        if (!SetWindowPos(hWndOpt, NULL, rectOpt.left, rectOpt.top, newWd, optHt, NULL))
            ReportErr(L"SetWindowPos failed!");
        if (buddyHWnd)
            SetWindowPos(buddyHWnd, NULL, rectOpt.left + newWd, rectOpt.top, newWd, optHt, NULL);
    }
    else
    {
        if (scrShtOrBmpLoad == 1)
        {
            SetWindowPos(hWndOpt, NULL, 0, rectOpt.top, newWd, optHt, SWP_NOSENDCHANGING);
            if (buddyHWnd)
                SetWindowPos(buddyHWnd, NULL, newWd, rectOpt.top, newWd, optHt, SWP_NOSENDCHANGING);
        }
        else
        {
            if (resizeType == SIZE_MAXIMIZED)
            {
                SetWindowPos(hWndOpt, NULL, -xCurrentScroll, rectOpt.top, newWd, optHt, SWP_NOSENDCHANGING);
                if (buddyHWnd)
                    SetWindowPos(buddyHWnd, NULL, -xCurrentScroll + newWd, rectOpt.top, newWd, optHt, SWP_NOSENDCHANGING);
            }
            else
            {
                if (oldResizeType == SIZE_MAXIMIZED)
                {
                    SetWindowPos(hWndOpt, NULL, rectOpt.left, rectOpt.top, newWd, optHt, SWP_NOSENDCHANGING);
                    if (buddyHWnd)
                        SetWindowPos(buddyHWnd, NULL, rectOpt.left + newWd, rectOpt.top, newWd, optHt, SWP_NOSENDCHANGING);
                }
                else
                {
                    SetWindowPos(hWndOpt, NULL, -xCurrentScroll, rectOpt.top, newWd, optHt, SWP_NOSENDCHANGING);
                    if (buddyHWnd)
                        SetWindowPos(buddyHWnd, NULL, -xCurrentScroll + newWd, rectOpt.top, newWd, optHt, SWP_NOSENDCHANGING);
                }
            }
        }
    }
    if (buddyHWnd)
        ctrlCt = 0;
    else
        ctrlCt += 1;
    if (resizeType == SIZE_RESTORED || (resizeType == END_SIZE_MOVE && (oldResizeType != START_SIZE_MOVE))) // curious case of oldResizeType as  START_SIZE_MOVE on a border click
        return rectOpt.top;
    else
        return oldOptTop;
}

void ScaleFont(HWND hWnd, int contSize, BOOL isUpDown)
{
    static int sizeForLabel = 0;
    static int sizeForUpDown = 0;
    static HFONT	hFont;
    static LOGFONT	lf;

    // Note default WM_PAINT for m_hWnd paints the control with default parms,
    // so for consistent results, this function should be called from a SubClass proc.
    // Also consider DrawText for centering- at extra processing cost

    if (sizeForUpDown)
    {
        lf.lfHeight = floor(4 * contSize / 7);
        HFONT hFontNew = CreateFontIndirectW(&lf);
        if (hFontNew)
            DeleteObject(hFont);
        hFont = hFontNew;
        (HFONT)SendMessageW(hWnd, WM_SETFONT, (WPARAM)hFont, TRUE);
    }
    else
    {
        hFont = (HFONT)SendMessageW(hWnd, WM_GETFONT, 0, 0);
        if (NULL == hFont)
            hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT); // SYSTEM_FONT not friendly
        GetObject(hFont, sizeof(LOGFONT), &lf);

        // lfHeight  is negative for DEFAULT_GUI_FONT
        //HDC hdcScreen = CreateDC(L"DISPLAY", (PCTSTR)NULL, (PCTSTR)NULL, (CONST DEVMODE*) NULL);
        //if (lf.lfHeight < 0) fontSize = MulDiv(-lf.lfHeight, 72, GetDeviceCaps(hdcScreen, LOGPIXELSY));

        if (isUpDown)
            sizeForUpDown = lf.lfHeight;
        else
            sizeForLabel = lf.lfHeight;
    }
}

int ScrollIt(HWND hWnd, int scrollType, int scrollDrag, int currentScroll, int minScroll, int maxScroll, int trackPos)
{

    int newPos;    // new position 

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
int ScrollInfo(HWND hWnd, int scrollXorY, int scrollType, int scrollDrag, int xNewSize, int yNewSize, int bmpWidth, int bmpHeight, BOOL newPic)
{

    int newPos, retVal = 0;
    BOOL setWindowThemeRunFlag = FALSE;


    static int oldWd = 0;
    static int xOldSize = xNewSize;
    static int yOldSize = 0;
    static int scrollStat = 0;
    static int oldScrollStat;

    // These variables are required for horizontal scrolling.
    static const int xMinScroll = 0;      // minimum horizontal scroll value (starts at 0 so rarely adjusted)
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
        if (isLoading || (scrollStat == 1 || scrollStat == 3))
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
        if (isLoading || (scrollStat > 1))
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
        }
        else
        {
            if (!scrollXorY)
            {
                if (newPic)
                    oldWd = wd;
                if (scrShtOrBmpLoad == 2)
                    bmpWidth = bmpWidth + oldWd - wd;
                if (!isLoading && (bmpWidth + wd > xNewSize))
                {
                    if ((scrShtOrBmpLoad == 2) || newPic || (xOldSize != xNewSize)) // else the HORZ scroll is the same
                    {
                        xMaxScroll = max(bmpWidth + wd - xNewSize, 0);
                        xCurrentScroll = min(xCurrentScroll, xMaxScroll);
                        siHORZ.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
                        siHORZ.nMin = xMinScroll;
                        siHORZ.nMax =bmpWidth + wd;
                        siHORZ.nPage = xNewSize;
                        //consider SM_CXVSCROLL=20. The height of the arrow bitmap on a vertical scroll bar,
                        //siHORZ.nMax = bmpWidth + ((bmpHeight > defFmHt)? GetSystemMetrics( SM_CXVSCROLL) : 0);
                         // Not dealing with xTrackPos.
                        SetScrollInfo(hWnd, SB_HORZ, &siHORZ, TRUE);
                        ShowScrollBar(hWnd, SB_HORZ, TRUE);
                        if (scrollStat < 2)
                            scrollStat = 1;
                        else
                            scrollStat = 3;
                        if (scrollStat != oldScrollStat)
                            scrollChanged = TRUE;
                        else
                            scrollChanged = FALSE;
                    }
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
                        // also removes the painted bitmap (or the low right  section of)
                        //  on SIZE_MAXIMIZED thus a timer is required to repaint it.
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
                xOldSize = xNewSize;
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
        }
        else
        {
            if (!scrollXorY)
            {
                if (!isLoading && (bmpHeight > yNewSize))
                {
                    if ((scrShtOrBmpLoad == 2) || newPic || (yOldSize != yNewSize))
                    {
                        yMaxScroll = max(bmpHeight - yNewSize, 0);
                        yCurrentScroll = min(yCurrentScroll, yMaxScroll);
                        siVERT.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
                        siVERT.nMin = yMinScroll;
                        siVERT.nMax = bmpHeight;
                        //consider SM_CXHSCROLL=21. The width of the arrow bitmap on a horizontal scroll bar,
                        //siVERT.nMax = bmpHeight + ((bmpWidth > defFmWd)? GetSystemMetrics(SM_CXHSCROLL) : 0);
                        siVERT.nPage = yNewSize;
                        siVERT.nPos = yCurrentScroll;
                        siVERT.nTrackPos = yTrackPos;
                        SetScrollInfo(hWnd, SB_VERT, &siVERT, TRUE);
                        ShowScrollBar(hWnd, SB_VERT, TRUE);
                        if ((scrollStat != 3))
                            (scrollStat == 1) ? (scrollStat = 3) : (scrollStat = 2);
                        if (scrollStat != oldScrollStat)
                            scrollChanged = TRUE;
                    }
                }
                else
                {
                    if (isLoading)
                        siVERT.cbSize = 0;
                    siVERT = { 0 };
                    yCurrentScroll = 0;
                    if (scrollStat > 1)
                    {
                        scrollChanged = TRUE;
                        if (!setWindowThemeRunFlag && IsThemeActive())
                            SetWindowTheme(hWnd, NULL, _T("Scrollbar"));

                        if (!ShowScrollBar(hWnd, SB_VERT, FALSE))
                            ReportErr(L"SB_VERT: ShowScrollBar failed!");
                        (scrollStat == 3) ? scrollStat = 1 : scrollStat = 0;
                        if (!scrollStat) // the other scrollbar re-appears!
                            ShowScrollBar(hWnd, SB_HORZ, FALSE);
                    }

                    //RedrawWindow(hWnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);
                }
                yOldSize = yNewSize;
                oldScrollStat = scrollStat;
            }
        }
        retVal = scrollStat;
        // The proposed addition to the next condition:
        // || (scrollStat == 3) || ((scrollStat == 1) && (bmpWidth + wd > xNewSize)) || ((scrollStat == 2) && (bmpHeight + ht > yNewSize))))
        // is to address a later window invalidation after sizing (rare) or a black rectangle
        // on LHS of bitmap during sizing when either a HORZ or VERT system scrollbar
        // is visible. In the latter, the image seen to automatically stretch to accomodate
        // the rectangle so that its RH client position is the same. Doesn't help. The cause
        // of the glitch is possibly related to stacked WindowPos calls before WM_PAINT.
        // Although the effects of not calling this function during sizing can differ, there is no improvementt.
        if (!timPaintBitmap && scrollChanged)
        {
            if (timPaintBitmap = (int)SetTimer(hWnd,
                IDT_PAINTBITMAP,
                IDT_TIMER_SMALL,
                (TIMERPROC)NULL))
                timTracker = IDT_TIMER_SMALL;
            else
                ReportErr(L"No timer is available.");
        }
    }
    break;
    }
    procEndWMSIZE = TRUE;
    return retVal;


}
BOOL Kleenup(HWND hWnd, HBITMAP& hBitmap, HBITMAP& hBitmapScroll, HGDIOBJ hdefBitmap, HGDIOBJ hdefBitmapScroll, HBITMAP& hbmpCompat, GpBitmap*& pgpbm, HDC& hdcMem, HDC& hdcMemIn, HDC& hdcMemScroll, HDC& hdcWinCl, int typeOfDC, BOOL noExit)
{
    // typeOfDC:- 1: PrintWindow, 2: Screenshot, 3: Image

    static BOOL exitOnceFlag = FALSE;
    if (exitOnceFlag)
        return TRUE;

    if (hdcWinCl && !ReleaseDC(hWnd, hdcWinCl) && noExit)
        ReportErr(L"hdcWinCl: Not released!");

    if (typeOfDC == 3)
    {
        if (pgpbm)
            GpStatus gps = GdipDisposeImage(pgpbm); //No return value
        if (hBitmap)
        {
            if (hdcMemIn)
            {
              SelectObject(hdcMemIn, hdefBitmap);
            if (hBitmap && !DeleteObject(hBitmap) && noExit)
                ReportErr(L"hBitmap: Cannot delete bitmap object!");
            if (!DeleteDC(hdcMemIn) && noExit)
                ReportErr(L"hdcMemIn: DeleteDC failed!");
            }
        }
    }
    if (typeOfDC != 2)
    {
        if (hdcMem)
        {
            if (typeOfDC == 1)
            {
                SelectObject(hdcMem, hdefBitmap);
                if (hBitmap && !DeleteObject(hBitmap) && noExit)
                    ReportErr(L"hBitmap: Cannot delete bitmap object!");
            }
            if (!DeleteDC(hdcMem) && noExit)
                ReportErr(L"hdcMem: DeleteDC failed!");
        }
        if (hBitmapScroll)
        {
            if (hdcMemScroll)
            {
            SelectObject(hdcMemScroll, hdefBitmapScroll);
            if (hBitmapScroll && !DeleteObject(hBitmapScroll) && noExit)
                // Happens when typeOfDC == 2 and hdcWinCl is not released
                ReportErr(L"hBitmapScroll: Cannot delete bitmap object!");
            if (!DeleteDC(hdcMemScroll) && noExit)
                ReportErr(L"hdcMemScroll: DeleteDC failed!");
            }
        }

    }
    if (typeOfDC == 2 || !typeOfDC)
    {
        if (hbmpCompat && !DeleteObject(hbmpCompat) && noExit)
            ReportErr(L"hbmpCompat: Not deleted!");
    }

    if (!noExit)
    {
        exitOnceFlag = TRUE;
        SetDragFullWindow(TRUE, TRUE);
        // For testing only:
        if (hdefBitmap && !DeleteObject(hdefBitmap))
            ReportErr(L"hdefBitmap: DC flushed and DeleteObject failed!");
        if (hdefBitmapScroll && !DeleteObject(hdefBitmapScroll))
            ReportErr(L"hdefBitmapScroll: DC flushed and DeleteObject failed!");
        DestroyWindow(hWnd);
    }

    return true;
}

BOOL CreateToolTipForRect(HWND hwndParent, int toolType)
{
    // Odd behaviour in a dual monitor setup:
    // Size the window off to cover part of the second monitor
    // shows the tooltip as expected. Now move the window back
    // into the primary monitor so the tooltip is turned off. All is as
    // expected, but mouse hover over the border of the window
    // that was offscreen, and the first tooltip pops up, only to 
    // remove itself when resized again within the primary monitor.
    // Never happens if the window is moved off-screen, initially.
    // SWP_NOSENDCHANGING seems to have some effect on it.
    static HWND hwndTT = 0;
    wchar_t off[1] = { L'\0' };
    wchar_t offScreen[55] = { L'f', L'S', L'i', L'z', L'e', L' ', L'n', L'o', L't', L' ', L's', L'e', L't', L':', L' ', L'P',  L'a',  L'r',  L't',  L' ',  L'o',  L'f',  L' ',  L'w',  L'i',  L'n',  L'd',  L'o',  L'w',  L' ', L'n', L'o',  L't', L' ', L'p', L'a', L'i', L'n', L't', L'e', L'd', L'.', L' ', L'O', L'f', L'f', L' ', L'S', L'c', L'r', L'e', L'e', L'n', L'?', L'\0' };
    // Create a tooltip.
    if (!hwndTT)
        hwndTT = CreateWindowExW(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
            WS_POPUP | TTS_NOPREFIX | TTS_BALLOON,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            hwndParent, NULL, hInst, NULL);

    SetWindowPos(hwndTT, HWND_TOPMOST, 0, 0, 0, 0,
        SWP_NOSENDCHANGING || SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    // Set up "tool" information. In this case, the "tool" is the entire parent window.

    TOOLINFOW ti = { 0 };
    ti.cbSize = sizeof(TOOLINFOW);
    ti.uFlags = TTF_SUBCLASS;
    ti.hwnd = hwndParent;
    ti.hinst = hInst;
    ti.lpszText = (LPWSTR)((toolType) ? offScreen : off);

    GetClientRect(hwndParent, &ti.rect);

    SendMessageW(hwndTT, TTM_SETMAXTIPWIDTH, 0, 150);

    // Associate the tooltip with the "tool" window.
    SendMessageW(hwndTT, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

    return (BOOL)toolType;
}

BOOL IsAllFormInWindow(HWND hWnd, BOOL toolTipOn, BOOL isMaximized)
{
    rectTmp = RectCl().RectCl(0, hWnd, 0);
    if (rectTmp.left < scrEdge.cx || rectTmp.right > scrEdge.cx + scrDims.cx || rectTmp.top + RectCl().ClMenuandTitle(hWnd) < scrEdge.cy || rectTmp.bottom > scrEdge.cy + scrDims.cy)
    {
        if (isMaximized)
            toolTipOn = CreateToolTipForRect(hWnd);
        else
        {
            if (!toolTipOn)
                toolTipOn = CreateToolTipForRect(hWnd, 1);
        }
    }
    else
        toolTipOn = CreateToolTipForRect(hWnd);

    return toolTipOn;
}

BOOL SetDragFullWindow(BOOL dragFullWindow, BOOL restoreDef)
{
    //https://devblogs.microsoft.com/oldnewthing/20050310-00/?p=36233
    static int defDragFullWindow = -1;

    if (restoreDef)
    {
        if (!defDragFullWindow)
        {
            if (!SystemParametersInfoW(SPI_GETDRAGFULLWINDOWS,
                NULL, &tmp, 0))
            {
                defDragFullWindow = 0;
                ReportErr(L"SPI_SETDRAGFULLWINDOWS: Cannot get info.");
            }
            if (tmp)
            {
                if (SystemParametersInfoW(SPI_SETDRAGFULLWINDOWS,
                    defDragFullWindow,
                    NULL,
                    NULL))
                    SendNotifyMessageW(HWND_BROADCAST, WM_SETTINGCHANGE,
                        SPI_GETDRAGFULLWINDOWS, 0);
                else
                    ReportErr(L"SPI_SETDRAGFULLWINDOWS: Cannot restore.");
            }
            // else user turned it off while ScrollCall was active
        }
    }
    else
    {
        if (defDragFullWindow < 0)
        {
            if (!SystemParametersInfoW(SPI_GETDRAGFULLWINDOWS,
                NULL, &defDragFullWindow, 0))
            {
                defDragFullWindow = 0;
                ReportErr(L"SPI_SETDRAGFULLWINDOWS: Cannot get info.");
            }
        }
        else
        {
            // Only turns on if default is off, else does nothing
            if (dragFullWindow && !defDragFullWindow)
            {
                if (SystemParametersInfoW(SPI_SETDRAGFULLWINDOWS,
                    dragFullWindow,
                    NULL,
                    NULL))
                    SendNotifyMessageW(HWND_BROADCAST, WM_SETTINGCHANGE,
                        SPI_GETDRAGFULLWINDOWS, 0);
                else
                    ReportErr(L"SPI_SETDRAGFULLWINDOWS: Cannot set info.");
            }
        }
    }
    return dragFullWindow;
}
void ResetControlPos(HWND hWnd, BOOL setToEdgeX, BOOL setToEdgeY)
{
    for (int tmp = 2; tmp <= 8; tmp++)
    {
        rectTmp = RectCl().RectCl(ctrlArray[tmp], hWnd, 2);
        int width = rectTmp.right - rectTmp.left;
        int height = rectTmp.bottom - rectTmp.top;
        if (setToEdgeX) // For printed DC or image with no HORZ scrollbar
        {
            switch (tmp)
            {
            case 7:
            {
                rectTmp.left = 0;
                // forget rectTmp.right = width - 1;
            }
            break;
            case 8:
            {
                rectTmp.left = wd;
            }
            break;
           default:
            {
               rectTmp.left = 0;
            }
            break;
            }
        }
        else
            rectTmp.left += xCurrentScroll;

        if (setToEdgeY)
        {
            switch (tmp)
            {
                case 3:
                {
                    rectTmp.top = ht + OPT_HEIGHT;
                }
                break;

                 case 7:
                {
                    rectTmp.top = 4.6 * ht;
                }
                break;
                 case 8:
                {
                    rectTmp.top = 4.6 * ht;
                }
                break;
                default:
                     rectTmp.top = (tmp - 2) * ht;
                break;
            }
        }
        else
            rectTmp.top += yCurrentScroll;

        SetWindowPos(ctrlArray[tmp], NULL, rectTmp.left, rectTmp.top, width, height, SWP_NOSENDCHANGING);
    }
}
void InitWindowDims(HWND hWnd, int scrollStat, int& xNewSize, int& yNewSize, UINT bmpWidth, UINT bmpHeight)
{
    // RectCl can always be initialised in WM_CREATE, however, the first notification
    // of WM_SIZE after WM_CREATE produces "wrong" values of xNewSize and yNewSize.
    // It's not until the scrollbars are initialised this will provide the required values.
    rectTmp = RectCl().RectCl(0, hWnd, 0);
    // RectCl().width(0) is factored for wd etc.
    if (bmpWidth)
    {
        // These conditions are duplicated in ScrollInfo
        if (bmpWidth > xNewSize && bmpHeight > yNewSize)
            scrollStat = 3;
        else
        {
            if (bmpWidth > xNewSize)
                scrollStat = 1;
            if (bmpHeight > yNewSize)
                scrollStat = 2;
        }
    }

    if (scrollStat == 3)
    {
        xNewSize = rectTmp.right - rectTmp.left - RectCl().HorzNCl(hWnd) - GetSystemMetrics(SM_CXVSCROLL);
        yNewSize = rectTmp.bottom - rectTmp.top - RectCl().ClMenuandTitle(hWnd) - GetSystemMetrics(SM_CXHSCROLL);
    }
    else
    {
        if (scrollStat == 1)
            xNewSize = rectTmp.right - rectTmp.left - RectCl().HorzNCl(hWnd);
        else
        {
            if (scrollStat == 2)
                yNewSize = rectTmp.bottom - rectTmp.top - RectCl().ClMenuandTitle(hWnd);
        }
    }
}
// https://docs.microsoft.com/en-us/cpp/cpp/lambda-expressions-in-cpp
auto conditionalOp = [](bool useOr, auto a, auto b)
{
    return useOr ? (a | b) : (a & ~b);
};

void ChangeRedrawStyle(HWND hWnd, BOOL removeStyle)
{
    LONG64 retVal = GetWindowLongPtrW(hWnd, GWL_STYLE);
    auto opRetVal = conditionalOp(removeStyle, retVal, CS_HREDRAW);
    if (!(SetWindowLongPtrW(hWnd, GWL_STYLE, opRetVal)))
    {
        ReportErr(L"CS_HREDRAW: Cannot change style.");
        return;
    }
    opRetVal = conditionalOp(removeStyle, retVal, CS_VREDRAW);
    if (!(SetWindowLongPtrW(hWnd, GWL_STYLE, opRetVal)))
        ReportErr(L"CS_VREDRAW: Cannot change style.");

}

wchar_t* ReallocateMem(wchar_t* aSource, int Size)
{
    // "Int" may be problematic
    //buffer1 = (wchar_t*)realloc(buffer, MAX_LOADSTRING);
    wchar_t* buffer = (wchar_t*)realloc(aSource, Size);

    if (buffer)
    {
        //if (buffer = aSource) original address still in scope
        //buffer[(Size - 2)/ SIZEOF_WCHAR] = '\0';
    }
    else
    {
        //exit(EXIT_FAILURE);
        wchar_t* buffer = (wchar_t*)calloc((size_t)Size - 1, sizeof(wchar_t)); // retry original size

    }
    return buffer;
}

//**************************************************************
// Functions for possible later use
//**************************************************************
int CallDeprecatedSetScrollPos(HWND hWnd, int scrollStat)
{
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
        scrollStat = 0;
        break;
    }
    return scrollStat;
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
        return (LRESULT)FALSE;
    }
    break;
    default:
        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
        break;
    }

}
//https://stackoverflow.com/a/39654760/2128797
std::vector<std::vector<unsigned>> getPixels(Gdiplus::Bitmap bitmap, int& width, int& height) {


    //Pass up the width and height, as these are useful for accessing pixels in the vector o' vectors.
    width = bitmap.GetWidth();
    height = bitmap.GetHeight();

    auto* bitmapData = new Gdiplus::BitmapData;

    //Lock the whole bitmap so we can read pixel data easily.
    Gdiplus::Rect rectTmp(0, 0, width, height);
    bitmap.LockBits(&rectTmp, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, bitmapData);

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


    Gdiplus::Rect rectTmp(0, 0, width, height);
    myBitmap.LockBits(&rectTmp, Gdiplus::ImageLockModeWrite | ImageLockModeUserInputBuf, PixelFormat32bppARGB, &bitmapData);
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