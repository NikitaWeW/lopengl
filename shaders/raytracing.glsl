#shader compute
#version 430 core
layout(local_size_x = 20, local_size_y = 20, local_size_z = 1) in;
layout(rgba16f) uniform image2D u_output;

layout(std430) readonly buffer indicesSSBO {
    int indices[];
};
layout(std430) readonly buffer positionsSSBO {
    vec3 positions[];
};

struct Model {
    uint indexOffset;
    uint vertexOffset;
    uint indicesCount;
};
uniform Model u_models[10];
uniform uint u_modelCount;

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
struct PointIntersection {
    bool exists;
    vec3 location;
};
struct BoolIntersection {
    bool exists;
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

uniform float u_time;
uniform Camera u_camera;

PointIntersection rayTriangle(Ray ray, Triangle triangle);
PointIntersection rayAABB(Ray ray, AABB aabb);
BoolIntersection rayAABBb(Ray ray, AABB aabb);
vec3 getBarycentric(vec3 p, vec3 a, vec3 b, vec3 c);
vec3 rayColor(Ray ray);
Ray calculateRay(vec2 texCoords, Camera camera);

AABB testAABB = AABB(
    vec3(-1, -1, -4),
    vec3(1, 1, -2)
);

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
    for(uint modelIndex = 0; modelIndex < u_modelCount; ++modelIndex) {
        for(uint indexIndex = u_models[modelIndex].indexOffset; indexIndex < u_models[modelIndex].indicesCount; indexIndex+=3) {
            Triangle currentTriangle = Triangle(
                positions[indices[indexIndex+0] + u_models[modelIndex].vertexOffset], 
                positions[indices[indexIndex+1] + u_models[modelIndex].vertexOffset], 
                positions[indices[indexIndex+2] + u_models[modelIndex].vertexOffset]
            );
            PointIntersection intersection = rayTriangle(ray, currentTriangle);
            if(intersection.exists) return vec3(0.7, 0.5, 0.3);
        }
    }
    return vec3(0.3, 0.5, 0.7);
}
Ray calculateRay(vec2 texCoords, Camera camera) {
    vec2 NDCcoords = texCoords * 2.0 - 1.0;
    float nearPlaneScale = tan(radians(camera.fov) * 0.5);
    vec2 viewPortCoords = vec2(NDCcoords.x * camera.aspect * nearPlaneScale, NDCcoords.y * nearPlaneScale);

    const vec3 rayDir = camera.forward + viewPortCoords.x * camera.right + viewPortCoords.y * camera.up;

    return Ray(normalize(rayDir), camera.position);
}
PointIntersection rayTriangle(Ray ray, Triangle triangle) {
    const vec3 normal = normalize(cross(triangle.B - triangle.A, triangle.C - triangle.A)); // change
    const float denominator = dot(normal, ray.direction);
    if(denominator == 0) return PointIntersection(false, vec3(0));
    const float t = dot(normal, triangle.A - ray.origin) / denominator;
    if(t < 0) return PointIntersection(false, vec3(0));
    const vec3 planePoint = ray.origin + ray.direction * t;

    vec3 barycentric = getBarycentric(planePoint, triangle.A, triangle.B, triangle.C);

    if (
        0 <= barycentric.x && barycentric.x <= 1 && 
        0 <= barycentric.y && barycentric.y <= 1 &&
        0 <= barycentric.z && barycentric.z <= 1
    ) return PointIntersection(true, planePoint);
    return PointIntersection(false, vec3(0));
}
vec3 getBarycentric(vec3 p, vec3 a, vec3 b, vec3 c) {
    const vec3 v0 = b - a;
    const vec3 v1 = c - a;
    const vec3 v2 = p - a;
    const float denominator = v0.x * v1.y - v1.x * v0.y;
    const float v = (v2.x * v1.y - v1.x * v2.y) / denominator;
    const float w = (v0.x * v2.y - v2.x * v0.y) / denominator;
    const float u = 1.0f - v - w;
    return vec3(v, w, u);
}
PointIntersection rayAABB(Ray ray, AABB aabb) {
    vec3 dirfrac = 1.0f / ray.direction;
    float t1 = (aabb.min.x - ray.origin.x)*dirfrac.x;
    float t2 = (aabb.max.x - ray.origin.x)*dirfrac.x;
    float t3 = (aabb.min.y - ray.origin.y)*dirfrac.y;
    float t4 = (aabb.max.y - ray.origin.y)*dirfrac.y;
    float t5 = (aabb.min.z - ray.origin.z)*dirfrac.z;
    float t6 = (aabb.max.z - ray.origin.z)*dirfrac.z;

    float tmin = max(max(min(t1, t2), min(t3, t4)), min(t5, t6));
    float tmax = min(min(max(t1, t2), max(t3, t4)), max(t5, t6));

    // if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is behind us
    // if tmin > tmax, ray doesn't intersect AABB
    if (tmax < 0 || tmin > tmax) return PointIntersection(false, vec3(0));
    if(tmin < 0) return PointIntersection(true, ray.origin + tmax * normalize(ray.direction));
    return PointIntersection(true, ray.origin + tmin * normalize(ray.direction));
}
BoolIntersection rayAABBb(Ray ray, AABB aabb) {
    vec3 dirfrac = 1.0f / ray.direction;
    float t1 = (aabb.min.x - ray.origin.x)*dirfrac.x;
    float t2 = (aabb.max.x - ray.origin.x)*dirfrac.x;
    float t3 = (aabb.min.y - ray.origin.y)*dirfrac.y;
    float t4 = (aabb.max.y - ray.origin.y)*dirfrac.y;
    float t5 = (aabb.min.z - ray.origin.z)*dirfrac.z;
    float t6 = (aabb.max.z - ray.origin.z)*dirfrac.z;

    float tmin = max(max(min(t1, t2), min(t3, t4)), min(t5, t6));
    float tmax = min(min(max(t1, t2), max(t3, t4)), max(t5, t6));

    return BoolIntersection(tmin < tmax);
}
