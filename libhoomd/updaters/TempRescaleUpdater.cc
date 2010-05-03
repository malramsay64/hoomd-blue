/*
Highly Optimized Object-oriented Many-particle Dynamics -- Blue Edition
(HOOMD-blue) Open Source Software License Copyright 2008, 2009 Ames Laboratory
Iowa State University and The Regents of the University of Michigan All rights
reserved.

HOOMD-blue may contain modifications ("Contributions") provided, and to which
copyright is held, by various Contributors who have granted The Regents of the
University of Michigan the right to modify and/or distribute such Contributions.

Redistribution and use of HOOMD-blue, in source and binary forms, with or
without modification, are permitted, provided that the following conditions are
met:

* Redistributions of source code must retain the above copyright notice, this
list of conditions, and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice, this
list of conditions, and the following disclaimer in the documentation and/or
other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of HOOMD-blue's
contributors may be used to endorse or promote products derived from this
software without specific prior written permission.

Disclaimer

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND/OR
ANY WARRANTIES THAT THIS SOFTWARE IS FREE OF INFRINGEMENT ARE DISCLAIMED.

IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// $Id$
// $URL$
// Maintainer: joaander

/*! \file TempRescaleUpdater.cc
    \brief Defines the TempRescaleUpdater class
*/

#ifdef WIN32
#pragma warning( push )
#pragma warning( disable : 4103 4244 )
#endif

#include <boost/python.hpp>
using namespace boost::python;

#include "TempRescaleUpdater.h"

#include <iostream>
#include <math.h>
#include <stdexcept>

using namespace std;

/*! \param sysdef System to set temperature on
    \param thermo ComputeThermo to compute the temperature with
    \param tset Temperature set point
*/
TempRescaleUpdater::TempRescaleUpdater(boost::shared_ptr<SystemDefinition> sysdef,
                                       boost::shared_ptr<ComputeThermo> thermo,
                                       boost::shared_ptr<Variant> tset)
        : Updater(sysdef), m_thermo(thermo), m_tset(tset)
    {
    assert(m_pdata);
    assert(thermo);
    }


/*! Perform the proper velocity rescaling
    \param timestep Current time step of the simulation
*/
void TempRescaleUpdater::update(unsigned int timestep)
    {
    // find the current temperature
    
    assert(m_thermo);
    m_thermo->compute(timestep);
    Scalar cur_temp = m_thermo->getTemperature();
    
    if (m_prof) m_prof->push("TempRescale");
    
    if (cur_temp < 1e-3)
        {
        cout << "Notice: TempRescaleUpdater cannot scale a 0 temperature to anything but 0, skipping this step" << endl;
        }
    else
        {
        // calculate a fraction to scale the velocities by
        Scalar fraction = sqrt(m_tset->getValue(timestep) / cur_temp);
        
        // scale the particles velocities
        assert(m_pdata);
        ParticleDataArrays arrays = m_pdata->acquireReadWrite();
        
        for (unsigned int i = 0; i < arrays.nparticles; i++)
            {
            arrays.vx[i] *= fraction;
            arrays.vy[i] *= fraction;
            arrays.vz[i] *= fraction;
            }
            
        m_pdata->release();
        }
        
    if (m_prof) m_prof->pop();
    }

/*! \param tset New temperature set point
    \note The new set point doesn't take effect until the next call to update()
*/
void TempRescaleUpdater::setT(boost::shared_ptr<Variant> tset)
    {
    m_tset = tset;
    }

void export_TempRescaleUpdater()
    {
    class_<TempRescaleUpdater, boost::shared_ptr<TempRescaleUpdater>, bases<Updater>, boost::noncopyable>
    ("TempRescaleUpdater", init< boost::shared_ptr<SystemDefinition>,
                                 boost::shared_ptr<ComputeThermo>,
                                 boost::shared_ptr<Variant> >())
    .def("setT", &TempRescaleUpdater::setT)
    ;
    }

#ifdef WIN32
#pragma warning( pop )
#endif

