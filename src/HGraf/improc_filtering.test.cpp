#include <HGraf/improc.hpp>
#include <HGraf/improc_filtering.hpp>

#include <flawed_assert.hpp>
#include <flawed_testing.hpp>

#include <random>
#include <cstring>

using namespace flawed;

std::unique_ptr<hg::Image<float>> im;

const unsigned int WIDTH = 640;
const unsigned int HEIGHT = 480;

createTestSuite("Improc filtering tests", [] {

    beforeEach([] {
        im = std::make_unique<hg::Image<float>>(WIDTH, HEIGHT);
    });

    createTest("Kernel construction succeeds", [] {
        hg::ImageFilter<3> filter( { { 1, 2, 3 },
                                     { 4, 5, 6 },
                                     { 7, 8, 9 } });

    });

    createTest("Kernel has right data", [] {
        hg::ImageFilter<3> filter( { { 1, 2, 3 },
                                     { 4, 5, 6 },
                                     { 7, 8, 9 } });

        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                fl_assert_eq(filter[i][j], i * 3 + j + 1);
            }
        }

    });

    createTest("Filtering returns image of same dimensions and stride", [] {
        std::unique_ptr<hg::Image<float>> res = hg::filterImage<float, 3>(*im, { { 1, 2, 3 },
                                                                                 { 4, 5, 6 },
                                                                                 { 7, 8, 9 } });
        fl_assert_eq(res->getWidth(), im->getWidth());
        fl_assert_eq(res->getHeight(), im->getHeight());
        fl_assert_eq(res->getByteStride(), im->getByteStride());
    });

    createTest("Identity filtering leaves image unmodified", [] {

        const char* seed_input = "I am a seed";
        std::seed_seq seed(seed_input, seed_input + strlen(seed_input));
        std::unique_ptr<std::mt19937> generator = std::make_unique<std::mt19937>(seed);
        std::uniform_real_distribution<float> distribution(0, 256);

        for (auto it = im->begin(); it != im->end(); ++it) {
            *it = distribution(*generator);
        }

        std::unique_ptr<hg::Image<float>> res = hg::filterImage<float, 1>(*im, { { 1 } });

        for (int i = 0; i < im->getHeight(); i++) {
            for (int j = 0; j < im->getWidth(); j++) {
                fl_assert_eq(res->getPixel(j, i), im->getPixel(j, i));

            }
        }
    });

});
