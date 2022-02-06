#include <HGraf/improc_io.hpp>

#include <concepts>


namespace hg {

    /**
     * Structs holding number of image components for different types
     */
    template<>
    constexpr int _ImNumComponents<float>::numComponents = 1;

    template<>
    constexpr int _ImNumComponents<double>::numComponents = 1;

    template<>
    constexpr int _ImNumComponents<unsigned char>::numComponents = 1;

    /**
     * Structs holding OIIO component type for different image types
     */
    template<>
    constexpr OIIO::TypeDesc _ImComponentType<float>::componentType = OIIO::TypeDesc::FLOAT;

    template<>
    constexpr OIIO::TypeDesc _ImComponentType<double>::componentType = OIIO::TypeDesc::DOUBLE;

    template<>
    constexpr OIIO::TypeDesc _ImComponentType<unsigned char>::componentType = OIIO::TypeDesc::UINT8;
};
