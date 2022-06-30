if (NOT OGS_USE_MPI)
    OgsTest(PROJECTFILE ThermoRichardsMechanics/LinearMechanics/mechanics_linear.prj)
    OgsTest(PROJECTFILE ThermoRichardsMechanics/FullySaturatedFlowMechanics/flow_fully_saturated.prj)
    OgsTest(PROJECTFILE ThermoRichardsMechanics/RichardsFlow2D/RichardsFlow_2d_small.prj RUNTIME 10)
    OgsTest(PROJECTFILE ThermoRichardsMechanics/A2/A2.prj RUNTIME 18)
    OgsTest(PROJECTFILE ThermoRichardsMechanics/OrthotropicSwelling/orthotropic_swelling_xy.xml)
    OgsTest(PROJECTFILE ThermoRichardsMechanics/OrthotropicSwelling/orthotropic_swelling_xyz.xml)
endif()

AddTest(
    NAME ThermoRichardsMechanics_liakopoulosHM
    PATH ThermoRichardsMechanics/LiakopoulosHM
    EXECUTABLE ogs
    EXECUTABLE_ARGS liakopoulos.prj
    WRAPPER time
    TESTER vtkdiff
    REQUIREMENTS NOT (OGS_USE_LIS OR OGS_USE_MPI)
    RUNTIME 17
    DIFF_DATA
    liakopoulos_t_300.vtu liakopoulos_t_300.vtu sigma sigma 1e-9 1e-12
    liakopoulos_t_300.vtu liakopoulos_t_300.vtu displacement displacement 1e-10 1e-12
    liakopoulos_t_300.vtu liakopoulos_t_300.vtu saturation saturation 1e-10 1e-12
    liakopoulos_t_600.vtu liakopoulos_t_600.vtu sigma sigma 1e-9 1e-12
    liakopoulos_t_600.vtu liakopoulos_t_600.vtu displacement displacement 1e-10 1e-12
    liakopoulos_t_600.vtu liakopoulos_t_600.vtu saturation saturation 1e-10 1e-12
    liakopoulos_t_7200.vtu liakopoulos_t_7200.vtu sigma sigma 1e-9 1e-12
    liakopoulos_t_7200.vtu liakopoulos_t_7200.vtu displacement displacement 1e-10 1e-12
    liakopoulos_t_7200.vtu liakopoulos_t_7200.vtu saturation saturation 1e-10 1e-12
)

AddTest(
    NAME ThermoRichardsMechanics_3D_ThermoElastic_Stress_Analysis
    PATH ThermoRichardsMechanics/Simple3DThermoMechanicsFromTM
    EXECUTABLE ogs
    EXECUTABLE_ARGS cube_1e3.prj
    WRAPPER time
    TESTER vtkdiff
    REQUIREMENTS NOT (OGS_USE_LIS OR OGS_USE_MPI)
    RUNTIME 17
    DIFF_DATA
    stress_analytical.vtu cube_1e3_tm_ts_17_t_72000.000000.vtu sigma sigma 1e-5 1e-12
    expected_cube_1e3_tm_ts_17_t_72000.000000.vtu cube_1e3_tm_ts_17_t_72000.000000.vtu displacement displacement 1e-10 1e-12
    expected_cube_1e3_tm_ts_17_t_72000.000000.vtu cube_1e3_tm_ts_17_t_72000.000000.vtu temperature temperature 1e-10 1e-12
    expected_cube_1e3_tm_ts_17_t_72000.000000.vtu cube_1e3_tm_ts_17_t_72000.000000.vtu sigma sigma 1e-6 1e-12
    expected_cube_1e3_tm_ts_17_t_72000.000000.vtu cube_1e3_tm_ts_17_t_72000.000000.vtu epsilon epsilon 1e-16 0
)

AddTest(
    NAME ThermoRichardsMechanics_HeatTransportInStationaryFlow
    PATH ThermoRichardsMechanics/HeatTransportInStationaryFlow
    EXECUTABLE ogs
    EXECUTABLE_ARGS HeatTransportInStationaryFlow.prj
    WRAPPER time
    TESTER vtkdiff
    REQUIREMENTS NOT (OGS_USE_LIS OR OGS_USE_MPI)
    RUNTIME 17
    DIFF_DATA
    HT_HeatTransportInStationaryFlow_ts_50_t_50000.000000.vtu HeatTransportInStationaryFlow_ts_50_t_50000.000000.vtu temperature  temperature 1e-6 1e-10
    HT_HeatTransportInStationaryFlow_ts_50_t_50000.000000.vtu HeatTransportInStationaryFlow_ts_50_t_50000.000000.vtu pressure  pressure 1e-10 1e-10
)

