#pragma once

#include <flawed_assert.hpp>

#include <iterator>
#include <vector>
#include <functional>

namespace hg {

    /*
     * Image class
     */
    template<typename T>
    class Image {

        unsigned int width, height;
        unsigned int elementStride;

        std::vector<T> data;

    public:

        Image(unsigned int width, unsigned int height, unsigned int strideBytes = 0, const T& fill = T());

        unsigned int getByteStride() const;
        unsigned int getWidth() const;
        unsigned int getHeight() const;

        const T& getPixel(int x, int y) const;
        T& getPixel(int x, int y);

        void setPixel(int x, int y, const T& t);

        void fill(const T& v);

        const T* getData() const;
        T* getData();

        template<typename NT>
        std::unique_ptr<hg::Image<NT>> transform(const std::function<NT(const T&)>& func) const;

        /*
         * Image::iterator
         */
        template<typename ImagePtrType, typename TT>
        class _generic_iterator {

            unsigned int x = 0, y = 0;
            ImagePtrType im;
        public:

            using iterator_category = std::input_iterator_tag;
            using value_type = TT;
            using difference_type = int;
            using pointer = TT*;
            using reference = TT&;

            explicit _generic_iterator<ImagePtrType, TT>(int x, int y, ImagePtrType im) : x(x), y(y), im(im) {}
            _generic_iterator& operator++() {
                x++;
                if (x == im->getWidth()) {
                    y++;
                    x = 0;
                }
                return *this;
            }

            _generic_iterator operator++(int) {
                _generic_iterator<ImagePtrType, TT> retval = *this;
                ++(*this);
                return retval;
            }

            bool operator==(_generic_iterator<ImagePtrType, TT> other) const {
                return this->x == other.x &&
                    this->y == other.y &&
                    this->im == other.im;
            }

            bool operator!=(_generic_iterator<ImagePtrType, TT> other) const { return !(*this == other);}
            TT& operator*() const { return this->im->getPixel(x, y); }
        };

        typedef _generic_iterator<Image<T>*, T> iterator;
        typedef _generic_iterator<const Image<T>*, const T> const_iterator;

        iterator begin() { return iterator(0, 0, this); std::cout << "Called normal begin" << std::endl; }
        iterator end() { return iterator(0, this->getHeight(), this); }

        const_iterator begin() const { return const_iterator(0, 0, this); std::cout << "Called const begin" << std::endl; }
        const_iterator end() const { return const_iterator(0, this->getHeight(), this); }

    };
};


// Implementation

namespace hg {

    template<typename T>
    Image<T>::Image(unsigned int width, unsigned int height, unsigned int strideBytes, const T& fill) {
        this->width = width;
        this->height = height;

        fl_assert(strideBytes % sizeof(T) == 0);

        this->elementStride = strideBytes == 0 ? width : (strideBytes / sizeof(T));

        fl_assert_ge(this->elementStride, width);

        this->data = std::vector<T>(elementStride * height);

        for (uint i = 0; i < height; i++) {
            for (uint j = 0; j < width; j++) {
                this->data[i * this->elementStride + j] = fill;
            }
        }
    }

    template<typename T>
    unsigned int Image<T>::getWidth() const {
        return this->width;
    }

    template<typename T>
    unsigned int Image<T>::getHeight() const {
        return this->height;
    }

    template<typename T>
    unsigned int Image<T>::getByteStride() const {
        return this->elementStride * sizeof(T);
    }

    template<typename T>
    T& Image<T>::getPixel(int x, int y) {
        return this->data[y * this->elementStride + x];
    }

    template<typename T>
    const T& Image<T>::getPixel(int x, int y) const {
        return this->data[y * this->elementStride + x];
    }

    template<typename T>
    void Image<T>::setPixel(int x, int y, const T& t) {
        this->data[y * this->elementStride + x] = t;
    }

    template<typename T>
    void Image<T>::fill(const T& v)
    {
        for (unsigned int i = 0; i < this->height; i++) {
            for (unsigned int j = 0; j < this->width; j++) {
                this->data[i * this->elementStride + j] = v;
            }
        }
    }

    template<typename T>
    const T* Image<T>::getData() const {
        return this->data.data();
    }

    template<typename T>
    T* Image<T>::getData() {
        return this->data.data();
    }

    template<typename T>
    template<typename NT>
    std::unique_ptr<hg::Image<NT>> Image<T>::transform(const std::function<NT(const T&)>& func) const {
        std::unique_ptr<hg::Image<NT>> nim =
            std::make_unique<hg::Image<NT>>(this->getWidth(), this->getHeight());

        for (int i = 0; i < this->getHeight(); i++) {
            for (int j = 0; j < this->getWidth(); j++) {
                nim->setPixel(j, i, func(this->getPixel(j, i)));
            }
        }

        return nim;
    }
};
