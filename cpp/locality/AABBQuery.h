// Copyright (c) 2010-2018 The Regents of the University of Michigan
// This file is from the freud project, released under the BSD 3-Clause License.

#ifndef AABBQUERY_H
#define AABBQUERY_H

#include <vector>

#include "SpatialData.h"
#include "Box.h"
#include "NeighborList.h"
#include "Index1D.h"
#include "AABBTree.h"

/*! \file AABBQuery.h
    \brief Build an AABB tree from points and query it for neighbors.
 * A bounding volume hierarchy (BVH) tree is a binary search tree. It is
 * constructed from axis-aligned bounding boxes (AABBs). The AABB for a node in
 * the tree encloses all child AABBs. A leaf AABB holds multiple particles. The
 * tree is constructed in a balanced way using a heuristic to minimize AABB
 * volume. We build one tree per particle type, and use point AABBs for the
 * particles. The neighbor list is built by traversing down the tree with an
 * AABB that encloses the pairwise cutoff for the particle. Periodic boundaries
 * are treated by translating the query AABB by all possible image vectors,
 * many of which are trivially rejected for not intersecting the root node.
 */

namespace freud { namespace locality {

class AABBQuery : public SpatialData
    {
    public:
        //! Constructs the compute
        AABBQuery();

        //! New-style constructor.
        AABBQuery(const box::Box &box, const vec3<float> *ref_points, unsigned int Nref);

        //! Destructor
        ~AABBQuery();

        void compute(box::Box& box, float rcut,
            const vec3<float> *ref_points, unsigned int Nref,
            const vec3<float> *points, unsigned int Np,
            bool exclude_ii);

        freud::locality::NeighborList *getNeighborList()
            {
            return &m_neighbor_list;
            }

        //! Given a set of points, find the k elements of this data structure
        //  that are the nearest neighbors for each point. Note that due to the
        //  different signature, this is not directly overriding the original
        //  method in SpatialData, so we have to explicitly invalidate calling
        //  with that signature.
        virtual std::shared_ptr<SpatialDataIterator> query(const vec3<float> point, unsigned int k) const
            {
            throw std::runtime_error("AABBQuery k-nearest-neighbor queries must use the function signature that provides rmax and scale guesses.");
            }
        std::shared_ptr<SpatialDataIterator> query(const vec3<float> point, unsigned int k, float r, float scale) const;

        //! Given a set of points, find all elements of this data structure
        //  that are within a certain distance r.
        virtual std::shared_ptr<SpatialDataIterator> queryBall(const vec3<float> point, float r) const;

        AABBTree m_aabb_tree; //!< AABB tree of points

    private:
        //! Driver for tree configuration
        void setupTree(unsigned int N, bool build_images);

        //! Maps particles by local id to their id within their type trees
        void mapParticlesByType();

        //! Computes the image vectors to query for
        void updateImageVectors();

        //! Driver to build AABB trees
        void buildTree(const vec3<float> *ref_points, unsigned int N);

        //! Traverses AABB trees to compute neighbors
        void traverseTree(const vec3<float> *ref_points, unsigned int Nref,
            const vec3<float> *points, unsigned int Np, bool exclude_ii);

        unsigned int m_Ntotal;
        std::vector<AABB> m_aabbs; //!< Flat array of AABBs of all types
        std::vector< vec3<float> > m_image_list; //!< List of translation vectors
        unsigned int m_n_images; //!< The number of image vectors to check

        box::Box m_box; //!< Simulation box where the particles belong
        float m_rcut; //!< Maximum distance between neighbors
        NeighborList m_neighbor_list; //!< Stored neighbor list
    };

//! Parent class of AABB iterators that knows how to traverse general AABB tree structures
/*! placeholder

*/
class AABBIterator : public SpatialDataIterator
    {
    public:
        //! Constructor
        AABBIterator(const AABBQuery* spatial_data, const vec3<float> point) : SpatialDataIterator(spatial_data, point), m_aabb_data(spatial_data)
        { }

        //! Empty Destructor
        virtual ~AABBIterator() {}

        //! Computes the image vectors to query for
        void updateImageVectors(float rmax);

    protected:
        const AABBQuery *m_aabb_data; //!< Link to the AABBQuery object
        std::vector< vec3<float> > m_image_list; //!< List of translation vectors
        unsigned int m_n_images;                 //!< The number of image vectors to check
    };

//! Iterator that gets nearest neighbors from AABB tree structures
/*! placeholder

*/
class AABBQueryIterator : public AABBIterator
    {
    public:
        //! Constructor
        AABBQueryIterator(const AABBQuery* spatial_data,
                const vec3<float> point, unsigned int k, float r, float scale) :
            AABBIterator(spatial_data, point), m_k(k), m_r(r), m_scale(scale), m_current_neighbors()
        {
        updateImageVectors(0); }

        //! Empty Destructor
        virtual ~AABBQueryIterator() {}

        //! Get the next element.
        virtual NeighborPoint next();

    protected:
        unsigned int m_k;  //!< Number of nearest neighbors to find.
        float m_r;  //!< Current ball cutoff distance. Used as a guess.
        float m_scale;  //!< The amount to scale m_r by when the current ball is too small.

        std::vector<NeighborPoint> m_current_neighbors; //!< The current set of found neighbors.
    };

//! Iterator that gets neighbors in a ball of size r using AABB tree structures
/*! placeholder

*/
class AABBQueryBallIterator : public AABBIterator
    {
    public:
        //! Constructor
        AABBQueryBallIterator(const AABBQuery* spatial_data,
                const vec3<float> point, float r) :
            AABBIterator(spatial_data, point), m_r(r), cur_image(0), cur_node_idx(0), cur_p(0)
        {
        updateImageVectors(m_r);
        }

        //! Empty Destructor
        virtual ~AABBQueryBallIterator() {}

        //! Get the next element.
        virtual NeighborPoint next();

    protected:
        float m_r;  //!< Search ball cutoff distance

    private:
        unsigned int cur_image; 
        unsigned int cur_node_idx; 
        unsigned int cur_p; 


    };
}; }; // end namespace freud::locality

#endif // AABBQUERY_H
