#pragma once

#ifdef USE_CARVE_CSG
#pragma region Carve includes and types

#include <carve\carve.hpp>
#include <carve\csg.hpp>

typedef carve::mesh::MeshSet<3> meshset_t;

typedef carve::mesh::Mesh<3> mesh_t;
typedef mesh_t::vertex_t vertex_t;
typedef mesh_t::edge_t edge_t;
typedef mesh_t::face_t face_t;
typedef face_t::aabb_t aabb_t;
typedef face_t::plane_t plane_t;
typedef face_t::vector_t vector_t;
typedef meshset_t::face_iter face_iter;

#pragma endregion  
#endif // USE_CARVE_CSG
