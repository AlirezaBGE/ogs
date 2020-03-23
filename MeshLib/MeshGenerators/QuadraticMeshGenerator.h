/**
 * @copyright
 * Copyright (c) 2012-2020, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/LICENSE.txt
 */

#include <memory>

namespace MeshLib
{
class Mesh;
}

namespace MeshLib
{
/// create a quadratic order mesh from the linear order mesh
std::unique_ptr<Mesh> createQuadraticOrderMesh(Mesh const& linear_mesh);

} // namespace MeshLib
