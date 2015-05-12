TopMeNot
========
Utility for applying HWND\_NOTOPMOST Z-order to an obnoxious HWND\_TOPMOST window

Provides an xkill-style mouse-oriented window selector interface for relieving
a window of its HWND\_TOPMOST position. Windows can permanently place themselves
in the foreground by invoking the SetWindowPos() function and specifying
HWND\_TOPMOST as the window handle to insert after in the ordered list of
windows as they appear on the desktop.  Annoying instances of this are the
Adobe Flash update window, PEiD, and Skype.exe under certain circumstances.
This utility allows the user to point to the problem window and set
HWND\_NOTOPMOST as the window to insert after, making it possible to see other
windows that were previously covered up by a rude "topmost" window.

Build and Test
==============

Dependencies:
* Windows 7 Platform SDK or similar

Build:
* In a Windows SDK prompt or similar, type:

	nmake

Test:
* Execute topnot.exe
* Click inside the window to begin acquisition
* When crosshairs appear, click the window that is stuck in the foreground
* topnot.exe will close
* Window should now be able to be covered by other windows
