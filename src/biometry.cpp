#include <finger_vascular_biometry/biometry.hpp>

KeypointsTuple Biometry::KAZEDetector(cv::Mat &src)
{
    cv::Mat img = src.clone();

    // SURF ///////////////////////////
    cv::Ptr<cv::KAZE> detector = cv::KAZE::create(false, true);

    std::vector<cv::KeyPoint> keypoints;
    cv::Mat descriptors;

    detector->detectAndCompute(img, cv::noArray(), keypoints, descriptors);
    std::cout << "KAZE kp: " << std::to_string(keypoints.size()) << std::endl;
    return make_tuple(keypoints, descriptors);
    ///////////////////////////////////
}

std::tuple<cv::Mat, int> Biometry::FLANNMatcher(KeypointsTuple m1, KeypointsTuple m2, cv::Mat imgA, cv::Mat imgB)
{
    cv::Ptr<cv::DescriptorMatcher> matcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::FLANNBASED);
    std::vector<std::vector<cv::DMatch>> knn_matches;
    matcher->knnMatch(std::get<1>(m1), std::get<1>(m2), knn_matches, 2);

    // Filter matches using the Lowe's ratio test.
    const float ratio_thresh = 0.72f;
    std::vector<cv::DMatch> good_matches;
    for (size_t i = 0; i < knn_matches.size(); i++)
    {
        if (knn_matches[i][0].distance < ratio_thresh * knn_matches[i][1].distance)
        {
            good_matches.push_back(knn_matches[i][0]);
        }
    }

    // float maxInc = 0.34f;
    // std::vector<DMatch> goodest_matches;

    // for (size_t i = 0; i < good_matches.size(); i++)
    // {
    //     int idx1 = good_matches[i].trainIdx;
    //     int idx2 = good_matches[i].queryIdx;

    //     const KeyPoint &kp1 = get<0>(m2)[idx1], &kp2 = get<0>(m1)[idx2];
    //     Point2f p1 = kp1.pt;
    //     Point2f p2 = kp2.pt;
    //     Point2f triangle = Point2f(std::abs(p2.x - p1.x), std::abs(p2.y - p1.y));

    //     float angle = std::atan2(triangle.y, triangle.x);

    //     if (std::abs(angle) < maxInc)
    //     {
    //         goodest_matches.push_back(good_matches[i]);
    //     }
    // }

    std::cout << "Good matches: " << std::to_string(good_matches.size()) << std::endl;
    // cout << "Goodest matches: " << to_string(goodest_matches.size()) << endl;

    if (imgA.rows > 1 && imgB.rows > 1)
    {
        // Draw matches.
        cv::Mat img_matches;
        drawMatches(imgA.clone(), std::get<0>(m1), imgB.clone(), std::get<0>(m2), good_matches, img_matches, cv::Scalar::all(-1),
                    cv::Scalar::all(-1), std::vector<char>(), cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

        // Return mathes image.
        return std::make_tuple(img_matches.clone(), good_matches.size());
    }
    else
    {
        return std::make_tuple(cv::Mat(), good_matches.size());
    }
}

