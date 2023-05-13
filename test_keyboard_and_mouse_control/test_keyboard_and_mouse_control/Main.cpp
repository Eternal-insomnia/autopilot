//#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <iostream>

using namespace std;

enum FLIGHT_MODES
{
	TAKEOFF,
	LANDING,
	FLY
};

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

void goUp()
{
	kbdSim('W');
}
void goDown()
{
	kbdSim('S');
}

void pitchRight()
{
	//kbdSim('E');
	kbdSim(102);
}

void pitchLeft() 
{
	//kbdSim('Q');
	kbdSim(100); // num left
}

void bendForward()
{
	kbdSim(104); // numpad key up
}
void bendBackward()
{
	kbdSim(98); // numpad key down
}

void controlSystem(FLIGHT_MODES mode)
{
	switch (mode)
	{
	case TAKEOFF:
		goUp();
	break;
	case LANDING:
		// check for obstacles down (?)
		goDown();
	break;
	default:
		if (1) // if mark is to the right from center on minimap
			pitchRight();
		else if (1) // if mark is to the left
			pitchLeft();
		//else
			bendForward();
		if (1) // if need to adjust height
			goUp();
		else if (1) // if helicopter is too high
			goDown();
	}
}

void main()
{

	std::string window_name;
	std::cout << "Enter window name: ";
	std::cin >> window_name;

	HWND windowHandle = FindWindowA(NULL, window_name.c_str()); //Minecraft* 1.19 - Одиночная игра
	INPUT* key;

	FLIGHT_MODES current_mode = FLY;
	int counter = 0;

	while (true)
	{
		if (windowHandle == NULL)
			cout << "not found";
		else
		{
			SetForegroundWindow(windowHandle);
			Sleep(10);

			// check for height, whatever


			// change mode when height reaches certain point
			if (counter < 5000)
				counter++;
			else
				current_mode = FLY;

			controlSystem(current_mode);
			
			//kbdSim(27);
				
			//kbdSim('W', 1000); //when u use this prog u need to switch kbd language no English
			
			//break;
		}
	}

	return;
}