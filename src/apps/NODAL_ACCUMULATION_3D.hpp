//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2017-21, Lawrence Livermore National Security, LLC
// and RAJA Performance Suite project contributors.
// See the RAJAPerf/LICENSE file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

///
/// NODAL_ACCUMULATION_3D kernel reference implementation:
///
/// NDPTRSET(m_domain->jp, m_domain->kp, x,x0,x1,x2,x3,x4,x5,x6,x7) ;
///
/// for (Index_type ii = ibegin; ii < iend; ++ii ) {
///   Index_type i = real_zones[ii];
///
///   Real_type val = 0.125 * vol[i] ;
///
///   x0[i] += val;
///   x1[i] += val;
///   x2[i] += val;
///   x3[i] += val;
///   x4[i] += val;
///   x5[i] += val;
///   x6[i] += val;
///   x7[i] += val;
///
/// }
///

#ifndef RAJAPerf_Apps_NODAL_ACCUMULATION_3D_HPP
#define RAJAPerf_Apps_NODAL_ACCUMULATION_3D_HPP

#define NODAL_ACCUMULATION_3D_DATA_SETUP \
  Real_ptr x = m_x; \
  Real_ptr vol = m_vol; \
  \
  Real_ptr x0,x1,x2,x3,x4,x5,x6,x7; \
  \
  Index_ptr real_zones = m_domain->real_zones;

#define NODAL_ACCUMULATION_3D_BODY_INDEX \
  Index_type i = real_zones[ii];

#define NODAL_ACCUMULATION_3D_BODY \
  Real_type val = 0.125 * vol[i]; \
  \
  x0[i] += val; \
  x1[i] += val; \
  x2[i] += val; \
  x3[i] += val; \
  x4[i] += val; \
  x5[i] += val; \
  x6[i] += val; \
  x7[i] += val;

#define NODAL_ACCUMULATION_3D_RAJA_ATOMIC_BODY(policy) \
  Real_type val = 0.125 * vol[i]; \
  \
  RAJA::atomicAdd<policy>(&x0[i], val); \
  RAJA::atomicAdd<policy>(&x1[i], val); \
  RAJA::atomicAdd<policy>(&x2[i], val); \
  RAJA::atomicAdd<policy>(&x3[i], val); \
  RAJA::atomicAdd<policy>(&x4[i], val); \
  RAJA::atomicAdd<policy>(&x5[i], val); \
  RAJA::atomicAdd<policy>(&x6[i], val); \
  RAJA::atomicAdd<policy>(&x7[i], val);



#include "common/KernelBase.hpp"

namespace rajaperf
{
class RunParams;

namespace apps
{
class ADomain;

class NODAL_ACCUMULATION_3D : public KernelBase
{
public:

  NODAL_ACCUMULATION_3D(const RunParams& params);

  ~NODAL_ACCUMULATION_3D();

  void setUp(VariantID vid);
  void updateChecksum(VariantID vid);
  void tearDown(VariantID vid);

  void runSeqVariant(VariantID vid);
  void runOpenMPVariant(VariantID vid);
  void runCudaVariant(VariantID vid);
  void runHipVariant(VariantID vid);
  void runOpenMPTargetVariant(VariantID vid);

private:
  Real_ptr m_x;
  Real_ptr m_vol;

  ADomain* m_domain;
  Index_type m_nodal_array_length;
  Index_type m_zonal_array_length;
};

} // end namespace apps
} // end namespace rajaperf

#endif // closing endif for header file include guard
