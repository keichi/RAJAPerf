//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2017-21, Lawrence Livermore National Security, LLC
// and RAJA Performance Suite project contributors.
// See the RAJAPerf/LICENSE file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#include "FIRST_SUM.hpp"

#include "RAJA/RAJA.hpp"

#include "common/DataUtils.hpp"

namespace rajaperf
{
namespace lcals
{


FIRST_SUM::FIRST_SUM(const RunParams& params)
  : KernelBase(rajaperf::Lcals_FIRST_SUM, params)
{
  setDefaultProblemSize(1000000);
  setDefaultReps(2000);

  setActualProblemSize( getTargetProblemSize() );

  m_N = getActualProblemSize();

  setItsPerRep( getActualProblemSize() );
  setKernelsPerRep(1);
  setBytesPerRep( (1*sizeof(Real_type ) + 0*sizeof(Real_type )) * (m_N-1) +
                  (0*sizeof(Real_type ) + 1*sizeof(Real_type )) * m_N );
  setFLOPsPerRep(1 * (getActualProblemSize()-1));

  setUsesFeature(Forall);

  setVariantDefined( Base_Seq );
  setVariantDefined( Lambda_Seq );
  setVariantDefined( RAJA_Seq );

  setVariantDefined( Base_OpenMP );
  setVariantDefined( Lambda_OpenMP );
  setVariantDefined( RAJA_OpenMP );

  setVariantDefined( Base_OpenMPTarget );
  setVariantDefined( RAJA_OpenMPTarget );

  setVariantDefined( Base_CUDA );
  setVariantDefined( RAJA_CUDA );

  setVariantDefined( Base_HIP );
  setVariantDefined( RAJA_HIP );
}

FIRST_SUM::~FIRST_SUM()
{
}

void FIRST_SUM::setUp(VariantID vid)
{
  allocAndInitDataConst(m_x, m_N, 0.0, vid);
  allocAndInitData(m_y, m_N, vid);
}

void FIRST_SUM::updateChecksum(VariantID vid)
{
  checksum[vid] += calcChecksum(m_x, getActualProblemSize());
}

void FIRST_SUM::tearDown(VariantID vid)
{
  (void) vid;
  deallocData(m_x);
  deallocData(m_y);
}

} // end namespace lcals
} // end namespace rajaperf
