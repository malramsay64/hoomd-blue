#ifndef __PPPM_FORCE_COMPUTE_H__
#define __PPPM_FORCE_COMPUTE_H__

#include "hoomd/ForceCompute.h"
#include "NeighborList.h"
#include "hoomd/ParticleGroup.h"

#ifdef ENABLE_MPI
#include "CommunicatorGrid.h"
#include "hoomd/extern/dfftlib/src/dfft_host.h"
#endif

#include "hoomd/extern/kiss_fftnd.h"

#include <boost/signals2.hpp>
#include <boost/bind.hpp>

const Scalar EPS_HOC(1.0e-7);

const unsigned int PPPM_MAX_ORDER = 7;

/*! Compute the long-ranged part of the particle-particle particle-mesh Ewald sum (PPPM)
 */
class PPPMForceCompute : public ForceCompute
    {
    public:
        //! Constructor
        PPPMForceCompute(boost::shared_ptr<SystemDefinition> sysdef,
            boost::shared_ptr<NeighborList> nlist,
            boost::shared_ptr<ParticleGroup> group);
        virtual ~PPPMForceCompute();

        //! Set the parameters
        virtual void setParams(unsigned int nx, unsigned int ny, unsigned int nz,
            unsigned int order, Scalar kappa, Scalar rcut);

        void computeForces(unsigned int timestep);

        /*! Returns the names of provided log quantities.
         */
        std::vector<std::string> getProvidedLogQuantities()
            {
            std::vector<std::string> list = ForceCompute::getProvidedLogQuantities();
            for (std::vector<std::string>::iterator it = m_log_names.begin(); it != m_log_names.end(); ++it)
                {
                list.push_back(*it);
                }
            return list;
            }

        /*! Returns the value of a specific log quantity.
         * \param quantity The name of the quantity to return the value of
         * \param timestep The current value of the time step
         */
        Scalar getLogValue(const std::string& quantity, unsigned int timestep);

        //! Get sum of charges
        Scalar getQSum();

        //! Get sum of squares of charges
        Scalar getQ2Sum();

    protected:
        /*! Compute the biased forces for this collective variable.
            The force that is written to the force arrays must be
            multiplied by the bias factor.

            \param timestep The current value of the time step
         */
        void computeBiasForces(unsigned int timestep);

        boost::shared_ptr<NeighborList> m_nlist; //!< The neighborlist to use for the computation
        boost::shared_ptr<ParticleGroup> m_group;//!< Group to compute properties for

        uint3 m_mesh_points;                //!< Number of sub-divisions along one coordinate
        uint3 m_global_dim;                 //!< Global grid dimensions
        uint3 m_n_ghost_cells;              //!< Number of ghost cells along every axis
        uint3 m_grid_dim;                   //!< Grid dimensions (including ghost cells)
        Scalar3 m_ghost_width;              //!< Dimensions of the ghost layer
        unsigned int m_ghost_offset;       //!< Offset in mesh due to ghost cells
        unsigned int m_n_cells;             //!< Total number of inner cells
        unsigned int m_radius;              //!< Stencil radius (in units of mesh size)
        unsigned int m_n_inner_cells;       //!< Number of inner mesh points (without ghost cells)
        GPUArray<Scalar> m_inf_f;           //!< Fourier representation of the influence function (real part)
        GPUArray<Scalar3> m_k;              //!< Mesh of k values
        Scalar m_qstarsq;                   //!< Short wave length cut-off squared for density harmonics
        bool m_need_initialize;             //!< True if we have not yet computed the influence function
        bool m_params_set;                  //!< True if parameters are set
        bool m_box_changed;                 //!< True if box has changed since last compute

        GPUArray<Scalar> m_virial_mesh;     //!< k-space mesh of virial tensor values

        Scalar m_kappa;                     //!< Screening parameter
        Scalar m_rcut;                      //!< Cutoff for short-ranged interaction
        int m_order;                        //!< Order of interpolation scheme

        Scalar m_q;                         //!< Total system charge
        Scalar m_q2;                        //!< Sum of charge squared

        GPUArray<Scalar> m_rho_coeff;       //!< Coefficients for computing the grid based charge density
        GPUArray<Scalar> m_gf_b;            //!< Green function coefficients

        //! Helper function to be called when box changes
        void setBoxChange()
            {
            m_box_changed = true;
            }

        //! Helper function to setup the mesh indices
        void setupMesh();

        //! Helper function to setup FFT and allocate the mesh arrays
        virtual void initializeFFT();

        //! Compute the optimal influence function
        virtual void computeInfluenceFunction();

        //! Helper function to assign particle coordinates to mesh
        virtual void assignParticles();

        //! Helper function to update the mesh arrays
        virtual void updateMeshes();

        //! Helper function to interpolate the forces
        virtual void interpolateForces();

        //! Helper function to calculate value of potential energy
        virtual Scalar computePE();

        //! Helper function to compute the virial
        virtual void computeVirial();

        //! Helper function to correct forces on excluded particles
        virtual void fixExclusions();

        //! Setup coefficients
        virtual void setupCoeffs();

    private:
        kiss_fftnd_cfg m_kiss_fft;         //!< The FFT configuration
        kiss_fftnd_cfg m_kiss_ifft;        //!< Inverse FFT configuration

        #ifdef ENABLE_MPI
        dfft_plan m_dfft_plan_forward;     //!< Distributed FFT for forward transform
        dfft_plan m_dfft_plan_inverse;     //!< Distributed FFT for inverse transform
        std::auto_ptr<CommunicatorGrid<kiss_fft_cpx> > m_grid_comm_forward; //!< Communicator for charge mesh
        std::auto_ptr<CommunicatorGrid<kiss_fft_cpx> > m_grid_comm_reverse; //!< Communicator for inv fourier mesh
        #endif

        bool m_kiss_fft_initialized;               //!< True if a local KISS FFT has been set up

        GPUArray<kiss_fft_cpx> m_mesh;             //!< The particle density mesh
        GPUArray<kiss_fft_cpx> m_fourier_mesh;     //!< The fourier transformed mesh
        GPUArray<kiss_fft_cpx> m_fourier_mesh_G_x;   //!< Fourier transformed mesh times the influence function, x-component
        GPUArray<kiss_fft_cpx> m_fourier_mesh_G_y;   //!< Fourier transformed mesh times the influence function, y-component
        GPUArray<kiss_fft_cpx> m_fourier_mesh_G_z;   //!< Fourier transformed mesh times the influence function, z-component
        GPUArray<kiss_fft_cpx> m_inv_fourier_mesh_x;   //!< Fourier transformed mesh times the influence function, x-component
        GPUArray<kiss_fft_cpx> m_inv_fourier_mesh_y;   //!< Fourier transformed mesh times the influence function, y-component
        GPUArray<kiss_fft_cpx> m_inv_fourier_mesh_z;   //!< Fourier transformed mesh times the influence function, z-component

        boost::signals2::connection m_boxchange_connection; //!< Connection to ParticleData box change signal

        std::vector<std::string> m_log_names;           //!< Name of the log quantity

        bool m_dfft_initialized;                   //! True if host dfft has been initialized

        //! Compute virial on mesh
        void computeVirialMesh();

        //! Compute number of ghost cellso
        uint3 computeGhostCellNum();

        //! root mean square error in force calculation
        Scalar rms(Scalar h, Scalar prd, Scalar natoms);

        //! computes coefficients for assigning charges to grid points
        void compute_rho_coeff();

        //! computes auxillary table for optimized influence function
        void compute_gf_denom();

        //! computes coefficients for the Green's function
        Scalar gf_denom(Scalar x, Scalar y, Scalar z);

    };

void export_PPPMForceCompute();

#endif