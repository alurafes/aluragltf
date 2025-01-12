#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <cjson/cJSON.h>

#define AGLTF_CHUNK_TYPE_JSON 0x4E4F534A
#define AGLTF_CHUNK_TYPE_BIN 0x004E4942

typedef struct agltf_json_buffer_view_t {
    size_t index;
    uint32_t buffer;
    uint32_t byte_length;
    uint32_t byte_offset;
    uint32_t target;
} agltf_json_buffer_view_t;

typedef enum agltf_json_accessor_type_t {
    agltf_json_accessor_tYPE_SCALAR,
    agltf_json_accessor_tYPE_VEC2,
    agltf_json_accessor_tYPE_VEC3,
    agltf_json_accessor_tYPE_VEC4,
    agltf_json_accessor_tYPE_MAT2,
    agltf_json_accessor_tYPE_MAT3,
    agltf_json_accessor_tYPE_MAT4,
    agltf_json_accessor_tYPE_UNKNOWN
} agltf_json_accessor_type_t;

typedef struct agltf_json_accessor_t {
    size_t index;
    agltf_json_buffer_view_t* buffer_view;
    uint32_t component_type;
    uint32_t count;
    agltf_json_accessor_type_t type;
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
    agltf_json_mesh_primitive_t* primitives;
} agltf_json_mesh_t;

static agltf_json_accessor_type_t get_accessor_type_from_string(char* accessor_type_string)
{
    if (strcmp(accessor_type_string, "SCALAR") == 0) return agltf_json_accessor_tYPE_SCALAR;
    if (strcmp(accessor_type_string, "VEC2") == 0) return agltf_json_accessor_tYPE_VEC2;
    if (strcmp(accessor_type_string, "VEC3") == 0) return agltf_json_accessor_tYPE_VEC3;
    if (strcmp(accessor_type_string, "VEC4") == 0) return agltf_json_accessor_tYPE_VEC4;
    if (strcmp(accessor_type_string, "MAT2") == 0) return agltf_json_accessor_tYPE_MAT2;
    if (strcmp(accessor_type_string, "MAT3") == 0) return agltf_json_accessor_tYPE_MAT3;
    if (strcmp(accessor_type_string, "MAT4") == 0) return agltf_json_accessor_tYPE_MAT4;
    return agltf_json_accessor_tYPE_UNKNOWN;
} 

// allocates memory
char* create_from_json_string(cJSON* object, char* key)
{
    char* string = cJSON_GetStringValue(cJSON_GetObjectItem(object, key));
    char* destination = malloc(strlen(string) + 1);
    strcpy(destination, string);
    return destination;
}

