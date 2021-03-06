// Copyright (c) 2009-2017 The Regents of the University of Michigan
// This file is part of the HOOMD-blue project, released under the BSD 3-Clause License.

// Maintainer: mphoward

/*!
 * \file mpcd/Sorter.h
 * \brief Declares mpcd::Sorter, which sorts particles in the cell list
 */

#ifndef MPCD_SORTER_H_
#define MPCD_SORTER_H_

#ifdef NVCC
#error This header cannot be compiled by nvcc
#endif

#include "SystemData.h"

#include "hoomd/Updater.h"
#include "hoomd/extern/pybind/include/pybind11/pybind11.h"

namespace mpcd
{

//! Sorts MPCD particles
/*!
 * The Sorter puts MPCD particles into an order that is more cache-friendly.
 * The natural way to do this for the algorithm is cell list order. The base
 * Sorter implements applyOrder() for applying the reordering map to the
 * mpcd::ParticleData. Specific sorting algorithms can be implemented by
 * deriving from Sorter and implementing computeOrder(). Any computeOrder()
 * must set the map from old particle index to new particle index, and the
 * reverse mapping.
 */
class Sorter : public ::Updater
    {
    public:
        //! Constructor
        Sorter(std::shared_ptr<mpcd::SystemData> sysdata);

        //! Destructor
        virtual ~Sorter();

        //! Update the particle data order
        virtual void update(unsigned int timestep);

    protected:
        std::shared_ptr<mpcd::SystemData> m_mpcd_sys;       //!< MPCD system data
        std::shared_ptr<mpcd::ParticleData> m_mpcd_pdata;   //!< MPCD particle data
        std::shared_ptr<mpcd::CellList> m_cl;               //!< MPCD cell list

        GPUVector<unsigned int> m_order;    //!< Maps new sorted index onto old particle indexes
        GPUVector<unsigned int> m_rorder;   //!< Maps old particle indexes onto new sorted indexes

        //! Compute the sorting order at the current timestep
        virtual void computeOrder(unsigned int timestep);

        //! Apply the sorting order
        virtual void applyOrder() const;
    };

namespace detail
{
//! Exports the mpcd::Sorter to python
void export_Sorter(pybind11::module& m);
} // end namespace detail
} // end namespace mpcd

#endif // MPCD_SORTER_H_
