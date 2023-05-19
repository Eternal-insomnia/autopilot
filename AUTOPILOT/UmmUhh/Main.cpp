#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <vector>
#include <Windows.h>
#include <iostream>

using namespace std;
using namespace cv;

//CONTROL BLOCK

enum FLIGHT_MODES
{
    TAKEOFF,
    LANDING,
    FLY
};

enum DIRECTION_INSTRUCTIONS
{
    LEFT,
    RIGHT,
    FORWARD,
    BACKWARD,
    LAND //? zachem?????
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
    kbdSim(102); // num key right(6)
}

void pitchLeft()
{
    kbdSim(100); // num key left(4)
}

void bendForward()
{
    kbdSim(104); // numpad key up(8)
}

void bendBackward()
{
    kbdSim(101); // numpad key down(5)
}

Mat hwnd2mat(HWND hwnd)
{
    HDC hwindowDC, hwindowCompatibleDC;

    int height, width, srcheight, srcwidth;
    HBITMAP hbwindow;
    Mat src;
    BITMAPINFOHEADER  bi;

    hwindowDC = GetDC(hwnd);
    hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
    SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

    RECT windowsize;    // get the height and width of the screen
    GetClientRect(hwnd, &windowsize);

    auto desktop = GetDesktopWindow();
    RECT desksize;
    GetClientRect(desktop, &desksize);  //change this to whatever size you want to resize to

    srcheight = desksize.bottom;
    srcwidth = desksize.right;

    //height = windowsize.bottom; // <-- масштабирует размер захватываемого окна под созданное окно(’”…Ќя)
    //width = windowsize.right;
    height = desksize.bottom;
    width = desksize.right;

    src.create(height, width, CV_8UC4);

    // create a bitmap
    hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
    bi.biSize = sizeof(BITMAPINFOHEADER);    //http://msdn.microsoft.com/en-us/library/windows/window/dd183402%28v=vs.85%29.aspx
    bi.biWidth = width;
    bi.biHeight = -height;  //this is the line that makes it draw upside down or not
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    // use the previously created device context with the bitmap
    SelectObject(hwindowCompatibleDC, hbwindow);
    // copy from the window device context to the bitmap device context
    StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, 0, 0, srcwidth, srcheight, SRCCOPY); //change SRCCOPY to NOTSRCCOPY for wacky colors !
    GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, src.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);  //copy from hwindowCompatibleDC to hbwindow

    // avoid memory leak
    DeleteObject(hbwindow);
    DeleteDC(hwindowCompatibleDC);
    ReleaseDC(hwnd, hwindowDC);

    return src;
}

auto process_img(Mat& image)
{
    auto& orig_image = image;

    Mat processed_img_stage2;

    cvtColor(image, processed_img_stage2, COLOR_BGR2HSV);

    Mat processed_img_stage3;

    inRange(processed_img_stage2, Scalar(123, 9, 200), Scalar(255, 200, 255), processed_img_stage3); //по HSV он будет 181, 208, 150

    return processed_img_stage3;
}

auto press_key(int num)
{
    return;
}

std::vector<DIRECTION_INSTRUCTIONS> findDirection(Mat& img)
{
    std::vector<DIRECTION_INSTRUCTIONS> instructions;
    std::vector<Point> pixels;
    cv::findNonZero(img, pixels);

    if (pixels.size() == 0)
    {
        cout << "nothing here\n";
        return instructions;
    }

    Point center(img.size().height/2, img.size().width/2);

    Point point = pixels.at(0);
    //cout << center << " " << point << endl;
    //cout << img.size().height << " " << img.size().width << endl;

    if (point.x - center.y > 10)
        instructions.push_back(RIGHT);
    if (point.x - center.y < -10)
        instructions.push_back(LEFT);
    if (point.y - center.x < -10)
        instructions.push_back(FORWARD);
    if (point.y - center.x > 10)
        instructions.push_back(BACKWARD);

    if (instructions.size() == 0)
        instructions.push_back(LAND);

    return instructions;
}

void controlSystem(DIRECTION_INSTRUCTIONS direction, FLIGHT_MODES mode)
{
    switch (mode)
    {
    case TAKEOFF:
        //goUp();
        cout << "\nTAKEOFF MODE\n";
        break;
    case LANDING:
        // check for obstacles down (?)
        //goDown();
        cout << "\nLANDING MODE\n";
        break;
    default:

        switch (direction)
        {
        case RIGHT:
            //pitchRight();
            cout << "GO RIGHT\n";
            break;
        case LEFT:
            //pitchLeft();
            cout << "GO LEFT\n";
            break;
        case FORWARD:
            //bendForward();
            cout << "GO FORWARD\n";
            break;
        case BACKWARD:
            //bendBackward();
            cout << "GO BACK\n";
            break;
        }

        //if (height < something)
        //goUp();
        //if (height > something)
            //goDown();


        //??? NIZHE DELETE ???



        //if (1) // if mark is to the right from center on minimap
        //    pitchRight();
        //else if (1) // if mark is to the left
        //    pitchLeft();
        ////else
        //bendForward();
        //if (1) // if need to adjust height
        //    goUp();
        //else if (1) // if helicopter is too high
        //    goDown();
    }
}

int main(int argc, char** argv)
{
    int key = 0;

    FLIGHT_MODES mode = TAKEOFF;
    Sleep(1000);
    mode = FLY;
    cout << "\nponeslos'\n";

    while (key != 27) //1-999-289-9633 - cheatcode for helicopter
    {
        auto s = FindWindowA(NULL, "Grand Theft Auto V");//Grand Theft Auto V
        HWND hwndDesktop = s;
        if (!hwndDesktop)
            continue;

        Mat src = hwnd2mat(hwndDesktop);
        //imshow("output", src);


        //filter
        auto new_screen = src;
        //imshow("filter", process_img(new_screen));

        //ROI u know
        Rect roi(17, 580, 188, 125);
        Mat image_roi = new_screen(roi);
        Mat obrez = process_img(image_roi);
        cv::imshow("obrez", obrez);

        Sleep(10);
        for (auto instructions : findDirection(obrez))
        {
            controlSystem(instructions, mode);
        }

        key = waitKey(27); // 27 is ESC
    }
}