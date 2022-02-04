#include "./improc.hpp"

#include <exception>
#include <filesystem>

#include <OpenImageIO/imageio.h>

namespace hg {

    enum class ImageComponentType {
        FLOAT,
        UINT8,
    };

    struct WriteImageInfo {
        ImageComponentType outputType;
        int numOutputComponents;
    };


    /*
     * Holds number of components for given image type
     */

    template<typename T>
    struct _ImNumComponents;

    template<>
    struct _ImNumComponents<float> {
        static constexpr int numComponents = 1;
    };

    template<>
    struct _ImNumComponents<double> {
        static constexpr int numComponents = 1;
    };

    template<typename T, int n>
    struct _ImNumComponents<std::array<T, n>> {
        static constexpr int numComponents = n;
    };


    /*
     * holds component type for image type
     */

    template<typename T>
    struct _ImComponentType;

    template<>
    struct _ImComponentType<float> { static constexpr OIIO::TypeDesc componentType = OIIO::TypeDesc::FLOAT; };

    template<>
    struct _ImComponentType<double> { static constexpr OIIO::TypeDesc componentType = OIIO::TypeDesc::DOUBLE; };

    template<>
    struct _ImComponentType<unsigned char> { static constexpr OIIO::TypeDesc componentType = OIIO::TypeDesc::UINT8; };

    template<typename T, int n>
    struct _ImComponentType<std::array<T, n>> {
        static constexpr OIIO::TypeDesc componentType =
            _ImComponentType<T>::componentType;
    };

    /*
     * returns image spec appropriate for this image component type
     */
    template<typename T>
    OIIO::ImageSpec _getWriteImageSpec(const Image<T>& im) {
        return OIIO::ImageSpec(im.getWidth(),
                               im.getHeight(),
                               _ImNumComponents<T>::numComponents,
                               _ImComponentType<T>::componentType);
    }


    namespace {
        template<typename T>
        void _writeImage(const Image<T>& im,
                         const std::string& file_name,
                         const OIIO::ImageSpec& spec) {
            std::unique_ptr<OIIO::ImageOutput> out = OIIO::ImageOutput::create(file_name);
            if (!out) {
                throw std::runtime_error("Could not open output image " + file_name);
            }

            out->open(file_name, spec);
            out->write_image(_ImComponentType<T>::componentType, im.getData());
            out->close();
        }
    }


    /*
     * Wrtes image with custom output format
     */
    template<typename T>
    void writeImage(const Image<T>& im,
                    const std::string& file_name,
                    const WriteImageInfo& wii) {

    }


    /**
     * writeImage: Writes the image where the type is assumed to be a tightly packed
     */
    template<typename T>
    void writeImage(const Image<T>& im,
                    const std::string& file_name) {

        OIIO::ImageSpec spec = _getWriteImageSpec(im);
        _writeImage(im, file_name, spec);
    }

    /**
     * readImage: Reads image from specified path
     */
    template<typename T>
    std::unique_ptr<hg::Image<T>> readImage(const std::string& file_name)  {
        if (!std::filesystem::exists(file_name)) {
            throw std::runtime_error("Could not open file_name");
        }

        std::unique_ptr<OIIO::ImageInput> inp = OIIO::ImageInput::open(file_name);
        const OIIO::ImageSpec& spec = inp->spec();

        std::unique_ptr<hg::Image<T>> im = std::make_unique<hg::Image<T>>(spec.width, spec.height);

        fl_assert_eq(_ImNumComponents<T>::numComponents, spec.nchannels);

        inp->read_image(_ImComponentType<T>::componentType, im->getData());

        return im;
    }
};
