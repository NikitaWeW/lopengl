#shader compute
#version 430 core
layout(local_size_x = 20, local_size_y = 20, local_size_z = 1) in;
layout(rgba16f) uniform image2D u_output;

layout(std430) readonly buffer indicesSSBO {
    uint indices[];
};
layout(std430) readonly buffer positionsSSBO {
    vec4 positions[];
};
layout(std430) readonly buffer normalsSSBO {
    vec4 normals[];
};

#define debugColor ?vec3(0,1,0):vec3(1,0,0) // output bool values as color
struct Camera {
    float fov;
    float aspect;
    vec3 position;
    vec3 forward;
    vec3 right;
    vec3 up;
};
struct Ray {
    vec3 direction;
    vec3 origin;
};
struct Triangle {
    vec3 A;
    vec3 B;
    vec3 C;
};
struct AABB {
    vec3 min;
    vec3 max;
};
struct Sphere {
    vec3 center;
    float radius;
};

struct Material {
    vec3 color;
    vec3 specularColor;
    vec3 emission;
    float smoothness;
    float specularProbability;
};
struct Model {
    uint indexOffset;
    uint vertexOffset;
    uint indicesCount;
    mat4 modelMat; // local to world space
    mat4 normalMat;
    AABB aabb;
    Material material;
};
struct Hitinfo {
    bool exists;
    vec3 normal;
    vec3 position;
    Material material;
};
uniform Model u_models[10];
uniform uint u_modelCount;

uniform float u_time;
uniform uint u_numAccumFrames;
uniform Camera u_camera;

// intersection tests
float rayTriangle(Ray ray, Triangle triangle);
float rayAABB(Ray ray, AABB aabb);
bool rayAABBb(Ray ray, AABB aabb);
float raySphere(Ray ray, Sphere sphere);
Hitinfo rayScene(Ray ray);

// random functions
uint rand(inout uint state); // return random uint in [0; 0xffffffffu]
float randNormalDistribution(inout uint state);
float randZeroOne(inout uint state); // return random float in [0; 1]
float randNegOneOne(inout uint state); // return random float in [-1; 1]
vec3 randUnitSphere(inout uint state);
vec3 randHemisphere(inout uint state, vec3 normal);

vec3 rayColor(Ray ray);
Ray calculateRay(vec2 texCoords, Camera camera);
float lerp(float a, float b, float x) { return a + x * (b - a); }
vec3 lerp(vec3 a, vec3 b, float x) { return a + x * (b - a); }

uint seed = 0;
const uint maxBounceCount = 10;
const uint raysPerPixel = 1;

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    vec2 numTexels = gl_NumWorkGroups.xy*gl_WorkGroupSize.xy;
    vec2 texCoords = vec2(texelCoord) / numTexels;
    float pixelIndex = texelCoord.y + (numTexels.x + numTexels.y) * texelCoord.x;
    seed = uint(u_time * pixelIndex);

    vec3 color = vec3(0);
    for(uint i = 0; i < raysPerPixel; ++i) {
        color += rayColor(calculateRay(texCoords, u_camera));
    }
    color /= raysPerPixel;
    if(u_numAccumFrames > 0) {
        float weight = 1.0 / (u_numAccumFrames + 1);
        color = (1 - weight) * imageLoad(u_output, texelCoord).xyz + weight * color;
    }
    imageStore(u_output, texelCoord, vec4(color, 1.0));
}

