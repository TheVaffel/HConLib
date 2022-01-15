
#include <flawed_assert.hpp>
#include <flawed_testing.hpp>

#include <HGraf/improc.hpp>

#include <array>

const unsigned int IMAGE_WIDTH = 640;
const unsigned int IMAGE_HEIGHT = 360;

createTestSuite("Image tests", [] {

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
});
