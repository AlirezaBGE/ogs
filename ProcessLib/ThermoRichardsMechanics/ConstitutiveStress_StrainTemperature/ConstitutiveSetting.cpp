/**
 * \file
 * \copyright
 * Copyright (c) 2012-2023, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/project/license
 *
 */

#include "ConstitutiveSetting.h"

namespace ProcessLib::ThermoRichardsMechanics
{
namespace ConstitutiveStress_StrainTemperature
{
template <int DisplacementDim>
void ConstitutiveSetting<DisplacementDim>::eval(
    ConstitutiveModels<DisplacementDim>& models, double const t,
    double const dt, ParameterLib::SpatialPosition const& x_position,
    MaterialPropertyLib::Medium const& medium,
    TemperatureData<DisplacementDim> const& T_data,
    CapillaryPressureData<DisplacementDim> const& p_cap_data,
    KelvinVector<DisplacementDim> const& eps_arg,
    KelvinVector<DisplacementDim> const& eps_prev_arg,
    StatefulData<DisplacementDim>& state,
    StatefulData<DisplacementDim> const& prev_state,
    MaterialStateData<DisplacementDim>& mat_state,
    ConstitutiveTempData<DisplacementDim>& tmp,
    OutputData<DisplacementDim>& out,
    ConstitutiveData<DisplacementDim>& cd) const
{
    namespace MPL = MaterialPropertyLib;

    auto& C_el_data = tmp.C_el_data;
    auto& biot_data = tmp.biot_data;
    auto& solid_compressibility_data = tmp.solid_compressibility_data;
    auto& dS_L_data = tmp.dS_L_data;
    auto& bishops_data = tmp.bishops_data;
    auto& bishops_data_prev = tmp.bishops_data_prev;
    auto& s_therm_exp_data = tmp.s_therm_exp_data;
    auto& rho_L_data = out.rho_L_data;
    auto& rho_S_data = out.rho_S_data;
    auto& mu_L_data = out.mu_L_data;
    auto& perm_data = tmp.perm_data;
    auto& darcy_data = out.darcy_data;
    auto& f_therm_exp_data = tmp.f_therm_exp_data;

    auto& swelling_data = tmp.swelling_data;
    auto& s_mech_data = cd.s_mech_data;
    auto& grav_data = cd.grav_data;
    auto& heat_data = cd.heat_data;
    auto& vap_data = cd.vap_data;
    auto& storage_data = cd.storage_data;

    auto& poro_data = state.poro_data;
    auto& S_L_data = state.S_L_data;

    SpaceTimeData const x_t{x_position, t, dt};
    MediaData const media_data{medium};

    // TODO will eps lag one iteration behind? (since it's not updated after
    // solving the global equation system)
    state.eps_data.eps.noalias() = eps_arg;

    models.elastic_tangent_stiffness_model.eval(x_t, T_data, C_el_data);

    models.biot_model.eval(x_t, media_data, biot_data);

    models.solid_compressibility_model.eval(x_t, biot_data, C_el_data,
                                            solid_compressibility_data);

    models.S_L_model.eval(x_t, media_data, p_cap_data, S_L_data, dS_L_data);

    models.bishops_model.eval(x_t, media_data, S_L_data, bishops_data);

    // TODO why not ordinary state tracking?
    models.bishops_model.eval(x_t, media_data, prev_state.S_L_data,
                              bishops_data_prev);

    models.poro_model.eval(x_t, media_data, solid_compressibility_data,
                           S_L_data, prev_state.S_L_data, bishops_data,
                           bishops_data_prev, p_cap_data, state.eps_data,
                           StrainData<DisplacementDim>{
                               eps_prev_arg} /* TODO why not eqU.eps_prev? */,
                           prev_state.poro_data, poro_data);

    if (biot_data() < poro_data.phi)
    {
        OGS_FATAL(
            "ThermoRichardsMechanics: Biot-coefficient {} is smaller than "
            "porosity {} in element/integration point {}/{}.",
            biot_data(), poro_data.phi, *x_position.getElementID(),
            *x_position.getIntegrationPoint());
    }

    models.swelling_model.eval(x_t, media_data, C_el_data, state.eps_data,
                               prev_state.eps_data, S_L_data, dS_L_data,
                               prev_state.S_L_data, prev_state.swelling_data,
                               state.swelling_data, swelling_data);

    models.s_therm_exp_model.eval(x_t, media_data, s_therm_exp_data);

    models.s_mech_model.eval(
        x_t, s_therm_exp_data, swelling_data, T_data, p_cap_data, biot_data,
        bishops_data, dS_L_data, state.eps_data,
        prev_state.eps_data /* TODO why is eps stateful? */, mat_state,
        prev_state.s_mech_data, state.s_mech_data, cd.total_stress_data,
        tmp.equiv_plast_strain_data, s_mech_data);

    models.rho_L_model.eval(x_t, media_data, p_cap_data, T_data, rho_L_data);

    /* {
        double const p_FR = -bishops_data.chi_S_L * p_cap_data.p_cap;
        // p_SR
        // TODO used by no MPL model
        variables.solid_grain_pressure =
            p_FR -
            Invariants::trace(state.s_mech_data.sigma_eff) / (3 * (1 - phi));
    } */

    models.rho_S_model.eval(x_t, media_data, poro_data, T_data, rho_S_data);

    models.grav_model.eval(poro_data, rho_S_data, rho_L_data, S_L_data,
                           dS_L_data, grav_data);

    models.mu_L_model.eval(x_t, media_data, rho_L_data, T_data, mu_L_data);

    models.transport_poro_model.eval(
        x_t, media_data, solid_compressibility_data, bishops_data,
        bishops_data_prev, p_cap_data, poro_data, state.eps_data,
        StrainData<DisplacementDim>{
            eps_prev_arg} /* TODO why not eqU.eps_prev? */,
        prev_state.transport_poro_data, state.transport_poro_data);

    models.perm_model.eval(x_t, media_data, S_L_data, p_cap_data, T_data,
                           mu_L_data, state.transport_poro_data,
                           cd.total_stress_data, tmp.equiv_plast_strain_data,
                           perm_data);

    models.th_osmosis_model.eval(x_t, media_data, T_data, rho_L_data,
                                 cd.th_osmosis_data);

    models.darcy_model.eval(p_cap_data, rho_L_data, perm_data,
                            cd.th_osmosis_data, darcy_data);

    models.heat_storage_and_flux_model.eval(
        x_t, media_data, rho_L_data, rho_S_data, S_L_data, dS_L_data, poro_data,
        perm_data, T_data, darcy_data, heat_data);

    models.vapor_diffusion_model.eval(x_t, media_data, rho_L_data, S_L_data,
                                      dS_L_data, poro_data, p_cap_data, T_data,
                                      vap_data);

    models.f_therm_exp_model.eval(x_t, media_data, p_cap_data, T_data,
                                  s_therm_exp_data, poro_data, rho_L_data,
                                  biot_data, f_therm_exp_data);

    models.storage_model.eval(x_t, biot_data, poro_data, rho_L_data, S_L_data,
                              dS_L_data, prev_state.S_L_data, p_cap_data,
                              solid_compressibility_data, storage_data);

    models.eq_p_model.eval(p_cap_data, T_data, S_L_data, dS_L_data, biot_data,
                           rho_L_data, perm_data, f_therm_exp_data, vap_data,
                           storage_data, cd.eq_p_data);

    models.eq_T_model.eval(heat_data, vap_data, cd.eq_T_data);
}

template struct ConstitutiveSetting<2>;
template struct ConstitutiveSetting<3>;

}  // namespace ConstitutiveStress_StrainTemperature
}  // namespace ProcessLib::ThermoRichardsMechanics
