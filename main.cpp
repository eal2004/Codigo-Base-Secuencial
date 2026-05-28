#include <iostream>
#include <vector>
#include <fstream>
#include <string>

const int WIDTH = 7680;
const int HEIGHT = 4320;
const int MAX_ITER = 256;

struct Pixel {
    unsigned char r, g, b;
};

void generateMandelbrot(std::vector<Pixel>& image) {
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            double pr = 1.5 * (x - WIDTH / 2.0) / (0.5 * WIDTH) - 0.5;
            double pi = (y - HEIGHT / 2.0) / (0.5 * HEIGHT);
            double newRe = 0, newIm = 0, oldRe = 0, oldIm = 0;
            int i;

            for (i = 0; i < MAX_ITER; ++i) {
                oldRe = newRe;
                oldIm = newIm;
                newRe = oldRe * oldRe - oldIm * oldIm + pr;
                newIm = 2 * oldRe * oldIm + pi;
                if ((newRe * newRe + newIm * newIm) > 4) break;
            }

            int index = y * WIDTH + x;
            image[index].r = i % 256;
            image[index].g = (i * 2) % 256;
            image[index].b = (i * 5) % 256;
        }
    }
}

void applyGaussianBlur(const std::vector<Pixel>& input, std::vector<Pixel>& output) {
    double kernel[5][5] = {
        {1, 4, 7, 4, 1},
        {4, 16, 26, 16, 4},
        {7, 26, 41, 26, 7},
        {4, 16, 26, 16, 4},
        {1, 4, 7, 4, 1}
    };
    double kernelSum = 273.0;

    for (int y = 2; y < HEIGHT - 2; ++y) {
        for (int x = 2; x < WIDTH - 2; ++x) {
            double sumR = 0, sumG = 0, sumB = 0;

            for (int ky = -2; ky <= 2; ++ky) {
                for (int kx = -2; kx <= 2; ++kx) {
                    int pixelIdx = (y + ky) * WIDTH + (x + kx);
                    sumR += input[pixelIdx].r * kernel[ky + 2][kx + 2];
                    sumG += input[pixelIdx].g * kernel[ky + 2][kx + 2];
                    sumB += input[pixelIdx].b * kernel[ky + 2][kx + 2];
                }
            }

            int currentIdx = y * WIDTH + x;
            output[currentIdx].r = static_cast<unsigned char>(sumR / kernelSum);
            output[currentIdx].g = static_cast<unsigned char>(sumG / kernelSum);
            output[currentIdx].b = static_cast<unsigned char>(sumB / kernelSum);
        }
    }
}

void savePPM(const std::string& filename, const std::vector<Pixel>& image) {
    std::ofstream file(filename, std::ios::binary);
    file << "P6\n" << WIDTH << " " << HEIGHT << "\n255\n";
    for (const auto& pixel : image) {
        file << pixel.r << pixel.g << pixel.b;
    }
    file.close();
}

int main() {
    std::vector<Pixel> image(WIDTH * HEIGHT);
    std::vector<Pixel> blurredImage(WIDTH * HEIGHT);

    std::cout << "Iniciando Tarea A: Generando Mandelbrot 8K..." << std::endl;
    generateMandelbrot(image);

    std::cout << "Iniciando Tarea B: Aplicando convolucion 2D pesada..." << std::endl;
    applyGaussianBlur(image, blurredImage);

    std::cout << "Guardando resultado final..." << std::endl;
    savePPM("fractal_procesado.ppm", blurredImage);

    std::cout << "Ejecucion secuencial finalizada." << std::endl;
    return 0;
}