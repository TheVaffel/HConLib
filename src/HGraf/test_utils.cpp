#include <random>
#include <functional>

#include "./test_utils.hpp"

std::function<float()> getUniformFloatRandom(const std::string& seed_input,
                                             float lower,
                                             float higher) {

    std::seed_seq seed(seed_input.begin(), seed_input.end());
    std::mt19937 generator(seed);
    std::uniform_real_distribution<float> distribution(lower, higher);
    return std::bind(distribution, generator);
}

std::function<unsigned char()> getUniformByteRandom(const std::string& seed_input) {
    std::seed_seq seed(seed_input.begin(), seed_input.end());
    std::mt19937 generator(seed);
    std::uniform_int_distribution<unsigned char> distribution(0, 255);
    return std::bind(distribution, generator);
}

std::unique_ptr<hg::Image<float>> getRandomFloatImage(int width, int height) {
    std::function<unsigned char()> dist = getUniformByteRandom("some_seed");
    std::unique_ptr<hg::Image<float>> im = std::make_unique<hg::Image<float>>(width, height);
    for (int i = 0; i < im->getHeight(); i++) {
        for (int j = 0; j < im->getWidth(); j++) {
            im->setPixel(j, i, dist() / 256.f);
        }
    }

    return im;
}


template <> float getMaxImageValue<float>() { return 1.0f; }
template <> unsigned char getMaxImageValue<unsigned char>()  { return 255; }
