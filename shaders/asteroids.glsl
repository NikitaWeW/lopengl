#shader vertex
#version 330 core

uniform vec3 u_planetPos;
uniform mat4 u_ringRotMat;
uniform mat4 u_viewMat;
uniform mat4 u_projectionMat;
uniform vec3 u_rotVector;
uniform float u_rotAngle;

const int first = 0; // optional: if the first vertex is not 0 in the draw call
const int verticesInCube = 16;
const float ringOut = 25;
const float ringIn = 15;
const float ringHeight = 0.5;
const float decayRate = 1;

const float cubeSize = 1.0f;
const vec3 cubePositions[verticesInCube] = vec3[](
    vec3(cubeSize, cubeSize, cubeSize),

    vec3( cubeSize, cubeSize, cubeSize),
    vec3(-cubeSize, cubeSize, cubeSize),
    vec3( cubeSize,-cubeSize, cubeSize),
    vec3(-cubeSize,-cubeSize, cubeSize),
    vec3(-cubeSize,-cubeSize,-cubeSize),
    vec3(-cubeSize, cubeSize, cubeSize),
    vec3(-cubeSize, cubeSize,-cubeSize),
    vec3( cubeSize, cubeSize, cubeSize),
    vec3( cubeSize, cubeSize,-cubeSize),
    vec3( cubeSize,-cubeSize, cubeSize),
    vec3( cubeSize,-cubeSize,-cubeSize),
    vec3(-cubeSize,-cubeSize,-cubeSize),
    vec3( cubeSize, cubeSize,-cubeSize),
    vec3(-cubeSize, cubeSize,-cubeSize),

    vec3(-cubeSize, cubeSize,-cubeSize) 
);
// Define 5 preset transformations (pre-multiplied rotation + scale)
const mat4 presetMatrices[5] = mat4[](
    mat4(
        0.007583, 0.000199, -0.006516, 0.000000, 
        -0.004916, -0.006390, -0.005916, 0.000000, 
        -0.004282, 0.007690, -0.004747, 0.000000, 
        0.000000, 0.000000, 0.000000, 1.000000
    ),
    mat4(
        -0.011233, 0.001062, -0.016513, 0.000000,
        0.000597, 0.019972, 0.000878, 0.000000,
        0.016537, 0.000000, -0.011249, 0.000000,
        0.000000, 0.000000, 0.000000, 1.000000
    ),
    mat4(
        0.029736, 0.003332, 0.002160, 0.000000,
        0.003971, -0.024951, -0.016177, 0.000000,
        0.000000, 0.016321, -0.025172, 0.000000,
        0.000000, 0.000000, 0.000000, 1.000000
    ),
    mat4(
        0.005424, -0.039628, -0.000443, 0.000000,
        0.020301, 0.003162, -0.034320, 0.000000,
        0.034036, 0.004429, 0.020541, 0.000000,
        0.000000, 0.000000, 0.000000, 1.000000
    ),
    mat4(
        0.003608, 0.049179, -0.008268, 0.000000,
        0.019671, -0.009022, -0.045074, 0.000000,
        -0.045826, 0.000000, -0.019999, 0.000000,
        0.000000, 0.000000, 0.000000, 1.000000
    )
);

// return random uint in [0; 0xffffffffu]
uint rand(inout uint state) {
	state = state * 747796405u + 2891336453u;
	uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	return (word >> 22u) ^ word;
}
// return random float in [a; b]
float rand(inout uint state, float a, float b) {
    return a + (b - a) * float(rand(state)) * (1.0 / float(0xffffffffu));
}

mat4 translate(mat4 mat, vec3 vec)
{
    mat[3] = mat[0] * vec[0] + mat[1] * vec[1] + mat[2] * vec[2] + mat[3];
    return mat;
}
mat4 rotate(mat4 mat, float angle, vec3 vec)
{ // thanks to https://github.com/g-truc/glm
    float a = angle;
    float cosA = cos(a);
    float sinA = sin(a);

    vec3 axis = vec3(normalize(vec));
    vec3 temp = vec3((1.0 - cosA) * axis);

    mat4 rotate;
    rotate[0][0] = cosA + temp[0] * axis[0];
    rotate[0][1] = temp[0] * axis[1] + sinA * axis[2];
    rotate[0][2] = temp[0] * axis[2] - sinA * axis[1];

    rotate[1][0] = temp[1] * axis[0] - sinA * axis[2];
    rotate[1][1] = cosA + temp[1] * axis[1];
    rotate[1][2] = temp[1] * axis[2] + sinA * axis[0];

    rotate[2][0] = temp[2] * axis[0] + sinA * axis[1];
    rotate[2][1] = temp[2] * axis[1] - sinA * axis[0];
    rotate[2][2] = cosA + temp[2] * axis[2];

    mat4 result;
    result[0] = mat[0] * rotate[0][0] + mat[1] * rotate[0][1] + mat[2] * rotate[0][2];
    result[1] = mat[0] * rotate[1][0] + mat[1] * rotate[1][1] + mat[2] * rotate[1][2];
    result[2] = mat[0] * rotate[2][0] + mat[1] * rotate[2][1] + mat[2] * rotate[2][2];
    result[3] = mat[3];
    return result;
}

void main()
{
    int vertexID = gl_VertexID - first;
    int asteroidID = vertexID / verticesInCube;
    int cubeVertexID = vertexID % verticesInCube;
    int presetID = asteroidID % 5;

    // generate model matrix
    mat4 modelMat = mat4(1.0f);
    uint asteroidSeed = uint(asteroidID);
    vec3 point;
    do { // while point not valid
        point = vec3(rand(asteroidSeed, -ringOut, ringOut), rand(asteroidSeed, -ringHeight, ringHeight), rand(asteroidSeed, -ringOut, ringOut));
    } while(abs(point.x) < ringIn && abs(point.z) < ringIn);
    float orbitHeight = length(point);
    // float orbitHeight = max(max(abs(point.x), abs(point.y)), abs(point.z));
    // ====
    modelMat = translate(modelMat, u_planetPos);
    modelMat = rotate(modelMat, u_rotAngle / (1.0 + decayRate * orbitHeight), vec3(0, 1, 0));
    // modelMat = modelMat * u_ringRotMat;
    modelMat = translate(modelMat, point);
    modelMat = modelMat * presetMatrices[presetID];

    vec3 position = cubePositions[cubeVertexID];
    gl_Position = u_projectionMat * u_viewMat * modelMat * vec4(position, 1);
}

#shader fragment
#version 330 core

out vec4 o_color;

void main()
{
    // TODO: insert something meaningful
    o_color.rgb = vec3(gl_FragCoord.xyz / 1000);
    o_color.a = 1;
}