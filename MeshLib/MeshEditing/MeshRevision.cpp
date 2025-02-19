/**
 * \file
 * \author Karsten Rink
 * \date   2014-02-14
 * \brief  Implementation of the MeshRevision class.
 *
 * \copyright
 * Copyright (c) 2012-2023, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/project/license
 *
 */

#include "MeshRevision.h"

#include <numeric>

#include "BaseLib/Algorithm.h"
#include "BaseLib/Logging.h"
#include "DuplicateMeshComponents.h"
#include "GeoLib/Grid.h"
#include "MathLib/GeometricBasics.h"
#include "MeshLib/Elements/Elements.h"
#include "MeshLib/Mesh.h"

namespace MeshLib
{
const std::array<unsigned, 8> MeshRevision::_hex_diametral_nodes = {
    {6, 7, 4, 5, 2, 3, 0, 1}};

MeshRevision::MeshRevision(MeshLib::Mesh& mesh) : _mesh(mesh) {}

unsigned MeshRevision::getNumberOfCollapsibleNodes(double eps) const
{
    std::vector<std::size_t> id_map(this->collapseNodeIndices(eps));
    std::size_t nNodes(id_map.size());
    unsigned count(0);
    for (std::size_t i = 0; i < nNodes; ++i)
    {
        if (i != id_map[i])
        {
            count++;
        }
    }
    return count;
}

MeshLib::Mesh* MeshRevision::simplifyMesh(const std::string& new_mesh_name,
                                          double eps,
                                          unsigned min_elem_dim) const
{
    if (this->_mesh.getNumberOfElements() == 0)
    {
        return nullptr;
    }

    std::vector<MeshLib::Element*> const& elements(this->_mesh.getElements());
    auto const node_ids = collapseNodeIndices(eps);
    std::vector<MeshLib::Node*> new_nodes =
        this->constructNewNodesArray(node_ids);
    std::vector<MeshLib::Element*> new_elements;
    std::vector<std::size_t> element_ids;

    for (std::size_t k(0); k < elements.size(); ++k)
    {
        MeshLib::Element const* const elem(elements[k]);
        unsigned n_unique_nodes(this->getNumberOfUniqueNodes(elem));
        if (n_unique_nodes == elem->getNumberOfBaseNodes() &&
            elem->getDimension() >= min_elem_dim)
        {
            ElementErrorCode e(elem->validate());
            if (e[ElementErrorFlag::NonCoplanar])
            {
                std::size_t const n_new_elements(
                    subdivideElement(elem, new_nodes, new_elements));
                if (n_new_elements == 0)
                {
                    ERR("Element {:d} has unknown element type.", k);
                    _mesh.resetNodeIDs();
                    BaseLib::cleanupVectorElements(new_nodes, new_elements);
                    return nullptr;
                }
                element_ids.insert(element_ids.end(), n_new_elements, k);
            }
            else
            {
                new_elements.push_back(MeshLib::copyElement(elem, new_nodes));
                element_ids.push_back(k);
            }
        }
        else if (n_unique_nodes < elem->getNumberOfBaseNodes() &&
                 n_unique_nodes > 1)
        {
            std::size_t const n_new_elements(reduceElement(
                elem, n_unique_nodes, new_nodes, new_elements, min_elem_dim));
            element_ids.insert(element_ids.end(), n_new_elements, k);
        }
        else
        {
            ERR("Something is wrong, more unique nodes than actual nodes");
        }
    }

    auto const& props = _mesh.getProperties();
    MeshLib::Properties const new_properties =
        copyProperties(props, node_ids, element_ids);

    _mesh.resetNodeIDs();
    if (!new_elements.empty())
    {
        return new MeshLib::Mesh(new_mesh_name, new_nodes, new_elements,
                                 new_properties);
    }

    BaseLib::cleanupVectorElements(new_nodes, new_elements);
    return nullptr;
}

std::vector<std::size_t> MeshRevision::collapseNodeIndices(double eps) const
{
    const std::vector<MeshLib::Node*>& nodes(_mesh.getNodes());
    const std::size_t nNodes(_mesh.getNumberOfNodes());
    std::vector<std::size_t> id_map(nNodes);
    const double half_eps(eps / 2.0);
    const double sqr_eps(eps * eps);
    std::iota(id_map.begin(), id_map.end(), 0);

    GeoLib::Grid<MeshLib::Node> const grid(nodes.begin(), nodes.end(), 64);

    for (std::size_t k = 0; k < nNodes; ++k)
    {
        MeshLib::Node const* const node(nodes[k]);
        if (node->getID() != k)
        {
            continue;
        }
        std::vector<std::vector<MeshLib::Node*> const*> node_vectors(
            grid.getPntVecsOfGridCellsIntersectingCube(*node, half_eps));

        const std::size_t nVectors(node_vectors.size());
        for (std::size_t i = 0; i < nVectors; ++i)
        {
            const std::vector<MeshLib::Node*>& cell_vector(*node_vectors[i]);
            const std::size_t nGridCellNodes(cell_vector.size());
            for (std::size_t j = 0; j < nGridCellNodes; ++j)
            {
                MeshLib::Node const* const test_node(cell_vector[j]);
                // are node indices already identical (i.e. nodes will be
                // collapsed)
                if (id_map[node->getID()] == id_map[test_node->getID()])
                {
                    continue;
                }

                // if test_node has already been collapsed to another node x,
                // ignore it (if the current node would need to be collapsed
                // with x it would already have happened when x was tested)
                if (test_node->getID() != id_map[test_node->getID()])
                {
                    continue;
                }

                // calc distance
                if (MathLib::sqrDist(*node, *test_node) < sqr_eps)
                {
                    id_map[test_node->getID()] = node->getID();
                }
            }
        }
    }
    return id_map;
}

std::vector<MeshLib::Node*> MeshRevision::constructNewNodesArray(
    const std::vector<std::size_t>& id_map) const
{
    const std::vector<MeshLib::Node*>& nodes(_mesh.getNodes());
    const std::size_t nNodes(nodes.size());
    std::vector<MeshLib::Node*> new_nodes;
    new_nodes.reserve(nNodes);
    for (std::size_t k = 0; k < nNodes; ++k)
    {
        // all nodes that have not been collapsed with other nodes are copied
        // into new array
        if (nodes[k]->getID() == id_map[k])
        {
            std::size_t const id(new_nodes.size());
            new_nodes.push_back(new MeshLib::Node(
                (*nodes[k])[0], (*nodes[k])[1], (*nodes[k])[2], id));
            nodes[k]->setID(id);  // the node in the old array gets the index of
                                  // the same node in the new array
        }
        // the other nodes are not copied and get the index of the nodes they
        // will have been collapsed with
        else
        {
            nodes[k]->setID(nodes[id_map[k]]->getID());
        }
    }
    return new_nodes;
}

unsigned MeshRevision::getNumberOfUniqueNodes(
    MeshLib::Element const* const element)
{
    unsigned const nNodes(element->getNumberOfBaseNodes());
    unsigned count(nNodes);

    for (unsigned i = 0; i < nNodes - 1; ++i)
    {
        for (unsigned j = i + 1; j < nNodes; ++j)
        {
            if (element->getNode(i)->getID() == element->getNode(j)->getID())
            {
                count--;
                break;
            }
        }
    }
    return count;
}

template <typename T>
void fillNodeProperty(std::vector<T>& new_prop,
                      std::vector<T> const& old_prop,
                      std::vector<size_t>
                          node_ids)
{
    std::size_t const n_nodes = node_ids.size();
    for (std::size_t i = 0; i < n_nodes; ++i)
    {
        if (node_ids[i] != i)
        {
            continue;
        }
        new_prop.push_back(old_prop[i]);
    }
}

template <typename T>
void fillElemProperty(std::vector<T>& new_prop,
                      std::vector<T> const& old_prop,
                      std::vector<size_t>
                          elem_ids)
{
    std::transform(elem_ids.cbegin(), elem_ids.cend(),
                   std::back_inserter(new_prop),
                   [&](std::size_t const i) { return old_prop[i]; });
}

MeshLib::Properties MeshRevision::copyProperties(
    MeshLib::Properties const& props,
    std::vector<std::size_t> const& node_ids,
    std::vector<std::size_t> const& elem_ids) const
{
    auto const prop_names = props.getPropertyVectorNames();
    MeshLib::Properties new_properties;

    for (auto name : prop_names)
    {
        if (props.existsPropertyVector<int>(name, MeshItemType::Node, 1))
        {
            auto p = props.getPropertyVector<int>(name, MeshItemType::Node, 1);
            auto new_node_vec = new_properties.createNewPropertyVector<int>(
                name, MeshItemType::Node, 1);
            fillNodeProperty(*new_node_vec, *p, node_ids);
            continue;
        }
        if (props.existsPropertyVector<float>(name, MeshItemType::Node, 1))
        {
            auto p =
                props.getPropertyVector<float>(name, MeshItemType::Node, 1);
            auto new_node_vec = new_properties.createNewPropertyVector<float>(
                name, MeshItemType::Node, 1);
            fillNodeProperty(*new_node_vec, *p, node_ids);
            continue;
        }
        if (props.existsPropertyVector<double>(name, MeshItemType::Node, 1))
        {
            auto p =
                props.getPropertyVector<double>(name, MeshItemType::Node, 1);
            auto new_node_vec = new_properties.createNewPropertyVector<double>(
                name, MeshItemType::Node, 1);
            fillNodeProperty(*new_node_vec, *p, node_ids);
            continue;
        }
        if (props.existsPropertyVector<int>(name, MeshItemType::Cell, 1))
        {
            auto p = props.getPropertyVector<int>(name, MeshItemType::Cell, 1);
            auto new_cell_vec = new_properties.createNewPropertyVector<int>(
                name, MeshItemType::Cell, 1);
            fillElemProperty(*new_cell_vec, *p, elem_ids);
            continue;
        }
        if (props.existsPropertyVector<float>(name, MeshItemType::Cell, 1))
        {
            auto p =
                props.getPropertyVector<float>(name, MeshItemType::Cell, 1);
            auto new_cell_vec = new_properties.createNewPropertyVector<float>(
                name, MeshItemType::Cell, 1);
            fillElemProperty(*new_cell_vec, *p, elem_ids);
            continue;
        }
        if (props.existsPropertyVector<double>(name, MeshItemType::Cell, 1))
        {
            auto p =
                props.getPropertyVector<double>(name, MeshItemType::Cell, 1);
            auto new_cell_vec = new_properties.createNewPropertyVector<double>(
                name, MeshItemType::Cell, 1);
            fillElemProperty(*new_cell_vec, *p, elem_ids);
            continue;
        }
        WARN("PropertyVector {:s} not being converted.", name);
    }
    return new_properties;
}

std::size_t MeshRevision::subdivideElement(
    MeshLib::Element const* const element,
    std::vector<MeshLib::Node*> const& nodes,
    std::vector<MeshLib::Element*>& elements) const
{
    if (element->getGeomType() == MeshElemType::QUAD)
    {
        return this->subdivideQuad(element, nodes, elements);
    }
    if (element->getGeomType() == MeshElemType::HEXAHEDRON)
    {
        return this->subdivideHex(element, nodes, elements);
    }
    if (element->getGeomType() == MeshElemType::PYRAMID)
    {
        return this->subdividePyramid(element, nodes, elements);
    }
    if (element->getGeomType() == MeshElemType::PRISM)
    {
        return this->subdividePrism(element, nodes, elements);
    }
    return 0;
}

std::size_t MeshRevision::reduceElement(
    MeshLib::Element const* const element,
    unsigned n_unique_nodes,
    std::vector<MeshLib::Node*> const& nodes,
    std::vector<MeshLib::Element*>& elements,
    unsigned min_elem_dim) const
{
    /***************
     * TODO: modify neighbouring elements if one elements has been subdivided
     ***************/
    if (element->getGeomType() == MeshElemType::TRIANGLE && min_elem_dim == 1)
    {
        elements.push_back(this->constructLine(element, nodes));
        return 1;
    }
    if ((element->getGeomType() == MeshElemType::QUAD) ||
        (element->getGeomType() == MeshElemType::TETRAHEDRON))
    {
        if (n_unique_nodes == 3 && min_elem_dim < 3)
        {
            elements.push_back(this->constructTri(element, nodes));
        }
        else if (min_elem_dim == 1)
        {
            elements.push_back(this->constructLine(element, nodes));
        }
        return 1;
    }
    if (element->getGeomType() == MeshElemType::HEXAHEDRON)
    {
        return reduceHex(element, n_unique_nodes, nodes, elements,
                         min_elem_dim);
    }
    if (element->getGeomType() == MeshElemType::PYRAMID)
    {
        this->reducePyramid(element, n_unique_nodes, nodes, elements,
                            min_elem_dim);
        return 1;
    }
    if (element->getGeomType() == MeshElemType::PRISM)
    {
        return reducePrism(element, n_unique_nodes, nodes, elements,
                           min_elem_dim);
    }

    ERR("Unknown element type.");
    return 0;
}

unsigned MeshRevision::subdivideQuad(
    MeshLib::Element const* const quad,
    std::vector<MeshLib::Node*> const& nodes,
    std::vector<MeshLib::Element*>& new_elements) const
{
    std::array tri1_nodes{nodes[quad->getNode(0)->getID()],
                          nodes[quad->getNode(1)->getID()],
                          nodes[quad->getNode(2)->getID()]};
    new_elements.push_back(new MeshLib::Tri(tri1_nodes));

    std::array tri2_nodes{nodes[quad->getNode(0)->getID()],
                          nodes[quad->getNode(2)->getID()],
                          nodes[quad->getNode(3)->getID()]};
    new_elements.push_back(new MeshLib::Tri(tri2_nodes));

    return 2;
}

unsigned MeshRevision::subdivideHex(
    MeshLib::Element const* const hex,
    std::vector<MeshLib::Node*> const& nodes,
    std::vector<MeshLib::Element*>& new_elements) const
{
    std::array<Node*, 6> prism1_nodes;
    prism1_nodes[0] = nodes[hex->getNode(0)->getID()];
    prism1_nodes[1] = nodes[hex->getNode(2)->getID()];
    prism1_nodes[2] = nodes[hex->getNode(1)->getID()];
    prism1_nodes[3] = nodes[hex->getNode(4)->getID()];
    prism1_nodes[4] = nodes[hex->getNode(6)->getID()];
    prism1_nodes[5] = nodes[hex->getNode(5)->getID()];
    auto* prism1(new MeshLib::Prism(prism1_nodes));
    this->subdividePrism(prism1, nodes, new_elements);
    delete prism1;

    std::array<Node*, 6> prism2_nodes;
    prism2_nodes[0] = nodes[hex->getNode(4)->getID()];
    prism2_nodes[1] = nodes[hex->getNode(6)->getID()];
    prism2_nodes[2] = nodes[hex->getNode(7)->getID()];
    prism2_nodes[3] = nodes[hex->getNode(0)->getID()];
    prism2_nodes[4] = nodes[hex->getNode(2)->getID()];
    prism2_nodes[5] = nodes[hex->getNode(3)->getID()];
    auto* prism2(new MeshLib::Prism(prism2_nodes));
    this->subdividePrism(prism2, nodes, new_elements);
    delete prism2;

    return 6;
}

unsigned MeshRevision::subdividePyramid(
    MeshLib::Element const* const pyramid,
    std::vector<MeshLib::Node*> const& nodes,
    std::vector<MeshLib::Element*>& new_elements) const
{
    auto addTetrahedron =
        [&pyramid, &nodes, &new_elements](std::size_t id0, std::size_t id1,
                                          std::size_t id2, std::size_t id3)
    {
        std::array<Node*, 4> tet_nodes;
        tet_nodes[0] = nodes[pyramid->getNode(id0)->getID()];
        tet_nodes[1] = nodes[pyramid->getNode(id1)->getID()];
        tet_nodes[2] = nodes[pyramid->getNode(id2)->getID()];
        tet_nodes[3] = nodes[pyramid->getNode(id3)->getID()];
        new_elements.push_back(new MeshLib::Tet(tet_nodes));
    };

    addTetrahedron(0, 1, 2, 4);

    addTetrahedron(0, 2, 3, 4);

    return 2;
}

unsigned MeshRevision::subdividePrism(
    MeshLib::Element const* const prism,
    std::vector<MeshLib::Node*> const& nodes,
    std::vector<MeshLib::Element*>& new_elements) const
{
    auto addTetrahedron =
        [&prism, &nodes, &new_elements](std::size_t id0, std::size_t id1,
                                        std::size_t id2, std::size_t id3)
    {
        std::array<Node*, 4> tet_nodes;
        tet_nodes[0] = nodes[prism->getNode(id0)->getID()];
        tet_nodes[1] = nodes[prism->getNode(id1)->getID()];
        tet_nodes[2] = nodes[prism->getNode(id2)->getID()];
        tet_nodes[3] = nodes[prism->getNode(id3)->getID()];
        new_elements.push_back(new MeshLib::Tet(tet_nodes));
    };

    addTetrahedron(0, 1, 2, 3);

    addTetrahedron(3, 2, 4, 5);

    addTetrahedron(2, 1, 3, 4);

    return 3;
}

unsigned MeshRevision::reduceHex(MeshLib::Element const* const org_elem,
                                 unsigned n_unique_nodes,
                                 std::vector<MeshLib::Node*> const& nodes,
                                 std::vector<MeshLib::Element*>& new_elements,
                                 unsigned min_elem_dim) const
{
    // TODO?
    // if two diametral nodes collapse, all kinds of bizarre (2D-)element
    // combinations could be the result. this case is currently not implemented!

    if (n_unique_nodes == 7)
    {
        // reduce to prism + pyramid
        for (unsigned i = 0; i < 7; ++i)
        {
            for (unsigned j = i + 1; j < 8; ++j)
            {
                if (org_elem->getNode(i)->getID() ==
                    org_elem->getNode(j)->getID())
                {
                    const std::array<unsigned, 4> base_nodes(
                        this->lutHexCuttingQuadNodes(i, j));
                    std::array pyr_nodes{
                        nodes[org_elem->getNode(base_nodes[0])->getID()],
                        nodes[org_elem->getNode(base_nodes[1])->getID()],
                        nodes[org_elem->getNode(base_nodes[2])->getID()],
                        nodes[org_elem->getNode(base_nodes[3])->getID()],
                        nodes[org_elem->getNode(i)->getID()]};
                    new_elements.push_back(new MeshLib::Pyramid(pyr_nodes));

                    if (i < 4 && j >= 4)
                    {
                        std::swap(i, j);
                    }
                    std::array prism_nodes{
                        nodes[org_elem->getNode(base_nodes[0])->getID()],
                        nodes[org_elem->getNode(base_nodes[3])->getID()],
                        nodes[org_elem->getNode(this->lutHexDiametralNode(j))
                                  ->getID()],
                        nodes[org_elem->getNode(base_nodes[1])->getID()],
                        nodes[org_elem->getNode(base_nodes[2])->getID()],
                        nodes[org_elem->getNode(this->lutHexDiametralNode(i))
                                  ->getID()]};
                    new_elements.push_back(new MeshLib::Prism(prism_nodes));
                    return 2;
                }
            }
        }
    }
    else if (n_unique_nodes == 6)
    {
        // reduce to prism
        for (unsigned i = 0; i < 6; ++i)
        {
            const MeshLib::Element* face(org_elem->getFace(i));
            if (face->getNode(0)->getID() == face->getNode(1)->getID() &&
                face->getNode(2)->getID() == face->getNode(3)->getID())
            {
                std::array prism_nodes{
                    nodes[org_elem
                              ->getNode(
                                  this->lutHexDiametralNode(getNodeIDinElement(
                                      *org_elem, face->getNode(0))))
                              ->getID()],
                    nodes[org_elem
                              ->getNode(
                                  this->lutHexDiametralNode(getNodeIDinElement(
                                      *org_elem, face->getNode(1))))
                              ->getID()],
                    nodes[org_elem
                              ->getNode(getNodeIDinElement(*org_elem,
                                                           face->getNode(2)))
                              ->getID()],
                    nodes[org_elem
                              ->getNode(
                                  this->lutHexDiametralNode(getNodeIDinElement(
                                      *org_elem, face->getNode(2))))
                              ->getID()],
                    nodes[org_elem
                              ->getNode(
                                  this->lutHexDiametralNode(getNodeIDinElement(
                                      *org_elem, face->getNode(3))))
                              ->getID()],
                    nodes[org_elem
                              ->getNode(getNodeIDinElement(*org_elem,
                                                           face->getNode(0)))
                              ->getID()]};
                new_elements.push_back(new MeshLib::Prism(prism_nodes));
                delete face;
                return 1;
            }
            if (face->getNode(0)->getID() == face->getNode(3)->getID() &&
                face->getNode(1)->getID() == face->getNode(2)->getID())
            {
                std::array prism_nodes{
                    nodes[org_elem
                              ->getNode(
                                  this->lutHexDiametralNode(getNodeIDinElement(
                                      *org_elem, face->getNode(0))))
                              ->getID()],
                    nodes[org_elem
                              ->getNode(
                                  this->lutHexDiametralNode(getNodeIDinElement(
                                      *org_elem, face->getNode(3))))
                              ->getID()],
                    nodes[org_elem
                              ->getNode(getNodeIDinElement(*org_elem,
                                                           face->getNode(2)))
                              ->getID()],
                    nodes[org_elem
                              ->getNode(
                                  this->lutHexDiametralNode(getNodeIDinElement(
                                      *org_elem, face->getNode(1))))
                              ->getID()],
                    nodes[org_elem
                              ->getNode(
                                  this->lutHexDiametralNode(getNodeIDinElement(
                                      *org_elem, face->getNode(2))))
                              ->getID()],
                    nodes[org_elem
                              ->getNode(getNodeIDinElement(*org_elem,
                                                           face->getNode(0)))
                              ->getID()]};
                new_elements.push_back(new MeshLib::Prism(prism_nodes));
                delete face;
                return 1;
            }
            delete face;
        }
        // reduce to four tets -> divide into 2 prisms such that each has one
        // collapsed node
        for (unsigned i = 0; i < 7; ++i)
        {
            for (unsigned j = i + 1; j < 8; ++j)
            {
                if (org_elem->getNode(i)->getID() ==
                    org_elem->getNode(j)->getID())
                {
                    for (unsigned k = i; k < 7; ++k)
                    {
                        for (unsigned l = k + 1; l < 8; ++l)
                        {
                            if (!(i == k && j == l) && org_elem->isEdge(i, j) &&
                                org_elem->isEdge(k, l) &&
                                org_elem->getNode(k)->getID() ==
                                    org_elem->getNode(l)->getID())
                            {
                                const std::pair<unsigned, unsigned> back(
                                    this->lutHexBackNodes(i, j, k, l));
                                if (back.first ==
                                        std::numeric_limits<unsigned>::max() ||
                                    back.second ==
                                        std::numeric_limits<unsigned>::max())
                                {
                                    ERR("Unexpected error during Hex "
                                        "reduction");
                                    return 0;
                                }

                                std::array<unsigned, 4> cutting_plane(
                                    this->lutHexCuttingQuadNodes(back.first,
                                                                 back.second));
                                std::array pris1_nodes{
                                    const_cast<MeshLib::Node*>(
                                        org_elem->getNode(back.first)),
                                    const_cast<MeshLib::Node*>(
                                        org_elem->getNode(cutting_plane[0])),
                                    const_cast<MeshLib::Node*>(
                                        org_elem->getNode(cutting_plane[3])),
                                    const_cast<MeshLib::Node*>(
                                        org_elem->getNode(back.second)),
                                    const_cast<MeshLib::Node*>(
                                        org_elem->getNode(cutting_plane[1])),
                                    const_cast<MeshLib::Node*>(
                                        org_elem->getNode(cutting_plane[2]))};
                                auto* prism1(new MeshLib::Prism(pris1_nodes));
                                unsigned nNewElements = this->reducePrism(
                                    prism1, 5, nodes, new_elements,
                                    min_elem_dim);
                                delete prism1;

                                std::array pris2_nodes{
                                    const_cast<MeshLib::Node*>(
                                        org_elem->getNode(
                                            this->lutHexDiametralNode(
                                                back.first))),
                                    const_cast<MeshLib::Node*>(
                                        org_elem->getNode(cutting_plane[0])),
                                    const_cast<MeshLib::Node*>(
                                        org_elem->getNode(cutting_plane[3])),
                                    const_cast<MeshLib::Node*>(
                                        org_elem->getNode(
                                            this->lutHexDiametralNode(
                                                back.second))),
                                    const_cast<MeshLib::Node*>(
                                        org_elem->getNode(cutting_plane[1])),
                                    const_cast<MeshLib::Node*>(
                                        org_elem->getNode(cutting_plane[2]))};
                                auto* prism2(new MeshLib::Prism(pris2_nodes));
                                nNewElements += this->reducePrism(
                                    prism2, 5, nodes, new_elements,
                                    min_elem_dim);
                                delete prism2;
                                return nNewElements;
                            }
                        }
                    }
                }
            }
        }
    }
    else if (n_unique_nodes == 5)
    {
        MeshLib::Element* tet1(this->constructFourNodeElement(org_elem, nodes));
        std::array<std::size_t, 4> first_four_nodes = {
            {tet1->getNode(0)->getID(), tet1->getNode(1)->getID(),
             tet1->getNode(2)->getID(), tet1->getNode(3)->getID()}};
        unsigned fifth_node(
            this->findPyramidTopNode(*org_elem, first_four_nodes));

        bool tet_changed(false);
        if (tet1->getGeomType() == MeshElemType::QUAD)
        {
            delete tet1;
            tet_changed = true;
            auto** tet1_nodes = new MeshLib::Node*[4];
            tet1_nodes[0] = nodes[first_four_nodes[0]];
            tet1_nodes[1] = nodes[first_four_nodes[1]];
            tet1_nodes[2] = nodes[first_four_nodes[2]];
            tet1_nodes[3] = nodes[org_elem->getNode(fifth_node)->getID()];
            new_elements.push_back(new MeshLib::Tet(tet1_nodes));
        }
        else
        {
            new_elements.push_back(tet1);
        }

        std::array tet2_nodes = {(tet_changed) ? nodes[first_four_nodes[0]]
                                               : nodes[first_four_nodes[1]],
                                 nodes[first_four_nodes[2]],
                                 nodes[first_four_nodes[3]],
                                 nodes[org_elem->getNode(fifth_node)->getID()]};
        new_elements.push_back(new MeshLib::Tet(tet2_nodes));
        return 2;
    }
    else if (n_unique_nodes == 4)
    {
        MeshLib::Element* elem(
            this->constructFourNodeElement(org_elem, nodes, min_elem_dim));
        if (elem)
        {
            new_elements.push_back(elem);
            return 1;
        }
    }
    else if (n_unique_nodes == 3 && min_elem_dim < 3)
    {
        new_elements.push_back(this->constructTri(org_elem, nodes));
        return 1;
    }
    else if (min_elem_dim == 1)
    {
        new_elements.push_back(this->constructLine(org_elem, nodes));
        return 1;
    }
    return 0;
}

void MeshRevision::reducePyramid(MeshLib::Element const* const org_elem,
                                 unsigned n_unique_nodes,
                                 std::vector<MeshLib::Node*> const& nodes,
                                 std::vector<MeshLib::Element*>& new_elements,
                                 unsigned min_elem_dim) const
{
    if (n_unique_nodes == 4)
    {
        MeshLib::Element* elem(
            this->constructFourNodeElement(org_elem, nodes, min_elem_dim));
        if (elem)
        {
            new_elements.push_back(elem);
        }
    }
    else if (n_unique_nodes == 3 && min_elem_dim < 3)
    {
        new_elements.push_back(this->constructTri(org_elem, nodes));
    }
    else if (n_unique_nodes == 2 && min_elem_dim == 1)
    {
        new_elements.push_back(this->constructLine(org_elem, nodes));
    }
}

unsigned MeshRevision::reducePrism(MeshLib::Element const* const org_elem,
                                   unsigned n_unique_nodes,
                                   std::vector<MeshLib::Node*> const& nodes,
                                   std::vector<MeshLib::Element*>& new_elements,
                                   unsigned min_elem_dim) const
{
    auto addTetrahedron =
        [&org_elem, &nodes, &new_elements](std::size_t id0, std::size_t id1,
                                           std::size_t id2, std::size_t id3)
    {
        std::array tet_nodes{nodes[org_elem->getNode(id0)->getID()],
                             nodes[org_elem->getNode(id1)->getID()],
                             nodes[org_elem->getNode(id2)->getID()],
                             nodes[org_elem->getNode(id3)->getID()]};
        new_elements.push_back(new MeshLib::Tet(tet_nodes));
    };

    // TODO?
    // In theory a node from the bottom triangle and a node from the top
    // triangle that are not connected by an edge could collapse, resulting in a
    // combination of tri and quad elements. This case is currently not tested.

    // if one of the non-triangle edges collapsed, elem can be reduced to a
    // pyramid, otherwise it will be two tets
    if (n_unique_nodes == 5)
    {
        for (unsigned i = 0; i < 5; ++i)
        {
            for (unsigned j = i + 1; j < 6; ++j)
            {
                if (i != j && org_elem->getNode(i)->getID() ==
                                  org_elem->getNode(j)->getID())
                {
                    // non triangle edge collapsed
                    if (i % 3 == j % 3)
                    {
                        addTetrahedron((i + 1) % 3, (i + 2) % 3, i,
                                       (i + 1) % 3 + 3);
                        addTetrahedron((i + 1) % 3 + 3, (i + 2) % 3, i,
                                       (i + 2) % 3 + 3);
                        return 2;
                    }

                    // triangle edge collapsed
                    const unsigned i_offset = (i > 2) ? i - 3 : i + 3;
                    const unsigned j_offset = (i > 2) ? j - 3 : j + 3;
                    const unsigned k = lutPrismThirdNode(i, j);
                    if (k == std::numeric_limits<unsigned>::max())
                    {
                        ERR("Unexpected error during prism reduction.");
                        return 0;
                    }
                    const unsigned k_offset = (i > 2) ? k - 3 : k + 3;

                    addTetrahedron(i_offset, j_offset, k_offset, i);

                    const unsigned l =
                        (MathLib::isCoplanar(*org_elem->getNode(i_offset),
                                             *org_elem->getNode(k_offset),
                                             *org_elem->getNode(i),
                                             *org_elem->getNode(k)))
                            ? j
                            : i;
                    const unsigned l_offset = (i > 2) ? l - 3 : l + 3;
                    addTetrahedron(l_offset, k_offset, i, k);
                    return 2;
                }
            }
        }
    }
    else if (n_unique_nodes == 4)
    {
        MeshLib::Element* elem(
            this->constructFourNodeElement(org_elem, nodes, min_elem_dim));
        if (elem)
        {
            new_elements.push_back(elem);
        }
    }
    else if (n_unique_nodes == 3 && min_elem_dim < 3)
    {
        new_elements.push_back(this->constructTri(org_elem, nodes));
    }
    else if (n_unique_nodes == 2 && min_elem_dim == 1)
    {
        new_elements.push_back(this->constructLine(org_elem, nodes));
    }
    return 1;
}

MeshLib::Element* MeshRevision::constructLine(
    MeshLib::Element const* const element,
    const std::vector<MeshLib::Node*>& nodes) const
{
    std::array<Node*, 2> line_nodes;
    line_nodes[0] = nodes[element->getNode(0)->getID()];
    line_nodes[1] = nullptr;
    for (unsigned i = 1; i < element->getNumberOfBaseNodes(); ++i)
    {
        if (element->getNode(i)->getID() != element->getNode(0)->getID())
        {
            line_nodes[1] = nodes[element->getNode(i)->getID()];
            break;
        }
    }
    assert(line_nodes[1] != nullptr);
    return new MeshLib::Line(line_nodes);
}

MeshLib::Element* MeshRevision::constructTri(
    MeshLib::Element const* const element,
    const std::vector<MeshLib::Node*>& nodes) const
{
    // TODO?
    // In theory three unique nodes could also be reduced to two lines e.g. with
    // a quad where two diametral nodes collapse. This case is currently not
    // implemented!
    std::array<Node*, 3> tri_nodes;
    tri_nodes[0] = nodes[element->getNode(0)->getID()];
    tri_nodes[2] = nullptr;
    for (unsigned i = 1; i < element->getNumberOfBaseNodes(); ++i)
    {
        if (element->getNode(i)->getID() != tri_nodes[0]->getID())
        {
            tri_nodes[1] = nodes[element->getNode(i)->getID()];
            for (unsigned j = i + 1; j < element->getNumberOfBaseNodes(); ++j)
            {
                if (element->getNode(j)->getID() != tri_nodes[1]->getID())
                {
                    tri_nodes[2] = nodes[element->getNode(j)->getID()];
                    break;
                }
            }
            if (tri_nodes[2])
            {
                break;
            }
        }
    }
    assert(tri_nodes[2] != nullptr);
    return new MeshLib::Tri(tri_nodes);
}

MeshLib::Element* MeshRevision::constructFourNodeElement(
    MeshLib::Element const* const element,
    std::vector<MeshLib::Node*> const& nodes,
    unsigned min_elem_dim) const
{
    std::array<Node*, 4> new_nodes;
    unsigned count(0);
    new_nodes[count++] = nodes[element->getNode(0)->getID()];
    for (unsigned i = 1; i < element->getNumberOfBaseNodes(); ++i)
    {
        if (count > 3)
        {
            break;
        }
        bool unique_node(true);
        for (unsigned j = 0; j < i; ++j)
        {
            if (element->getNode(i)->getID() == element->getNode(j)->getID())
            {
                unique_node = false;
                break;
            }
        }
        if (unique_node)
        {
            new_nodes[count++] = nodes[element->getNode(i)->getID()];
        };
    }

    // test if quad or tet
    const bool isQuad(MathLib::isCoplanar(*new_nodes[0], *new_nodes[1],
                                          *new_nodes[2], *new_nodes[3]));
    if (isQuad && min_elem_dim < 3)
    {
        MeshLib::Element* elem(new MeshLib::Quad(new_nodes));
        for (unsigned i = 1; i < 3; ++i)
        {
            if (elem->validate().none())
            {
                return elem;
            }

            // change node order if not convex
            MeshLib::Node* tmp = new_nodes[i + 1];
            new_nodes[i + 1] = new_nodes[i];
            new_nodes[i] = tmp;
        }
        return elem;
    }
    if (!isQuad)
    {
        return new MeshLib::Tet(new_nodes);
    }
    // is quad but min elem dim == 3

    return nullptr;
}

unsigned MeshRevision::findPyramidTopNode(
    MeshLib::Element const& element,
    std::array<std::size_t, 4> const& base_node_ids)
{
    const std::size_t nNodes(element.getNumberOfBaseNodes());
    for (std::size_t i = 0; i < nNodes; ++i)
    {
        bool top_node = true;
        for (unsigned j = 0; j < 4; ++j)
        {
            if (element.getNode(i)->getID() == base_node_ids[j])
            {
                top_node = false;
            }
        }
        if (top_node)
        {
            return i;
        }
    }
    return std::numeric_limits<unsigned>::max();  // should never be reached if
                                                  // called correctly
}

unsigned MeshRevision::lutHexDiametralNode(unsigned id)
{
    return _hex_diametral_nodes[id];
}

std::array<unsigned, 4> MeshRevision::lutHexCuttingQuadNodes(unsigned id1,
                                                             unsigned id2)
{
    std::array<unsigned, 4> nodes{};
    if (id1 == 0 && id2 == 1)
    {
        nodes[0] = 3;
        nodes[1] = 2;
        nodes[2] = 5;
        nodes[3] = 4;
    }
    else if (id1 == 1 && id2 == 2)
    {
        nodes[0] = 0;
        nodes[1] = 3;
        nodes[2] = 6;
        nodes[3] = 5;
    }
    else if (id1 == 2 && id2 == 3)
    {
        nodes[0] = 1;
        nodes[1] = 0;
        nodes[2] = 7;
        nodes[3] = 6;
    }
    else if (id1 == 3 && id2 == 0)
    {
        nodes[0] = 2;
        nodes[1] = 1;
        nodes[2] = 4;
        nodes[3] = 7;
    }
    else if (id1 == 4 && id2 == 5)
    {
        nodes[0] = 0;
        nodes[1] = 1;
        nodes[2] = 6;
        nodes[3] = 7;
    }
    else if (id1 == 5 && id2 == 6)
    {
        nodes[0] = 1;
        nodes[1] = 2;
        nodes[2] = 7;
        nodes[3] = 4;
    }
    else if (id1 == 6 && id2 == 7)
    {
        nodes[0] = 2;
        nodes[1] = 3;
        nodes[2] = 4;
        nodes[3] = 5;
    }
    else if (id1 == 7 && id2 == 4)
    {
        nodes[0] = 3;
        nodes[1] = 0;
        nodes[2] = 5;
        nodes[3] = 6;
    }
    else if (id1 == 0 && id2 == 4)
    {
        nodes[0] = 3;
        nodes[1] = 7;
        nodes[2] = 5;
        nodes[3] = 1;
    }
    else if (id1 == 1 && id2 == 5)
    {
        nodes[0] = 0;
        nodes[1] = 4;
        nodes[2] = 6;
        nodes[3] = 2;
    }
    else if (id1 == 2 && id2 == 6)
    {
        nodes[0] = 1;
        nodes[1] = 5;
        nodes[2] = 7;
        nodes[3] = 3;
    }
    else if (id1 == 3 && id2 == 7)
    {
        nodes[0] = 2;
        nodes[1] = 6;
        nodes[2] = 4;
        nodes[3] = 0;
    }

    else if (id1 == 1 && id2 == 0)
    {
        nodes[0] = 2;
        nodes[1] = 3;
        nodes[2] = 4;
        nodes[3] = 5;
    }
    else if (id1 == 2 && id2 == 1)
    {
        nodes[0] = 3;
        nodes[1] = 0;
        nodes[2] = 5;
        nodes[3] = 6;
    }
    else if (id1 == 3 && id2 == 2)
    {
        nodes[0] = 0;
        nodes[1] = 1;
        nodes[2] = 6;
        nodes[3] = 7;
    }
    else if (id1 == 0 && id2 == 3)
    {
        nodes[0] = 1;
        nodes[1] = 2;
        nodes[2] = 7;
        nodes[3] = 4;
    }
    else if (id1 == 5 && id2 == 4)
    {
        nodes[0] = 1;
        nodes[1] = 0;
        nodes[2] = 7;
        nodes[3] = 6;
    }
    else if (id1 == 6 && id2 == 5)
    {
        nodes[0] = 2;
        nodes[1] = 1;
        nodes[2] = 4;
        nodes[3] = 7;
    }
    else if (id1 == 7 && id2 == 6)
    {
        nodes[0] = 3;
        nodes[1] = 2;
        nodes[2] = 5;
        nodes[3] = 4;
    }
    else if (id1 == 4 && id2 == 7)
    {
        nodes[0] = 0;
        nodes[1] = 3;
        nodes[2] = 6;
        nodes[3] = 5;
    }
    else if (id1 == 4 && id2 == 0)
    {
        nodes[0] = 7;
        nodes[1] = 3;
        nodes[2] = 1;
        nodes[3] = 5;
    }
    else if (id1 == 5 && id2 == 1)
    {
        nodes[0] = 4;
        nodes[1] = 0;
        nodes[2] = 2;
        nodes[3] = 6;
    }
    else if (id1 == 6 && id2 == 2)
    {
        nodes[0] = 5;
        nodes[1] = 1;
        nodes[2] = 3;
        nodes[3] = 7;
    }
    else if (id1 == 7 && id2 == 3)
    {
        nodes[0] = 6;
        nodes[1] = 2;
        nodes[2] = 0;
        nodes[3] = 4;
    }
    return nodes;
}

std::pair<unsigned, unsigned> MeshRevision::lutHexBackNodes(unsigned i,
                                                            unsigned j,
                                                            unsigned k,
                                                            unsigned l)
{
    // collapsed edges are *not* connected
    std::pair<unsigned, unsigned> back(std::numeric_limits<unsigned>::max(),
                                       std::numeric_limits<unsigned>::max());
    if (lutHexDiametralNode(i) == k)
    {
        back.first = i;
        back.second = lutHexDiametralNode(l);
    }
    else if (lutHexDiametralNode(i) == l)
    {
        back.first = i;
        back.second = lutHexDiametralNode(k);
    }
    else if (lutHexDiametralNode(j) == k)
    {
        back.first = j;
        back.second = lutHexDiametralNode(l);
    }
    else if (lutHexDiametralNode(j) == l)
    {
        back.first = j;
        back.second = lutHexDiametralNode(k);
    }
    // collapsed edges *are* connected
    else if (i == k)
    {
        back.first = lutHexDiametralNode(l);
        back.second = j;
    }
    else if (i == l)
    {
        back.first = lutHexDiametralNode(k);
        back.second = j;
    }
    else if (j == k)
    {
        back.first = lutHexDiametralNode(l);
        back.second = i;
    }
    else if (j == l)
    {
        back.first = lutHexDiametralNode(k);
        back.second = i;
    }
    return back;
}

unsigned MeshRevision::lutPrismThirdNode(unsigned id1, unsigned id2)
{
    if ((id1 == 0 && id2 == 1) || (id1 == 1 && id2 == 2))
    {
        return 2;
    }
    if ((id1 == 1 && id2 == 2) || (id1 == 2 && id2 == 1))
    {
        return 0;
    }
    if ((id1 == 0 && id2 == 2) || (id1 == 2 && id2 == 0))
    {
        return 1;
    }
    if ((id1 == 3 && id2 == 4) || (id1 == 4 && id2 == 3))
    {
        return 5;
    }
    if ((id1 == 4 && id2 == 5) || (id1 == 5 && id2 == 4))
    {
        return 3;
    }
    if ((id1 == 3 && id2 == 5) || (id1 == 5 && id2 == 3))
    {
        return 4;
    }
    return std::numeric_limits<unsigned>::max();
}
}  // end namespace MeshLib
