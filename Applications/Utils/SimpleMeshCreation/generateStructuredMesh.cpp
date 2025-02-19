/**
 * \file
 * \copyright
 * Copyright (c) 2012-2023, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/project/license
 */

#include <tclap/CmdLine.h>

#ifdef USE_PETSC
#include <mpi.h>
#endif

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "BaseLib/Error.h"
#include "BaseLib/Subdivision.h"
#include "BaseLib/TCLAPCustomOutput.h"
#include "InfoLib/GitInfo.h"
#include "MathLib/Point3d.h"
#include "MeshLib/Elements/Element.h"
#include "MeshLib/IO/writeMeshToFile.h"
#include "MeshLib/Mesh.h"
#include "MeshLib/MeshEnums.h"
#include "MeshLib/MeshGenerators/MeshGenerator.h"
#include "MeshLib/Node.h"

namespace
{
/// Get dimension of the mesh element type.
/// @param eleType element type
unsigned getDimension(MeshLib::MeshElemType eleType)
{
    switch (eleType)
    {
        case MeshLib::MeshElemType::LINE:
            return 1;
        case MeshLib::MeshElemType::QUAD:
        case MeshLib::MeshElemType::TRIANGLE:
            return 2;
        case MeshLib::MeshElemType::HEXAHEDRON:
        case MeshLib::MeshElemType::PRISM:
        case MeshLib::MeshElemType::PYRAMID:
        case MeshLib::MeshElemType::TETRAHEDRON:
            return 3;
        case MeshLib::MeshElemType::POINT:
        case MeshLib::MeshElemType::INVALID:
            return 0;
    }
    return 0;
}

}  // end namespace

