#pragma once

#include "primitives.hpp"

#include <random>
#include <memory>
#include <concepts>


namespace hg {

    /*
     * HasPosition - require that the inner type of the hierarchy nodes must have the falg::Vec3 getPosition() function defined
     */

    template<typename T>

    concept Positioned = requires(T t) {
        { t.getPosition() } -> std::same_as<falg::Vec3>;
    };


    /*
     * Forward declaration
     */

    template<Positioned T>
    class BVHNode;


    /*
     * Misc Utils
     */

    struct RandInt {
        std::default_random_engine generator;
        std::uniform_int_distribution<int> distribution;

        RandInt(int start, int end_inc);
        int get();
    };


    /*
     * BVH - Bounding volume hierarchy, storing spatial information in
     */

    template<Positioned T>
    class BVH {
        BVHNode<T> *root;
        RandInt rand_int;

        BVH();

    public:
        static std::shared_ptr<const BVH<T>> createBVH(T* elements, int numElements);
        void getWithin(const falg::Vec3& p, float eps, std::vector<T>& results) const;

        ~BVH();
    };

    template<Positioned T>
    class BVHNode {
        BVHNode<T> *left, *right;
        std::vector<T> leaf_elements;
        Box bounding_box;

        BVHNode(T* elements, int num_elements, RandInt& rand_int);
        void getWithin(const falg::Vec3& p, float eps, std::vector<T>& results) const;

        void destroyRecursive();

        template<Positioned tt>
        friend class BVH;
    };
};