AddTest(
    NAME ThermoRichardsMechanics_point_heat_injection
    PATH ThermoRichardsMechanics/PointHeatSource
    RUNTIME 25
    EXECUTABLE ogs
    EXECUTABLE_ARGS point_heat_source_2D.prj
    WRAPPER time
    TESTER vtkdiff
    REQUIREMENTS NOT OGS_USE_MPI
    DIFF_DATA
    expected_pointheatsource_quadratic-mesh_ts_10_t_50000.000000.vtu PointHeatSource_ts_10_t_50000.000000.vtu displacement displacement 1e-6 5.0e-3
    expected_pointheatsource_quadratic-mesh_ts_10_t_50000.000000.vtu PointHeatSource_ts_10_t_50000.000000.vtu pressure pressure 0.2 0.2
    expected_pointheatsource_quadratic-mesh_ts_10_t_50000.000000.vtu PointHeatSource_ts_10_t_50000.000000.vtu temperature temperature 5e-5 5e-4
    expected_pointheatsource_quadratic-mesh_ts_10_t_50000.000000.vtu PointHeatSource_ts_10_t_50000.000000.vtu epsilon epsilon 5e-6 1e-5
    expected_pointheatsource_quadratic-mesh_ts_10_t_50000.000000.vtu PointHeatSource_ts_10_t_50000.000000.vtu sigma sigma 200.0 200.0
)

AddTest(
    NAME ThermoRichardsMechanics_TaskCDECOVALEX2023
    PATH ThermoRichardsMechanics/TaskCDECOVALEX2023
    EXECUTABLE ogs
    EXECUTABLE_ARGS Decovalex-0.prj
    WRAPPER time
    TESTER vtkdiff
    REQUIREMENTS NOT (OGS_USE_LIS OR OGS_USE_MPI)
    RUNTIME 17
    DIFF_DATA
    Decovalex-0_ts_10_t_864000.000000.vtu Decovalex-0_ts_10_t_864000.000000.vtu sigma sigma 1e-9 1e-8
    Decovalex-0_ts_10_t_864000.000000.vtu Decovalex-0_ts_10_t_864000.000000.vtu displacement displacement 1e-10 1e-12
    Decovalex-0_ts_10_t_864000.000000.vtu Decovalex-0_ts_10_t_864000.000000.vtu saturation saturation 1e-10 1e-12
    Decovalex-0_ts_10_t_864000.000000.vtu Decovalex-0_ts_10_t_864000.000000.vtu temperature temperature 1e-10 1e-12
    Decovalex-0_ts_10_t_864000.000000.vtu Decovalex-0_ts_10_t_864000.000000.vtu velocity velocity 1e-10 1e-12
    Decovalex-0_ts_10_t_864000.000000.vtu Decovalex-0_ts_10_t_864000.000000.vtu liquid_density liquid_density 1e-10 1e-11
)

AddTest(
    NAME ThermoRichardsMechanics_CTF1
    PATH ThermoRichardsMechanics/CTF1
    EXECUTABLE ogs
    EXECUTABLE_ARGS CTF1.prj
    WRAPPER time
    TESTER vtkdiff
    REQUIREMENTS NOT (OGS_USE_LIS OR OGS_USE_MPI)
    RUNTIME 17
    DIFF_DATA
    CTF1_14.000000.vtu CTF1_14.000000.vtu sigma sigma 1e-9 1e-8
    CTF1_14.000000.vtu CTF1_14.000000.vtu displacement displacement 1e-10 1e-12
    CTF1_14.000000.vtu CTF1_14.000000.vtu saturation saturation 1e-10 1e-12
    CTF1_14.000000.vtu CTF1_14.000000.vtu temperature temperature 1e-10 1e-12
)

