#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <wiringPi.h>

int main(int argc, char* argv[])
{
    cv::Mat img;
    cv::VideoCapture vc(0);

    while (true)
    {
        // Activate Lights ////////////////
        wiringPiSetup();
        pinMode(4, OUTPUT);
        pinMode(5, OUTPUT);

        digitalWrite(4, HIGH); //980nm
        digitalWrite(5, HIGH); //850nm
        ///////////////////////////////////

        // Take image /////////////////////
        vc.read(img);
        cv::imshow("Img", img);
        ///////////////////////////////////

        // Deactivate Lights //////////////
        digitalWrite(4, LOW);
        digitalWrite(5, LOW);
        ///////////////////////////////////
        
        cv::waitKey(1);
    }
    
}