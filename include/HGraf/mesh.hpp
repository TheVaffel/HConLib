#pragma once

#include <FlatAlg.hpp>

#include <vector>
#include <concepts>

namespace hg {

    /*
     * Class forward declarations
     */

    class HalfEdgeMesh;


    template<typename T>
    concept Mesh_C = requires(T t) {
        { t.positions } -> std::same_as<std::vector<falg::Vec3>>;
        { t.indices } -> std::same_as<std::vector<uint32_t>>;
    };
    

    /*
     * IndexedMesh - Mesh with positions and indices
     */

    struct Mesh {
        std::vector<falg::Vec3> positions;

        std::vector<uint32_t> indices;
    };


    /*
     * IndexedNormalMesh - Mesh with positions and normals with indices
     */

    struct NormalMesh {
        std::vector<falg::Vec3> positions;
        std::vector<falg::Vec3> normals;

        std::vector<uint32_t> indices;
    };


    /*
     * HalfEdge - Represents one side of an edge in a mesh
     */

    struct HalfEdge {
        HalfEdge *opposite = nullptr, *next = nullptr;
        int start_index, end_index;

    private:
        unsigned int index_in_half_edge_mesh;

        friend class HalfEdgeMesh;
    };


    /*
     * HalfEdgeMesh - Mesh purely constructed from HalfEdges
     */

    class HalfEdgeMesh {
        std::vector<HalfEdge*> half_edges;

        bool validateHalfEdgeMesh() const;
        void add_edge_to_list(HalfEdge* edge);
        void remove_edge_from_list(HalfEdge* edge);

    public:

        const HalfEdge* const* getData() const;
        HalfEdge** getData();
        unsigned int getSize() const;

        void splitEdge(HalfEdge* edge, int new_vertex_index);
        int mergeEdge(HalfEdge* edge);
        void splitTriangle(HalfEdge* one_edge, int new_vertex_index);
        void constructIndices(std::vector<uint32_t>& indices) const;
        void reconstructMesh(NormalMesh& mesh);

        template<Mesh_C T>
        HalfEdgeMesh(const T& mesh);
        ~HalfEdgeMesh();
    };
};
