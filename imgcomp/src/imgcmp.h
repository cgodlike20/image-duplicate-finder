#include <pqxx/pqxx>
#include <opencv2/opencv.hpp>

#ifndef IMGCMP_H
#define IMGCMP_H

#define orb_size 150

namespace ic {
    std::string descriptorsToString(cv::Mat descriptors);
    std::vector<int> parseArrayString(const std::string& arrayString);
    cv::Mat parseStringToMat(const std::string& arrayString);
    void matchDescriptors(cv::Mat descriptors1, cv::Mat descriptors2, std::string imgName1, std::string imgName2);
}

#endif