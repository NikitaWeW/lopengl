#shader compute
#version 430 core
layout(local_size_x = 10, local_size_y = 10, local_size_z = 1) in;

layout(rgba16f) uniform image2D u_output;

void main() {
    // ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    // vec2 texCoords = vec2(texelCoord) / gl_NumWorkGroups.xy;

    // imageStore(u_output, texelCoord, vec4(texCoords.xy, 0, 1.0));


	vec4 value = vec4(0.0, 0.0, 0.0, 1.0);
	ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
	float speed = 100;
	// the width of the texture
	float width = 1000;

	value.x = mod(float(texelCoord.x) + t * speed, width) / (gl_NumWorkGroups.x * gl_WorkGroupSize.x);
	value.y = float(texelCoord.y)/(gl_NumWorkGroups.y*gl_WorkGroupSize.y);
	imageStore(imgOutput, texelCoord, value);
}