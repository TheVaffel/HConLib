#include <array>

#include <HGraf.hpp>

int main() {

    hg::Image<float> u(100, 100);

    for (unsigned int i = 0; i < 20; i++) {
        for (unsigned int j = 0; j < 20; j++) {
            u.setPixel(j, i, 10);
        }
    }

    float sum = 0.f;

    for (uint i = 0; i < u.getHeight(); i++) {
        for (uint j = 0; j < u.getWidth(); j++) {
            sum += u.getPixel(i, j);
        }
    }

    std::cout << "Sum was " << sum << std::endl;

    hg::Image<std::array<float, 3>> a(100, 100);

    for (unsigned int i = 0; i < 20; i++) {
        for (unsigned int j = 0; j < 20; j++) {
            a.setPixel(i, j, {1, 200, 3 });
        }
    }

    return 0;
}
