#pragma once
#include <general.hpp>
#include <vertex.hpp>
#include <edge.hpp>
#include <half_edge.hpp>
#include <face.hpp>
#include <mesh.hpp>

namespace HalfMesh
{
    template< typename TopologicalAtrributeType, typename DataStoreValueType  >
    class Property
    {
    public:

            void set_value(TopologicalAtrributeType mesh_entity, DataStoreValueType _value)
            {
                entity_to_value_map[mesh_entity] = _value;
            }

            DataStoreValueType get_value(TopologicalAtrributeType mesh_entity)
            {
                return entity_to_value_map[mesh_entity];
            }

        private:
            std::unordered_map< TopologicalAtrributeType, DataStoreValueType > entity_to_value_map;
    };

    template< typename DataType > using EdgeProperty = Property< Edge*, DataType >;
    template< typename DataType > using FaceProperty = Property< Face*, DataType >;
    template< typename DataType > using HalfEdgeProperty = Property< HalfEdge*, DataType >;
    template< typename DataType > using VertexProperty = Property< Vertex*, DataType >;
}