#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/text.hpp"
#include <vector>
#include <Windows.h>
#include <iostream>
//#include <thread>

using namespace std;
using namespace cv;

//CONTROL BLOCK

// a

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
    LAND
};

//std::vector<DIRECTION_INSTRUCTIONS> findDirection(Mat& img);

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
    kbdSim('D');
}

void pitchLeft()
{
    kbdSim('A');
}

void bendForward()
{
    kbdSim('U'); //perenaznachit' keys in game
}

void bendBackward()
{
    kbdSim('J'); //perenaznachit' keys in game
}

void controlSystem(DIRECTION_INSTRUCTIONS direction, FLIGHT_MODES mode, int& height)
{
    switch (mode)
    {
    case TAKEOFF:
        // goUp();
        for (int i = 0; i < 10; i++)
            goUp();
        cout << "TAKEOFF MODE\n";
        break;
    case LANDING:
        // check for obstacles down (?)
        if (direction == LEFT)
        {
            kbdSim('A');
            kbdSim('H');
            cout << "left\n";
        }

        else if (direction == RIGHT)
        {
            kbdSim('D');
            kbdSim('K');
            cout << "right\n";
        }

        if (direction == FORWARD)
        {
            kbdSim('U', 150);
            cout << "forward\n";
        }
        else if (direction == BACKWARD)
        {
            bendBackward();
            cout << "back\n";
        }
        else
            kbdSim('S', 50);
        Sleep(50);

        //cout << "LANDING MODE\n";
        break;
    default:

        switch (direction)
        {
        case RIGHT:
            pitchRight();
            cout << "GO RIGHT\n";
            break;
        case LEFT:
            pitchLeft();
            cout << "GO LEFT\n";
            break;
        case FORWARD:
            bendForward();
            goUp();
            cout << "GO FORWARD\n";
            break;
        case BACKWARD:
            bendBackward();
            cout << "GO BACK\n";
            break;
        }

        if (height < 150)
            goUp();
        if (height > 300)
            goDown();


        //??? NIZHE DELETE ??? pohui



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

//PROCESS IMAGE BLOCK

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

    inRange(processed_img_stage2, Scalar(123, 9, 200), Scalar(255, 200, 255), processed_img_stage3); //our HSV code is 181, 208, 150

    return processed_img_stage3;
}

//FINDING THE DIRECTION

std::vector<DIRECTION_INSTRUCTIONS> findDirection(Mat& img)
{
    std::vector<DIRECTION_INSTRUCTIONS> instructions;
    std::vector<Point> pixels;
    cv::findNonZero(img, pixels);
    int used = 0;

    if (pixels.size() < 5)
    {
        cout << "nothing here\n";
        return instructions;
    }

    Point center(img.size().height / 2, img.size().width / 2);

    Point point = pixels.at(0);
    point.x += 4; // absolutniy center of point
    //cout << center << " " << point << endl;
    //cout << img.size().height << " " << img.size().width << endl;

    if ((point.x - center.y > 5) || ((point.y - center.x > 20) && (point.x - center.y > 0)))
    {
        instructions.push_back(RIGHT);
        used++;
    }
    if ((point.x - center.y < 5) || ((point.y - center.x > 20) && (point.x - center.y <= 0)))
    {
        instructions.push_back(LEFT);
        used++;
    }
    if (point.y - center.x < -10)
    {
        instructions.push_back(FORWARD);
        used++;
    }
    if ((point.y - center.x > 10) || ((point.y - center.x < -10) && (point.y - center.x > -30)))
    {
        instructions.push_back(BACKWARD);
        used++;
    }
    if (used == 0)
        instructions.push_back(LAND);

    return instructions;
}

std::vector<DIRECTION_INSTRUCTIONS> landDirection(Mat& img)
{
    std::vector<DIRECTION_INSTRUCTIONS> instructions;
    std::vector<Point> pixels;
    cv::findNonZero(img, pixels);

    if (pixels.size() < 5)
    {
        cout << "nothing here\n";
        return instructions;
    }

    Point center(img.size().height / 2, img.size().width / 2);

    Point point = pixels.at(0);
    point.x += 4; // absolutniy center of point

    if ((point.x - center.y > 5) || ((point.y - center.x > 20) && (point.x - center.y > 0)))
    {
        instructions.push_back(RIGHT);
    }
    if ((point.x - center.y < 5) || ((point.y - center.x > 20) && (point.x - center.y <= 0)))
    {
        instructions.push_back(LEFT);
    }
    if (point.y - center.x < -2)
    {
        instructions.push_back(FORWARD);
    }
    if (point.y - center.x > 4)
    {
        instructions.push_back(BACKWARD);
    }

    return instructions;
}

//HEIGHT BLOCK

void processHeight(int& height, bool& stopSign, Mat& picture, Ptr<text::OCRTesseract> ocr, bool& startReadingHeight)
{
    while (!startReadingHeight)
        Sleep(100);

    while (!stopSign)
    {
        // do shit
        std::string height_string;
        ocr->run(picture, height_string);
        // we get Height: XXX (YYY) message and need YYY
        bool reading = false;
        std::string result_number;
        for (int i = 0; i < height_string.length(); ++i)
        {
            if (height_string[i] == ')')
                break;

            if (reading)
                result_number += height_string[i];

            if (height_string[i] == '(')
                reading = true;
        }

        height = std::stoi(result_number);
        Sleep(500);
    }
}

int main(int argc, char** argv)
{
    int key = 0;
    FLIGHT_MODES mode = TAKEOFF;

    cout << "\nponeslos'\n";
    Sleep(3000);
    cout << "y\n";
    kbdSim('W', 5000);

    bool check = false; //poka ne rabotaet

    auto ocr = text::OCRTesseract::create();

    Mat picture;
    int height = 0;
    bool stopSign = false;
    bool startReadingHeight = false;

    //std::thread readHeight(processHeight, ref(height), ref(stopSign), ref(picture), ref(ocr), ref(startReadingHeight));

    while (key != 27) //1-999-289-9633 or BUZZ-OFF - cheatcode for helicopter(???i??????, ??i???????, ???????? ?i??)
    {
        //if (pollKey() == 't')
        //    check = (check == 1 ? 0 : 1);

        //if (!check)
        //    continue;

        auto s = FindWindowA(NULL, "Grand Theft Auto V");//Grand Theft Auto V
        HWND hwndDesktop = s;
        if (!hwndDesktop)
            continue;

        Mat src = hwnd2mat(hwndDesktop);
        //imshow("output", src);


        //filter
        Mat new_screen = src;
        //imshow("filter", process_img(new_screen));

        //!!! HEIGHT!!! ! ! ! ! !!! !KIO !JIOFJIOEJI
        Rect roi_height(17, 580, 188, 125); // закидываем все сообщение: Height: XXX (XXX) meters
        Mat test = new_screen(roi_height);
        //cvtColor(test, test, COLOR_BGR2GRAY);
        //threshold(test, test, 100, 255, THRESH_BINARY);
        //bitwise_not(test, test);
        picture = test;
        imshow("picture", picture);
        //processHeight(height, stopSign, picture, ocr, startReadingHeight);

        startReadingHeight = true;


        //ROI u know
        Rect roi(17, 580, 188, 125);
        Mat image_roi = new_screen(roi);
        Mat obrez = process_img(image_roi);
        //imshow("obrez", obrez);

        if (mode == TAKEOFF)
        {
            controlSystem(LAND, mode, height); // first argument doesnt matter
            if (height > 150)
                mode = FLY;
            else
                continue;
        }
        // suda ne idem, poka ne naberem visotu)
        Sleep(10);
        for (auto instructions : findDirection(obrez))
        {
            if (instructions == LAND)
            {
                mode = LANDING;
                //instructions : landDirection(obrez);
                //if (high == 0) continue;
            }
            controlSystem(instructions, mode, height);
        }

        key = waitKey(27); // 27 is ESC
    }
    stopSign = true;
    //readHeight.join();
}
