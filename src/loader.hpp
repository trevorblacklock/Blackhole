#ifndef LOADER_HPP_INCLUDED
#define LOADER_HPP_INCLUDED

#include <atomic>
#include <fstream>
#include <iostream>
#include <ranges>
#include <spng.h>
#include <thread>
#include <vector>

// Structure used to store image data and basic information
struct Image {
    std::vector<uint8_t> m_data;
    const char*          m_path;
    uint32_t             m_width, m_height;
    size_t               m_size;
    bool                 m_flip;

    Image(const char* path) : m_path(path) {};
};

// Class used to load multiple images concurrently
class ImageLoader {
 private:
    bool                     m_flip;
    std::atomic<uint16_t>    m_num_active_threads;
    std::vector<std::thread> m_threads;

    void reader(Image* image);
    void run();

 public:
    std::vector<Image> m_images;

    // Constructor specifying if image is flipped along y,
    // Defaults to false, meaning top left of image is 0,0 coordinate.
    template<typename Container>
    ImageLoader(Container files, bool flip = false) : m_flip(flip) {
        for (const char* path : files)
            m_images.emplace_back(Image(path));
        run();
    }

    ImageLoader(const char* file, bool flip = false) : m_flip(flip) {
        m_images.emplace_back(Image(file));
        run();
    }
};

#endif
