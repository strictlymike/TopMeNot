#define OEMRESOURCE
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

#define DEBUG					(0)

#if DEBUG
// If DEBUG is enabled, a console main() function will be defined instead of a
// WinMain(), a NULL HINSTANCE will be created to satisfy references to
// hInstance in main(), and PDEBUG() will be substituted with printf() by the C
// pre-processor. Useful for experimentation.
#define PDEBUG printf
#define ARGC argc
#define ARGV argv
#else
#define PDEBUG
#define ARGC __argc
#define ARGV __argv
#endif

int Usage(int Err);
int PresentError(char *Err);
BOOL TopNot(HWND Tgt);
LRESULT CALLBACK WndProc(
	HWND hWnd,
	UINT Msg,
	WPARAM wParam,
	LPARAM lParam
   );

// In order to set the mouse cursor even when the mouse pointer is over another
// window, SetCursor() requires SetCapture() to have been called in response to
// a mouse click in the client area. This prohibits the xkill-style interface
// wherein the system-wide mouse cursor is immediately changed upon executing
// the program.
//
// Meanwhile, SetSystemCursor() achieves an xkill-syle interface, but destroys
// cursor resources that are supplied to it, forcing the programmer to create
// copies in order to be able to restore the cursor.  The SetSystemCursor()
// function is less than optimal because if this program is interrupted, it
// could leave the normal Windows arrow cursor in an abnormal state, however
// this does permit an xkill-style interface. SetSystemCursor() is not really
// meant for this job, but the option is provided to use the /now argument to
// achieve this behavior. If the worst happens, the cursor can be restored by
// logging off and logging back on again.
BOOL ImmediateSystemwideCrosshairs = FALSE;
HCURSOR CopyArrow = NULL;
HCURSOR CopyCrosshairs = NULL;

