/**
 * \file
 * \author Norihiro Watanabe
 * \date   2012-08-03
 *
 * \copyright
 * Copyright (c) 2012-2023, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/project/license
 */

#pragma once

#include <memory>
#include <vector>

#include "TimeStepAlgorithm.h"

namespace NumLib
{
/**
 * \brief Fixed time stepping algorithm
 *
 * This algorithm returns time step size defined by a user priori.
 */
class FixedTimeStepping final : public TimeStepAlgorithm
{
public:
    /**
     * Constructor with homogeneous time step size
     *
     * A user provides a single time step size \f$\Delta t\f$. Total number of
     * time steps is calculated by
     * \f[
     *  n=\frac{t_{\rm n} - t_0}{\Delta t}
     * \f].
     *
     * @param t0 start time
     * @param t_end end time
     * @param dt uniform time step size
     */
    FixedTimeStepping(double t0, double t_end, double dt);

    /**
     * Constructor with user-specified time step sizes
     *
     * A user can specify \f$\Delta t\f$ for each time step (i.e. \f$\Delta t_1,
     * \Delta t_2, ..., \Delta t_n\f$). Time at \f$m\f$ th step is given as
     * \f[
     *  t_{m}=\sum_{i=1}^m \Delta t_i + t_0
     * \f].
     *
     * @param t0         start time
     * @param tn         finish time
     * @param vec_all_dt a vector of all time steps
     */
    FixedTimeStepping(double t0, double tn,
                      const std::vector<double>& vec_all_dt);

    std::tuple<bool, double> next(double solution_error, int number_iterations,
                                  NumLib::TimeStep& ts_previous,
                                  NumLib::TimeStep& ts_current) override;

    /// reset the current step size from the previous time
    void resetCurrentTimeStep(const double dt, TimeStep& /*ts_previous*/,
                              TimeStep& /*ts_current*/) override
    {
        _dt_vector.push_back(dt);
    }

private:
    /// a vector of time step sizes
    std::vector<double> _dt_vector;
};

}  // namespace NumLib
