#include "glb.h"

uint8_t get_accessor_type_number_of_componenets(agltf_json_accessor_type_t type)
{
    switch (type)
    {
        case AGLTF_JSON_ACCESSOR_TYPE_SCALAR: return 1;
        case AGLTF_JSON_ACCESSOR_TYPE_VEC2: return 2;
        case AGLTF_JSON_ACCESSOR_TYPE_VEC3: return 3;
        case AGLTF_JSON_ACCESSOR_TYPE_VEC4: 
        case AGLTF_JSON_ACCESSOR_TYPE_MAT2: return 4;
        case AGLTF_JSON_ACCESSOR_TYPE_MAT3: return 9;
        case AGLTF_JSON_ACCESSOR_TYPE_MAT4: return 16;
        default: return 0;
    }
}

uint8_t get_component_type_element_size(agltf_json_component_type_t type)
{
    switch (type)
    {
        case AGLTF_JSON_COMPONENT_TYPE_SIGNED_BYTE: return sizeof(char);
        case AGLTF_JSON_COMPONENT_TYPE_UNSIGNED_BYTE: return sizeof(unsigned char);
        case AGLTF_JSON_COMPONENT_TYPE_SIGNED_SHORT: return sizeof(int16_t);
        case AGLTF_JSON_COMPONENT_TYPE_UNSIGNED_SHORT: return sizeof(uint16_t);
        case AGLTF_JSON_COMPONENT_TYPE_UNSIGNED_INT: return sizeof(uint32_t);
        case AGLTF_JSON_COMPONENT_TYPE_FLOAT: return sizeof(float);
        default: return 0;
    }
}

agltf_json_accessor_type_t get_accessor_type_from_string(char* accessor_type_string)
{
    if (strcmp(accessor_type_string, "SCALAR") == 0) return AGLTF_JSON_ACCESSOR_TYPE_SCALAR;
    if (strcmp(accessor_type_string, "VEC2") == 0) return AGLTF_JSON_ACCESSOR_TYPE_VEC2;
    if (strcmp(accessor_type_string, "VEC3") == 0) return AGLTF_JSON_ACCESSOR_TYPE_VEC3;
    if (strcmp(accessor_type_string, "VEC4") == 0) return AGLTF_JSON_ACCESSOR_TYPE_VEC4;
    if (strcmp(accessor_type_string, "MAT2") == 0) return AGLTF_JSON_ACCESSOR_TYPE_MAT2;
    if (strcmp(accessor_type_string, "MAT3") == 0) return AGLTF_JSON_ACCESSOR_TYPE_MAT3;
    if (strcmp(accessor_type_string, "MAT4") == 0) return AGLTF_JSON_ACCESSOR_TYPE_MAT4;
    return AGLTF_JSON_ACCESSOR_TYPE_UNKNOWN;
} 

agltf_json_component_type_t get_component_type_from_value(uint32_t component_type_value)
{
    if (component_type_value < 5120 || component_type_value > 5126) return AGLTF_JSON_COMPONENT_TYPE_UNKNOWN;
    return (agltf_json_component_type_t) component_type_value;
}

agltf_result_t read_file_header(FILE* file, agltf_stat_t* stat)
{
    fread(&stat->magic, sizeof(uint32_t), 1, file);
    if (stat->magic != AGLTF_MAGIC)
    {
        return AGLTF_NOT_GLTF_FILE_ERROR;
    }
    fread(&stat->version, sizeof(uint32_t), 1, file);
    if (stat->version != 2)
    {
        return AGLTF_UNSUPPORTED_GLTF_VERSION_ERROR;
    }
    fread(&stat->length, sizeof(uint32_t), 1, file);
    if (stat->length == 0)
    {
        return AGLTF_EMPTY_GLTF_FILE_ERROR;
    }
    return AGLTF_SUCCESS;
}

