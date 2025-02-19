/**
 * \file
 * \copyright
 * Copyright (c) 2012-2023, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/project/license
 *
 */

#pragma once

#include "ProcessLib/ThermoRichardsMechanics/ConstitutiveCommon/EquivalentPlasticStrainData.h"
#include "ProcessLib/ThermoRichardsMechanics/ConstitutiveCommon/MaterialState.h"
#include "ProcessLib/ThermoRichardsMechanics/ConstitutiveCommon/Saturation.h"
#include "ProcessLib/ThermoRichardsMechanics/ConstitutiveCommon/TotalStressData.h"
#include "TraitsBase.h"

namespace ProcessLib::ThermoRichardsMechanics
{
namespace ConstitutiveStressSaturation_StrainPressureTemperature
{
template <int DisplacementDim>
struct SolidMechanicsDataStateful
{
    // TODO get rid of that
    KelvinVector<DisplacementDim> eps_m = KVnan<DisplacementDim>();
};

template <int DisplacementDim>
struct SolidMechanicsDataStateless
{
    KelvinMatrix<DisplacementDim> stiffness_tensor = KMnan<DisplacementDim>();
    KelvinVector<DisplacementDim> J_uT_BT_K_N = KVnan<DisplacementDim>();
    KelvinVector<DisplacementDim> J_up_BT_K_N = KVnan<DisplacementDim>();
};

template <int DisplacementDim>
struct SolidMechanicsModel
{
    explicit SolidMechanicsModel(
        SolidConstitutiveRelation<DisplacementDim> const& solid_material)
        : solid_material_(solid_material),
          tangent_operator_blocks_view_{
              solid_material.createTangentOperatorBlocksView()}
    {
    }

    void eval(SpaceTimeData const& x_t,
              TemperatureData<DisplacementDim> const& T_data,
              CapillaryPressureData<DisplacementDim> const& p_cap_data,
              StrainData<DisplacementDim> const& eps_data,
              StrainData<DisplacementDim> const& eps_prev_data,
              MaterialStateData<DisplacementDim>& mat_state,
              SolidMechanicsDataStateful<DisplacementDim> const& prev_state,
              SolidMechanicsDataStateful<DisplacementDim>& current_state,
              TotalStressData<DisplacementDim> const& total_stress_data_prev,
              TotalStressData<DisplacementDim>& total_stress_data,
              EquivalentPlasticStrainData& equiv_plast_strain_data,
              SolidMechanicsDataStateless<DisplacementDim>& current_stateless,
              SaturationData const& S_L_prev_data, SaturationData& S_L_data,
              SaturationDataDeriv& dS_L_data) const;

private:
    SolidConstitutiveRelation<DisplacementDim> const& solid_material_;

    MSM::OGSMFrontTangentOperatorBlocksView<
        DisplacementDim, boost::mp11::mp_list<MSM::Strain, MSM::LiquidPressure>,
        boost::mp11::mp_list<MSM::Stress, MSM::Saturation>,
        boost::mp11::mp_list<MSM::Temperature>>
        tangent_operator_blocks_view_;
};

extern template struct SolidMechanicsModel<2>;
extern template struct SolidMechanicsModel<3>;
}  // namespace ConstitutiveStressSaturation_StrainPressureTemperature
}  // namespace ProcessLib::ThermoRichardsMechanics
