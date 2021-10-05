#pragma once

#include "bvh.hpp"
#include "primitives_impl.hpp"

namespace hg {

    /*
     * BVH member functions
     */

    template<Positioned T>
    BVH<T>::BVH() : rand_int(1, 3) { }

    template<Positioned T>
    BVH<T>::~BVH() {
        this->root->destroyRecursive();
        delete this->root;
    }

    template<Positioned T>
    std::shared_ptr<const BVH<T>> BVH<T>::createBVH(T* elements, int numElements) {
        BVH<T>* ptr = new BVH<T>();

        ptr->root = new BVHNode<T>(elements, numElements, ptr->rand_int);

        return std::shared_ptr<const BVH<T>>(ptr);
    }

    template<Positioned T>
    void BVH<T>::getWithin(const falg::Vec3& p, float eps, std::vector<T>& results) const {
        this->root->getWithin(p, eps, results);
    }

    /*
     * BVHNode member functions
     */

    template<Positioned T>
    bool _comparator_x(const T& p0, const T& p1) {
        return p0.getPosition().x() < p1.getPosition().x();
    }

    template<Positioned T>
    bool _comparator_y(const T& p0, const T& p1) {
        return p0.getPosition().y() < p1.getPosition().y();
    }

    template<Positioned T>
    bool _comparator_z(const T& p0, const T& p1) {
        return p0.getPosition().z() < p1.getPosition().z();
    }

    template<Positioned T>
    BVHNode<T>::BVHNode(T* elements, int num_elements, RandInt& rand_int) {
        for (int i = 0; i < num_elements; i++) {
            this->bounding_box.add(elements[i].getPosition());
        }

        if (num_elements <= 3) {
            this->leaf_elements = std::vector<T>(elements, elements + num_elements);
            this->left = nullptr;
            this->right = nullptr;
            return;
        }

        int rr = rand_int.get();

        if (rr == 0) {
            std::sort(elements, elements + num_elements, _comparator_x<T>);
        } else if(rr == 1) {
            std::sort(elements, elements + num_elements, _comparator_y<T>);
        } else {
            std::sort(elements, elements + num_elements, _comparator_z<T>);
        }

        int half_count = num_elements / 2;
        int other_half = (num_elements + 1) / 2;
        this->left = new BVHNode<T>(elements, half_count, rand_int);
        this->right = new BVHNode<T>(elements + half_count, other_half, rand_int);
    }

    template<Positioned T>
    void BVHNode<T>::getWithin(const falg::Vec3& p, float eps, std::vector<T>& results) const {
        if (this->left == nullptr && this->right == nullptr) {
            for (const T& tp : leaf_elements) {
                if ((tp.getPosition() - p).sqNorm() <= eps * eps) {
                    results.push_back(tp);
                }
            }
            return;
        }

        if (!this->bounding_box.containsWithin(p, eps)) {
            if (this->left != nullptr) {
                this->left->getWithin(p, eps, results);
            }
            if (this->right != nullptr) {
                this->right->getWithin(p, eps, results);
            }
        }
    }

    template<Positioned T>
    void BVHNode<T>::destroyRecursive() {
        if (this->left != nullptr) {
            this->left->destroyRecursive();
            delete this->left;
        }

        if (this->right != nullptr) {
            this->right->destroyRecursive();
            delete this->right;
        }
    }
};