void Biometry::PreprocessImage(cv::Mat &src, cv::Mat &dst)
{
    cv::Mat img1, img2, img3, img4, img5, img6, img7, img8;

    // Umbralizar /////////////////////
    cv::threshold(src, img1, 60, 255, cv::THRESH_BINARY);

    cv::Mat framed = cv::Mat::zeros(cv::Size2d(img1.cols + 2, img1.rows + 2), CV_8U);
    cv::Rect r = cv::Rect2d(1, 1, img1.cols, img1.rows);
    img1.copyTo(framed(r));

    cv::Canny(framed, img2, 10, 50);

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size2d(5, 5));
    cv::dilate(img2, img2, kernel);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(img2, contours, cv::RETR_TREE, cv::CHAIN_APPROX_NONE);

    cv::Mat contoursImg = cv::Mat::zeros(img2.size(), CV_8U);
    double maxArea = 0;
    int mIdx = -1;
    for (int i = 0; i < contours.size(); i++)
    {
        double area = cv::contourArea(contours[i]);
        if (area > maxArea)
        {
            maxArea = area;
            mIdx = i;
        }
    }

    if (mIdx >= 0)
        cv::drawContours(contoursImg, contours, mIdx, cv::Scalar::all(255), cv::FILLED);

    cv::bitwise_and(src, src, img3, contoursImg(r));
    ///////////////////////////////////

    // CLAHE //////////////////////////
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
    clahe->setClipLimit(6);

    clahe->apply(img3, img3);
    ///////////////////////////////////

    // HFE + HPF //////////////////////
    HPF(img3, img4, FILTER_HFE);
    HPF(img4, img5, FILTER_HPF);
    cv::resize(img5, img6, img3.size());
    cv::bitwise_and(img6, img6, img7, contoursImg(r));
    clahe->apply(img7, img7);
    ///////////////////////////////////

    // CGF ////////////////////////////
    CGF(img7, img8);
    cv::Mat mask;
    cv::bitwise_not(contoursImg(r), mask);
    cv::normalize(img8, img8, 1.0, 0.0, 4, -1, mask);
    img8.convertTo(img8, CV_8U);
    cv::threshold(img8, img8, 0, 255, cv::THRESH_BINARY);

    kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
    cv::dilate(img8, img8, kernel);
    cv::erode(img8, img8, kernel);

    kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
    cv::erode(img8, img8, kernel);
    cv::dilate(img8, img8, kernel);
    ///////////////////////////////////

    cv::Mat cropMask = cv::Mat::zeros(img8.size(), CV_8U);
    cv::Rect cropROI = cv::Rect2d(22, 35, img8.cols - 60, img8.rows - 35 - 70);
    cropMask(cropROI).setTo(255);
    cv::bitwise_and(img8, cropMask, img8);

    dst = img8.clone();
}

void Biometry::CGF(cv::Mat &src, cv::Mat &dst)
{
    float sigma = 5;     // 5-pixel width
    float DeltaF = 1.12; //[0.5, 2,5]

    sigma = sig;
    DeltaF = dF / 100.0f;

    float fc = ((1 / M_PI) * sqrt(log(2) / 2) * ((pow(2, DeltaF) + 1) / (pow(2, DeltaF) - 1))) / sigma;

    cv::Mat g = cv::Mat::zeros(cv::Size2d(30, 30), CV_32F);
    cv::Mat G = cv::Mat::zeros(cv::Size2d(30, 30), CV_32F);
    cv::Mat Gim = cv::Mat::zeros(cv::Size2d(30, 30), CV_32F);
    for (int i = 0; i < g.cols; i++)
    {
        for (int j = 0; j < g.rows; j++)
        {
            g.at<float>(cv::Point(i, j)) = (1 / (2 * M_PI * pow(sigma, 2))) * exp(-(pow(i - g.cols / 2, 2) + pow(j - g.rows / 2, 2)) / (2 * pow(sigma, 2)));
            G.at<float>(cv::Point(i, j)) = g.at<float>(cv::Point(i, j)) * cos(2 * M_PI * fc * sqrt(pow(i - g.cols / 2, 2) + pow(j - g.rows / 2, 2)));
            Gim.at<float>(cv::Point(i, j)) = g.at<float>(cv::Point(i, j)) * sin(2 * M_PI * fc * sqrt(pow(i - g.cols / 2, 2) + pow(j - g.rows / 2, 2)));
        }
    }

    cv::Mat res;
    cv::filter2D(src, res, CV_32F, G);
    dst = res.clone();
}

cv::Mat Biometry::DFTModule(cv::Mat src[], bool shift)
{
    cv::Mat magImg;
    cv::magnitude(src[0], src[1], magImg); // Magnitud = planes[0]

    magImg += cv::Scalar::all(1); // Cambia a escala log
    cv::log(magImg, magImg);

    // Recorta el espectro si tiene rows o cols impares
    magImg = magImg(cv::Rect(0, 0, magImg.cols & -2, magImg.rows & -2));

    if (shift)
        FFTShift(magImg, magImg);

    cv::normalize(magImg, magImg, 0, 1, cv::NORM_MINMAX);

    return magImg;
}

