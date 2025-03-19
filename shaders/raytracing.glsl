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

struct Material {
    vec3 color;
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
uniform Camera u_camera;

float rayTriangle(Ray ray, Triangle triangle);
float rayAABB(Ray ray, AABB aabb);
bool rayAABBb(Ray ray, AABB aabb);
Hitinfo rayScene(Ray ray);
vec3 rayColor(Ray ray);
Ray calculateRay(vec2 texCoords, Camera camera);

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    vec2 numTexels = gl_NumWorkGroups.xy*gl_WorkGroupSize.xy;
    vec2 texCoords = vec2(texelCoord) / numTexels;
    float pixelIndex = texelCoord.y + (numTexels.x + numTexels.y) * texelCoord.x;

    Ray ray = calculateRay(texCoords, u_camera);
    vec3 color = rayColor(ray);
    imageStore(u_output, texelCoord, vec4(color, 1.0));
}

vec3 rayColor(Ray ray) {
    Hitinfo info = rayScene(ray);
    vec3 lightpos = vec3(1, 1, -2);
    return info.exists ? max(dot(normalize(lightpos - info.position), info.normal), 0) * info.material.color * (1 / length(lightpos - info.position)) + 0.01 : vec3(0);
}
Hitinfo rayScene(Ray ray) {
    Hitinfo info;
    float closestIntersection = 1.0/0.0;
    for(uint modelIndex = 0; modelIndex < u_modelCount; ++modelIndex) {
        if(!rayAABBb(ray, AABB(vec3(u_models[modelIndex].modelMat * vec4(u_models[modelIndex].aabb.min, 1)), vec3(u_models[modelIndex].modelMat * vec4(u_models[modelIndex].aabb.max, 1))))) {
            continue;
        }
        for(uint indexIndex = u_models[modelIndex].indexOffset; indexIndex < u_models[modelIndex].indicesCount + u_models[modelIndex].indexOffset; indexIndex+=3) {
            Triangle triangle = Triangle(
                (u_models[modelIndex].modelMat * vec4(positions[indices[indexIndex+0] + u_models[modelIndex].vertexOffset])).xyz, 
                (u_models[modelIndex].modelMat * vec4(positions[indices[indexIndex+1] + u_models[modelIndex].vertexOffset])).xyz, 
                (u_models[modelIndex].modelMat * vec4(positions[indices[indexIndex+2] + u_models[modelIndex].vertexOffset])).xyz
            );
            float intersection = rayTriangle(ray, triangle);
            if(intersection != -1 && intersection < closestIntersection) {
                closestIntersection = intersection;
                info.normal = (u_models[modelIndex].normalMat * vec4(normals[indices[indexIndex] + u_models[modelIndex].vertexOffset])).xyz;
                info.material = u_models[modelIndex].material;
            }
        }
    }
    info.exists = closestIntersection != 1.0/0.0;
    info.position = ray.origin + closestIntersection * normalize(ray.direction);
    return info;
}
Ray calculateRay(vec2 texCoords, Camera camera) {
    vec2 NDCcoords = texCoords * 2.0 - 1.0;
    float nearPlaneScale = tan(radians(camera.fov) * 0.5);
    vec2 viewPortCoords = vec2(NDCcoords.x * camera.aspect * nearPlaneScale, NDCcoords.y * nearPlaneScale);

    const vec3 rayDir = camera.forward + viewPortCoords.x * camera.right + viewPortCoords.y * camera.up;

    return Ray(normalize(rayDir), camera.position);
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
    return determinant >= 1E-8 && dst >= 0 && u >= 0 && v >= 0 && w >= 0 ? dst : -1;
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
