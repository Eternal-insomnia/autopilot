#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/ml.hpp"
#include <vector>
#include <Windows.h>
#include <iostream>
#include <cstdlib>

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
    LAND 
};

std::vector<DIRECTION_INSTRUCTIONS> findDirection(Mat& img);

// global variables ///////////////////////////////////////////////////////////////////////////////
const int MIN_CONTOUR_AREA = 100;

const int RESIZED_IMAGE_WIDTH = 20;
const int RESIZED_IMAGE_HEIGHT = 30;

///////////////////////////////////////////////////////////////////////////////////////////////////
class ContourWithData {
public:
    // member variables ///////////////////////////////////////////////////////////////////////////
    std::vector<cv::Point> ptContour;           // contour
    cv::Rect boundingRect;                      // bounding rect for contour
    float fltArea;                              // area of contour

    ///////////////////////////////////////////////////////////////////////////////////////////////
    bool checkIfContourIsValid() {                              // obviously in a production grade program
        if (fltArea < MIN_CONTOUR_AREA) return false;           // we would have a much more robust function for 
        return true;                                            // identifying if a contour is valid !!
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    static bool sortByBoundingRectXPosition(const ContourWithData& cwdLeft, const ContourWithData& cwdRight) {      // this function allows us to sort
        return(cwdLeft.boundingRect.x < cwdRight.boundingRect.x);                                                   // the contours from left to right
    }

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

void controlSystem(DIRECTION_INSTRUCTIONS direction, FLIGHT_MODES mode, int height)
{
    switch (mode)
    {
    case TAKEOFF:
        if (height < 150)
        //for (int i = 0; i < 10; i++)
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
        else if (height > 300) // ?
            goDown();
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

int getHeight(cv::Mat matTestingNumbers, std::vector<ContourWithData>& allContoursWithData, std::vector<ContourWithData>& validContoursWithData,
              cv::Ptr<cv::ml::KNearest> kNearest)
{
    //cv::Mat matTestingNumbers = cv::imread("test1.png");            // read in the test numbers image

    if (matTestingNumbers.empty()) {                                // if unable to open image
        std::cout << "error: image not read from file\n\n";         // show error message on command line
        return(0);                                                  // and exit program
    }

    cv::Mat matGrayscale;           //
    cv::Mat matBlurred;             // declare more image variables
    cv::Mat matThresh;              //
    cv::Mat matThreshCopy;          //

    cv::cvtColor(matTestingNumbers, matGrayscale, cv::COLOR_BGRA2GRAY);         // convert to grayscale

    // blur
    cv::GaussianBlur(matGrayscale,              // input image
        matBlurred,                // output image
        cv::Size(5, 5),            // smoothing window width and height in pixels
        0);                        // sigma value, determines how much the image will be blurred, zero makes function choose the sigma value

    // filter image from grayscale to black and white
    cv::adaptiveThreshold(matBlurred,                           // input image
        matThresh,                            // output image
        255,                                  // make pixels that pass the threshold full white
        cv::ADAPTIVE_THRESH_GAUSSIAN_C,       // use gaussian rather than mean, seems to give better results
        cv::THRESH_BINARY_INV,                // invert so foreground will be white, background will be black
        11,                                   // size of a pixel neighborhood used to calculate threshold value
        2);                                   // constant subtracted from the mean or weighted mean

    matThreshCopy = matThresh.clone();              // make a copy of the thresh image, this in necessary b/c findContours modifies the image

    std::vector<std::vector<cv::Point> > ptContours;        // declare a vector for the contours
    std::vector<cv::Vec4i> v4iHierarchy;                    // declare a vector for the hierarchy (we won't use this in this program but this may be helpful for reference)

    cv::findContours(matThreshCopy,             // input image, make sure to use a copy since the function will modify this image in the course of finding contours
        ptContours,                             // output contours
        v4iHierarchy,                           // output hierarchy
        cv::RETR_EXTERNAL,                      // retrieve the outermost contours only
        cv::CHAIN_APPROX_SIMPLE);               // compress horizontal, vertical, and diagonal segments and leave only their end points

    for (int i = 0; i < ptContours.size(); i++) {               // for each contour
        ContourWithData contourWithData;                                                    // instantiate a contour with data object
        contourWithData.ptContour = ptContours[i];                                          // assign contour to contour with data
        contourWithData.boundingRect = cv::boundingRect(contourWithData.ptContour);         // get the bounding rect
        contourWithData.fltArea = cv::contourArea(contourWithData.ptContour);               // calculate the contour area
        allContoursWithData.push_back(contourWithData);                                     // add contour with data object to list of all contours with data
    }

    for (int i = 0; i < allContoursWithData.size(); i++) {                      // for all contours
        if (allContoursWithData[i].checkIfContourIsValid()) {                   // check if valid
            validContoursWithData.push_back(allContoursWithData[i]);            // if so, append to valid contour list
        }
    }
    // sort contours from left to right
    std::sort(validContoursWithData.begin(), validContoursWithData.end(), ContourWithData::sortByBoundingRectXPosition);

    std::string strFinalString;         // declare final string, this will have the final number sequence by the end of the program

    for (int i = 0; i < validContoursWithData.size(); i++) {            // for each contour

        // draw a green rect around the current char
        cv::rectangle(matTestingNumbers,                            // draw rectangle on original image
            validContoursWithData[i].boundingRect,        // rect to draw
            cv::Scalar(0, 255, 0),                        // green
            2);                                           // thickness

        cv::Mat matROI = matThresh(validContoursWithData[i].boundingRect);          // get ROI image of bounding rect

        cv::Mat matROIResized;
        cv::resize(matROI, matROIResized, cv::Size(RESIZED_IMAGE_WIDTH, RESIZED_IMAGE_HEIGHT));     // resize image, this will be more consistent for recognition and storage

        cv::Mat matROIFloat;
        matROIResized.convertTo(matROIFloat, CV_32FC1);             // convert Mat to float, necessary for call to find_nearest

        cv::Mat matROIFlattenedFloat = matROIFloat.reshape(1, 1);

        cv::Mat matCurrentChar(0, 0, CV_32F);

        kNearest->findNearest(matROIFlattenedFloat, 1, matCurrentChar);     // finally we can call find_nearest !!!

        float fltCurrentChar = (float)matCurrentChar.at<float>(0, 0);

        strFinalString = strFinalString + char(int(fltCurrentChar));        // append current char to full string
    }

    std::cout << "\n\n" << "numbers read = " << strFinalString << "\n\n";       // show the full string

    cv::imshow("matTestingNumbers", matTestingNumbers);     // show input image with green boxes drawn around found digits

    return atoi(strFinalString.c_str());
}

int main(int argc, char** argv)
{
    int key = 0;
    FLIGHT_MODES mode = FLY;

    cout << "\nponeslos'\n";
    Sleep(3000);
    cout << "y\n";
    kbdSim('W', 5000);

    bool check = false; //poka ne rabotaet

    std::vector<ContourWithData> allContoursWithData;           // declare empty vectors,
    std::vector<ContourWithData> validContoursWithData;         // we will fill these shortly


    // read in training classifications ///////////////////////////////////////////////////

    cv::Mat matClassificationInts;      // we will read the classification numbers into this variable as though it is a vector

    cv::FileStorage fsClassifications("classifications.xml", cv::FileStorage::READ);        // open the classifications file

    if (fsClassifications.isOpened() == false) {                                                    // if the file was not opened successfully
        std::cout << "error, unable to open training classifications file, exiting program\n\n";    // show error message
        return(0);                                                                                  // and exit program
    }

    fsClassifications["classifications"] >> matClassificationInts;      // read classifications section into Mat classifications variable
    fsClassifications.release();                                        // close the classifications file

    // read in training images ////////////////////////////////////////////////////////////

    cv::Mat matTrainingImagesAsFlattenedFloats;         // we will read multiple images into this single image variable as though it is a vector

    cv::FileStorage fsTrainingImages("images.xml", cv::FileStorage::READ);          // open the training images file

    if (fsTrainingImages.isOpened() == false) {                                                 // if the file was not opened successfully
        std::cout << "error, unable to open training images file, exiting program\n\n";         // show error message
        return(0);                                                                              // and exit program
    }

    fsTrainingImages["images"] >> matTrainingImagesAsFlattenedFloats;           // read images section into Mat training images variable
    fsTrainingImages.release();                                                 // close the traning images file

    // train //////////////////////////////////////////////////////////////////////////////

    cv::Ptr<cv::ml::KNearest>  kNearest(cv::ml::KNearest::create());            // instantiate the KNN object

    // finally we get to the call to train, note that both parameters have to be of type Mat (a single Mat)
    // even though in reality they are multiple images / numbers
    kNearest->train(matTrainingImagesAsFlattenedFloats, cv::ml::ROW_SAMPLE, matClassificationInts);
    // RECOGNITION TRAINING OVER


    while (key != 27) //1-999-289-9633 or BUZZ-OFF - cheatcode for helicopter(гелiкоптир, гвiнторкил, воздушна цiль)
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
        auto new_screen = src;
        //imshow("filter", process_img(new_screen));

        Rect roi_height(228, 230, 14, 444); // change numbers
        Mat height_screen = new_screen(roi_height);

        //ROI u know
        Rect roi(17, 580, 188, 125);
        Mat image_roi = new_screen(roi);
        Mat obrez = process_img(image_roi);
        //imshow("obrez", obrez);

        int height = 0;
        if (mode != LANDING)
            height = getHeight(height_screen, allContoursWithData, validContoursWithData, kNearest);

        if (mode == TAKEOFF && height > 150)
            mode = FLY;

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
}
