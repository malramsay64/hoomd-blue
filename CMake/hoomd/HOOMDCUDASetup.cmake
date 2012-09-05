# Maintainer: joaander

##################################
## Find CUDA
if (SINGLE_PRECISION)
    # If CUDA is enabled, set it up
    if (ENABLE_CUDA)
        # the package is needed
        find_package(CUDA REQUIRED REQUIRED)
        
        if (${CUDA_VERSION} VERSION_LESS 4.0)
            message(SEND_ERROR "CUDA 3.2 and older are not supported")
        endif (${CUDA_VERSION} VERSION_LESS 4.0)

        include_directories(${CUDA_INCLUDE_DIRS})

        # Find Thrust
        find_package(Thrust)

        if (${THRUST_VERSION} VERSION_LESS 1.5.0)
            message(SEND_ERROR "Thrust version ${THRUST_VERSION} found, >= 1.5.0 is required")
        endif (${THRUST_VERSION} VERSION_LESS 1.5.0)

        # hide some variables users don't need to see
        mark_as_advanced(CUDA_SDK_ROOT_DIR)
        if (CUDA_TOOLKIT_ROOT_DIR)
            mark_as_advanced(CUDA_TOOLKIT_ROOT_DIR)
        endif (CUDA_TOOLKIT_ROOT_DIR)
        mark_as_advanced(CUDA_VERBOSE_BUILD)
        mark_as_advanced(CUDA_BUILD_EMULATION)
    endif (ENABLE_CUDA)
endif (SINGLE_PRECISION)

# setup CUDA compile options
if (ENABLE_CUDA)
    # setup nvcc to build for all CUDA architectures. Allow user to modify the list if desired
    if (CUDA_VERSION VERSION_GREATER 2.99) 
        if (CUDA_VERSION VERSION_GREATER 4.1) 
            set(CUDA_ARCH_LIST 12 13 20 30 CACHE STRING "List of target sm_ architectures to compile CUDA code for. Separate with semicolons.")
        else()
            set(CUDA_ARCH_LIST 12 13 20 CACHE STRING "List of target sm_ architectures to compile CUDA code for. Separate with semicolons.")
        endif()
    else (CUDA_VERSION VERSION_GREATER 2.99)
        set(CUDA_ARCH_LIST 12 13 CACHE STRING "List of target sm_ architectures to compile CUDA code for. Separate with semicolons.")
    endif (CUDA_VERSION VERSION_GREATER 2.99)
    
    foreach(_cuda_arch ${CUDA_ARCH_LIST})
        list(APPEND CUDA_NVCC_FLAGS "-gencode=arch=compute_${_cuda_arch},code=sm_${_cuda_arch}")
    endforeach (_cuda_arch)

    if (CUDA_VERSION VERSION_EQUAL 3.1 OR CUDA_VERSION VERSION_EQUAL 3.2) 
        message(STATUS "Enabling reg usage workaround for CUDA 3.1/3.2") 
        list(APPEND CUDA_NVCC_FLAGS "-Xptxas;-abi=no")
    endif (CUDA_VERSION VERSION_EQUAL 3.1 OR CUDA_VERSION VERSION_EQUAL 3.2) 
    
    # need to know the minumum supported CUDA_ARCH
    set(_cuda_arch_list_sorted ${CUDA_ARCH_LIST})
    list(SORT _cuda_arch_list_sorted)
    list(GET _cuda_arch_list_sorted 0 _cuda_min_arch)
    add_definitions(-DCUDA_ARCH=${_cuda_min_arch})

    # only generage ptx code for the maximum supported CUDA_ARCH (saves on file size)
    list(REVERSE _cuda_arch_list_sorted)
    list(GET _cuda_arch_list_sorted 0 _cuda_max_arch)
    list(APPEND CUDA_NVCC_FLAGS "-gencode=arch=compute_${_cuda_max_arch},code=compute_${_cuda_max_arch}")
endif (ENABLE_CUDA)

# embed the CUDA libraries into the lib dir
if (ENABLE_EMBED_CUDA)

    # determine the directory of the found cuda libs
    get_filename_component(_cuda_libdir ${CUDA_CUDART_LIBRARY} PATH)
    FILE(GLOB _cuda_libs ${_cuda_libdir}/libcudart* ${_cuda_libdir}/libcufft*)
    install(PROGRAMS ${_cuda_libs} DESTINATION ${LIB_INSTALL_DIR})

endif (ENABLE_EMBED_CUDA)

# automatically handle setting ccbin to /usr when needed
if (CMAKE_COMPILER_IS_GNUCXX AND CMAKE_VERSION VERSION_GREATER 2.8.7)
    # CMAKE_CXX_COMPILER_VERSION is only available on 2.8.8 and newer
    
    # need to set ccbin to  when gcc is unsupported
    # this assumes that the user is on a system where CUDA is supported and /usr/bin/gcc will work - if they aren't, then it is their problem
    
    if (CUDA_VERSION VERSION_EQUAL 4.1)
        if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.5.99)
            message(STATUS "CUDA 4.1 doesn't support gcc >= 4.6, falling back on /usr/bin/gcc")
            list(APPEND CUDA_NVCC_FLAGS "-ccbin;/usr/bin/gcc")
        endif()
    endif()
    
    if (CUDA_VERSION VERSION_EQUAL 4.2)
        if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.6.99)
            message(STATUS "CUDA 4.1 doesn't support gcc >= 4.7, falling back on /usr/bin/gcc")
            list(APPEND CUDA_NVCC_FLAGS "-ccbin;/usr/bin/gcc")
        endif()
    endif()
endif()