AddTest(
    NAME ThermoRichardsMechanics_CreepBGRa_SimpleAxisymmetricCreep
    PATH ThermoRichardsMechanics/SimpleAxisymmetricCreep
    EXECUTABLE ogs
    EXECUTABLE_ARGS SimpleAxisymmetricCreep.prj
    WRAPPER time
    TESTER vtkdiff
    REQUIREMENTS NOT (OGS_USE_LIS OR OGS_USE_MPI)
    DIFF_DATA
    expected_SimpleAxisymmetricCreep_ts_370_t_360.000000.vtu SimpleAxisymmetricCreep_ts_370_t_360.000000.vtu displacement displacement 1e-13 1e-10
    expected_SimpleAxisymmetricCreep_ts_370_t_360.000000.vtu SimpleAxisymmetricCreep_ts_370_t_360.000000.vtu sigma sigma 2e-7 0
    expected_SimpleAxisymmetricCreep_ts_370_t_360.000000.vtu SimpleAxisymmetricCreep_ts_370_t_360.000000.vtu epsilon epsilon 1e-11 0
)

#PETSc
AddTest(
    NAME ParallelFEM_ThermoRichardsMechanics_3D_ThermoElastic_Stress_Analysis
    PATH ThermoRichardsMechanics/Simple3DThermoMechanicsFromTM
    RUNTIME 280
    EXECUTABLE ogs
    EXECUTABLE_ARGS cube_1e3.prj
    WRAPPER mpirun
    WRAPPER_ARGS -np 3
    TESTER vtkdiff
    REQUIREMENTS OGS_USE_MPI
    DIFF_DATA
    cube_1e3_tm_ts_17_t_72000_000000_0.vtu cube_1e3_tm_ts_17_t_72000_000000_0.vtu velocity velocity 1e-10 1e-9
    cube_1e3_tm_ts_17_t_72000_000000_1.vtu cube_1e3_tm_ts_17_t_72000_000000_1.vtu velocity velocity 1e-10 1e-9
    cube_1e3_tm_ts_17_t_72000_000000_0.vtu cube_1e3_tm_ts_17_t_72000_000000_0.vtu displacement displacement 1e-10 1e-9
    cube_1e3_tm_ts_17_t_72000_000000_1.vtu cube_1e3_tm_ts_17_t_72000_000000_1.vtu displacement displacement 1e-10 1e-9
    cube_1e3_tm_ts_17_t_72000_000000_0.vtu cube_1e3_tm_ts_17_t_72000_000000_0.vtu sigma sigma 1e-7 1e-9
    cube_1e3_tm_ts_17_t_72000_000000_1.vtu cube_1e3_tm_ts_17_t_72000_000000_1.vtu sigma sigma 1e-7 1e-9
    cube_1e3_tm_ts_17_t_72000_000000_0.vtu cube_1e3_tm_ts_17_t_72000_000000_0.vtu epsilon epsilon 1e-10 1e-9
    cube_1e3_tm_ts_17_t_72000_000000_1.vtu cube_1e3_tm_ts_17_t_72000_000000_1.vtu epsilon epsilon 1e-10 1e-9
)

AddTest(
    NAME ParallelFEM_ThermoRichardsMechanics_FullySaturatedFlowMechanics
    PATH ThermoRichardsMechanics/FullySaturatedFlowMechanics/PETSc
    RUNTIME 10
    EXECUTABLE ogs
    EXECUTABLE_ARGS flow_fully_saturated_petsc.prj
    WRAPPER mpirun
    WRAPPER_ARGS -np 2
    TESTER vtkdiff
    REQUIREMENTS OGS_USE_MPI
    DIFF_DATA
    flow_fully_saturated_ts_2_t_2_000000_0.vtu flow_fully_saturated_ts_2_t_2_000000_0.vtu pressure pressure 1e-10 1e-9
    flow_fully_saturated_ts_2_t_2_000000_0.vtu flow_fully_saturated_ts_2_t_2_000000_0.vtu velocity velocity 1e-10 1e-9
    flow_fully_saturated_ts_2_t_2_000000_1.vtu flow_fully_saturated_ts_2_t_2_000000_1.vtu velocity velocity 1e-10 1e-9
    flow_fully_saturated_ts_2_t_2_000000_0.vtu flow_fully_saturated_ts_2_t_2_000000_0.vtu displacement displacement 1e-10 1e-9
    flow_fully_saturated_ts_2_t_2_000000_1.vtu flow_fully_saturated_ts_2_t_2_000000_1.vtu displacement displacement 1e-10 1e-9
    flow_fully_saturated_ts_2_t_2_000000_0.vtu flow_fully_saturated_ts_2_t_2_000000_0.vtu sigma sigma 1e-10 1e-9
    flow_fully_saturated_ts_2_t_2_000000_1.vtu flow_fully_saturated_ts_2_t_2_000000_1.vtu sigma sigma 1e-10 1e-9
    flow_fully_saturated_ts_2_t_2_000000_0.vtu flow_fully_saturated_ts_2_t_2_000000_0.vtu epsilon epsilon 1e-10 1e-9
    flow_fully_saturated_ts_2_t_2_000000_1.vtu flow_fully_saturated_ts_2_t_2_000000_1.vtu epsilon epsilon 1e-10 1e-9
)

