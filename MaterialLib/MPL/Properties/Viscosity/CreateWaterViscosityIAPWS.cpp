/**
 * \file
 * \copyright
 * Copyright (c) 2012-2023, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/project/license
 *
 * Created on March 4, 2021, 4:38 PM
 */

#include "CreateWaterViscosityIAPWS.h"

#include "BaseLib/ConfigTree.h"
#include "MaterialLib/MPL/Property.h"
#include "WaterViscosityIAPWS.h"

namespace MaterialPropertyLib
{
std::unique_ptr<Property> createWaterViscosityIAPWS(
    BaseLib::ConfigTree const& config)
{
    //! \ogs_file_param{properties__property__type}
    config.checkConfigParameter("type", "WaterViscosityIAPWS");
    DBUG("Create WaterViscosityIAPWS phase property");

    // Second access for storage.
    //! \ogs_file_param{properties__property__name}
    auto property_name = config.peekConfigParameter<std::string>("name");

    //! \ogs_file_param_special{properties__property__WaterViscosityIAPWS}
    return std::make_unique<WaterViscosityIAPWS>(std::move(property_name));
}
}  // namespace MaterialPropertyLib