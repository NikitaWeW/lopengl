#shader compute
#version 430 core
layout(local_size_x = 20, local_size_y = 20, local_size_z = 1) in;

layout(rgba16f) uniform image2D u_output;

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    vec2 texCoords = vec2(texelCoord) / gl_NumWorkGroups.xy;

    imageStore(u_output, texelCoord, vec4(texCoords.xy, 0, 1.0));
}