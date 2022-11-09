/**
 * \file
 * \copyright
 * Copyright (c) 2012-2022, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/project/license
 *
 * Created on February 17, 2021, 3:47 PM
 */

#pragma once

#include <memory>
#include <vector>

namespace BaseLib
{
class ConfigTree;
}

namespace ParameterLib
{
struct CoordinateSystem;
struct ParameterBase;
}  // namespace ParameterLib

namespace MaterialPropertyLib
{
class Property;

std::unique_ptr<Property> createSaturationWeightedThermalConductivity(
    int const geometry_dimension,
    BaseLib::ConfigTree const& config,
    std::vector<std::unique_ptr<ParameterLib::ParameterBase>> const& parameters,
    ParameterLib::CoordinateSystem const* const local_coordinate_system);
}  // namespace MaterialPropertyLib
