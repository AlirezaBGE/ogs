/**
 * \file
 * \author Norbert Grunwald
 * \date   Sep 10, 2019
 *
 * \copyright
 * Copyright (c) 2012-2023, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/project/license
 */
#pragma once

#include "CapillaryPressureSaturation/CreateCapillaryPressureRegularizedVanGenuchten.h"
#include "CapillaryPressureSaturation/CreateCapillaryPressureVanGenuchten.h"
#include "CapillaryPressureSaturation/CreateSaturationBrooksCorey.h"
#include "CapillaryPressureSaturation/CreateSaturationExponential.h"
#include "CapillaryPressureSaturation/CreateSaturationLiakopoulos.h"
#include "CapillaryPressureSaturation/CreateSaturationVanGenuchten.h"
#include "CreateAverageMolarMass.h"
#include "CreateBishopsPowerLaw.h"
#include "CreateBishopsSaturationCutoff.h"
#include "CreateClausiusClapeyron.h"
#include "CreateConstant.h"
#include "CreateCurve.h"
#include "CreateDupuitPermeability.h"
#include "CreateEffectiveThermalConductivityPorosityMixing.h"
#include "CreateEmbeddedFracturePermeability.h"
#include "CreateExponential.h"
#include "CreateFunction.h"
#include "CreateGasPressureDependentPermeability.h"
#include "CreateIdealGasLaw.h"
#include "CreateIdealGasLawBinaryMixture.h"
#include "CreateKozenyCarmanModel.h"
#include "CreateLinear.h"
#include "CreateOrthotropicEmbeddedFracturePermeability.h"
#include "CreateParameter.h"
#include "CreatePermeabilityMohrCoulombFailureIndexModel.h"
#include "CreatePermeabilityOrthotropicPowerLaw.h"
#include "CreatePorosityFromMassBalance.h"
#include "CreateSaturationDependentSwelling.h"
#include "CreateSaturationDependentThermalConductivity.h"
#include "CreateSpecificHeatCapacityWithLatentHeat.h"
#include "CreateStrainDependentPermeability.h"
#include "CreateTemperatureDependentDiffusion.h"
#include "CreateTemperatureDependentFraction.h"
#include "CreateTransportPorosityFromMassBalance.h"
#include "CreateVermaPruessModel.h"
#include "CreateVolumeFractionAverage.h"
#include "CreateWaterSaturationTemperatureIAPWSIF97Region4.h"
#include "Density/CreateWaterDensityIAPWSIF97Region1.h"
#include "Density/CreateWaterVapourDensity.h"
#include "Enthalpy/CreateLinearWaterVapourLatentHeat.h"
#include "Enthalpy/CreateWaterVapourLatentHeatWithCriticalTemperature.h"
#include "RelativePermeability/CreateRelPermBrooksCorey.h"
#include "RelativePermeability/CreateRelPermBrooksCoreyNonwettingPhase.h"
#include "RelativePermeability/CreateRelPermLiakopoulos.h"
#include "RelativePermeability/CreateRelPermNonWettingPhaseVanGenuchtenMualem.h"
#include "RelativePermeability/CreateRelPermUdell.h"
#include "RelativePermeability/CreateRelPermUdellNonwettingPhase.h"
#include "RelativePermeability/CreateRelPermVanGenuchten.h"
#include "SwellingStress/CreateLinearSaturationSwellingStress.h"
#include "ThermalConductivity/CreateSaturationWeightedThermalConductivity.h"
#include "ThermalConductivity/CreateSoilThermalConductivitySomerton.h"
#include "ThermalConductivity/CreateWaterThermalConductivityIAPWS.h"
#include "VapourDiffusion/CreateVapourDiffusionFEBEX.h"
#include "VapourDiffusion/CreateVapourDiffusionPMQ.h"
#include "Viscosity/CreateLiquidViscosityVogels.h"
#include "Viscosity/CreateWaterViscosityIAPWS.h"
