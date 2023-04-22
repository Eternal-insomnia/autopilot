#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <Windows.h>
#include <iostream>

using namespace std;
using namespace cv;

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

    //height = windowsize.bottom;
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

    Mat processed_img;


    auto rotate = cv::getRotationMatrix2D(Point(((image.cols - 1), (image.rows - 1))), 0, 1);

    warpAffine(image, processed_img, rotate, Point((image.cols - 1), (image.rows - 1)));

    Mat processed_img_stage2;

    cvtColor(processed_img, processed_img_stage2, cv::COLOR_BGR2GRAY);

    Mat processed_img_stage3;

    Canny(processed_img_stage2, processed_img_stage3, 200, 300);

    return processed_img_stage3;
}

auto press_key(int num)
{
    return;
}

int main(int argc, char** argv)
{
    //HWND hwndDesktop = GetDesktopWindow();
    //namedWindow("output", WINDOW_NORMAL);
    int key = 0;

    int W = 0x11; //key number(i think)

    while (key != 27)
    {
        //HWND hwndDesktop = GetWindow(GetCapture(), 1);
        auto s = FindWindowA(NULL, "Minecraft* 1.19 - Одиночная игра");
        HWND hwndDesktop = s;
        //std::cout << hwndDesktop;

        if (!hwndDesktop)
            continue;

        Mat src = hwnd2mat(hwndDesktop);
        // you can do some image processing here
        imshow("output", src);

        auto new_screen = process_img(src);
        imshow("фы", new_screen);

        key = waitKey(60); // you can change wait time
    }

}