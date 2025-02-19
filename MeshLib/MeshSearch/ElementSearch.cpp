/**
 * \file
 * \copyright
 * Copyright (c) 2012-2023, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/project/license
 *
 */

#include "ElementSearch.h"

#include "BaseLib/Algorithm.h"
#include "MeshLib/Elements/Element.h"
#include "MeshLib/Node.h"

namespace MeshLib
{
ElementSearch::ElementSearch(const MeshLib::Mesh& mesh) : _mesh(mesh) {}

template <typename Container, typename Predicate>
std::vector<std::size_t> filter(Container const& container, Predicate const& p)
{
    std::vector<std::size_t> matchedIDs;
    std::size_t i = 0;
    for (auto value : container)
    {
        if (p(value))
        {
            matchedIDs.push_back(i);
        }
        i++;
    }
    return matchedIDs;
}

std::size_t ElementSearch::searchByElementType(MeshElemType eleType)
{
    auto matchedIDs = filter(_mesh.getElements(), [&](MeshLib::Element* e)
                             { return e->getGeomType() == eleType; });

    this->updateUnion(matchedIDs);
    return matchedIDs.size();
}

std::size_t ElementSearch::searchByContent(double eps)
{
    auto matchedIDs = filter(_mesh.getElements(), [&eps](MeshLib::Element* e)
                             { return e->getContent() < eps; });

    this->updateUnion(matchedIDs);
    return matchedIDs.size();
}

std::size_t ElementSearch::searchByBoundingBox(GeoLib::AABB const& aabb)
{
    auto matchedIDs =
        filter(_mesh.getElements(),
               [&aabb](MeshLib::Element* e)
               {
                   std::size_t const nElemNodes(e->getNumberOfBaseNodes());
                   for (std::size_t n = 0; n < nElemNodes; ++n)
                   {
                       if (aabb.containsPoint(*e->getNode(n), 0))
                       {
                           return true;  // any node of element is in aabb.
                       }
                   }
                   return false;  // no nodes of element are in aabb.
               });

    this->updateUnion(matchedIDs);
    return matchedIDs.size();
}

std::size_t ElementSearch::searchByNodeIDs(
    const std::vector<std::size_t>& nodes)
{
    std::vector<std::size_t> connected_elements;
    for (std::size_t node_id : nodes)
    {
        auto const& elements = _mesh.getElementsConnectedToNode(node_id);
        std::transform(begin(elements), end(elements),
                       back_inserter(connected_elements),
                       [](Element const* const e) { return e->getID(); });
    }

    BaseLib::makeVectorUnique(connected_elements);

    this->updateUnion(connected_elements);
    return connected_elements.size();
}

void ElementSearch::updateUnion(const std::vector<std::size_t>& vec)
{
    std::vector<std::size_t> vec_temp(vec.size() + _marked_elements.size());
    auto it = std::set_union(vec.begin(), vec.end(), _marked_elements.begin(),
                             _marked_elements.end(), vec_temp.begin());
    vec_temp.resize(it - vec_temp.begin());
    _marked_elements.assign(vec_temp.begin(), vec_temp.end());
}

}  // end namespace MeshLib
