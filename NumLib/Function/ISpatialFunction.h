/**
 * \author Norihiro Watanabe
 * \date   2013-08-13
 *
 * \file
 * \copyright
 * Copyright (c) 2012-2023, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/project/license
 */

#pragma once

#include <vector>

#include "MathLib/Point3d.h"

namespace NumLib
{
/**
 * \brief Interface class for any functions of spatial coordinates
 * \f$f(x,y,z)\f$
 */
class ISpatialFunction
{
public:
    virtual ~ISpatialFunction() = default;

    /**
     * return a value at the given point
     * \param pnt  a point object
     * \return evaluated value
     */
    virtual double operator()(const MathLib::Point3d& pnt) const = 0;
};

}  // namespace NumLib