AddTest(
    NAME ParallelFEM_ThermoRichardsMechanics_point_heat_injection
    PATH ThermoRichardsMechanics/PointHeatSource
    RUNTIME 635
    EXECUTABLE ogs
    EXECUTABLE_ARGS point_heat_source_2D.prj
    WRAPPER mpirun
    WRAPPER_ARGS -np 3
    TESTER vtkdiff
    REQUIREMENTS OGS_USE_MPI
    DIFF_DATA
    PointHeatSource_ts_10_t_50000_000000_0.vtu PointHeatSource_ts_10_t_50000_000000_0.vtu displacement displacement 1e-10 1.0e-9
    PointHeatSource_ts_10_t_50000_000000_0.vtu PointHeatSource_ts_10_t_50000_000000_0.vtu pressure pressure 1e-10 1.0e-6
    PointHeatSource_ts_10_t_50000_000000_0.vtu PointHeatSource_ts_10_t_50000_000000_0.vtu temperature temperature 1e-10 1.0e-9
    PointHeatSource_ts_10_t_50000_000000_0.vtu PointHeatSource_ts_10_t_50000_000000_0.vtu epsilon epsilon 1e-10 1.0e-9
    PointHeatSource_ts_10_t_50000_000000_0.vtu PointHeatSource_ts_10_t_50000_000000_0.vtu sigma sigma 1e-10 1.0e-6
#
    PointHeatSource_ts_10_t_50000_000000_1.vtu PointHeatSource_ts_10_t_50000_000000_1.vtu displacement displacement 1e-10 1.0e-9
    PointHeatSource_ts_10_t_50000_000000_1.vtu PointHeatSource_ts_10_t_50000_000000_1.vtu pressure pressure 1e-10 1.0e-6
    PointHeatSource_ts_10_t_50000_000000_1.vtu PointHeatSource_ts_10_t_50000_000000_1.vtu temperature temperature 1e-10 1.0e-9
    PointHeatSource_ts_10_t_50000_000000_1.vtu PointHeatSource_ts_10_t_50000_000000_1.vtu epsilon epsilon 1e-10 1.0e-9
    PointHeatSource_ts_10_t_50000_000000_1.vtu PointHeatSource_ts_10_t_50000_000000_1.vtu sigma sigma 1e-10 1.0e-6
#
    PointHeatSource_ts_10_t_50000_000000_2.vtu PointHeatSource_ts_10_t_50000_000000_2.vtu displacement displacement 1e-10 1.0e-9
    PointHeatSource_ts_10_t_50000_000000_2.vtu PointHeatSource_ts_10_t_50000_000000_2.vtu pressure pressure 1e-10 1.0e-6
    PointHeatSource_ts_10_t_50000_000000_2.vtu PointHeatSource_ts_10_t_50000_000000_2.vtu temperature temperature 1e-10 1.0e-9
    PointHeatSource_ts_10_t_50000_000000_2.vtu PointHeatSource_ts_10_t_50000_000000_2.vtu epsilon epsilon 1e-10 1.0e-9
    PointHeatSource_ts_10_t_50000_000000_2.vtu PointHeatSource_ts_10_t_50000_000000_2.vtu sigma sigma 1e-10 1.0e-6
)

