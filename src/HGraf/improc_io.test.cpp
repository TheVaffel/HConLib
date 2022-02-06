#include <HGraf/improc_io.hpp>

#include <flawed_assert.hpp>
#include <flawed_testing.hpp>

#include <fstream>
#include <filesystem>
#include <limits>

#include <HGraf/improc_diff.hpp>

#include "./test_utils.hpp"

const int IMAGE_WIDTH = 480;
const int IMAGE_HEIGHT = 360;

const std::string IMAGE_FILE_NAME = "some_file.png";
const std::filesystem::path IMAGE_PATH = std::filesystem::path(IMAGE_FILE_NAME);

void cleanup() {
    if (std::filesystem::exists(IMAGE_PATH)) {
        std::filesystem::remove(IMAGE_PATH);
    }
}

std::unique_ptr<TestSuite> test_suite = createTestSuite("Image IO", [] {

    beforeAll([] {
        flawed::registerComparator<hg::Image<float>,
                                   hg::ImageMaxDiffComparator<float>>();
        flawed::registerComparator<hg::Image<std::array<float, 3>>,
                                   hg::ImageMaxDiffComparator<std::array<float, 3>>>();
        flawed::registerComparator<hg::Image<std::array<float, 4>>,
                                   hg::ImageMaxDiffComparator<std::array<float, 4>>>();
        flawed::registerComparator<hg::Image<std::array<unsigned char, 4>>,
                                   hg::ImageMaxDiffComparator<std::array<unsigned char, 4>>>();
        flawed::registerComparator<hg::Image<falg::Vector<4>>,
                                   hg::ImageMaxDiffComparator<falg::Vector<4>>>();
    });

    afterEach([] {
        cleanup();
    });

    createTest("writeImage creates a file", [] {

        hg::Image<float> im(IMAGE_WIDTH, IMAGE_HEIGHT);

        hg::writeImage(im, IMAGE_FILE_NAME);

        std::ifstream ifs;
        ifs.open(IMAGE_FILE_NAME);

        fl_assert(ifs.is_open());
    });

    createTest("writeImage throws when file can't be created", [] {
        hg::Image<float> im(IMAGE_WIDTH, IMAGE_HEIGHT);
        fl_assert_throwing([&] { hg::writeImage(im, ""); });
    });

    createTest("writeImage writes an image with right dimensions", [] {
        hg::Image<float> im(IMAGE_WIDTH, IMAGE_HEIGHT);
        hg::writeImage(im, IMAGE_FILE_NAME);

        std::unique_ptr<OIIO::ImageInput> input = OIIO::ImageInput::open(IMAGE_FILE_NAME);
        const OIIO::ImageSpec& spec = input->spec();

        fl_assert_eq(im.getWidth(), spec.width);
        fl_assert_eq(im.getHeight(), spec.height);

        input->close();
    });

    createTest("readImage throws when file can't be opened", [] {
        fl_assert_throwing([] {
            std::unique_ptr<hg::Image<float>> image = hg::readImage<float>("");
        });
    });

    createTest("readImage reads image from writeImage with correct dimensions", [] {
        hg::Image<float> im(IMAGE_WIDTH, IMAGE_HEIGHT);
        hg::writeImage(im, IMAGE_FILE_NAME);

        std::unique_ptr<hg::Image<float>> res = hg::readImage<float>(IMAGE_FILE_NAME);

        fl_assert_eq(im.getWidth(), res->getWidth());
        fl_assert_eq(im.getHeight(), res->getHeight());
    });

    createTest("readImage results in same image as written with writeImage using float format", [] {
        std::unique_ptr<hg::Image<float>> im = getRandomFloatImage(IMAGE_WIDTH, IMAGE_HEIGHT);
        hg::writeImage(*im, IMAGE_FILE_NAME);

        std::unique_ptr<hg::Image<float>> res = hg::readImage<float>(IMAGE_FILE_NAME);

        for (int i = 0; i < im->getHeight(); i++) {
            for (int j = 0; j < im->getWidth(); j++) {
                fl_assert_tolerance(im->getPixel(j, i), res->getPixel(j, i), 1e-2);
            }
        }
    });

    createTest("read/writes 3-component float array", [] {
        std::unique_ptr<hg::Image<std::array<float, 3>>> im = getRandomFloatArrayImage<3>(IMAGE_WIDTH, IMAGE_HEIGHT);

        hg::writeImage(*im, IMAGE_FILE_NAME);
        std::unique_ptr<hg::Image<std::array<float, 3>>> res = hg::readImage<std::array<float, 3>>(IMAGE_FILE_NAME);

        fl_assert_tolerance(*im, *res, 1e-2f);
    });

    createTest("read/write with non-matching number of components throws", [] {
        auto im = getRandomFloatArrayImage<3>(IMAGE_WIDTH, IMAGE_HEIGHT);

        hg::writeImage(*im, IMAGE_FILE_NAME);

        fl_assert_throwing([] {
            hg::readImage<float>(IMAGE_FILE_NAME);
        });
    });


    createTest("read/writes 4-component byte array", [] {
        std::unique_ptr<hg::Image<std::array<unsigned char, 4>>> im = getRandomByteArrayImage<4>(IMAGE_WIDTH, IMAGE_HEIGHT);
        hg::writeImage(*im, IMAGE_FILE_NAME);
        std::unique_ptr<hg::Image<std::array<unsigned char, 4>>> res =
            hg::readImage<std::array<unsigned char, 4>>(IMAGE_FILE_NAME);

        fl_assert_tolerance(*im, *res, 2);
    });

    createTest("write 3-component float, read 3-component byte array", [] {
        auto im = getRandomFloatArrayImage<3>(IMAGE_WIDTH, IMAGE_HEIGHT);
        hg::writeImage(*im, IMAGE_FILE_NAME);
        auto res = hg::readImage<std::array<unsigned char, 3>>(IMAGE_FILE_NAME);

        auto transformed_res = res->transform<std::array<float, 3>>([] (const std::array<unsigned char, 3>& arr) {
            return std::array<float, 3> { arr[0] / 255.0f, arr[1] / 255.0f, arr[2] / 255.0f };
        });

        fl_assert_tolerance(*im, *transformed_res, 1e-2);
    });

    createTest("write/read 4-component FlatAlg vector", [] {
        auto im = getRandomVectorImage<4>(IMAGE_WIDTH, IMAGE_HEIGHT);
        hg::writeImage(*im, IMAGE_FILE_NAME);

        auto res = hg::readImage<falg::Vec4>(IMAGE_FILE_NAME);

        fl_assert_tolerance(*im, *res, 1e-2);
    });
});
