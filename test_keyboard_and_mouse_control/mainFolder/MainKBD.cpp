//#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <iostream>

using namespace std;

void mouseSim(HWND windowHandle)
{
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

	SetCursorPos(0, 0);

	SetCapture(windowHandle);
	mouse_event(MOUSEEVENTF_ABSOLUTE, 100, 0, 0, 0);
	SetCursorPos(0, 0);
	ReleaseCapture();
}

typedef short ABOBA;

void kbdSim(ABOBA key, DWORD time = 100)
{
	UINT mappedkey;
	INPUT input = { 0 };
	key = VkKeyScan(key);
	mappedkey = MapVirtualKey(LOBYTE(key), 0);
	input.type = INPUT_KEYBOARD;
	input.ki.dwFlags = KEYEVENTF_SCANCODE;
	input.ki.wScan = mappedkey;
	SendInput(1, &input, sizeof(INPUT));
	Sleep(time);
	input.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
	SendInput(1, &input, sizeof(INPUT));
}

void goStraight()
{
	kbdSim('W');
	return;
}
void goLeft()
{
	kbdSim('A');
	return;
}
void goRight()
{
	kbdSim('D');
	return;
}
void goBack()
{
	kbdSim('S');
	return;
}

//VK_NUMPAD0 - 9 = 0x60 - 0x69 or 96 - 105

void goUp()
{
	kbdSim(0x68);//numpad8
	return;
}

void goDown()
{
	kbdSim(0x65);//numpad5
	return;
}

void main()
{
	HWND windowHandle = FindWindowA(NULL, "Grand Theft Auto V"); //Minecraft* 1.19 - Одиночная игра
	INPUT* key;
	while (true)
	{
		if (windowHandle == NULL)
			cout << "not found";
		else
		{
			SetForegroundWindow(windowHandle);
			Sleep(2000);

			kbdSim(27);
			kbdSim('W', 1000); //when u use this prog u need to switch kbd language no English

			break;
		}
	}

	return;
}