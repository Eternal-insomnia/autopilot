//#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <iostream>

using namespace std;

void main()
{
	HWND windowHandle = FindWindowA(NULL, "Minecraft* 1.19 - Одиночная игра"); //Minecraft* 1.19 - Одиночная игра
	INPUT* key;
	while (true)
	{
		if (windowHandle == NULL)
			cout << "not found";
		else
		{
			SetForegroundWindow(windowHandle);
			Sleep(1000);

			key = new INPUT;
			key->type = INPUT_KEYBOARD;
			key->ki.wVk = 27;
			key->ki.dwFlags = 0;
			key->ki.time = 0;
			key->ki.wScan = 0;
			key->ki.dwExtraInfo = 0;

			//SendInput(1, key, sizeof(INPUT));

			//key->ki.dwExtraInfo = KEYEVENTF_KEYUP;
			//SendInput(1, key, sizeof(INPUT));
			//вроде как должно отжимать кнопку, но она сама отжимается 

			Sleep(1000);
			key->ki.wVk = 'W';
			SendInput(1, key, sizeof(INPUT));

			Sleep(1000);
			key->ki.dwExtraInfo = KEYEVENTF_KEYUP;
			SendInput(1, key, sizeof(INPUT));

			//PODLIVA

			// Get the window client area.
			RECT rc;
			GetClientRect(windowHandle, &rc);

			// Convert the client area to screen coordinates.
			POINT pt = { rc.left, rc.top };
			POINT pt2 = { rc.right, rc.bottom };
			ClientToScreen(windowHandle, &pt);
			ClientToScreen(windowHandle, &pt2);
			SetRect(&rc, pt.x, pt.y, pt2.x, pt2.y);
			
			// Confine the cursor.
			ClipCursor(&rc);

			//SetCursorPos(0, 0);

			//SetCapture(windowHandle);
			//mouse_event(MOUSEEVENTF_ABSOLUTE, 100, 0, 0, 0);
			//SetCursorPos(0, 0);
			//ReleaseCapture();

			delete key;
		}
	}

	return;
}