// Entry point
#if DEBUG
int
main(int argc, char **argv)
{
	HINSTANCE hInstance = NULL;
#else
int WINAPI
WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
	int nCmdShow
   )
{
#endif
    WNDCLASSEX wc = {0};
 	HWND Tgt = NULL;
	HWND hWnd = NULL;
	MSG Msg;
	BOOL Ok = FALSE;

	if (ARGC == 2)
	{
		if (!strcmp(ARGV[1], "/?"))
		{
			return Usage(0);
		}

		if (!strcmpi(ARGV[1], "/now"))
		{
			ImmediateSystemwideCrosshairs = TRUE;
		}
		else
		{
			// If argument is neither /? nor /now, treat it as a window caption

			Tgt = FindWindow(NULL, ARGV[1]);

			if (Tgt == NULL)
			{
				return PresentError("No window found with that title");
			}

			Ok = TopNot(Tgt);

			return Ok? 0: 1;
		}
	}
	else if (ARGC > 2)
	{
		return Usage(1);
	}

	// If no argument was specified, allow the user to specify a window by
	// clicking on it...

    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszClassName = "WindowSelector";
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if(!RegisterClassEx(&wc))
    {
        return PresentError("Failed to register window class");
    }

    hWnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        "WindowSelector",
        "Top Me Not",
        WS_OVERLAPPEDWINDOW,
        0,
		0,
		250,
		145,
		/* If the xkill-style interface is in use, then create an invisble
		 * message-only window */
        ImmediateSystemwideCrosshairs? HWND_MESSAGE: NULL,
		NULL,
		hInstance,
		NULL
	   );

    if(hWnd == NULL)
    {
        return PresentError("Failed to create window");
    }

    ShowWindow(hWnd, SW_SHOW);

	// Message loop for window procedure
    while(GetMessage(&Msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return Msg.wParam;
}

int
Usage(int Err)
{
	MessageBox(
		NULL,

		"Takes a window out of the topmost position, allowing it to be "
		"covered by other windows when it is deactivated.\n"
		"\n"
		"Usage: topnot [<windowtitle> | /now | /?]\n"
		"\n"
		"Options:\n"
		"\n"
		"<windowtitle>\tOptional title of the topmost window to target\n"
		"/now\t\tImmediately display crosshairs\n"
		"/?\t\tDisplay this help message\n"
		"\n"
		"Without any options, this utility will present a window prompting "
		"the user to click in the client area and drag the cursor to the "
		"target window. If the /now argument is specified, no window will "
		"be presented, and the mouse cursor will immediately be changed to "
		"a crosshairs, signifying that the next window that is clicked will "
		"be targeted. If a <windowtitle> is specified, the window with that "
		"title will be targeted. If /? is specified, this help message will "
		"be displayed."

		,
		"Top Me Not",
		(Err? MB_ICONEXCLAMATION: MB_ICONQUESTION) | MB_OK
	   );

	return Err;
}

// Remove HWND_TOPMOST
BOOL
TopNot(HWND Tgt)
{
	BOOL Ok;

	Ok = SetWindowPos(Tgt, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
	if (!Ok)
	{
		PresentError("Failed to modify window position");
	}

	return Ok;
}

int
PresentError(char *Err)
{
	MessageBox(NULL, Err, "Error", MB_ICONEXCLAMATION | MB_OK);

	return 1;
}

// Handles Window Messages sent to the window created in WinMain() / main().
LRESULT CALLBACK
WndProc(
	HWND hWnd,
	UINT Msg,
	WPARAM wParam,
	LPARAM lParam
   )
{
	HWND Tgt;
	HWND Parent;
	POINT pWhere;
	BOOL Ok;

	HDC hDc;
	RECT Rect;
	char * Label = "\n\n\nClick here and drag to the offending window\n"
		"(Or press Escape to quit)";
	PAINTSTRUCT ps;
	HFONT hFont, hOldFont; 

    switch(Msg)
    {
		case WM_CREATE:
			if (ImmediateSystemwideCrosshairs)
			{
				SetCapture(hWnd);
				CopyArrow = CopyCursor(LoadCursor(NULL, IDC_ARROW));
				CopyCrosshairs = CopyCursor(LoadCursor(NULL, IDC_CROSS));
				SetSystemCursor(CopyCrosshairs, OCR_NORMAL);
			}
			else
			{
				/* Position this window in the topmost position so that the
				 * user can see this interface over the topmost window that
				 * they are trying to target. */
				SetWindowPos(
					hWnd,
					HWND_TOPMOST,
					0,
					0,
					0,
					0,
					SWP_NOMOVE|SWP_NOSIZE
				   );
			}
			break;

		case WM_LBUTTONDOWN:
			if (!ImmediateSystemwideCrosshairs)
			{
				ShowWindow(hWnd, SW_MINIMIZE);
				SetCapture(hWnd);
				SetCursor(LoadCursor(NULL, IDC_CROSS));
			}
			break;

		case WM_PAINT:
			GetClientRect(hWnd, &Rect);
			hDc = BeginPaint(hWnd, &ps);
			hFont = (HFONT)GetStockObject(ANSI_VAR_FONT); 
			hOldFont = (HFONT)SelectObject(hDc, hFont);
			DrawTextA(
				hDc,
				Label,
				strlen(Label),
				&Rect,
				DT_CENTER | DT_VCENTER
			   );
			SelectObject(hDc, hOldFont); 
			EndPaint(hWnd, &ps);
			break;

		// If Alt+Tab or other user or system activity causes the message-only
		// window to lose focus, then exit, restoring the cursor in the
		// WM_DESTROY handler.
		case WM_ACTIVATE:
			if (ImmediateSystemwideCrosshairs && (wParam == WA_INACTIVE))
			{
				DestroyWindow(hWnd);
			}
			break;

		// If Escape is pressed, stand down, restoring the cursor if necessary
		// in the WM_DESTROY handler.
		case WM_KEYDOWN:
			if (wParam == VK_ESCAPE)
			{
				DestroyWindow(hWnd);
			}
			break;

		// When the user releases the left mouse button, get the point where
		// the mouse button was released, convert it to screen coordinates, get
		// the window residing there, find its root window (in case the user
		// clicked a sub-window such as a static text control), and set
		// HWND_NOTOPMOST using SetWindowPos().
		case WM_LBUTTONUP:
			pWhere.x = GET_X_LPARAM(lParam);
			pWhere.y = GET_Y_LPARAM(lParam);

			// Convert to screen coordinates
			ClientToScreen(hWnd, &pWhere);

			// What window falls under the cursor
			Tgt = WindowFromPoint(pWhere);

			// Get top-level window (as opposed to a static text or other
			// control that the user may have clicked on) against which to call
			// SetWindowPos() and set HWND_NOTOPMOST.
			Tgt = GetAncestor(Tgt, GA_ROOT);

			if (Tgt)
			{
				TopNot(Tgt);
			}

            DestroyWindow(hWnd);
			break;

        case WM_CLOSE:
            DestroyWindow(hWnd);
			break;

		// All DestroyWindow() paths lead here:
		// https://msdn.microsoft.com/en-us/library/windows/desktop/ff381396%28
		// v=vs.85%29.aspx
        case WM_DESTROY:
			if (ImmediateSystemwideCrosshairs)
			{
				SetSystemCursor(CopyArrow, OCR_NORMAL);
			}
            PostQuitMessage(0);
			break;

        default:
            return DefWindowProc(hWnd, Msg, wParam, lParam);
    }
    return 0;
}
