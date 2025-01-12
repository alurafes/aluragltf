#include "glb.h"

int main()
{
    agltf_glb_t gltf;
    agltf_result_t result = agltf_create_glb("test.glb", &gltf);
    if (result != AGLTF_SUCCESS) return result;
    return 0;
}