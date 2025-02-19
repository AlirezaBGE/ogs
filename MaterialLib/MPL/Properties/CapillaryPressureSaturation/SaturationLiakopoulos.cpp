/**
 * \file
 * \author Norbert Grunwald
 * \date   27.06.2018
 * \brief
 *
 * \copyright
 * Copyright (c) 2012-2023, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/project/license
 *
 */

#include "SaturationLiakopoulos.h"

#include <algorithm>
#include <cmath>

#include "MaterialLib/MPL/Medium.h"
#include "MathLib/MathTools.h"

namespace MaterialPropertyLib
{
PropertyDataType SaturationLiakopoulos::value(
    VariableArray const& variable_array,
    ParameterLib::SpatialPosition const& /*pos*/, double const /*t*/,
    double const /*dt*/) const
{
    const double p_cap = variable_array.capillary_pressure;

    if (p_cap < 0.)
        return 1.;

    return std::max(residual_liquid_saturation_,
                    1. - parameter_a_ * std::pow(p_cap, parameter_b_));
}

PropertyDataType SaturationLiakopoulos::dValue(
    VariableArray const& variable_array, Variable const variable,
    ParameterLib::SpatialPosition const& /*pos*/, double const /*t*/,
    double const /*dt*/) const
{
    if (variable != Variable::capillary_pressure)
    {
        OGS_FATAL(
            "SaturationLiakopoulos::dValue is implemented for derivatives with "
            "respect to capillary pressure only.");
    }

    const double p_cap = variable_array.capillary_pressure;
    if (p_cap <= 0.)
    {
        return 0.;
    }
    const double p_cap_restricted = std::min(p_cap, p_cap_max_);

    return -parameter_a_ * parameter_b_ *
           std::pow(p_cap_restricted, parameter_b_ - 1.);
}

PropertyDataType SaturationLiakopoulos::d2Value(
    VariableArray const& variable_array, Variable const variable1,
    Variable const variable2, ParameterLib::SpatialPosition const& /*pos*/,
    double const /*t*/, double const /*dt*/) const
{
    if ((variable1 != Variable::capillary_pressure) &&
        (variable2 != Variable::capillary_pressure))
    {
        OGS_FATAL(
            "SaturationLiakopoulos::d2Value is implemented for derivatives "
            "with respect to capillary pressure only.");
    }

    const double p_cap = variable_array.capillary_pressure;

    if (p_cap < 0.)
    {
        return 0.;
    }
    const double p_cap_restricted = std::min(p_cap, p_cap_max_);
    return -parameter_a_ * (parameter_b_ - 1.) * parameter_b_ *
           std::pow(p_cap_restricted, parameter_b_ - 2.);
}

}  // namespace MaterialPropertyLib
