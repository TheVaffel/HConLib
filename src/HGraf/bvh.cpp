#include <HGraf/bvh.hpp>

namespace hg {
    /*
     * Misc utils
     */

    RandInt::RandInt(int start, int end_inc) : distribution(start, end_inc) {}

    int RandInt::get() {
        return this->distribution(this->generator);
    }

};
