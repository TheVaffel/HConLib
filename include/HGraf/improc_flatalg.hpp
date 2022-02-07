#pragma once
#include <HGraf/improc_io.hpp>
#include <FlatAlg.hpp>

namespace hg {
    template<int n>
    struct _ImComponentType<falg::Vector<n>> {
        static const OIIO::TypeDesc componentType;
    };

    template<int n>
    const OIIO::TypeDesc _ImComponentType<falg::Vector<n>>::componentType =
        _ImComponentType<float>::componentType;


    template<int n>
    struct _ImNumComponents<falg::Vector<n>> {
        static const int numComponents;
    };

    template<int n>
    const int _ImNumComponents<falg::Vector<n>>::numComponents = n;
};
