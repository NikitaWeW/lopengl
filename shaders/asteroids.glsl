#shader vertex
#version 330 core

uniform vec3 u_planetPos;
uniform mat4 u_viewMat;
uniform mat4 u_projectionMat;
uniform int u_first = 0; // optional: if the first vertex is not 0 in the draw call

const int verticesInCube = 16;
const float ringOut = 25;
const float ringIn = 15;
const vec3 ringRot = vec3(0, 0, 0);
const float ringHeight = 2;

const vec3 cubePositions[verticesInCube] = vec3[](
    vec3( 0.5, 0.5, 0.5),

    vec3( 0.5, 0.5, 0.5),
    vec3(-0.5, 0.5, 0.5),
    vec3( 0.5,-0.5, 0.5),
    vec3(-0.5,-0.5, 0.5),
    vec3(-0.5,-0.5,-0.5),
    vec3(-0.5, 0.5, 0.5),
    vec3(-0.5, 0.5,-0.5),
    vec3( 0.5, 0.5, 0.5),
    vec3( 0.5, 0.5,-0.5),
    vec3( 0.5,-0.5, 0.5),
    vec3( 0.5,-0.5,-0.5),
    vec3(-0.5,-0.5,-0.5),
    vec3( 0.5, 0.5,-0.5),
    vec3(-0.5, 0.5,-0.5),

    vec3(-0.5, 0.5,-0.5) 
);

// return random uint in [0; 0xffffffffu]
uint rand(inout uint state) {
	state = state * 747796405u + 2891336453u;
	uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	return (word >> 22u) ^ word;
}
// return random float in [0; 1]
float randZeroOne(inout uint state) {
    return float(rand(state)) * (1.0 / float(0xffffffffu));
}
// return random float in [a; b]
float randMapped(inout uint state, float a, float b) {
    return a + (b - a) * randZeroOne(state);
}

mat4 translate(mat4 mat, vec3 vec)
{
    mat[3] = mat[0] * vec[0] + mat[1] * vec[1] + mat[2] * vec[2] + mat[3];
    return mat;
}
mat4 rotate(mat4 mat, float angle, vec3 vec)
{
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
mat4 scale(mat4 mat, vec3 vec)
{
    mat[0] = mat[0] * vec[0];
    mat[1] = mat[1] * vec[1];
    mat[2] = mat[2] * vec[2];
    mat[3] = mat[3];
    return mat;
}

void main()
{
    int vertexID = gl_VertexID - u_first;
    int asteroidID = vertexID / verticesInCube;
    int cubeVertexID = vertexID % verticesInCube;

    // generate model matrix
    mat4 modelMat = mat4(1.0f);
    uint asteroidSeed = uint(asteroidID);
    vec3 point;
    do { // while point not valid
        point = vec3(randMapped(asteroidSeed, -ringOut, ringOut), randMapped(asteroidSeed, -ringHeight, ringHeight), randMapped(asteroidSeed, -ringOut, ringOut));
    } while(abs(point.x) < ringIn && abs(point.z) < ringIn);
    modelMat = translate(modelMat, point);
    modelMat = rotate(modelMat, randMapped(asteroidSeed, 0.0, 360.0), vec3(0.6f, 0.4f, 0.8f));
    modelMat = scale(modelMat, vec3(randMapped(asteroidSeed, 0.01, 0.1)));
    // generate ring model matrix
    mat4 ringModelMat = mat4(1.0);
    ringModelMat = translate(ringModelMat, u_planetPos);
    ringModelMat = rotate(ringModelMat, radians(ringRot.x), vec3(1, 0, 0));
    ringModelMat = rotate(ringModelMat, radians(ringRot.y), vec3(0, 1, 0));
    ringModelMat = rotate(ringModelMat, radians(ringRot.z), vec3(0, 0, 1));
    // ===========

    vec3 position = cubePositions[cubeVertexID];
    gl_Position = u_projectionMat * u_viewMat * ringModelMat * modelMat * vec4(position, 1);
}

#shader fragment
#version 330 core

out vec4 o_color;

void main()
{
    // insert something meaningful
    o_color.rgb = vec3(gl_FragCoord.xyz / 1000);
    o_color.a = 1;
}