agltf_result_t read_chunk(FILE* file, agltf_chunk_t* chunk)
{
    fread(&chunk->chunk_length, sizeof(uint32_t), 1, file);
    if (chunk->chunk_length == 0)
    {
        return AGLTF_EMPTY_GLTF_CHUNK_ERROR;
    }
    fread(&chunk->chunk_type, sizeof(uint32_t), 1, file);
    if (chunk->chunk_type != AGLTF_CHUNK_TYPE_JSON && chunk->chunk_type != AGLTF_CHUNK_TYPE_BIN)
    {
        return AGLTF_INVALID_GLTF_CHUNK_TYPE_ERROR;
    }
    chunk->chunk_data = malloc(chunk->chunk_length);
    size_t read_bytes = fread(chunk->chunk_data, sizeof(char), chunk->chunk_length, file);
    if (read_bytes == 0)
    {
        return AGLTF_EMPTY_GLTF_CHUNK_ERROR;
    }
    return AGLTF_SUCCESS;
}

void free_chunk_data(agltf_chunk_t* chunk)
{
    free(chunk->chunk_data);
}

agltf_result_t set_buffer_view_from_json(cJSON* object, agltf_glb_t *gltf)
{
    cJSON* json_buffer_views = cJSON_GetObjectItem(object, "bufferViews");

    if (json_buffer_views == NULL)
    {
        return AGLTF_INVALID_JSON_STRUCTURE_ERROR;
    }

    cJSON* json_buffer_view;
    gltf->buffer_views_count = cJSON_GetArraySize(json_buffer_views);
    gltf->buffer_views = malloc(gltf->buffer_views_count * sizeof(agltf_json_buffer_view_t));
    size_t buffer_view_index = 0;
    cJSON_ArrayForEach(json_buffer_view, json_buffer_views)
    {
        agltf_json_buffer_view_t* buffer_view = &gltf->buffer_views[buffer_view_index];
        buffer_view->index = buffer_view_index;
        buffer_view->buffer = (uint32_t) cJSON_GetNumberValue(cJSON_GetObjectItem(json_buffer_view, "buffer"));
        buffer_view->byte_length = (uint32_t) cJSON_GetNumberValue(cJSON_GetObjectItem(json_buffer_view, "byteLength"));
        buffer_view->byte_offset = (uint32_t) cJSON_GetNumberValue(cJSON_GetObjectItem(json_buffer_view, "byteOffset"));
        buffer_view->target = (uint32_t) cJSON_GetNumberValue(cJSON_GetObjectItem(json_buffer_view, "target"));
        buffer_view_index++;
    }
    return AGLTF_SUCCESS;
}

void free_buffer_views(agltf_glb_t *gltf)
{
    free(gltf->buffer_views);
}

agltf_result_t set_accessors_from_json(cJSON* object, agltf_glb_t *gltf)
{
    cJSON* json_accessors = cJSON_GetObjectItem(object, "accessors");

    if (json_accessors == NULL)
    {
        return AGLTF_INVALID_JSON_STRUCTURE_ERROR;
    }

    cJSON* json_accessor;
    gltf->accessors_count = cJSON_GetArraySize(json_accessors);
    gltf->accessors = malloc(gltf->accessors_count * sizeof(agltf_json_accessor_t));
    size_t accessor_index = 0;
    cJSON_ArrayForEach(json_accessor, json_accessors)
    {
        agltf_json_accessor_t* accessor = &gltf->accessors[accessor_index];
        accessor->index = accessor_index;
        accessor->buffer_view = &gltf->buffer_views[(size_t) cJSON_GetNumberValue(cJSON_GetObjectItem(json_accessor, "bufferView"))];
        accessor->component_type = get_component_type_from_value((uint32_t) cJSON_GetNumberValue(cJSON_GetObjectItem(json_accessor, "componentType")));
        accessor->count = (uint32_t) cJSON_GetNumberValue(cJSON_GetObjectItem(json_accessor, "count"));
        accessor->type = get_accessor_type_from_string(cJSON_GetStringValue(cJSON_GetObjectItem(json_accessor, "type")));
        accessor_index++;
    }
    return AGLTF_SUCCESS;
}

void free_accessors(agltf_glb_t *gltf)
{
    free(gltf->accessors);
}

