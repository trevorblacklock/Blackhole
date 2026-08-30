#include "loader.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void ImageLoader::run() {
    // Determine the number of readers needed
    uint16_t maxThreads = std::thread::hardware_concurrency();

    // Loop through all image files, careful to not overwhelm the processor
    for (Image& image : m_images) {
        // Block if active threads are equal to the maximum
        m_num_active_threads.wait(maxThreads);
        // Start thread
        m_threads.emplace_back(&ImageLoader::reader, this, &image);
        // Increment number of active threads
        m_num_active_threads++;
    }

    // Join all the workers
    for (std::thread& thread : m_threads)
        thread.join();

    // Clear thread data
    m_threads.clear();
}

void ImageLoader::reader(Image* image) {
    // Load the file
    auto bytes    = stbi_load(image->m_path, &image->m_width, &image->m_height,
                              &image->m_channels, 3);
    image->m_size = image->m_width * image->m_height * image->m_channels;
    image->m_data.assign(bytes, bytes + image->m_size);
    stbi_image_free(bytes);

    // Notify next worker we are finished
    m_num_active_threads--;
    m_num_active_threads.notify_one();
}