void Biometry::HPF(cv::Mat &src, cv::Mat &dst, uint8_t filterType)
{
    cv::Mat padded; // Expande la imagen al tamaño óptimo
    int m = cv::getOptimalDFTSize(src.rows);
    int n = cv::getOptimalDFTSize(src.cols); // Añade valores cero en el borde
    cv::copyMakeBorder(src, padded, 0, m - src.rows, 0, n - src.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));

    cv::Mat planes[] = {cv::Mat_<float>(padded), cv::Mat::zeros(padded.size(), CV_32F)};
    cv::Mat complexImg;
    cv::merge(planes, 2, complexImg); // Añade un plano complejo

    /// DFT
    cv::dft(complexImg, complexImg);
    cv::split(complexImg, planes);
    ///

    /// FILTER
    cv::Mat H = cv::Mat::zeros(src.size(), CV_32F);
    cv::Mat filt = cv::Mat::zeros(src.size(), CV_32F);
    cv::Mat HF = complexImg.clone();

    // float D0 = 40.0f;
    // float k1 = filterType ? 0.5f : -4;
    // float k2 = filterType ? 0.75f : 10.9;
    float D0;
    float k1, k2;

    if (filterType)
    {
        D0 = (d0hfe > 0) ? d0hfe : 1;
        k1 = (k1hfe - 5000) / 100.0f;
        k2 = (k2hfe - 5000) / 100.0f;
    }
    else
    {
        D0 = (d0hpf > 0) ? d0hpf : 1;
        k1 = (k1hpf - 5000) / 1000.0f;
        k2 = (k2hpf - 5000) / 1000.0f;
    }

    // float a = 10.9, b = -4;

    for (int i = 0; i < H.cols; i++)
    {
        for (int j = 0; j < H.rows; j++)
        {
            H.at<float>(cv::Point(i, j)) = 1.0 - exp(-(pow(i - H.cols / 2, 2) + pow(j - H.rows / 2, 2)) / (2 * pow(D0, 2)));
        }
    }

    filt = k1 + k2 * H;

    FFTShift(HF, HF);
    split(HF, planes);
    for (int i = 0; i < filt.cols; i++)
    {
        for (int j = 0; j < filt.rows; j++)
        {
            planes[0].at<float>(cv::Point(i, j)) *= filt.at<float>(cv::Point(i, j));
            planes[1].at<float>(cv::Point(i, j)) *= filt.at<float>(cv::Point(i, j));
        }
    }

    cv::Mat filteredImg;
    merge(planes, 2, filteredImg);

    FFTShift(filteredImg, filteredImg);
    ///

    /// IDFT
    cv::Mat result;
    cv::dft(filteredImg, result, cv::DFT_INVERSE | cv::DFT_REAL_OUTPUT);
    cv::normalize(result, result, 0, 1, cv::NORM_MINMAX);
    result.convertTo(result, CV_8U, 255);
    cv::equalizeHist(result, result);
    ///

    dst = result.clone();
}

void Biometry::FFTShift(const cv::Mat &src, cv::Mat &dst)
{
    dst = src.clone();
    int cx = dst.cols / 2;
    int cy = dst.rows / 2;

    cv::Mat q1(dst, cv::Rect(0, 0, cx, cy));
    cv::Mat q2(dst, cv::Rect(cx, 0, cx, cy));
    cv::Mat q3(dst, cv::Rect(0, cy, cx, cy));
    cv::Mat q4(dst, cv::Rect(cx, cy, cx, cy));

    cv::Mat temp;
    q1.copyTo(temp);
    q4.copyTo(q1);
    temp.copyTo(q4);
    q2.copyTo(temp);
    q3.copyTo(q2);
    temp.copyTo(q3);
}
