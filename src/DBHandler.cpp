#include <finger_vascular_biometry/DBHandler.hpp>

bool DBHandler::Login(std::string* username, cv::Mat& img)
    {
        cv::Mat imgPre;
        //cv::Mat img, imgPre;
        //img = cv::imread("original4.png", cv::ImreadModes::IMREAD_GRAYSCALE); // Use camera instead
        Biometry::PreprocessImage(img, imgPre);
        KeypointsTuple features = Biometry::KAZEDetector(imgPre);
        std::tuple<std::string, std::vector<cv::KeyPoint>, cv::Mat> bestMatch = FindBestMatch(features);

        if (std::get<0>(bestMatch) == "")
            return false;
        else
        {
            *username = std::get<0>(bestMatch);
            return true;
        }
    }

bool DBHandler::Register(std::string username, cv::Mat& img)
    {
        cv::Mat imgPre;
        //cv::Mat img, imgPre;

        // img = cv::imread("original4.png", cv::ImreadModes::IMREAD_GRAYSCALE); // Use camera instead
        Biometry::PreprocessImage(img, imgPre);
        KeypointsTuple features = Biometry::KAZEDetector(imgPre);
        return !WriteEntry(features, username);
    }

std::tuple<std::string, std::vector<cv::KeyPoint>, cv::Mat> DBHandler::FindBestMatch(KeypointsTuple features)
    {
        sqlite3 *db;
        int rc = sqlite3_open_v2("resources/database.db", &db, SQLITE_OPEN_READONLY, NULL);
        if (rc != SQLITE_OK)
        {
            std::cerr << "DB open failed: " << sqlite3_errmsg(db) << std::endl;
            throw;
        }

        sqlite3_stmt* stmt = NULL;
        std::string query = "SELECT * FROM data";

        rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL);
        if (rc != SQLITE_OK)
        {
            std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            throw;
        }
        
        std::tuple<std::string, std::vector<cv::KeyPoint>, cv::Mat> bestMatch;
        int maxMatches = 0;
        while (true)
        {
            rc = sqlite3_step(stmt);
            if (rc == SQLITE_ROW)
            {
                int id = sqlite3_column_int(stmt, 0);
                std::tuple<std::string, std::vector<cv::KeyPoint>, cv::Mat> entry = ReadEntry(id, db);

                // Match Features /////////////////
                std::tuple<cv::Mat, int> matchKAZE = Biometry::FLANNMatcher(features, std::make_tuple(std::get<1>(entry), std::get<2>(entry)));
                ///////////////////////////////////

                if (std::get<1>(matchKAZE) > maxMatches)
                {
                    maxMatches = std::get<1>(matchKAZE);
                    bestMatch = entry;
                }
            }
            else
                break;
        }
        
        if (maxMatches > MINMATCHES) // Match found
        {
            std::cout << maxMatches << " found for user " << std::get<0>(bestMatch) << std::endl;
            return bestMatch;
        }
        else
        {
            return std::make_tuple(std::string(""), std::vector<cv::KeyPoint>(), cv::Mat());
        }
    }

std::tuple<std::string, std::vector<cv::KeyPoint>, cv::Mat> DBHandler::ReadEntry(int id, sqlite3 *e_db)
    {
        sqlite3 *db;
        int rc;

        if (e_db != NULL)
        {
            db = e_db;
        }
        else
        {
            rc = sqlite3_open_v2("resources/database.db", &db, SQLITE_OPEN_READONLY, NULL);
            if (rc != SQLITE_OK)
            {
                std::cerr << "DB open failed: " << sqlite3_errmsg(db) << std::endl;
                throw;
            }
        }
        
        sqlite3_stmt* stmt = NULL;
        std::string query = "SELECT * FROM data WHERE id=?";

        rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL);
        if (rc != SQLITE_OK)
        {
            std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            throw;
        }

        rc = sqlite3_bind_int(stmt, 1, id);
        if (rc != SQLITE_OK)
        {
            std::cerr << "bind int failed: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            throw;
        }
        
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW)
        {
            char* name = (char*)sqlite3_column_text(stmt, 1);
            std::string name_cpp = std::string(name);

            int nKeypoints = sqlite3_column_int(stmt, 2);
            char* keypointsBinary = (char*)sqlite3_column_blob(stmt, 3);

            std::vector<cv::KeyPoint> keypoints;
            for (int i = 0; i < nKeypoints; i++)
            {
                std::vector<char> kp(&keypointsBinary[i*sizeof(cv::KeyPoint)], &keypointsBinary[i*sizeof(cv::KeyPoint)] + sizeof(cv::KeyPoint));
                cv::KeyPoint* kpPtr = reinterpret_cast<cv::KeyPoint*>(&kp[0]);
                keypoints.push_back(*kpPtr);

                kp.clear();
                kp.shrink_to_fit();
            }

            int descriptorSize = sqlite3_column_bytes(stmt, 4);
            char* dPtr = (char*)sqlite3_column_blob(stmt, 4);
            std::vector<char> dData(dPtr, dPtr + descriptorSize);
            cv::Mat descriptor = DecodeKazeDescriptor(dData, nKeypoints);
            
            sqlite3_finalize(stmt);
            sqlite3_close(db);

            dData.clear();
            dData.shrink_to_fit();

            keypointsBinary = NULL;
            delete[] keypointsBinary;

            return make_tuple(name_cpp, keypoints, descriptor);
        }
        else
        {
            std::cerr << "No entry with id " << id << " was found." << std::endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            throw;
        }
    }

