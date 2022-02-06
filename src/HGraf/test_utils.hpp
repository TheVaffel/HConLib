
#include <functional>
#include <string>
#include <memory>

#include <HGraf/improc.hpp>

#include <FlatAlg.hpp>

std::function<float()> getUniformFloatRandom(const std::string &seed_input,
                                             float lower, float higher);

std::function<unsigned char()> getUniformByteRandom(const std::string &seed_input);

std::unique_ptr<hg::Image<float>> getRandomFloatImage(int width, int height);

namespace {
    template <typename T, int n>
    std::unique_ptr<hg::Image<std::array<T, n>>> getRandomTypedArrayImage(const std::function<T()>& dist,
                                                                          int width, int height) {
        std::unique_ptr<hg::Image<std::array<T, n>>> im =
            std::make_unique<hg::Image<std::array<T, n>>>(width, height);
        for (int i = 0; i < im->getHeight(); i++) {
            for (int j = 0; j < im->getWidth(); j++) {
                std::array<T, n> arr;
                for (int k = 0; k < n; k++) {
                    // Make sure alpha channel is always at max
                    // to avoid spicy compression tricks on image write
                    arr[k] = k == 3 ? 255 : dist();
                }

                im->setPixel(j, i, arr);
            }
        }
        return im;
    }
};
template<int n>
std::unique_ptr<hg::Image<std::array<float, n>>> getRandomFloatArrayImage(int width, int height) {
    std::function<float()> dist = getUniformFloatRandom("some_seed", 0, 1);
    return getRandomTypedArrayImage<float, n>(dist, width, height);
}

template <int n>
std::unique_ptr<hg::Image<std::array<unsigned char, n>>> getRandomByteArrayImage(int width, int height) {
    std::function<unsigned char()> dist = getUniformByteRandom("some_seed");
    return getRandomTypedArrayImage<unsigned char, n>(dist, width, height);
}

template <int n>
auto getRandomVectorImage(int width, int height) {
    auto dist = getUniformFloatRandom("some_seed", 0, 1);
    auto im = std::make_unique<hg::Image<falg::Vector<n>>>(width, height);

    for (auto it = im->begin(); it != im->end(); ++it) {
        falg::Vector<n> val;
        for (int i = 0; i < n; i++) {
            val[i] = i == 3 ? 1.0 : dist();
        }
        *it = val;
    }

    return im;
}



#include <HGraf/improc_io.hpp>
namespace hg {
    template<int n>
    struct _ImComponentType<falg::Vector<n>> {
        static constexpr OIIO::TypeDesc componentType =
            OIIO::TypeDesc::FLOAT;
    };

    template<int n>
    struct _ImNumComponents<falg::Vector<n>> {
        static constexpr int numComponents = n;
    };
};