AddTest(
    NAME ParallelFEM_ThermoRichardsMechanics_point_heat_injection_gml
    PATH ThermoRichardsMechanics/PointHeatSource
    RUNTIME 635
    EXECUTABLE ogs
    EXECUTABLE_ARGS point_heat_source_2D_gml.prj
    WRAPPER mpirun
    WRAPPER_ARGS -np 3
    TESTER vtkdiff
    REQUIREMENTS OGS_USE_MPI
    DIFF_DATA
    PointHeatSource_gml_ts_10_t_50000_000000_0.vtu PointHeatSource_gml_ts_10_t_50000_000000_0.vtu displacement displacement 1e-10 1.0e-9
    PointHeatSource_gml_ts_10_t_50000_000000_0.vtu PointHeatSource_gml_ts_10_t_50000_000000_0.vtu pressure pressure 1e-10 1.0e-6
    PointHeatSource_gml_ts_10_t_50000_000000_0.vtu PointHeatSource_gml_ts_10_t_50000_000000_0.vtu temperature temperature 1e-10 1.0e-9
    PointHeatSource_gml_ts_10_t_50000_000000_0.vtu PointHeatSource_gml_ts_10_t_50000_000000_0.vtu epsilon epsilon 1e-10 1.0e-9
    PointHeatSource_gml_ts_10_t_50000_000000_0.vtu PointHeatSource_gml_ts_10_t_50000_000000_0.vtu sigma sigma 1e-10 1.0e-6
#
    PointHeatSource_gml_ts_10_t_50000_000000_1.vtu PointHeatSource_gml_ts_10_t_50000_000000_1.vtu displacement displacement 1e-10 1.0e-9
    PointHeatSource_gml_ts_10_t_50000_000000_1.vtu PointHeatSource_gml_ts_10_t_50000_000000_1.vtu pressure pressure 1e-10 1.0e-6
    PointHeatSource_gml_ts_10_t_50000_000000_1.vtu PointHeatSource_gml_ts_10_t_50000_000000_1.vtu temperature temperature 1e-10 1.0e-9
    PointHeatSource_gml_ts_10_t_50000_000000_1.vtu PointHeatSource_gml_ts_10_t_50000_000000_1.vtu epsilon epsilon 1e-10 1.0e-9
    PointHeatSource_gml_ts_10_t_50000_000000_1.vtu PointHeatSource_gml_ts_10_t_50000_000000_1.vtu sigma sigma 1e-10 1.0e-6
#
    PointHeatSource_gml_ts_10_t_50000_000000_2.vtu PointHeatSource_gml_ts_10_t_50000_000000_2.vtu displacement displacement 1e-10 1.0e-9
    PointHeatSource_gml_ts_10_t_50000_000000_2.vtu PointHeatSource_gml_ts_10_t_50000_000000_2.vtu pressure pressure 1e-10 1.0e-6
    PointHeatSource_gml_ts_10_t_50000_000000_2.vtu PointHeatSource_gml_ts_10_t_50000_000000_2.vtu temperature temperature 1e-10 1.0e-9
    PointHeatSource_gml_ts_10_t_50000_000000_2.vtu PointHeatSource_gml_ts_10_t_50000_000000_2.vtu epsilon epsilon 1e-10 1.0e-9
    PointHeatSource_gml_ts_10_t_50000_000000_2.vtu PointHeatSource_gml_ts_10_t_50000_000000_2.vtu sigma sigma 1e-10 1.0e-6
)

