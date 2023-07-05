#include "imgcmp.h"
#include <pqxx/pqxx>
#include <opencv2/opencv.hpp>

namespace ic {
    std::string descriptorsToString(cv::Mat descriptors){
        std::stringstream strStream;

        // Insert curly braces
        strStream.put('{');
        for (int row = 0; row < descriptors.rows; row++){
            for (int col = 0; col < descriptors.cols; col++){
                // Cast uchar to int and add to stingstream
                int value = static_cast<int>(descriptors.at<uchar>(row, col));
                strStream << value << ",";
            }
        }
        // Delete the last coma
        strStream.seekp(-1, std::ios_base::end);
        // Insert curly braces
        strStream.put('}');;
        return strStream.str();
    }

    std::vector<int> parseArrayString(const std::string& arrayString) {
        std::vector<int> result;

        // Remove leading and trailing curly braces
        std::string trimmedString = arrayString.substr(1, arrayString.size() - 2);
    
        // Tokenize the string using comma as the delimiter
        std::istringstream iss(trimmedString);
        std::string token;
        while (std::getline(iss, token, ',')){
            // Convert each token to an integer and add it to the result vector
            result.push_back(std::stoi(token));
        }
        return result;
    }

    cv::Mat parseStringToMat(const std::string& arrayString) {
        cv::Mat outputMat(orb_size, 32, CV_8UC1);
        int row = 0;
        int column = 0;

        // Remove leading and trailing curly braces
        std::string trimmedString = arrayString.substr(1, arrayString.size() - 2);
    
        // Tokenize the string using comma as the delimiter
        std::istringstream iStrStream(trimmedString);
        std::string token;
        while (std::getline(iStrStream, token, ',')){
            // Convert each token to an integer and insert into the matrix
            outputMat.at<uchar>(row, column) = std::stoi(token);

            // Increment column and row if needed, check conditions not to exceed the mat size
            ++column;
            if(column < 32){
                continue;
            }
            ++row;
            column = 0;
            if(row >= orb_size){
                break;
            }
        } 
        return outputMat;
    }

    void matchDescriptors(cv::Mat descriptors1, cv::Mat descriptors2, std::string imgName1, std::string imgName2){
        cv::BFMatcher kpMatcher(cv::NORM_HAMMING);
        std::vector<cv::DMatch> matches;

        // Empirically, in this app, the distance of 30 or 40 is a threshold 
        int distanceSub30 = 0;
        int distanceSub40 = 0;

        // Match descriptors and compare distances
        kpMatcher.match(descriptors1, descriptors2, matches);
        for (int i = 0; i < matches.size(); i++) {
            if (matches[i].distance > 40) {
                continue;
            }
            if (matches[i].distance <= 30) {
                distanceSub30++;
            }
            distanceSub40++;
        }

        // Only one condition check for 90% of matches
        if(distanceSub30 < 15){
            return;
        }
        if(distanceSub30 >= 30){
            std::cout << "  Image '" << imgName1 << "' is a copy of image '"<< imgName2 << "'" << std::endl;
            std::cout << "  Sub 30 = " << distanceSub30 << "; Sub 40 = "<< distanceSub40 << std::endl;
            return;
        }
        if(distanceSub40 >= 30){
            std::cout << "      Image '" << imgName1 << "' might be a copy of image '"<< imgName2 << "'" << std::endl;
            std::cout << "      Sub 30 = " << distanceSub30 << "; Sub 40 = "<< distanceSub40 << std::endl;          
        }        
    }
}