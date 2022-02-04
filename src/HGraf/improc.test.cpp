
#include <flawed_assert.hpp>
#include <flawed_testing.hpp>

#include <HGraf/improc.hpp>
#include <HGraf/improc_diff.hpp>

#include <array>

#include "./test_utils.hpp"

const unsigned int IMAGE_WIDTH = 640;
const unsigned int IMAGE_HEIGHT = 360;

std::unique_ptr<TestSuite> test_suite = createTestSuite("Image tests", [] {

    beforeAll([] {
        flawed::registerComparator<hg::Image<float>, hg::ImageMaxDiffComparator<float>>();
    });

    createTest("Image construction works fine", [] {
        hg::Image<float> image(123, 123);
    });

    createTest("Returns correct dimensions", [] {
        const hg::Image<float> im(IMAGE_WIDTH, IMAGE_HEIGHT);

        fl_assert_eq(im.getWidth(), IMAGE_WIDTH);
        fl_assert_eq(im.getHeight(), IMAGE_HEIGHT);
    });

    createTest("Returns bytestride IMAGE_WIDTH * sizeof(Type)", [] {
        const hg::Image<std::array<float, 3>> im(IMAGE_WIDTH, IMAGE_HEIGHT);

        fl_assert_eq(im.getByteStride(), IMAGE_WIDTH * sizeof(std::array<float, 3>));
    });

    createTest("Returns byteStride equal to input byteStride", [] {

        unsigned int byteStride = sizeof(std::array<float, 3>) * (IMAGE_WIDTH + 2);
        const hg::Image<std::array<float, 3>> im(IMAGE_WIDTH, IMAGE_HEIGHT, byteStride);

        fl_assert_eq(im.getByteStride(), byteStride);
    });

    createTest("Throws on byteStride not divisible by element size", [] {
        int byteStride = sizeof(std::array<float, 3>) * (IMAGE_WIDTH + 2) + 1;

        fl_assert_throwing(([byteStride] {
            const hg::Image<std::array<float, 3>> im(IMAGE_WIDTH, IMAGE_HEIGHT, byteStride);
        }));
    });

    createTest("Throws on byteStride less than what is necessary for width", [] {
        int byteStride = sizeof(std::array<float, 3>) * (IMAGE_WIDTH - 3);
        fl_assert_throwing(([byteStride] {
            const hg::Image<std::array<float, 3>> im(IMAGE_WIDTH, IMAGE_HEIGHT, byteStride);
        }));
    });

    createTest("Iterator is created without error", [] {
        hg::Image<float> im(IMAGE_WIDTH, IMAGE_HEIGHT);
        im.begin();
        im.end();
    });

    createTest("Iterator iterates through correct number of pixels", [] {
        hg::Image<float> im(IMAGE_WIDTH, IMAGE_HEIGHT);
        unsigned int count = 0;
        for (hg::Image<float>::iterator it = im.begin(); it != im.end(); ++it) {
            count++;
        }

        fl_assert_eq(count, IMAGE_WIDTH * IMAGE_HEIGHT);
    });

    createTest("Constant iterator also iterates through correct number of pixels", [] {
        const hg::Image<float> im(IMAGE_WIDTH, IMAGE_HEIGHT);
        unsigned int count = 0;
        for (hg::Image<float>::const_iterator it = im.begin(); it != im.end(); ++it) {
            count++;
        }

        fl_assert_eq(count, IMAGE_WIDTH * IMAGE_HEIGHT);
    });

    createTest("Iterator skips pixels outside width", [] {
        hg::Image<float> im(IMAGE_WIDTH, IMAGE_HEIGHT, sizeof(float) * (IMAGE_WIDTH + 10));
        unsigned int count = 0;
        for (hg::Image<float>::iterator it = im.begin(); it != im.end(); ++it) {
            count++;
        }

        fl_assert_eq(count, IMAGE_WIDTH * IMAGE_HEIGHT);
    });

    createTest("Fill sets all elements to correct value", [] {
        hg::Image<std::array<float, 3>> im(IMAGE_WIDTH, IMAGE_HEIGHT);

        std::array<float, 3> fill_value = { 1, 2, 3 };

        im.fill(fill_value);

        for (hg::Image<std::array<float, 3>>::iterator it = im.begin(); it != im.end(); ++it) {
            fl_assert_eq(*it, fill_value);
        }
    });

    createTest("Iterators also fill with correct value", [] {
        hg::Image<int> im(IMAGE_WIDTH, IMAGE_HEIGHT);

        int i = 0;
        for (hg::Image<int>::iterator it = im.begin(); it != im.end(); ++it) {
            *it = i;
            i++;
        }

        i = 0;
        for (hg::Image<int>::iterator it = im.begin(); it != im.end(); ++it) {
            fl_assert_eq(*it, i);
            i++;
        }
    });

    createTest("Identity transformation gives same image", [] {
        std::unique_ptr<hg::Image<float>> im = getRandomFloatImage(IMAGE_WIDTH, IMAGE_HEIGHT);

        std::unique_ptr<hg::Image<float>> res = im->transform<float>([] (float f) { return f; });

        fl_assert_tolerance(*im, *res, 1e-10);
    });

    createTest("Gray transform gives grayed image", [] {
        std::unique_ptr<hg::Image<std::array<float, 3>>> im = getRandomFloatArrayImage<3>(IMAGE_WIDTH, IMAGE_HEIGHT);

        std::unique_ptr<hg::Image<float>> res = im->transform<float>([] (const std::array<float, 3>& arr) {
            return (2 * arr[0] + 3 * arr[1] + arr[2]) / 6.0f;
        });

        for (int i = 0; i < im->getHeight(); i++) {
            for (int j = 0; j < im->getWidth(); j++) {
                std::array<float, 3> pix = im->getPixel(j, i);
                fl_assert_tolerance(res->getPixel(j, i), (pix[0] * 2 + pix[1] * 3 + pix[2]) / 6.f, 1e-7);
            }
        }
    });
});