int DBHandler::WriteEntry(KeypointsTuple features, std::string name)
    {
        // First check if user already exists
        /////////////////////////////////////////
        sqlite3 *db = NULL;
        int rc = sqlite3_open_v2("resources/database.db", &db, SQLITE_OPEN_READWRITE, NULL);
        if (rc != SQLITE_OK)
        {
            std::cerr << "DB open failed: " << sqlite3_errmsg(db) << std::endl;
            throw;
        }

        sqlite3_stmt* stmt = NULL;
        std::string query = "SELECT * FROM data";

        rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL);
        if (rc != SQLITE_OK)
        {
            std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            throw;
        }
        
        while (true)
        {
            rc = sqlite3_step(stmt);
            if (rc == SQLITE_ROW)
            {
                char* name_c_str = (char*)sqlite3_column_text(stmt, 1);
                std::string name_str = std::string(name_c_str);

                if (name_str == name)
                {
                    return 1;
                }
            }
            else
                break;
        }

        sqlite3_finalize(stmt);

        /////////////////////////////////////////
        char* descriptorBuffer = EncodeF32Image(std::get<1>(features));
        /////////////////////////////////////////

        /////////////////////////////////////////
        int keypointCount = std::get<0>(features).size();
        char* keypointsBuffer = new char[sizeof(cv::KeyPoint)*keypointCount];
        int keypointsByteCounter = 0;
        const void* kpptr = NULL;
        kpptr = keypointsBuffer;

        for (int i = 0; i < keypointCount; i++)
        {
            char* kp = reinterpret_cast<char*>(&std::get<0>(features)[i]);
            for (int j = 0; j < sizeof(cv::KeyPoint); j++)
            {
                keypointsBuffer[keypointsByteCounter] = kp[j];
                keypointsByteCounter++;
            }    
        }   
        /////////////////////////////////////////

        /////////////////////////////////////////
        stmt = NULL;
        query = "INSERT INTO data(id, name, nKeypoints, keypoints, descriptor) VALUES(NULL, ?, ?, ?, ?)";

        rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL);
        if (rc != SQLITE_OK)
        {
            std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            throw;
        }

        rc = sqlite3_bind_text(stmt, 1, name.c_str(), -1, NULL);
        if (rc != SQLITE_OK)
        {
            std::cerr << "bind text failed: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            throw;
        }

        rc = sqlite3_bind_int(stmt, 2, keypointCount);
        if (rc != SQLITE_OK)
        {
            std::cerr << "bind int failed: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            throw;
        }

        rc = sqlite3_bind_blob(stmt, 3, keypointsBuffer, keypointsByteCounter, SQLITE_STATIC);
        if (rc != SQLITE_OK)
        {
            std::cerr << "bind 1 failed: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            throw;
        }

        rc = sqlite3_bind_blob(stmt, 4, descriptorBuffer, std::get<1>(features).rows * std::get<1>(features).cols * sizeof(float), SQLITE_STATIC);
        if (rc != SQLITE_OK)
        {
            std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            throw;
        }

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
        {
            std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            throw;
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);
        /////////////////////////////////////////

        free(descriptorBuffer);
        delete[] keypointsBuffer;

        return 0;
    }

char* DBHandler::EncodeF32Image(cv::Mat& img)
    {
        char* buffer = (char*)malloc(sizeof(float) * img.rows * img.cols);
        int ptr = 0;

        for (int i = 0; i < img.rows; i++)
        {
            for (int j = 0; j < img.cols; j++)
            {
                float val = img.at<float>(cv::Point(j, i));
                char* byteVal = reinterpret_cast<char*>(&val);
                
                for (int k = 0; k < sizeof(float); k++, ptr++)
                {
                    buffer[ptr] = byteVal[k];
                }          
            }
        }
        
        return buffer;
    }

cv::Mat DBHandler::DecodeKazeDescriptor(std::vector<char> &buffer, int nKeypoints)
    {
        int cols = 64;
        int rows = nKeypoints;

        cv::Mat img = cv::Mat::zeros(cv::Size(cols, rows), CV_32F);

        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                std::vector<char> byteVal(&buffer[(j + i*cols)*sizeof(float)], &buffer[(j + i*cols)*sizeof(float)] + sizeof(float));
                float* val = reinterpret_cast<float*>(&byteVal[0]);

                img.at<float>(cv::Point(j, i)) = *val;
                byteVal.clear();
                byteVal.shrink_to_fit();
            }       
        }

        return img;
    }
