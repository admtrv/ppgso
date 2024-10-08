// Example raw1_gradient
// - Illustrates the concept of a framebuffer
// - We do not really need any libraries or hardware to do computer graphics
// - In this case the framebuffer is simply saved as a raw RGB image

#include <iostream>
#include <fstream>
#include <cstdint>

// Size of the framebuffer
const unsigned int SIZE = 512;

// A simple RGB struct will represent a pixel in the framebuffer
// Note: each channel is range <0, 255>
//       uint8_t or char would be signed so the range would be <-128, 127>
struct Pixel {
  uint8_t r,g,b;
};

int main() {
  // Initialize a framebuffer
  // NOTE: The framebuffer is allocated on the heap and needs to be freed manually.
  // Allocating on the stack may cause stack overflow depending on the size of the image.
  auto framebuffer = new Pixel[SIZE][SIZE];

  // Example: Generate a simple gradient
  for (unsigned int x = 0; x < SIZE; ++x) {
    for (unsigned int y = 0; y < SIZE; ++y) {
      framebuffer[x][y].r = (uint8_t)(x / 2);
      framebuffer[x][y].g = (uint8_t)(y / 2);
      framebuffer[x][y].b = 0;
    }
  }

  // Task1: Load RAW image file here instead

  // save the raw image to a file
  std::cout << "Generating result.raw file ..." << std::endl;
  std::ofstream raw("raw1_gradient.raw", std::ios::binary);
  raw.write(reinterpret_cast<char *>(framebuffer), sizeof(Pixel)*SIZE*SIZE);
  raw.close();

  delete[](framebuffer);

  std::cout << "Done." << std::endl;
  return EXIT_SUCCESS;
}
