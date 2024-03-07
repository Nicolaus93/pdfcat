#include <iostream>
#include <opencv2/opencv.hpp>
#include <sys/ioctl.h> // for ioctl
#include <unistd.h>    // for STDOUT_FILENO
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>
#include <poppler/cpp/poppler-page-renderer.h>
#include <poppler/cpp/poppler-image.h>

using namespace std;
using namespace cv;
using namespace poppler;

cv::Mat popplerImageToCvMat(const poppler::image& img) {
    if (!img.is_valid()) {
        return cv::Mat();
    }

    int width = img.width();
    int height = img.height();
    int format = CV_8UC4; // Assuming the poppler::image is in format ARGB

    // Since we cannot directly use the data pointer from poppler::image in cv::Mat constructor
    // due to the const correctness and type mismatch, we'll create a cv::Mat and then copy the data.
    cv::Mat mat(height, width, format);

    // Copy the data into the cv::Mat. Note that poppler::image data is stored in ARGB format,
    // but OpenCV expects data in BGR(A) format, so a conversion is necessary.
    if (img.const_data()) {
        // Copy the data into mat. Note the need for a reinterpret_cast here to match the expected pointer type.
        std::memcpy(mat.data, reinterpret_cast<const unsigned char*>(img.const_data()), width * height * 4);

        // Convert from ARGB (used by poppler) to BGRA (expected by OpenCV).
        cv::cvtColor(mat, mat, cv::COLOR_BGRA2BGR);
    }

    return mat.clone(); // Return a clone to ensure the data is continuous.
}

// Function to get the size of the terminal
void getTerminalSize(int& width, int& height) {
    struct winsize size;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
    width = size.ws_col;
    height = size.ws_row;
}

// Function to convert an image to ASCII art with ANSI terminal colors
void imageToAscii(const Mat& img) {
    int terminal_width, terminal_height;
    getTerminalSize(terminal_width, terminal_height);
    
    // Resize the image to match the terminal width while maintaining aspect ratio
    int width = terminal_width; // Adjust width to fit within the terminal
    int height = terminal_height - 1;
    Mat resized_img;
    resize(img, resized_img, Size(width, height), 0, 0, INTER_AREA);

    // Iterate through each pixel in the image and print the corresponding ANSI color code
    for (int i = 0; i < resized_img.rows; ++i) {
        for (int j = 0; j < resized_img.cols; ++j) {
            Vec3b pixel = resized_img.at<Vec3b>(i, j);
            cout << "\033[48;2;" << (int)pixel[2] << ";" << (int)pixel[1] << ";" << (int)pixel[0] << "m "; // Print colored space
        }
        cout << "\033[0m" << endl; // Reset color after each row
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <file_path>" << endl;
        return 1;
    }

    string file_path = argv[1];
    string file_extension = file_path.substr(file_path.find_last_of(".") + 1);
    transform(file_extension.begin(), file_extension.end(), file_extension.begin(), ::tolower);

    if (file_extension == "pdf") {
     
        // Load the PDF document
        auto doc = poppler::document::load_from_file(file_path);
        if (!doc) {
            cerr << "Failed to load PDF document." << endl;
            return 1;
        }
    
        // Create a page renderer
        poppler::page_renderer renderer;
        
        // Render the first page of the document
        auto page = doc->create_page(0);
        if (!page) {
            cerr << "Failed to load the first page." << endl;
            return 1;
        }
        
        // Render the page to a poppler::image
        poppler::image img = renderer.render_page(page,
                                                300, // xres
                                                300, // yres
                                                0,   // x
                                                0,   // y
                                                -1,  // width
                                                -1); // height

        // Convert the poppler::image to a cv::Mat
        cv::Mat mat = popplerImageToCvMat(img);
        // Convert and print the image as ASCII art
        imageToAscii(mat);
    } else {
        Mat img = imread(file_path);
        if (img.empty()) {
            cerr << "Error: Unable to load image!" << endl;
            return 1;
        }

        imageToAscii(img);
    }

    return 0;
}
