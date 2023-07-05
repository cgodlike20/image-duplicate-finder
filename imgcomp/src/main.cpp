#include "imgcmp.h"
#include <pqxx/pqxx>
#include <opencv2/opencv.hpp>

namespace fs = std::filesystem;

// Replace with your own db credentials and folder path
const std::string connectionStr = "dbname=database user=postgres password=postgres hostaddr=127.0.0.1 port=5432";
const std::string folderPath = "/path-to-image-folder/";

int main(int argc, char** argv )
{
    std::cout << std::endl << "Starting up..." << std::endl;

    // Init ORB keypoint detector
    cv::Ptr<cv::ORB> keypointDetector = cv::ORB::create(orb_size);

    // Init vars for detection and filename sync
    cv::Mat inputImage, descriptors, descriptorsToCompare;
    std::vector<cv::KeyPoint> keypoints;
    std::string filePath, entryToCompare;

    std::cout << std::endl << "Connecting to db..." << std::endl;
    // Connect do db
    pqxx::connection connection(connectionStr);

    try{
        std::vector<std::string> fileNames;
        std::vector<std::string> databaseNames;

        // Fetch filenames from the folder
        // and insert them into a vector
        for (const auto& entry : fs::directory_iterator(folderPath)){
            if (fs::is_regular_file(entry) && entry.path().extension() == ".png"){
                fileNames.push_back(entry.path().stem().string());
            }
        }

        // Fetch database names  
        pqxx::work transaction(connection);
        std::string sqlQuery = "SELECT name FROM public.imgcomp";
        pqxx::result result = transaction.exec(sqlQuery);

        // Insert database names into a vector
        for (const auto& row : result) {
            databaseNames.push_back(row["name"].as<std::string>());
        }

        // Compare lists to identify missing files and database entries
        std::sort(fileNames.begin(), fileNames.end());
        std::sort(databaseNames.begin(), databaseNames.end());

        std::vector<std::string> missingFiles;
        std::vector<std::string> missingEntries;

        std::set_difference(fileNames.begin(), fileNames.end(),
                            databaseNames.begin(), databaseNames.end(),
                            std::back_inserter(missingEntries));
        std::set_difference(databaseNames.begin(), databaseNames.end(),
                            fileNames.begin(), fileNames.end(),
                            std::back_inserter(missingFiles));

        // Synchronize the database with the folder
        std::cout << std::endl << "Synching database..." << std::endl << std::endl;

        for (const auto& missingFile : missingFiles){
            std::cout << "Delete from database: " << missingFile << std::endl;
            transaction.exec0("DELETE FROM public.imgcomp WHERE name = '" + missingFile + "'");
        }
        for (const auto& missingEntry : missingEntries){
            std::cout << "Add to database: " << missingEntry << std::endl;

            // Create descriptors for missingEntry
            filePath = folderPath + missingEntry + ".png";
            inputImage = cv::imread(filePath);
            keypointDetector->detectAndCompute(inputImage, cv::noArray(), keypoints, descriptors);

            // Fetch descriptors from db...
            result = transaction.exec("SELECT * FROM public.imgcomp");
            for (auto const &row: result){
                entryToCompare = row["name"].as<std::string>();
                descriptorsToCompare = ic::parseStringToMat(row["descriptors"].as<std::string>());
                //  ...And compare them with missing entry
                ic::matchDescriptors(descriptors, descriptorsToCompare, missingEntry, entryToCompare);
            }

            // Insert the new entry into the database
            sqlQuery = "INSERT INTO public.imgcomp (name, descriptors) VALUES ('" + missingEntry + "','" + ic::descriptorsToString(descriptors) + "')";
            transaction.exec(sqlQuery);
        }
        transaction.commit();
    } catch (const std::exception& e){
        std::cerr << "Error: " << e.what() << std::endl;
        connection.close();
        return 1;
    }

    connection.close();
    std::cout << std::endl << "Database synchronized" << std::endl;
    return 0;
}