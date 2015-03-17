#ifndef RGBDREADER_HH
#define RGBDREADER_HH

#include <vector>
#include <fstream>
#include <string>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <pcl/common/common_headers.h>


namespace RGBDReader {

/**
 *  @class Reader
 *
 *  @brief Abstract reader class to handle differente benchmarks readers.
 */
class Reader {
public:
    int width;
    int height;

    // Camera parameters
    float fx;
    float fy;
    float cx;
    float cy;
};


/**
 *  @brief Reader class for the ICL-NUIM benchmark.
 */
class ICL_NUIM_Reader : public Reader{
public:
    ICL_NUIM_Reader() {
        width = 640; 
        height = 480; 
        fx = 481.2f; 
        fy = -480.0f; 
        cx = 319.5f;
        cy = 239.5f;
    }

    /**
     * @brief Reads the content of the depth file and initializes a
     * pcl::pcl::PointCloud.
     *
     * @param filename [in] Path to a depth text file from the ICL-NUIM dataset.
     * @param cloud [in,out] Resulting cloud.
     */
    template<class OutputCloud>
    void readCloud(const std::string filename, OutputCloud &cloud) {
        cloud.resize(width * height);
        cloud.width = width;
        cloud.height = height;

        std::vector<float> depth_value(width * height);
        std::ifstream file;
        file.open(filename.c_str());

        for (size_t i = 0; i < width * height; i++) {
            file >> depth_value[i]; 
        }

        #ifdef _OPENMP
        #pragma omp parallel for shared(cloud, depth_value)
        #endif
        for (size_t i = 0; i < width * height; i++) {
            int x = i % width;
            int y = i / width;

            float X = (x - cx) / fx;
            float Y = (y - cy) / -fy;
            float Z = depth_value[i] / sqrt(X * X + Y * Y + 1);

            cloud.points[i].x = X * Z;
            cloud.points[i].y = Y * Z;
            cloud.points[i].z = Z;
        }

        file.close();
    }

    void readMat(const std::string filename, cv::Mat *img);
};


class RGBD_TUM_Reader : public Reader{
public:

    RGBD_TUM_Reader() { 
        width = 640;
        height = 480; 
        //fx = 525.0f;
        //fy = 525.0f; 
        //cx = 319.5f; 
        //cy = 239.5f;
        fx = 568.2044096400148;
        fy = 567.6742468263647;
        cx = 322.9273157602054;
        cy = 240.32862519022834;
    }

    /**
     * @brief Reads the content of the depth file and initializes a
     * pcl::pcl::PointCloud.
     *
     * @param filename [in] Path to a depth text file from the ICL-NUIM dataset.
     * @param cloud [in,out] Resulting cloud.
     */
    template<class OutputCloud>
    void readCloud(const std::string filename, OutputCloud &cloud) {
        cloud.resize(width * height);
        cloud.width = width;
        cloud.height = height;

        std::ifstream file;
        file.open(filename.c_str());

        cv::Mat img = cv::imread(filename, CV_LOAD_IMAGE_ANYDEPTH);
        width = img.cols;
        height = img.rows;

        bool dense = true;

        #ifdef _OPENMP
        #pragma omp parallel for shared(cloud, img)
        #endif
        for (size_t i = 0; i < width * height; i++) {

            float X, Y, Z;
            int x = i % width;
            int y = i / width;

            unsigned short depth = img.at<unsigned short>(i);

            // Handle non depth information
            if (depth == 0) {
                X = Y = Z = std::numeric_limits<float>::quiet_NaN();
                dense = false;
            } else {
                Z = static_cast<float>(depth) / 5000.0f;
                X = (x - cx) * Z / fx;
                Y = (y - cy) * Z / -fy;
            }

            cloud.points[i].x = X;
            cloud.points[i].y = Y;
            cloud.points[i].z = Z;
        }

        cloud.is_dense = dense;

        file.close();
    }

    void readMat(const std::string filename, cv::Mat *img);
};

} // namespace RGBDReader

#endif // RGBDREADER_HH
