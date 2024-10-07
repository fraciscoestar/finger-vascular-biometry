#ifndef BIOMETRY_H
#define BIOMETRY_H

#include <iostream>
#include <vector>
#include <tuple>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>

#define FILTER_HPF 0
#define FILTER_HFE 1
#define MINMATCHES 65

using KeypointsTuple = std::tuple<std::vector<cv::KeyPoint>, cv::Mat>;

class Biometry
{
public:
    Biometry() = delete;

    static KeypointsTuple KAZEDetector(cv::Mat &src);

    static std::tuple<cv::Mat, int> FLANNMatcher(KeypointsTuple m1, KeypointsTuple m2, cv::Mat imgA = cv::Mat(), cv::Mat imgB = cv::Mat());

    static void PreprocessImage(cv::Mat &src, cv::Mat &dst);

    static void CGF(cv::Mat &src, cv::Mat &dst);

    static cv::Mat DFTModule(cv::Mat src[], bool shift);

    static void HPF(cv::Mat &src, cv::Mat &dst, uint8_t filterType);

    static void FFTShift(const cv::Mat &src, cv::Mat &dst);

private:
    static const int d0hfe = 10, d0hpf = 40, k1hfe = 5300, k2hfe = 7327, k1hpf = 3050, k2hpf = 5463, sig = 5, dF=436;
};

#endif
