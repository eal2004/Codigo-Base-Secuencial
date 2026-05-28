#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <omp.h>

const int WIDTH = 7680;
const int HEIGHT = 4320;
const int MAX_ITER = 256;

struct Pixel {
    unsigned char r, g, b;
};

void generateMandelbrot(std::vector<Pixel>& image) {
    #pragma omp parallel for schedule(dynamic, 16)
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
                if (newRe * newRe + newIm * newIm > 4) break;
            }

            int index = y * WIDTH + x;
            image[index].r = i % 256;
            image[index].g = i * 2 % 256;
            image[index].b = i * 5 % 256;
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

#pragma omp parallel for schedule(static)
    for (int y = 2; y < HEIGHT - 2; ++y) {
#pragma omp simd
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

void calculateHistogramAtomic(const std::vector<Pixel>& image, std::vector<int>& histR, std::vector<int>& histG, std::vector<int>& histB) {
    #pragma omp parallel for
    for (size_t i = 0; i < image.size(); ++i) {
        #pragma omp atomic
        histR[image[i].r]++;

        #pragma omp atomic
        histG[image[i].g]++;

        #pragma omp atomic
        histB[image[i].b]++;
    }
}

void calculateHistogramReduction(const std::vector<Pixel>& image, std::vector<int>& histR, std::vector<int>& histG, std::vector<int>& histB) {
    int* ptrR = histR.data();
    int* ptrG = histG.data();
    int* ptrB = histB.data();

    #pragma omp parallel for reduction(+:ptrR[:256], ptrG[:256], ptrB[:256])
    for (size_t i = 0; i < image.size(); ++i) {
        ptrR[image[i].r]++;
        ptrG[image[i].g]++;
        ptrB[image[i].b]++;
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

    double startMandelbrot = omp_get_wtime();
    std::cout << "Iniciando Tarea A: Generando Mandelbrot 8K..." << std::endl;
    generateMandelbrot(image);
    double endMandelbrot = omp_get_wtime();
    std::cout << "Tiempo Tarea A: " << endMandelbrot - startMandelbrot << " segundos." << std::endl;

    double startBlur = omp_get_wtime();
    std::cout << "Iniciando Tarea B: Aplicando convolucion 2D pesada..." << std::endl;
    applyGaussianBlur(image, blurredImage);
    double endBlur = omp_get_wtime();
    std::cout << "Tiempo Tarea B: " << endBlur - startBlur << " segundos." << std::endl;

    std::vector<int> histR_atomic(256, 0), histG_atomic(256, 0), histB_atomic(256, 0);
    std::vector<int> histR_reduct(256, 0), histG_reduct(256, 0), histB_reduct(256, 0);

    double startAtomic = omp_get_wtime();
    std::cout << "Iniciando Histograma Atomico..." << std::endl;
    calculateHistogramAtomic(blurredImage, histR_atomic, histG_atomic, histB_atomic);
    double endAtomic = omp_get_wtime();
    std::cout << "Tiempo Histograma Atomico: " << endAtomic - startAtomic << " segundos." << std::endl;

    double startReduct = omp_get_wtime();
    std::cout << "Iniciando Histograma Reduccion..." << std::endl;
    calculateHistogramReduction(blurredImage, histR_reduct, histG_reduct, histB_reduct);
    double endReduct = omp_get_wtime();
    std::cout << "Tiempo Histograma Reduccion: " << endReduct - startReduct << " segundos." << std::endl;

    std::cout << "Guardando resultado final..." << std::endl;
    savePPM("fractal_procesado_paralelo.ppm", blurredImage);

    std::cout << "Ejecucion paralela finalizada." << std::endl;
    return 0;
}