agltf_result_t set_primitive_attributes_from_json(agltf_glb_t *gltf, cJSON* object, agltf_json_mesh_primitive_t *primitive)
{
    cJSON* json_attributes = cJSON_GetObjectItem(object, "attributes");

    if (json_attributes == NULL)
    {
        return AGLTF_INVALID_JSON_STRUCTURE_ERROR;
    }

    size_t json_attributes_count = calculate_cjson_children_count(json_attributes);
    primitive->attribute_count = json_attributes_count;
    primitive->attributes = malloc(json_attributes_count * sizeof(agltf_json_mesh_primitive_attribute_t));
    
    cJSON* json_attributes_child = json_attributes->child;
    size_t json_attributes_index = 0;
    while (json_attributes_child != NULL)
    {
        agltf_json_mesh_primitive_attribute_t* attribute = &primitive->attributes[json_attributes_index];
        attribute->name = create_string_copy(json_attributes_child->string);
        attribute->accessor = &gltf->accessors[(size_t) json_attributes_child->valuedouble];
        json_attributes_child = json_attributes_child->next;
        json_attributes_index++;
    }
    return AGLTF_SUCCESS;
}

void free_primitive_attributes(agltf_json_mesh_primitive_t *primitive)
{
    for (size_t i = 0; i < primitive->attribute_count; ++i)
    {
        agltf_json_mesh_primitive_attribute_t* attribute = &primitive->attributes[i];
        free(attribute->name);
    }
    free(primitive->attributes);
}

agltf_result_t set_primitives_from_json(agltf_glb_t *gltf, cJSON* object, agltf_json_mesh_t *mesh)
{
    agltf_result_t result = AGLTF_SUCCESS;
    cJSON* json_primitives = cJSON_GetObjectItem(object, "primitives");

    if (json_primitives == NULL)
    {
        return AGLTF_INVALID_JSON_STRUCTURE_ERROR;
    }

    cJSON* json_primitive;
    mesh->primitives_count = cJSON_GetArraySize(json_primitives);
    mesh->primitives = malloc(mesh->primitives_count * sizeof(agltf_json_mesh_primitive_t));
    size_t primitive_index = 0;
    cJSON_ArrayForEach(json_primitive, json_primitives)
    {
        agltf_json_mesh_primitive_t* primitive = &mesh->primitives[primitive_index];
        primitive->index = primitive_index;

        result = set_primitive_attributes_from_json(gltf, json_primitive, primitive);
        if (result != AGLTF_SUCCESS)
        {
            free(mesh->primitives);
            return AGLTF_INVALID_JSON_STRUCTURE_ERROR;
        }
    
        if (cJSON_HasObjectItem(json_primitive, "indices"))
        {
            primitive->indices = &gltf->accessors[(size_t) cJSON_GetNumberValue(cJSON_GetObjectItem(json_primitive, "indices"))];
        }
        primitive_index++;
    }
    return result;
}

void free_primitives(agltf_json_mesh_t *mesh)
{
    for (size_t i = 0; i < mesh->primitives_count; ++i)
    {
        agltf_json_mesh_primitive_t* primitive = &mesh->primitives[i];
        free_primitive_attributes(primitive);
    }
    free(mesh->primitives);
}

agltf_result_t set_meshses_from_json(cJSON* object, agltf_glb_t *gltf)
{
    agltf_result_t result = AGLTF_SUCCESS;
    cJSON* json_meshes = cJSON_GetObjectItem(object, "meshes");

    if (json_meshes == NULL)
    {
        return AGLTF_INVALID_JSON_STRUCTURE_ERROR;
    }

    cJSON* json_mesh;
    gltf->meshes_count = cJSON_GetArraySize(json_meshes);
    gltf->meshes = malloc(gltf->meshes_count * sizeof(agltf_json_mesh_t));
    size_t mesh_index = 0;
    cJSON_ArrayForEach(json_mesh, json_meshes)
    {
        agltf_json_mesh_t* mesh = &gltf->meshes[mesh_index];
        mesh->name = create_from_json_string(json_mesh, "name"); // mallocs

        result = set_primitives_from_json(gltf, json_mesh, mesh);
        if (result != AGLTF_SUCCESS)
        {
            free(mesh->name);
            free(gltf->meshes);
            return AGLTF_INVALID_JSON_STRUCTURE_ERROR;
        }

        mesh_index++;
    }
    return result;
}