vec3 rayColor(Ray ray) {
    vec3 raycolor = vec3(1);
    vec3 incominglight = vec3(0);
    for(uint i = 0; i < maxBounceCount; ++i) {
        Hitinfo info = rayScene(ray);
        if(info.exists) {
            ray.origin = info.position;
            vec3 diffuseDir = info.normal + randUnitSphere(seed);
            vec3 specularDir = reflect(ray.direction, info.normal);
            bool isSpecular = info.material.specularProbability >= randZeroOne(seed);
            ray.direction = lerp(diffuseDir, specularDir, info.material.smoothness * float(isSpecular));
            incominglight += info.material.emission * raycolor;
            raycolor *= lerp(info.material.color, info.material.specularColor, float(isSpecular));
        } else {
            break;
        }
    }
    return vec3(incominglight);
}
Hitinfo rayScene(Ray ray) {
    Hitinfo info;
    float closestIntersection = 1.0/0.0;
    for(uint modelIndex = 0; modelIndex < u_modelCount; ++modelIndex) {
        if(!rayAABBb(ray, AABB(vec3(u_models[modelIndex].modelMat * vec4(u_models[modelIndex].aabb.min, 1)), vec3(u_models[modelIndex].modelMat * vec4(u_models[modelIndex].aabb.max, 1))))) {
            continue;
        }
        // for(uint indexIndex = u_models[modelIndex].indexOffset; indexIndex < u_models[modelIndex].indicesCount + u_models[modelIndex].indexOffset; indexIndex+=3) {
        //     Triangle triangle = Triangle(
        //         (u_models[modelIndex].modelMat * positions[indices[indexIndex+0] + u_models[modelIndex].vertexOffset]).xyz, 
        //         (u_models[modelIndex].modelMat * positions[indices[indexIndex+1] + u_models[modelIndex].vertexOffset]).xyz, 
        //         (u_models[modelIndex].modelMat * positions[indices[indexIndex+2] + u_models[modelIndex].vertexOffset]).xyz
        //     );
        //     float intersection = rayTriangle(ray, triangle);
        //     if(intersection != -1 && intersection < closestIntersection) {
        //         closestIntersection = intersection;
        //         info.normal = (u_models[modelIndex].normalMat * normals[indices[indexIndex] + u_models[modelIndex].vertexOffset]).xyz;
        //         info.material = u_models[modelIndex].material;
        //     }
        // }

        Sphere sphere = Sphere(vec3(u_models[modelIndex].modelMat[3][0], u_models[modelIndex].modelMat[3][1], u_models[modelIndex].modelMat[3][2]), u_models[modelIndex].modelMat[0][0]);
        float intersection = raySphere(ray, sphere);
        if(intersection != -1 && intersection < closestIntersection) {
            closestIntersection = intersection;
            info.position = ray.origin + closestIntersection * normalize(ray.direction);
            info.normal = normalize(info.position - sphere.center);
            info.material = u_models[modelIndex].material;
        }

    }
    info.exists = closestIntersection != 1.0/0.0;
    return info;
}
Ray calculateRay(vec2 texCoords, Camera camera) {
    vec2 NDCcoords = texCoords * 2.0 - 1.0;
    float nearPlaneScale = tan(radians(camera.fov) * 0.5);
    vec2 viewPortCoords = vec2(NDCcoords.x * camera.aspect * nearPlaneScale, NDCcoords.y * nearPlaneScale);

    const vec3 rayDir = camera.forward + viewPortCoords.x * camera.right + viewPortCoords.y * camera.up;

    return Ray(normalize(rayDir + randUnitSphere(seed) * 0.0001), camera.position);
}
float rayTriangle(Ray ray, Triangle triangle) {
    // https://stackoverflow.com/a/42752998
    vec3 edgeAB = triangle.B - triangle.A;
    vec3 edgeAC = triangle.C - triangle.A;
    vec3 normalVector = cross(edgeAB, edgeAC);
    vec3 ao = ray.origin - triangle.A;
    vec3 dao = cross(ao, ray.direction);

    float determinant = -dot(ray.direction, normalVector);
    float invDet = 1 / determinant;

    // Calculate dst to triangle & barycentric coordinates of intersection point
    float dst = dot(ao, normalVector) * invDet;
    float u = dot(edgeAC, dao) * invDet;
    float v = -dot(edgeAB, dao) * invDet;
    float w = 1 - u - v;

    // Initialize hit info
    float expr = float(determinant >= 1E-8 && dst >= 0 && u >= 0 && v >= 0 && w >= 0);
    return expr * dst + (1 - expr) * -1;
}
float rayAABB(Ray ray, AABB aabb) {
    vec3 rayInvDir = 1 / ray.direction;
    vec3 tMin = (aabb.min - ray.origin) * rayInvDir;
    vec3 tMax = (aabb.max - ray.origin) * rayInvDir;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);

    bool hit = tFar >= tNear && tFar > 0;
    float dst = hit ? tNear > 0 ? tNear : 0 : -1;
    return dst;
}
bool rayAABBb(Ray ray, AABB aabb) {
    vec3 rayInvDir = 1 / ray.direction;
    vec3 tMin = (aabb.min - ray.origin) * rayInvDir;
    vec3 tMax = (aabb.max - ray.origin) * rayInvDir;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);

    return tFar >= tNear && tFar > 0;
}
float raySphere(Ray ray, Sphere sphere) {
    const vec3 OC = sphere.center - ray.origin;
    const float a = dot(ray.direction, ray.direction);
    const float b = -2.0 * dot(ray.direction, OC);
    const float c = dot(OC, OC) - sphere.radius * sphere.radius;
    const float discriminant = b*b - 4*a*c;
    if(discriminant < 0) return -1;
    
    const float sqrtDiscriminant = sqrt(discriminant);
    float t = min((-b - sqrtDiscriminant) / (2*a), (-b + sqrtDiscriminant) / (2*a));

    if(t < 0) return -1;

    return t;
}

uint rand(inout uint state) {
	state = state * 747796405u + 2891336453u;
	uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	return (word >> 22u) ^ word;
}
float randZeroOne(inout uint state) {
    return float(rand(state)) * (1.0 / float(0xffffffffu));
}
float randNegOneOne(inout uint state) {
    return randZeroOne(state) * 2.0 - 1.0;
}
vec3 randUnitSphere(inout uint state) {
    return normalize(vec3(randNormalDistribution(state) * 2 - 1, randNormalDistribution(state) * 2 - 1, randNormalDistribution(state))) * 2 - 1;
}
vec3 randHemisphere(inout uint state, vec3 normal) {
    vec3 randSphere = randUnitSphere(state);

    if(dot(normal, randSphere) < 0) return -randSphere;
    return randSphere;
}
float randNormalDistribution(inout uint state) {
    // Thanks to https://stackoverflow.com/a/6178290
    float theta = 2 * 3.1415926 * randZeroOne(state);
    float rho = sqrt(-2 * log(randZeroOne(state)));
    return rho * cos(theta);
}