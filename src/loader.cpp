#include "loader.hpp"

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
    FILE* bytes = fopen(image->m_path, "rb");

    // Error handle if the image does not open
    if (!bytes) {
        std::cout << "Failed to open image file." << std::endl;
        return;
    }

    // Create a decoder
    spng_ctx* ctx = spng_ctx_new(0);
    spng_set_png_file(ctx, bytes);

    // Get the image size
    spng_decoded_image_size(ctx, SPNG_FMT_PNG, &image->m_size);

    // Get the image dimensions
    spng_ihdr info;
    spng_get_ihdr(ctx, &info);
    image->m_width  = info.width;
    image->m_height = info.height;

    // Allocate space for image
    image->m_data.resize(image->m_size);
    uint8_t* data = image->m_data.data();

    // Setup progressive decode
    spng_decode_image(ctx, data, image->m_size, SPNG_FMT_PNG,
                      SPNG_DECODE_PROGRESSIVE);

    // We want to support flipping images so we decode row by row
    uint32_t stride = image->m_size / info.height;

    for (uint32_t idx : std::views::iota(0UL, info.height)) {
        spng_decode_row(ctx,
                        data + (m_flip ? info.height - idx - 1 : idx) * stride,
                        stride);
    }

    // Free memory and close file
    spng_ctx_free(ctx);
    fclose(bytes);

    // Notify next worker we are finished
    m_num_active_threads--;
    m_num_active_threads.notify_one();
}
