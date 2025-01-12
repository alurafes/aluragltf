#ifndef ALURA_GLB_PARSER_TYPES_H
#define ALURA_GLB_PARSER_TYPES_H

#include <stdint.h>

typedef enum agltf_result_t {
    AGLTF_SUCCESS,
    AGLTF_FILE_OPEN_ERROR,
    AGLTF_NOT_GLTF_FILE_ERROR,
    AGLTF_UNSUPPORTED_GLTF_VERSION_ERROR,
    AGLTF_EMPTY_GLTF_FILE_ERROR,
    AGLTF_EMPTY_GLTF_CHUNK_ERROR,
    AGLTF_INVALID_GLTF_CHUNK_TYPE_ERROR,
    AGLTF_INVALID_JSON_STRING_ERROR,
    AGLTF_INVALID_JSON_STRUCTURE_ERROR,
} agltf_result_t;

typedef struct agltf_json_buffer_view_t {
    size_t index;
    uint32_t buffer;
    uint32_t byte_length;
    uint32_t byte_offset;
    uint32_t target;
} agltf_json_buffer_view_t;

typedef enum agltf_json_component_type_t {
    AGLTF_JSON_COMPONENT_TYPE_SIGNED_BYTE = 5120,
    AGLTF_JSON_COMPONENT_TYPE_UNSIGNED_BYTE,
    AGLTF_JSON_COMPONENT_TYPE_SIGNED_SHORT,
    AGLTF_JSON_COMPONENT_TYPE_UNSIGNED_SHORT,
    AGLTF_JSON_COMPONENT_TYPE_UNSIGNED_INT = 5125,
    AGLTF_JSON_COMPONENT_TYPE_FLOAT,
    AGLTF_JSON_COMPONENT_TYPE_UNKNOWN,
} agltf_json_component_type_t;

typedef enum agltf_json_accessor_type_t {
    AGLTF_JSON_ACCESSOR_TYPE_SCALAR,
    AGLTF_JSON_ACCESSOR_TYPE_VEC2,
    AGLTF_JSON_ACCESSOR_TYPE_VEC3,
    AGLTF_JSON_ACCESSOR_TYPE_VEC4,
    AGLTF_JSON_ACCESSOR_TYPE_MAT2,
    AGLTF_JSON_ACCESSOR_TYPE_MAT3,
    AGLTF_JSON_ACCESSOR_TYPE_MAT4,
    AGLTF_JSON_ACCESSOR_TYPE_UNKNOWN
} agltf_json_accessor_type_t;

typedef struct agltf_accessor_data_t {
    void* data;
    size_t size;
    uint8_t number_of_components;
} agltf_accessor_data_t;

typedef struct agltf_json_accessor_t {
    size_t index;
    agltf_json_buffer_view_t* buffer_view;
    agltf_json_component_type_t component_type;
    uint32_t count;
    agltf_json_accessor_type_t type;
    agltf_accessor_data_t data;
} agltf_json_accessor_t;

typedef struct agltf_json_mesh_primitive_attribute_t {
    char* name;
    agltf_json_accessor_t* accessor;
} agltf_json_mesh_primitive_attribute_t;

typedef struct agltf_json_mesh_primitive_t {
    size_t index;
    size_t attribute_count;
    agltf_json_mesh_primitive_attribute_t* attributes;
    agltf_json_accessor_t* indices;
    agltf_json_accessor_t* material;
    // there are more primitives, but right now I don't care about any other
} agltf_json_mesh_primitive_t;

typedef struct agltf_json_mesh_t {
    char* name;
    size_t primitives_count; 
    agltf_json_mesh_primitive_t* primitives;
} agltf_json_mesh_t;

typedef struct agltf_stat_t {
    uint32_t magic;
    uint32_t version;
    uint32_t length;
} agltf_stat_t;

typedef struct agltf_chunk_t {
    uint32_t chunk_length;
    uint32_t chunk_type;
    char* chunk_data;
} agltf_chunk_t;

typedef struct agltf_glb_t {
    size_t buffer_views_count;
    agltf_json_buffer_view_t* buffer_views;
    size_t accessors_count;
    agltf_json_accessor_t* accessors;
    size_t meshes_count;
    agltf_json_mesh_t* meshes;
} agltf_glb_t;

#endif