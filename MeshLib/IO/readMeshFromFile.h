/**
 * \file
 * \author Karsten Rink
 * \date   2012-09-27
 * \brief  Definition of readMeshFromFile function.
 *
 * \copyright
 * Copyright (c) 2012-2023, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/project/license
 *
 *
 * @file readMeshFromFile.h
 * @date 2012-09-27
 * @author Karsten Rink
 */

#pragma once

#include <string>

namespace MeshLib
{
class Mesh;

namespace IO
{
MeshLib::Mesh* readMeshFromFile(const std::string &file_name);
}
}
