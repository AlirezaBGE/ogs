/**
 * \file
 * \copyright
 * Copyright (c) 2012-2023, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/project/license
 *
 */

#include "TH2MProcess.h"

#include <cassert>

#include "CreateTH2MLocalAssemblers.h"
#include "MaterialLib/SolidModels/MechanicsBase.h"  // for the instantiation of process data
#include "MathLib/KelvinVector.h"
#include "MeshLib/Elements/Utils.h"
#include "NumLib/DOF/DOFTableUtil.h"
#include "ProcessLib/Process.h"
#include "ProcessLib/Utils/ComputeResiduum.h"
#include "ProcessLib/Utils/SetIPDataInitialConditions.h"
#include "TH2MProcessData.h"

namespace ProcessLib
{
namespace TH2M
{
template <int DisplacementDim>
TH2MProcess<DisplacementDim>::TH2MProcess(
    std::string name,
    MeshLib::Mesh& mesh,
    std::unique_ptr<ProcessLib::AbstractJacobianAssembler>&& jacobian_assembler,
    std::vector<std::unique_ptr<ParameterLib::ParameterBase>> const& parameters,
    unsigned const integration_order,
    std::vector<std::vector<std::reference_wrapper<ProcessVariable>>>&&
        process_variables,
    TH2MProcessData<DisplacementDim>&& process_data,
    SecondaryVariableCollection&& secondary_variables,
    bool const use_monolithic_scheme)
    : Process(std::move(name), mesh, std::move(jacobian_assembler), parameters,
              integration_order, std::move(process_variables),
              std::move(secondary_variables), use_monolithic_scheme),
      _process_data(std::move(process_data))
{
    // TODO (naumov) remove ip suffix. Probably needs modification of the mesh
    // properties, s.t. there is no "overlapping" with cell/point data.
    // See getOrCreateMeshProperty.
    _integration_point_writer.emplace_back(
        std::make_unique<MeshLib::IntegrationPointWriter>(
            "sigma_ip",
            static_cast<int>(mesh.getDimension() == 2 ? 4 : 6) /*n components*/,
            integration_order, local_assemblers_,
            &LocalAssemblerInterface::getSigma));

    _integration_point_writer.emplace_back(
        std::make_unique<MeshLib::IntegrationPointWriter>(
            "swelling_stress_ip",
            static_cast<int>(mesh.getDimension() == 2 ? 4 : 6) /*n components*/,
            integration_order, local_assemblers_,
            &LocalAssemblerInterface::getSwellingStress));

    _integration_point_writer.emplace_back(
        std::make_unique<MeshLib::IntegrationPointWriter>(
            "saturation_ip", 1 /*n components*/, integration_order,
            local_assemblers_, &LocalAssemblerInterface::getSaturation));

    _integration_point_writer.emplace_back(
        std::make_unique<MeshLib::IntegrationPointWriter>(
            "epsilon_ip",
            static_cast<int>(mesh.getDimension() == 2 ? 4 : 6) /*n components*/,
            integration_order, local_assemblers_,
            &LocalAssemblerInterface::getEpsilon));
}

template <int DisplacementDim>
bool TH2MProcess<DisplacementDim>::isLinear() const
{
    return false;
}

template <int DisplacementDim>
MathLib::MatrixSpecifications
TH2MProcess<DisplacementDim>::getMatrixSpecifications(
    const int process_id) const
{
    // For the monolithic scheme or the M process (deformation) in the staggered
    // scheme.
    if (_use_monolithic_scheme || process_id == deformation_process_id)
    {
        auto const& l = *_local_to_global_index_map;
        return {l.dofSizeWithoutGhosts(), l.dofSizeWithoutGhosts(),
                &l.getGhostIndices(), &this->_sparsity_pattern};
    }

    // For staggered scheme and T or H process (pressure).
    auto const& l = *_local_to_global_index_map_with_base_nodes;
    return {l.dofSizeWithoutGhosts(), l.dofSizeWithoutGhosts(),
            &l.getGhostIndices(), &_sparsity_pattern_with_linear_element};
}

template <int DisplacementDim>
void TH2MProcess<DisplacementDim>::constructDofTable()
{
    // Create single component dof in every of the mesh's nodes.
    _mesh_subset_all_nodes = std::make_unique<MeshLib::MeshSubset>(
        _mesh, _mesh.getNodes(), _process_data.use_TaylorHood_elements);
    // Create single component dof in the mesh's base nodes.
    _base_nodes = MeshLib::getBaseNodes(_mesh.getElements());
    _mesh_subset_base_nodes = std::make_unique<MeshLib::MeshSubset>(
        _mesh, _base_nodes, _process_data.use_TaylorHood_elements);

    // TODO move the two data members somewhere else.
    // for extrapolation of secondary variables of stress or strain
    std::vector<MeshLib::MeshSubset> all_mesh_subsets_single_component{
        *_mesh_subset_all_nodes};
    _local_to_global_index_map_single_component =
        std::make_unique<NumLib::LocalToGlobalIndexMap>(
            std::move(all_mesh_subsets_single_component),
            // by location order is needed for output
            NumLib::ComponentOrder::BY_LOCATION);

    if (_use_monolithic_scheme)
    {
        // For gas pressure, which is the first
        std::vector<MeshLib::MeshSubset> all_mesh_subsets{
            *_mesh_subset_base_nodes};

        // For capillary pressure, which is the second
        all_mesh_subsets.push_back(*_mesh_subset_base_nodes);

        // For temperature, which is the third
        all_mesh_subsets.push_back(*_mesh_subset_base_nodes);

        // For displacement.
        std::generate_n(
            std::back_inserter(all_mesh_subsets),
            getProcessVariables(monolithic_process_id)[deformation_process_id]
                .get()
                .getNumberOfGlobalComponents(),
            [&]() { return *_mesh_subset_all_nodes; });

        std::vector<int> const vec_n_components{
            n_gas_pressure_components, n_capillary_pressure_components,
            n_temperature_components, n_displacement_components};

        _local_to_global_index_map =
            std::make_unique<NumLib::LocalToGlobalIndexMap>(
                std::move(all_mesh_subsets), vec_n_components,
                NumLib::ComponentOrder::BY_LOCATION);
        assert(_local_to_global_index_map);
    }
    else
    {
        OGS_FATAL("A Staggered version of TH2M is not implemented.");
    }
}

template <int DisplacementDim>
void TH2MProcess<DisplacementDim>::initializeConcreteProcess(
    NumLib::LocalToGlobalIndexMap const& dof_table,
    MeshLib::Mesh const& mesh,
    unsigned const integration_order)
{
    createLocalAssemblers<DisplacementDim>(
        mesh.getElements(), dof_table, local_assemblers_,
        NumLib::IntegrationOrder{integration_order}, mesh.isAxiallySymmetric(),
        _process_data);

    auto add_secondary_variable = [&](std::string const& name,
                                      int const num_components,
                                      auto get_ip_values_function)
    {
        _secondary_variables.addSecondaryVariable(
            name,
            makeExtrapolator(num_components, getExtrapolator(),
                             local_assemblers_,
                             std::move(get_ip_values_function)));
    };

    add_secondary_variable("sigma",
                           MathLib::KelvinVector::KelvinVectorType<
                               DisplacementDim>::RowsAtCompileTime,
                           &LocalAssemblerInterface::getIntPtSigma);
    add_secondary_variable("swelling_stress",
                           MathLib::KelvinVector::KelvinVectorType<
                               DisplacementDim>::RowsAtCompileTime,
                           &LocalAssemblerInterface::getIntPtSwellingStress);
    add_secondary_variable("epsilon",
                           MathLib::KelvinVector::KelvinVectorType<
                               DisplacementDim>::RowsAtCompileTime,
                           &LocalAssemblerInterface::getIntPtEpsilon);
    add_secondary_variable("velocity_gas", DisplacementDim,
                           &LocalAssemblerInterface::getIntPtDarcyVelocityGas);
    add_secondary_variable(
        "velocity_liquid", DisplacementDim,
        &LocalAssemblerInterface::getIntPtDarcyVelocityLiquid);
    add_secondary_variable(
        "diffusion_velocity_vapour_gas", DisplacementDim,
        &LocalAssemblerInterface::getIntPtDiffusionVelocityVapourGas);
    add_secondary_variable(
        "diffusion_velocity_gas_gas", DisplacementDim,
        &LocalAssemblerInterface::getIntPtDiffusionVelocityGasGas);
    add_secondary_variable(
        "diffusion_velocity_solute_liquid", DisplacementDim,
        &LocalAssemblerInterface::getIntPtDiffusionVelocitySoluteLiquid);
    add_secondary_variable(
        "diffusion_velocity_liquid_liquid", DisplacementDim,
        &LocalAssemblerInterface::getIntPtDiffusionVelocityLiquidLiquid);

    add_secondary_variable("saturation", 1,
                           &LocalAssemblerInterface::getIntPtSaturation);
    add_secondary_variable("vapour_pressure", 1,
                           &LocalAssemblerInterface::getIntPtVapourPressure);
    add_secondary_variable("porosity", 1,
                           &LocalAssemblerInterface::getIntPtPorosity);
    add_secondary_variable("gas_density", 1,
                           &LocalAssemblerInterface::getIntPtGasDensity);
    add_secondary_variable("solid_density", 1,
                           &LocalAssemblerInterface::getIntPtSolidDensity);
    add_secondary_variable("liquid_density", 1,
                           &LocalAssemblerInterface::getIntPtLiquidDensity);
    add_secondary_variable("mole_fraction_gas", 1,
                           &LocalAssemblerInterface::getIntPtMoleFractionGas);
    add_secondary_variable("mass_fraction_gas", 1,
                           &LocalAssemblerInterface::getIntPtMassFractionGas);
    add_secondary_variable(
        "mass_fraction_liquid", 1,
        &LocalAssemblerInterface::getIntPtMassFractionLiquid);

    add_secondary_variable(
        "relative_permeability_gas", 1,
        &LocalAssemblerInterface::getIntPtRelativePermeabilityGas);
    add_secondary_variable(
        "relative_permeability_liquid", 1,
        &LocalAssemblerInterface::getIntPtRelativePermeabilityLiquid);

    add_secondary_variable(
        "intrinsic_permeability", DisplacementDim * DisplacementDim,
        &LocalAssemblerInterface::getIntPtIntrinsicPermeability);

    add_secondary_variable("enthalpy_gas", 1,
                           &LocalAssemblerInterface::getIntPtEnthalpyGas);

    add_secondary_variable("enthalpy_liquid", 1,
                           &LocalAssemblerInterface::getIntPtEnthalpyLiquid);

    add_secondary_variable("enthalpy_solid", 1,
                           &LocalAssemblerInterface::getIntPtEnthalpySolid);

    _process_data.element_saturation = MeshLib::getOrCreateMeshProperty<double>(
        const_cast<MeshLib::Mesh&>(mesh), "saturation_avg",
        MeshLib::MeshItemType::Cell, 1);

    _process_data.gas_pressure_interpolated =
        MeshLib::getOrCreateMeshProperty<double>(
            const_cast<MeshLib::Mesh&>(mesh), "gas_pressure_interpolated",
            MeshLib::MeshItemType::Node, 1);

    _process_data.capillary_pressure_interpolated =
        MeshLib::getOrCreateMeshProperty<double>(
            const_cast<MeshLib::Mesh&>(mesh), "capillary_pressure_interpolated",
            MeshLib::MeshItemType::Node, 1);

    _process_data.liquid_pressure_interpolated =
        MeshLib::getOrCreateMeshProperty<double>(
            const_cast<MeshLib::Mesh&>(mesh), "liquid_pressure_interpolated",
            MeshLib::MeshItemType::Node, 1);

    _process_data.temperature_interpolated =
        MeshLib::getOrCreateMeshProperty<double>(
            const_cast<MeshLib::Mesh&>(mesh), "temperature_interpolated",
            MeshLib::MeshItemType::Node, 1);

    setIPDataInitialConditions(_integration_point_writer, mesh.getProperties(),
                               local_assemblers_);

    // Initialize local assemblers after all variables have been set.
    GlobalExecutor::executeMemberOnDereferenced(
        &LocalAssemblerInterface::initialize, local_assemblers_,
        *_local_to_global_index_map);
}

template <int DisplacementDim>
void TH2MProcess<DisplacementDim>::initializeBoundaryConditions()
{
    if (_use_monolithic_scheme)
    {
        initializeProcessBoundaryConditionsAndSourceTerms(
            *_local_to_global_index_map, monolithic_process_id);
        return;
    }

    // Staggered scheme:
    OGS_FATAL("A Staggered version of TH2M is not implemented.");
}

template <int DisplacementDim>
void TH2MProcess<DisplacementDim>::setInitialConditionsConcreteProcess(
    std::vector<GlobalVector*>& x, double const t, int const process_id)
{
    if (process_id != 0)
    {
        return;
    }

    DBUG("Set initial conditions of TH2MProcess.");

    GlobalExecutor::executeMemberOnDereferenced(
        &LocalAssemblerInterface::setInitialConditions, local_assemblers_,
        getDOFTable(process_id), *x[process_id], t, _use_monolithic_scheme,
        process_id);
}

template <int DisplacementDim>
void TH2MProcess<DisplacementDim>::assembleConcreteProcess(
    const double t, double const dt, std::vector<GlobalVector*> const& x,
    std::vector<GlobalVector*> const& xdot, int const process_id,
    GlobalMatrix& M, GlobalMatrix& K, GlobalVector& b)
{
    DBUG("Assemble the equations for TH2M");

    AssemblyMixin<TH2MProcess<DisplacementDim>>::assemble(t, dt, x, xdot,
                                                          process_id, M, K, b);
}

template <int DisplacementDim>
void TH2MProcess<DisplacementDim>::assembleWithJacobianConcreteProcess(
    const double t, double const dt, std::vector<GlobalVector*> const& x,
    std::vector<GlobalVector*> const& xdot, int const process_id,
    GlobalMatrix& M, GlobalMatrix& K, GlobalVector& b, GlobalMatrix& Jac)
{
    if (!_use_monolithic_scheme)
    {
        OGS_FATAL("A Staggered version of TH2M is not implemented.");
    }

    AssemblyMixin<TH2MProcess<DisplacementDim>>::assembleWithJacobian(
        t, dt, x, xdot, process_id, M, K, b, Jac);
}

template <int DisplacementDim>
void TH2MProcess<DisplacementDim>::preTimestepConcreteProcess(
    std::vector<GlobalVector*> const& x, double const t, double const dt,
    const int process_id)
{
    DBUG("PreTimestep TH2MProcess.");

    if (hasMechanicalProcess(process_id))
    {
        ProcessLib::ProcessVariable const& pv =
            getProcessVariables(process_id)[0];

        GlobalExecutor::executeSelectedMemberOnDereferenced(
            &LocalAssemblerInterface::preTimestep, local_assemblers_,
            pv.getActiveElementIDs(), *_local_to_global_index_map,
            *x[process_id], t, dt);
    }

    AssemblyMixin<TH2MProcess<DisplacementDim>>::updateActiveElements(
        process_id);
}

template <int DisplacementDim>
void TH2MProcess<DisplacementDim>::postTimestepConcreteProcess(
    std::vector<GlobalVector*> const& x,
    std::vector<GlobalVector*> const& x_dot, double const t, double const dt,
    const int process_id)
{
    DBUG("PostTimestep TH2MProcess.");
    auto const dof_tables = getDOFTables(x.size());
    ProcessLib::ProcessVariable const& pv = getProcessVariables(process_id)[0];
    GlobalExecutor::executeSelectedMemberOnDereferenced(
        &LocalAssemblerInterface::postTimestep, local_assemblers_,
        pv.getActiveElementIDs(), dof_tables, x, x_dot, t, dt);
}

template <int DisplacementDim>
void TH2MProcess<DisplacementDim>::computeSecondaryVariableConcrete(
    double const t, double const dt, std::vector<GlobalVector*> const& x,
    GlobalVector const& x_dot, const int process_id)
{
    if (process_id != 0)
    {
        return;
    }

    DBUG("Compute the secondary variables for TH2MProcess.");
    auto const dof_tables = getDOFTables(x.size());

    ProcessLib::ProcessVariable const& pv = getProcessVariables(process_id)[0];
    GlobalExecutor::executeSelectedMemberOnDereferenced(
        &LocalAssemblerInterface::computeSecondaryVariable, local_assemblers_,
        pv.getActiveElementIDs(), dof_tables, t, dt, x, x_dot, process_id);
}

template <int DisplacementDim>
std::vector<std::string>
TH2MProcess<DisplacementDim>::initializeAssemblyOnSubmeshes(
    std::vector<std::reference_wrapper<MeshLib::Mesh>> const& meshes)
{
    INFO("TH2M process initializeSubmeshOutput().");
    const int process_id = 0;
    std::vector<std::string> residuum_names{
        "GasMassFlowRate", "LiquidMassFlowRate", "HeatFlowRate", "NodalForces"};

    AssemblyMixin<TH2MProcess<DisplacementDim>>::initializeAssemblyOnSubmeshes(
        process_id, meshes, residuum_names);

    return residuum_names;
}

template <int DisplacementDim>
std::tuple<NumLib::LocalToGlobalIndexMap*, bool>
TH2MProcess<DisplacementDim>::getDOFTableForExtrapolatorData() const
{
    const bool manage_storage = false;
    return std::make_tuple(_local_to_global_index_map_single_component.get(),
                           manage_storage);
}

template <int DisplacementDim>
NumLib::LocalToGlobalIndexMap const& TH2MProcess<DisplacementDim>::getDOFTable(
    const int process_id) const
{
    if (hasMechanicalProcess(process_id))
    {
        return *_local_to_global_index_map;
    }

    // For the equation of pressure
    return *_local_to_global_index_map_with_base_nodes;
}

template <int DisplacementDim>
std::vector<NumLib::LocalToGlobalIndexMap const*>
TH2MProcess<DisplacementDim>::getDOFTables(int const number_of_processes) const
{
    std::vector<NumLib::LocalToGlobalIndexMap const*> dof_tables;
    dof_tables.reserve(number_of_processes);
    std::generate_n(std::back_inserter(dof_tables), number_of_processes,
                    [&]() { return &getDOFTable(dof_tables.size()); });
    return dof_tables;
}
template class TH2MProcess<2>;
template class TH2MProcess<3>;

}  // namespace TH2M
}  // namespace ProcessLib