AddTest(
    NAME ParallelFEM_ThermoRichardsMechanics_TaskCDECOVALEX2023
    PATH ThermoRichardsMechanics/TaskCDECOVALEX2023
    RUNTIME 500
    EXECUTABLE ogs
    EXECUTABLE_ARGS Decovalex-0.prj
    WRAPPER mpirun
    WRAPPER_ARGS -np 3
    TESTER vtkdiff
    REQUIREMENTS OGS_USE_MPI
    DIFF_DATA
#
    Decovalex-0_ts_10_t_864000_000000_0.vtu Decovalex-0_ts_10_t_864000_000000_0.vtu displacement displacement 1e-10 1.0e-9
    Decovalex-0_ts_10_t_864000_000000_0.vtu Decovalex-0_ts_10_t_864000_000000_0.vtu pressure pressure 1e-10 1.0e-6
    Decovalex-0_ts_10_t_864000_000000_0.vtu Decovalex-0_ts_10_t_864000_000000_0.vtu saturation saturation 1e-10 1.0e-6
    Decovalex-0_ts_10_t_864000_000000_0.vtu Decovalex-0_ts_10_t_864000_000000_0.vtu temperature temperature 1e-10 1.0e-9
    Decovalex-0_ts_10_t_864000_000000_0.vtu Decovalex-0_ts_10_t_864000_000000_0.vtu epsilon epsilon 1e-10 1.0e-9
    Decovalex-0_ts_10_t_864000_000000_0.vtu Decovalex-0_ts_10_t_864000_000000_0.vtu sigma sigma 1e-10 1.0e-6
#
    Decovalex-0_ts_10_t_864000_000000_1.vtu Decovalex-0_ts_10_t_864000_000000_1.vtu displacement displacement 1e-10 1.0e-9
    Decovalex-0_ts_10_t_864000_000000_1.vtu Decovalex-0_ts_10_t_864000_000000_1.vtu pressure pressure 1e-10 1.0e-6
    Decovalex-0_ts_10_t_864000_000000_1.vtu Decovalex-0_ts_10_t_864000_000000_1.vtu saturation saturation 1e-10 1.0e-6
    Decovalex-0_ts_10_t_864000_000000_1.vtu Decovalex-0_ts_10_t_864000_000000_1.vtu temperature temperature 1e-10 1.0e-9
    Decovalex-0_ts_10_t_864000_000000_1.vtu Decovalex-0_ts_10_t_864000_000000_1.vtu epsilon epsilon 1e-10 1.0e-9
    Decovalex-0_ts_10_t_864000_000000_1.vtu Decovalex-0_ts_10_t_864000_000000_1.vtu sigma sigma 1e-10 1.0e-6
#
    Decovalex-0_ts_10_t_864000_000000_2.vtu Decovalex-0_ts_10_t_864000_000000_2.vtu displacement displacement 1e-10 1.0e-9
    Decovalex-0_ts_10_t_864000_000000_2.vtu Decovalex-0_ts_10_t_864000_000000_2.vtu pressure pressure 1e-10 1.0e-6
    Decovalex-0_ts_10_t_864000_000000_2.vtu Decovalex-0_ts_10_t_864000_000000_2.vtu saturation saturation 1e-10 1.0e-6
    Decovalex-0_ts_10_t_864000_000000_2.vtu Decovalex-0_ts_10_t_864000_000000_2.vtu temperature temperature 1e-10 1.0e-9
    Decovalex-0_ts_10_t_864000_000000_2.vtu Decovalex-0_ts_10_t_864000_000000_2.vtu epsilon epsilon 1e-10 1.0e-9
    Decovalex-0_ts_10_t_864000_000000_2.vtu Decovalex-0_ts_10_t_864000_000000_2.vtu sigma sigma 1e-10 1.0e-6
)
# ThermoRichardsMechanics; thermo_osmosis and thermo_filtration effects, linear poroelastic, column consolidation
AddTest(
    NAME ThermoRichardsMechanics_thermo_osmosis_filtration_effects_Column
    PATH ThermoRichardsMechanics/ThermoOsmosis
    RUNTIME 15
    EXECUTABLE ogs
    EXECUTABLE_ARGS Column.prj
    WRAPPER time
    TESTER vtkdiff
    REQUIREMENTS NOT OGS_USE_MPI
    DIFF_DATA
    expected_Column_ts_68_t_7200000.000000.vtu Column_ts_68_t_7200000.000000.vtu displacement displacement 1e-5 1e-5
    expected_Column_ts_68_t_7200000.000000.vtu Column_ts_68_t_7200000.000000.vtu pressure pressure 1e-5 1e-5
    expected_Column_ts_68_t_7200000.000000.vtu Column_ts_68_t_7200000.000000.vtu temperature temperature 1e-5 1e-5
    expected_Column_ts_68_t_7200000.000000.vtu Column_ts_68_t_7200000.000000.vtu epsilon epsilon 1e-5 1e-5
    expected_Column_ts_68_t_7200000.000000.vtu Column_ts_68_t_7200000.000000.vtu sigma sigma 1e-5 1e-5
)
