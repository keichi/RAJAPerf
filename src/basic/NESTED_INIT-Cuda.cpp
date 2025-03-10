//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2017-21, Lawrence Livermore National Security, LLC
// and RAJA Performance Suite project contributors.
// See the RAJAPerf/LICENSE file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#include "NESTED_INIT.hpp"

#include "RAJA/RAJA.hpp"

#if defined(RAJA_ENABLE_CUDA)

#include "common/CudaDataUtils.hpp"

#include <iostream>

namespace rajaperf
{
namespace basic
{

  //
  // Define thread block size for CUDA execution
  //
  constexpr size_t i_block_sz = 32;
  constexpr size_t j_block_sz = 8;
  constexpr size_t k_block_sz = 1;

#define NESTED_INIT_THREADS_PER_BLOCK_CUDA \
  dim3 nthreads_per_block(i_block_sz, j_block_sz, k_block_sz);

#define NESTED_INIT_NBLOCKS_CUDA \
  dim3 nblocks(static_cast<size_t>(RAJA_DIVIDE_CEILING_INT(ni, i_block_sz)), \
               static_cast<size_t>(RAJA_DIVIDE_CEILING_INT(nj, j_block_sz)), \
               static_cast<size_t>(RAJA_DIVIDE_CEILING_INT(nk, k_block_sz)));


#define NESTED_INIT_DATA_SETUP_CUDA \
  allocAndInitCudaDeviceData(array, m_array, m_array_length);

#define NESTED_INIT_DATA_TEARDOWN_CUDA \
  getCudaDeviceData(m_array, array, m_array_length); \
  deallocCudaDeviceData(array);

__global__ void nested_init(Real_ptr array,
                            Index_type ni, Index_type nj, Index_type nk)
{
  Index_type i = blockIdx.x * blockDim.x + threadIdx.x;
  Index_type j = blockIdx.y * blockDim.y + threadIdx.y;
  Index_type k = blockIdx.z;

  if ( i < ni && j < nj && k < nk ) {
    NESTED_INIT_BODY;
  }
}

template< typename Lambda >
__global__ void nested_init_lam(Index_type ni, Index_type nj, Index_type nk,
                                Lambda body)
{
  Index_type i = blockIdx.x * blockDim.x + threadIdx.x;
  Index_type j = blockIdx.y * blockDim.y + threadIdx.y;
  Index_type k = blockIdx.z;

  if ( i < ni && j < nj && k < nk ) {
    body(i, j, k);
  }
}


void NESTED_INIT::runCudaVariant(VariantID vid)
{
  const Index_type run_reps = getRunReps();

  NESTED_INIT_DATA_SETUP;

  if ( vid == Base_CUDA ) {

    NESTED_INIT_DATA_SETUP_CUDA;

    startTimer();
    for (RepIndex_type irep = 0; irep < run_reps; ++irep) {

      NESTED_INIT_THREADS_PER_BLOCK_CUDA;
      NESTED_INIT_NBLOCKS_CUDA;

      nested_init<<<nblocks, nthreads_per_block>>>(array,
                                                   ni, nj, nk);
      cudaErrchk( cudaGetLastError() );

    }
    stopTimer();

    NESTED_INIT_DATA_TEARDOWN_CUDA;

  } else if ( vid == Lambda_CUDA ) {

    NESTED_INIT_DATA_SETUP_CUDA;

    startTimer();
    for (RepIndex_type irep = 0; irep < run_reps; ++irep) {

      NESTED_INIT_THREADS_PER_BLOCK_CUDA;
      NESTED_INIT_NBLOCKS_CUDA;

      nested_init_lam<<<nblocks, nthreads_per_block>>>(ni, nj, nk,
        [=] __device__ (Index_type i, Index_type j, Index_type k) {
          NESTED_INIT_BODY;
        }
      );
      cudaErrchk( cudaGetLastError() );

    }
    stopTimer();

    NESTED_INIT_DATA_TEARDOWN_CUDA;

  } else if ( vid == RAJA_CUDA ) {

    NESTED_INIT_DATA_SETUP_CUDA;

    using EXEC_POL =
      RAJA::KernelPolicy<
        RAJA::statement::CudaKernelFixedAsync<i_block_sz * j_block_sz,
          RAJA::statement::Tile<1, RAJA::tile_fixed<j_block_sz>,
                                   RAJA::cuda_block_y_direct,
            RAJA::statement::Tile<0, RAJA::tile_fixed<i_block_sz>,
                                     RAJA::cuda_block_x_direct,
              RAJA::statement::For<2, RAJA::cuda_block_z_direct,      // k
                RAJA::statement::For<1, RAJA::cuda_thread_y_direct,   // j
                  RAJA::statement::For<0, RAJA::cuda_thread_x_direct, // i
                    RAJA::statement::Lambda<0>
                  >
                >
              >
            >
          >
        >
      >;


    startTimer();
    for (RepIndex_type irep = 0; irep < run_reps; ++irep) {

      RAJA::kernel<EXEC_POL>( RAJA::make_tuple(RAJA::RangeSegment(0, ni),
                                               RAJA::RangeSegment(0, nj),
                                               RAJA::RangeSegment(0, nk)),
        [=] __device__ (Index_type i, Index_type j, Index_type k) {
        NESTED_INIT_BODY;
      });

    }
    stopTimer();

    NESTED_INIT_DATA_TEARDOWN_CUDA;

  } else {
     getCout() << "\n  NESTED_INIT : Unknown Cuda variant id = " << vid << std::endl;
  }
}

} // end namespace basic
} // end namespace rajaperf

#endif  // RAJA_ENABLE_CUDA
