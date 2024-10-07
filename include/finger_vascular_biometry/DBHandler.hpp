#ifndef DBHANDLER_H
#define DBHANDLER_H

#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <finger_vascular_biometry/biometry.hpp>
#include "sqlite3.h"

class DBHandler
{
public:
    DBHandler() = delete;

    static bool Login(std::string* username, cv::Mat& img);

    static bool Register(std::string username, cv::Mat& img);

    static std::tuple<std::string, std::vector<cv::KeyPoint>, cv::Mat> FindBestMatch(KeypointsTuple features);

    static std::tuple<std::string, std::vector<cv::KeyPoint>, cv::Mat> ReadEntry(int id, sqlite3 *e_db = NULL);

    static int WriteEntry(KeypointsTuple features, std::string name);

    static char* EncodeF32Image(cv::Mat& img);

    static cv::Mat DecodeKazeDescriptor(std::vector<char> &buffer, int nKeypoints);
};

#endif 