int main(int argc, char* argv[])
{
    TCLAP::CmdLine cmd(
        "Structured mesh generator.\n"
        "The documentation is available at "
        "https://docs.opengeosys.org/docs/tools/meshing/"
        "structured-mesh-generation.\n\n"
        "OpenGeoSys-6 software, version " +
            GitInfoLib::GitInfo::ogs_version +
            ".\n"
            "Copyright (c) 2012-2023, OpenGeoSys Community "
            "(http://www.opengeosys.org)",
        ' ', GitInfoLib::GitInfo::ogs_version);

    auto tclapOutput = std::make_unique<BaseLib::TCLAPCustomOutput>();
    cmd.setOutput(tclapOutput.get());

    std::vector<std::string> allowed_ele_types;
    allowed_ele_types.emplace_back("line");
    allowed_ele_types.emplace_back("tri");
    allowed_ele_types.emplace_back("quad");
    allowed_ele_types.emplace_back("hex");
    allowed_ele_types.emplace_back("prism");
    allowed_ele_types.emplace_back("tet");
    allowed_ele_types.emplace_back("pyramid");
    TCLAP::ValuesConstraint<std::string> allowedVals(allowed_ele_types);
    TCLAP::ValueArg<std::string> eleTypeArg(
        "e", "element-type",
        "element type to be created: line | tri | quad | hex | prism | tet | "
        "pyramid",
        true, "line", &allowedVals);
    cmd.add(eleTypeArg);
    TCLAP::ValueArg<std::string> mesh_out(
        "o", "mesh-output-file",
        "the name of the file the mesh will be written to", true, "",
        "file name of output mesh");
    cmd.add(mesh_out);
    TCLAP::ValueArg<double> lengthXArg(
        "", "lx", "length of a domain in x direction", false, 10.0, "real");
    cmd.add(lengthXArg);
    TCLAP::ValueArg<double> lengthYArg(
        "", "ly", "length of a domain in y direction", false, 10.0, "real");
    cmd.add(lengthYArg);
    TCLAP::ValueArg<double> lengthZArg(
        "", "lz", "length of a domain in z direction", false, 10.0, "real");
    cmd.add(lengthZArg);
    TCLAP::ValueArg<unsigned> nsubdivXArg(
        "", "nx", "the number of subdivision in x direction", false, 10,
        "integer");
    cmd.add(nsubdivXArg);
    TCLAP::ValueArg<unsigned> nsubdivYArg(
        "", "ny", "the number of subdivision in y direction", false, 10,
        "integer");
    cmd.add(nsubdivYArg);
    TCLAP::ValueArg<unsigned> nsubdivZArg(
        "", "nz", "the number of subdivision in z direction", false, 10,
        "integer");
    cmd.add(nsubdivZArg);
    // in case of gradual refinement
    TCLAP::ValueArg<double> d0XArg(
        "", "dx0", "initial cell length in x direction", false, 1, "real");
    cmd.add(d0XArg);
    TCLAP::ValueArg<double> d0YArg(
        "", "dy0", "initial cell length in y direction", false, 1, "real");
    cmd.add(d0YArg);
    TCLAP::ValueArg<double> d0ZArg(
        "", "dz0", "initial cell length in z direction", false, 1, "real");
    cmd.add(d0ZArg);
    TCLAP::ValueArg<double> dmaxXArg(
        "", "dx-max", "maximum cell length in x direction", false,
        std::numeric_limits<double>::max(), "real");
    cmd.add(dmaxXArg);
    TCLAP::ValueArg<double> dmaxYArg(
        "", "dy-max", "maximum cell length in y direction", false,
        std::numeric_limits<double>::max(), "real");
    cmd.add(dmaxYArg);
    TCLAP::ValueArg<double> dmaxZArg(
        "", "dz-max", "maximum cell length in z direction", false,
        std::numeric_limits<double>::max(), "real");
    cmd.add(dmaxZArg);
    TCLAP::ValueArg<double> multiXArg("", "mx", "multiplier in x direction",
                                      false, 1, "real");
    cmd.add(multiXArg);
    TCLAP::ValueArg<double> multiYArg("", "my", "multiplier in y direction",
                                      false, 1, "real");
    cmd.add(multiYArg);
    TCLAP::ValueArg<double> multiZArg("", "mz", "multiplier in z direction",
                                      false, 1, "real");
    cmd.add(multiZArg);
    TCLAP::ValueArg<double> originXArg(
        "", "ox", "mesh origin (lower left corner) in x direction", false, 0,
        "real");
    cmd.add(originXArg);
    TCLAP::ValueArg<double> originYArg(
        "", "oy", "mesh origin (lower left corner) in y direction", false, 0,
        "real");
    cmd.add(originYArg);
    TCLAP::ValueArg<double> originZArg(
        "", "oz", "mesh origin (lower left corner) in z direction", false, 0,
        "real");
    cmd.add(originZArg);

    // parse arguments
    cmd.parse(argc, argv);
#ifdef USE_PETSC
    MPI_Init(&argc, &argv);
#endif
    const std::string eleTypeName(eleTypeArg.getValue());
    const MeshLib::MeshElemType eleType =
        MeshLib::String2MeshElemType(eleTypeName);
    const unsigned dim = getDimension(eleType);

    bool dim_used[3] = {false};
    for (unsigned i = 0; i < dim; i++)
    {
        dim_used[i] = true;
    }

    std::vector<TCLAP::ValueArg<double>*> vec_lengthArg = {
        &lengthXArg, &lengthYArg, &lengthZArg};
    std::vector<TCLAP::ValueArg<unsigned>*> vec_ndivArg = {
        &nsubdivXArg, &nsubdivYArg, &nsubdivZArg};
    std::vector<TCLAP::ValueArg<double>*> vec_d0Arg = {&d0XArg, &d0YArg,
                                                       &d0ZArg};
    std::vector<TCLAP::ValueArg<double>*> vec_dMaxArg = {&dmaxXArg, &dmaxYArg,
                                                         &dmaxZArg};
    std::vector<TCLAP::ValueArg<double>*> vec_multiArg = {
        &multiXArg, &multiYArg, &multiZArg};
    MathLib::Point3d const origin(
        {originXArg.getValue(), originYArg.getValue(), originZArg.getValue()});

    const bool isLengthSet =
        std::any_of(vec_lengthArg.begin(), vec_lengthArg.end(),
                    [&](TCLAP::ValueArg<double>* arg) { return arg->isSet(); });
    if (!isLengthSet)
    {
        ERR("Missing input: Length information is not provided at all.");
#ifdef USE_PETSC
        MPI_Finalize();
#endif
        return EXIT_FAILURE;
    }
    for (unsigned i = 0; i < 3; i++)
    {
        if (dim_used[i] && !vec_lengthArg[i]->isSet())
        {
            ERR("Missing input: Length for dimension [{:d}] is required but "
                "missing.",
                i);
#ifdef USE_PETSC
            MPI_Finalize();
#endif
            return EXIT_FAILURE;
        }
    }

    std::vector<double> length(dim);
    std::vector<unsigned> n_subdivision(dim);
    std::vector<double> vec_dx(dim);
    for (unsigned i = 0; i < dim; i++)
    {
        length[i] = vec_lengthArg[i]->getValue();
        n_subdivision[i] = vec_ndivArg[i]->getValue();
        vec_dx[i] = length[i] / n_subdivision[i];
    }

    std::vector<std::unique_ptr<BaseLib::ISubdivision>> vec_div;
    vec_div.reserve(dim);
    for (unsigned i = 0; i < dim; i++)
    {
        if (vec_multiArg[i]->isSet())
        {
            if (vec_ndivArg[i]->isSet())
            {
                // number of partitions in direction is specified
                if (vec_d0Arg[i]->isSet())
                {
                    OGS_FATAL(
                        "Specifying all of --m?, --d?0 and --n? for coordinate "
                        "'?' is not supported.");
                }
                vec_div.emplace_back(new BaseLib::GradualSubdivisionFixedNum(
                    length[i], vec_ndivArg[i]->getValue(),
                    vec_multiArg[i]->getValue()));
            }
            else
            {
                vec_div.emplace_back(new BaseLib::GradualSubdivision(
                    length[i], vec_d0Arg[i]->getValue(),
                    vec_dMaxArg[i]->getValue(), vec_multiArg[i]->getValue()));
            }
        }
        else
        {
            vec_div.emplace_back(
                new BaseLib::UniformSubdivision(length[i], n_subdivision[i]));
        }
    }

    // generate a mesh
    std::unique_ptr<MeshLib::Mesh> mesh;
    switch (eleType)
    {
        case MeshLib::MeshElemType::LINE:
            mesh.reset(
                MeshLib::MeshGenerator::generateLineMesh(*vec_div[0], origin));
            break;
        case MeshLib::MeshElemType::TRIANGLE:
            mesh.reset(MeshLib::MeshGenerator::generateRegularTriMesh(
                *vec_div[0], *vec_div[1], origin));
            break;
        case MeshLib::MeshElemType::QUAD:
            mesh.reset(MeshLib::MeshGenerator::generateRegularQuadMesh(
                *vec_div[0], *vec_div[1], origin));
            break;
        case MeshLib::MeshElemType::HEXAHEDRON:
            mesh.reset(MeshLib::MeshGenerator::generateRegularHexMesh(
                *vec_div[0], *vec_div[1], *vec_div[2], origin));
            break;
        case MeshLib::MeshElemType::PRISM:
            mesh.reset(MeshLib::MeshGenerator::generateRegularPrismMesh(
                length[0], length[1], length[2], n_subdivision[0],
                n_subdivision[1], n_subdivision[2], origin));
            break;
        case MeshLib::MeshElemType::TETRAHEDRON:
            mesh.reset(MeshLib::MeshGenerator::generateRegularTetMesh(
                *vec_div[0], *vec_div[1], *vec_div[2], origin));
            break;
        case MeshLib::MeshElemType::PYRAMID:
            mesh.reset(MeshLib::MeshGenerator::generateRegularPyramidMesh(
                *vec_div[0], *vec_div[1], *vec_div[2], origin));
            break;
        default:
            ERR("Given element type is not supported.");
            break;
    }

    if (mesh)
    {
        INFO("Mesh created: {:d} nodes, {:d} elements.",
             mesh->getNumberOfNodes(), mesh->getNumberOfElements());

        // write into a file
        MeshLib::IO::writeMeshToFile(
            *(mesh.get()), std::filesystem::path(mesh_out.getValue()));
    }

#ifdef USE_PETSC
    MPI_Finalize();
#endif
    return EXIT_SUCCESS;
}