void free_meshes(agltf_glb_t *gltf)
{
    for (size_t i = 0; i < gltf->meshes_count; ++i)
    {
        agltf_json_mesh_t* mesh = &gltf->meshes[i];
        free(mesh->name);
        free_primitives(mesh);
    }
    free(gltf->meshes);
}

agltf_result_t parse_gltf_json(char* json_string, size_t json_length, agltf_glb_t *gltf)
{
    agltf_result_t result = AGLTF_SUCCESS;
    cJSON *json = cJSON_ParseWithLength(json_string, json_length);
    if (json == NULL)
    {
        return AGLTF_INVALID_JSON_STRING_ERROR;
    }
    result = set_buffer_view_from_json(json, gltf);
    if (result != AGLTF_SUCCESS) goto free_json;

    result = set_accessors_from_json(json, gltf);
    if (result != AGLTF_SUCCESS) goto free_buffer_views;
    
    result = set_meshses_from_json(json, gltf);
    if (result != AGLTF_SUCCESS) goto free_accessors;

    cJSON_Delete(json);

goto finish;

free_accessors:
    free_accessors(gltf);
free_buffer_views:
    free_buffer_views(gltf);
free_json:
    cJSON_Delete(json);
finish:
    return result;    
}

void free_gltf_json(agltf_glb_t *gltf)
{
    free_meshes(gltf);
    free_accessors(gltf);
    free_buffer_views(gltf);
}

agltf_result_t set_accessors_data(agltf_glb_t *gltf, agltf_chunk_t *chunk)
{
    for (size_t i = 0; i < gltf->accessors_count; ++i)
    {
        agltf_json_accessor_t* accessor = &gltf->accessors[i];
        accessor->data.number_of_components = get_accessor_type_number_of_componenets(accessor->type);
        accessor->data.size = accessor->count * get_component_type_element_size(accessor->component_type);
        accessor->data.data = malloc(accessor->data.size);
        memcpy(accessor->data.data, &chunk->chunk_data[accessor->buffer_view->byte_offset], accessor->data.size);
    }
    return AGLTF_SUCCESS;
}

void free_accessors_data(agltf_glb_t *gltf)
{
    for (size_t i = 0; i < gltf->accessors_count; ++i)
    {
        agltf_json_accessor_t* accessor = &gltf->accessors[i];
        free(accessor->data.data);
    }
}

agltf_result_t agltf_create_glb(const char* path, agltf_glb_t* gltf)
{
    agltf_result_t result = AGLTF_SUCCESS;
    FILE* glb_file = fopen(path, "r");
    if (glb_file == NULL)
    {
        return AGLTF_FILE_OPEN_ERROR;
    }

    agltf_stat_t stat;
    result = read_file_header(glb_file, &stat);
    if (result != AGLTF_SUCCESS) goto close_file;

    agltf_chunk_t json_chunk;
    result = read_chunk(glb_file, &json_chunk);
    if (result != AGLTF_SUCCESS) goto close_file;

    result = parse_gltf_json(json_chunk.chunk_data, json_chunk.chunk_length, gltf);
    if (result != AGLTF_SUCCESS) goto free_json_chunk;
    
    agltf_chunk_t binary_chunk;
    result = read_chunk(glb_file, &binary_chunk);
    if (result != AGLTF_SUCCESS) goto free_gltf_json;

    result = set_accessors_data(gltf, &binary_chunk);
    if (result != AGLTF_SUCCESS) goto free_binary_chunk;

    free_chunk_data(&binary_chunk);
    free_chunk_data(&json_chunk);
    fclose(glb_file);

goto finish;

free_binary_chunk:
    free_chunk_data(&binary_chunk);
free_gltf_json:
    free_gltf_json(gltf);
free_json_chunk:
    free_chunk_data(&json_chunk);
close_file:
    fclose(glb_file);

finish:
    return result;
}

void agltf_free_glb(agltf_glb_t* gltf)
{
    free_accessors_data(gltf);
    free_gltf_json(gltf);
}