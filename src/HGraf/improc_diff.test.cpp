#include <flawed_testing.hpp>
#include <flawed_assert.hpp>

#include <HGraf/improc_diff.hpp>
#include <HGraf/improc_ssim.hpp>

#include "./test_utils.hpp"

const int IMAGE_WIDTH = 480;
const int IMAGE_HEIGHT = 360;

std::unique_ptr<hg::Image<std::array<float, 3>>> image1;
hg::Image<std::array<float, 3>> image2(0, 0);

std::unique_ptr<TestSuite> test_suite = createTestSuite("improc_diff", [] {

    beforeEach([] {
        image1 = getRandomFloatArrayImage<3>(IMAGE_WIDTH, IMAGE_HEIGHT);
        image2 = *image1;
    });

    createTest("ImgMaxDiff is 0 for equal images", [] {
        hg::ImageMaxDiffComparator<std::array<float, 3>> comparator;

        float diff = comparator.compare(*image1, image2);

        fl_assert_eq(diff, 0);
    });

    createTest("ImgMaxDiff is 1 for images with one pixel of difference 1", [] {
        int some_x = 80;
        int some_y = 90;

        std::array<float, 3> some_value_low = image1->getPixel(some_x, some_y);
        std::array<float, 3> some_value_high = image1->getPixel(some_x, some_y);

        some_value_low[1] = 0.0f;
        some_value_high[1] = 1.0f;

        image1->setPixel(80, 90, some_value_low);
        image2.setPixel(80, 90, some_value_high);

        hg::ImageMaxDiffComparator<std::array<float, 3>> comparator;
        float diff = comparator.compare(*image1, image2);

        fl_assert_eq(diff, 1.0);
    });

    createTest("SSIM is 1 for equal images", [] {

        std::function<float(const std::array<float, 3>&)> func = [] (const std::array<float, 3>& arr) {
            return (arr[0] * 2 + arr[1] * 3 + arr[3]) / 6;
        };

        auto im1_grey = image1->transform(func);
        auto im2_grey = image2.transform(func);

        float diff = hg::computeSSIM(*im1_grey, *im2_grey);

        fl_assert_eq(diff, 1);
    });

    createTest("SSIM is close to -1 for inverted uniformly random images", [] {
        hg::ImageSSIMComparator<float> comparator;

        std::function<float(const std::array<float, 3>&)> func = [] (const std::array<float, 3>& arr) {
            return (arr[0] * 2 + arr[1] * 3 + arr[3]) / 6;
        };

        auto im1_grey = image1->transform(func);
        auto im2_inv = image2.transform(func)->transform<float>([] (float f) { return 1 - f; });

        float diff = hg::computeSSIM(*im1_grey, *im2_inv);

        fl_assert_tolerance(diff, -1.f, 0.01f);

    });
});
