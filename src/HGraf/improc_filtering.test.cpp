#include <HGraf/improc.hpp>
#include <HGraf/improc_filtering.hpp>

#include <FlatAlg.hpp>

#include <flawed_assert.hpp>
#include <flawed_testing.hpp>

#include "./test_utils.hpp"

using namespace flawed;

std::unique_ptr<hg::Image<float>> im;

const unsigned int WIDTH = 640;
const unsigned int HEIGHT = 480;

std::unique_ptr<flawed::TestSuite> test_suite = createTestSuite("Improc filtering tests", [] {
    beforeAll([] {
        flawed::registerComparator<falg::Matrix<3, 1>, falg::FlMatrixComparator<3, 1>>();
    });

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

    createTest("Filter scales correctly", [] {
        constexpr int filter_size = 3;

        hg::ImageFilter<filter_size> filter( { { 1, 2, 3 },
                                               { 3, 4, 6 },
                                               { 6, 2, 9 } } );

        const float factor = 0.7f;

        hg::ImageFilter<filter_size> filter0 = filter * factor;
        hg::ImageFilter<filter_size> filter1 = factor * filter;

        for (int i = 0; i < filter_size; i++) {
            for (int j = 0; j < filter_size; j++) {
                fl_assert_tolerance(filter0[i][j], filter[i][j] * factor, 1e-6);
                fl_assert_tolerance(filter1[i][j], filter[i][j] * factor, 1e-6);
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

        auto dist = getUniformFloatRandom("someSeed", 0, 256);

        for (auto it = im->begin(); it != im->end(); ++it) {
            *it = dist();
        }

        std::unique_ptr<hg::Image<float>> res = hg::filterImage<float, 1>(*im, { { 1 } });

        for (int i = 0; i < im->getHeight(); i++) {
            for (int j = 0; j < im->getWidth(); j++) {
                fl_assert_eq(res->getPixel(j, i), im->getPixel(j, i));

            }
        }
    });

    createTest("Block blur takes average inside frame of image", [] {
        auto dist = getUniformFloatRandom("someSeed", 0, 256);

        hg::Image<falg::Vec3> newIm(WIDTH, HEIGHT);
        for (auto it = newIm.begin(); it != newIm.end(); ++it) {
            for (int i = 0; i < 3; i++) {
                (*it)[i] = dist();
            }
        }

        hg::ImageFilter<3> filter = { { 1, 1, 1 }, { 1, 1, 1 }, { 1, 1, 1 } };
        filter = filter * (1.f / 9);

        std::unique_ptr<hg::Image<falg::Vec3>> blurred =
            hg::filterImage(newIm, filter);


        for (int i = 1; i < newIm.getHeight() - 1; i++) {
            for (int j = 1; j < newIm.getWidth() - 1; j++) {
                falg::Vec3 sum;
                for (int k = -1; k <= 1; k++) {
                    for (int l = -1; l <= 1; l++) {
                        sum += newIm.getPixel(j + l, i + k);
                    }
                }

                sum /= 9;

                fl_assert_tolerance(sum, blurred->getPixel(j, i), 0.001);
            }
        }
    });


    createTest("Left shift filter works with kernel sizes 3, 5, 7, 9", [] {

        hg::Image<float> newIm(WIDTH / 4, HEIGHT / 4);
        auto dist = getUniformFloatRandom("someSeed", 0, 256);

        for (auto it = newIm.begin(); it != newIm.end(); ++it) {
            *it = dist();
        }

        hg::ImageFilter<3> filter3 = hg::ImageFilter<3>::zeros();
        hg::ImageFilter<5> filter5 = hg::ImageFilter<5>::zeros();
        hg::ImageFilter<7> filter7 = hg::ImageFilter<7>::zeros();
        hg::ImageFilter<9> filter9 = hg::ImageFilter<9>::zeros();

        filter3[3 / 2][3 / 2 - 1] = 1;
        filter5[5 / 2][5 / 2 - 1] = 1;
        filter7[7 / 2][7 / 2 - 1] = 1;
        filter9[9 / 2][9 / 2 - 1] = 1;

        std::unique_ptr<hg::Image<float>> res3 = hg::filterImage(newIm, filter3);
        std::unique_ptr<hg::Image<float>> res5 = hg::filterImage(newIm, filter5);
        std::unique_ptr<hg::Image<float>> res7 = hg::filterImage(newIm, filter7);
        std::unique_ptr<hg::Image<float>> res9 = hg::filterImage(newIm, filter9);

        for (int i = 0; i < newIm.getHeight(); i++) {
            for (int j = 1; j < newIm.getWidth(); j++) {
                float p = newIm.getPixel(j - 1, i);

                fl_assert_eq(p, res3->getPixel(j, i));
                fl_assert_eq(p, res5->getPixel(j, i));
                fl_assert_eq(p, res7->getPixel(j, i));
                fl_assert_eq(p, res9->getPixel(j, i));
            }
        }

    });

    createTest("Gaussian filter has derivatives away from center in cardinal directions and has sum one", [] {
        constexpr unsigned int s = 7;

        int mid = s / 2;

        hg::ImageFilter<s> g = hg::ImageFilter<7>::gaussianApprox();

        double sum = 0;

        for (int i = 0; i < s; i++) {
            for (int j = 0; j < s; j++) {
                sum += g[i][j];

                if (i >= s - 1 || j >= s - 1) {
                    continue;
                }

                float diffx = g[i][j + 1] - g[i][j];
                float diffy = g[i + 1][j] - g[i][j];

                if (j >= mid) {
                    fl_assert_lt(diffx, 0);
                } else {
                    fl_assert_gt(diffx, 0);
                }

                if (i >= mid) {
                    fl_assert_lt(diffy, 0);
                } else {
                    fl_assert_gt(diffy, 0);
                }

            }
        }

        fl_assert_tolerance(sum, 1.0, 1e-6);
    });
 });
