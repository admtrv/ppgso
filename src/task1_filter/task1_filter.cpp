// Task 1 - Load a 512x512 image lena.raw
//        - Apply specified per-pixel transformation to each pixel
//        - Save as result.raw
#include <fstream>
#include <iostream>
#include <cstdint>

// Grayscale constants
#define GRAY_RED 0.299
#define GRAY_GREEN 0.587
#define GRAY_BLUE 0.114

// Brightness constants
#define BRIGHTNESS_FACTOR 1.5

// Size of the framebuffer
const unsigned int SIZE = 512;

// A simple RGB struct will represent a pixel in the framebuffer
struct Pixel {
    uint8_t r,g,b;
};

int main()
{
  // Initialize a framebuffer
  auto framebuffer = new Pixel[SIZE][SIZE];

  // Open file lena.raw (this is 512x512 RAW GRB format)
  std::ifstream inputFile("lena.raw", std::ios::binary);
  if (!inputFile)
  {
      std::cout << "Error while open lena.raw" << std::endl;
      delete[] framebuffer;
      return EXIT_FAILURE;
  }

  // Read data to framebuffer and close the file
  inputFile.read(reinterpret_cast<char*>(framebuffer), SIZE * SIZE * sizeof(Pixel));
  inputFile.close();

  // Traverse the framebuffer
  for (unsigned int y = 0; y < SIZE; y++) {
      for (unsigned int x = 0; x < SIZE; x++) {
          // Apply pixel operation
          Pixel& pixel = framebuffer[y][x];

          if (x < SIZE / 2)
          {
              auto brightness = static_cast<uint8_t>(GRAY_RED * pixel.r + GRAY_GREEN * pixel.g + GRAY_BLUE * pixel.b);

              pixel.r = brightness;
              pixel.g = brightness;
              pixel.b = brightness;
          }
          else
          {
              pixel.r = static_cast<uint8_t>(std::min(static_cast<int>(pixel.r * BRIGHTNESS_FACTOR), 255));
              pixel.g = static_cast<uint8_t>(std::min(static_cast<int>(pixel.g * BRIGHTNESS_FACTOR), 255));
              pixel.b = static_cast<uint8_t>(std::min(static_cast<int>(pixel.b * BRIGHTNESS_FACTOR), 255));
          }

      }
  }

  // Open file result.raw
  std::ofstream outputFile("result.raw", std::ios::binary);
  if (!outputFile) {
      std::cout << "Error while open result.raw" << std::endl;
      delete[] framebuffer;
      return EXIT_FAILURE;
  }

  std::cout << "Generating result.raw file ..." << std::endl;

  // Write the framebuffer to the file and close it
  outputFile.write(reinterpret_cast<const char*>(framebuffer), SIZE * SIZE * sizeof(Pixel));
  outputFile.close();

  std::cout << "Done." << std::endl;
  delete[] framebuffer;
  return EXIT_SUCCESS;
}