int main()
{
    FILE* glbFile = fopen("cube.glb", "r");

    if (glbFile == NULL)
    {
        return 1;
    }

    uint32_t magic;
    size_t read_bytes = fread(&magic, sizeof(magic), 1, glbFile);
    if (magic != 0x46546C67)
    {
        return 1;
    }

    uint32_t version;
    read_bytes = fread(&version, sizeof(version), 1, glbFile);
    if (version != 2)
    {
        return 1;
    }

    uint32_t length;
    read_bytes = fread(&length, sizeof(length), 1, glbFile);
    if (version == 0)
    {
        return 1;
    }

    uint32_t chunk_length;
    read_bytes = fread(&chunk_length, sizeof(chunk_length), 1, glbFile);
    if (chunk_length == 0)
    {
        return 1;
    }
    uint32_t chunk_type;
    read_bytes = fread(&chunk_type, sizeof(chunk_type), 1, glbFile);
    if (chunk_type != AGLTF_CHUNK_TYPE_JSON && chunk_type != AGLTF_CHUNK_TYPE_BIN)
    {
        return 1;
    }
    char* chunk_data = malloc(chunk_length);
    chunk_data[chunk_length] = 0;
    read_bytes = fread(chunk_data, sizeof(char), chunk_length, glbFile);
    if (read_bytes == 0)
    {
        return 1;
    }

    cJSON *json = cJSON_ParseWithLength(chunk_data, chunk_length);
    if (json == NULL)
    {
        return 1;
    }

    cJSON* json_buffer_views = cJSON_GetObjectItem(json, "bufferViews");
    cJSON* json_buffer_view;
    int buffer_views_size = cJSON_GetArraySize(json_buffer_views);
    agltf_json_buffer_view_t* buffer_views = malloc(buffer_views_size * sizeof(agltf_json_buffer_view_t));
    size_t buffer_view_index = 0;
    cJSON_ArrayForEach(json_buffer_view, json_buffer_views)
    {
        agltf_json_buffer_view_t* buffer_view = &buffer_views[buffer_view_index];
        buffer_view->index = buffer_view_index;
        buffer_view->buffer = (uint32_t) cJSON_GetNumberValue(cJSON_GetObjectItem(json_buffer_view, "buffer"));
        buffer_view->byte_length = (uint32_t) cJSON_GetNumberValue(cJSON_GetObjectItem(json_buffer_view, "byteLength"));
        buffer_view->byte_offset = (uint32_t) cJSON_GetNumberValue(cJSON_GetObjectItem(json_buffer_view, "byteOffset"));
        buffer_view->target = (uint32_t) cJSON_GetNumberValue(cJSON_GetObjectItem(json_buffer_view, "target"));
        buffer_view_index++;
    }

    cJSON* json_accessors = cJSON_GetObjectItem(json, "accessors");
    cJSON* json_accessor;
    int json_accessors_size = cJSON_GetArraySize(json_accessors);
    agltf_json_accessor_t* accessors = malloc(json_accessors_size * sizeof(agltf_json_accessor_t));
    size_t accessor_index = 0;
    cJSON_ArrayForEach(json_accessor, json_accessors)
    {
        agltf_json_accessor_t* accessor = &accessors[accessor_index];
        accessor->index = accessor_index;
        accessor->buffer_view = &buffer_views[(size_t) cJSON_GetNumberValue(cJSON_GetObjectItem(json_accessor, "bufferView"))];
        accessor->component_type = (uint32_t) cJSON_GetNumberValue(cJSON_GetObjectItem(json_accessor, "componentType")); // todo
        accessor->count = (uint32_t) cJSON_GetNumberValue(cJSON_GetObjectItem(json_accessor, "count"));
        accessor->type = get_accessor_type_from_string(cJSON_GetStringValue(cJSON_GetObjectItem(json_accessor, "type")));
        accessor_index++;
    }

    cJSON* json_mesh;
    cJSON* json_meshes = cJSON_GetObjectItem(json, "meshes");
    int json_meshes_size = cJSON_GetArraySize(json_accessors);
    agltf_json_mesh_t* meshes = malloc(json_meshes_size * sizeof(agltf_json_mesh_t));
    size_t mesh_index = 0;
    cJSON_ArrayForEach(json_mesh, json_meshes)
    {
        agltf_json_mesh_t* mesh = &meshes[mesh_index];
        mesh->name = create_from_json_string(json_mesh, "name");

        cJSON* json_primitive;
        cJSON* json_primitives = cJSON_GetObjectItem(json_mesh, "primitives");
        int json_primitives_size = cJSON_GetArraySize(json_primitives);
        mesh->primitives = malloc(json_primitives_size * sizeof(agltf_json_mesh_primitive_t));
        size_t primitive_index = 0;
        cJSON_ArrayForEach(json_primitive, json_primitives)
        {
            agltf_json_mesh_primitive_t* primitive = &mesh->primitives[primitive_index];
            primitive->index = primitive_index;

            cJSON* json_attributes = cJSON_GetObjectItem(json_primitive, "attributes");
            size_t json_attributes_count = 0;

            cJSON* json_attributes_child = json_attributes->child;
            while (json_attributes_child != NULL)
            {
                json_attributes_count++;
                json_attributes_child = json_attributes_child->next;
            }

            primitive->attribute_count = json_attributes_count;
            primitive->attributes = malloc(json_attributes_count * sizeof(agltf_json_mesh_primitive_attribute_t));
            json_attributes_child = json_attributes->child;
            size_t json_attributes_index = 0;
            while (json_attributes_child != NULL)
            {
                agltf_json_mesh_primitive_attribute_t* attribute = &primitive->attributes[json_attributes_index];
                attribute->name = malloc(strlen(json_attributes_child->string) + 1);
                strcpy(attribute->name, json_attributes_child->string);
                attribute->accessor = &accessors[(size_t) json_attributes_child->valuedouble];
                json_attributes_child = json_attributes_child->next;
                json_attributes_index++;
            }

            if (cJSON_HasObjectItem(json_primitive, "indices"))
            {
                primitive->indices = &accessors[(size_t) cJSON_GetNumberValue(cJSON_GetObjectItem(json_primitive, "indices"))];
            }
            // todo: material
            primitive_index++;
        }
        mesh_index++;
    }

    fclose(glbFile);
    cJSON_Delete(json);
    return 0;
}