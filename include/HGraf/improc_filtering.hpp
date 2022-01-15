#pragma once

#include "./improc.hpp"
#include <algorithm>

#include <flawed_assert.hpp>

namespace hg {
    template<unsigned int kernel_size, typename T = float>
    class ImageFilter {
        std::array<std::array<T, kernel_size>, kernel_size> data;

    public:
        ImageFilter(const std::initializer_list<std::initializer_list<T>>& init);

        std::array<T, kernel_size>& operator[](unsigned int i);
        const std::array<T, kernel_size>& operator[](unsigned int i) const;
    };

    template<typename T, unsigned int s>
    std::unique_ptr<Image<T>> filterImage(const Image<T>& im,
                                            const ImageFilter<s>& kernel) {
        static_assert(s % 2 == 1, "[improc] Kernel dimensions must be odd");

        unsigned int w = im.getWidth();
        unsigned int h = im.getHeight();

        std::unique_ptr<Image<T>> res = std::make_unique<Image<T>>(w, h,
                                                                   im.getByteStride());
        unsigned int sh = s / 2;
        for (unsigned int i = 0; i < h; i++) {
            for (unsigned int j = 0; j < w; j++) {
                unsigned int minkx = std::max(0, (int)sh - (int)j);
                unsigned int minky = std::max(0, (int)sh - (int)i);

                unsigned int maxkx = std::min(s, w - j - sh);
                unsigned int maxky = std::min(s, h - i - sh);

                T sum;

                for (unsigned int k = minky; k < maxky; k++) {
                    for (unsigned int l = minkx; l < maxkx; l++) {
                        T pixel = im.getPixel(j + l - sh, i + k - sh);
                        sum = sum + pixel* kernel[k][l];
                    }
                }

                res->setPixel(j, i, sum);
            }
        }

        return res;
    }
};


/*
 * Implementation
 */
namespace hg {
    template<unsigned int kernel_size, typename T>
    ImageFilter<kernel_size, T>::ImageFilter(const std::initializer_list<std::initializer_list<T>>& init) {
        fl_assert_eq(init.size(), kernel_size);

        for (unsigned int i = 0; i < kernel_size; i++) {
            fl_assert_eq(std::begin(init)->size(), kernel_size);
        }

        for (unsigned int i = 0; i < kernel_size; i++) {
            const std::initializer_list<T>& thisRow = std::begin(init)[i];
            for (unsigned int j = 0; j < kernel_size; j++) {
                this->data[i][j] = std::begin(thisRow)[j];
            }
        }
    }


    template<unsigned int kernel_size, typename T>
    std::array<T, kernel_size>& ImageFilter<kernel_size, T>::operator[](unsigned int i) {
        return data[i];
    }

    template<unsigned int kernel_size, typename T>
    const std::array<T, kernel_size>& ImageFilter<kernel_size, T>::operator[](unsigned int i) const {
        return data[i];
    }
};
