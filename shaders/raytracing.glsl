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

struct Model {
    uint indexOffset;
    uint vertexOffset;
    uint indicesCount;
};
uniform Model u_models[10];
uniform uint u_modelCount;

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

uniform float u_time;
uniform Camera u_camera;

float rayTriangle(Ray ray, Triangle triangle);
float rayAABB(Ray ray, AABB aabb);
bool rayAABBb(Ray ray, AABB aabb);
vec3 getBarycentric(vec3 p, vec3 a, vec3 b, vec3 c);
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
    float closestIntersection = 1.0/0.0;
    for(uint modelIndex = 0; modelIndex < u_modelCount; ++modelIndex) {
        for(uint indexIndex = u_models[modelIndex].indexOffset; indexIndex < u_models[modelIndex].indicesCount + u_models[modelIndex].indexOffset; indexIndex+=3) {
            Triangle triangle = Triangle(
                positions[indices[indexIndex+0] + u_models[modelIndex].vertexOffset].xyz, 
                positions[indices[indexIndex+1] + u_models[modelIndex].vertexOffset].xyz, 
                positions[indices[indexIndex+2] + u_models[modelIndex].vertexOffset].xyz
            );
            float intersection = rayTriangle(ray, triangle);
            if(intersection != -1) {
                closestIntersection = min(intersection, closestIntersection);
            }
        }
    }
    if(closestIntersection == 1.0/0.0) return vec3(0.3, 0.5, 0.7); // miss
    return vec3(ray.origin + closestIntersection * ray.direction);
}
Ray calculateRay(vec2 texCoords, Camera camera) {
    vec2 NDCcoords = texCoords * 2.0 - 1.0;
    float nearPlaneScale = tan(radians(camera.fov) * 0.5);
    vec2 viewPortCoords = vec2(NDCcoords.x * camera.aspect * nearPlaneScale, NDCcoords.y * nearPlaneScale);

    const vec3 rayDir = camera.forward + viewPortCoords.x * camera.right + viewPortCoords.y * camera.up;

    return Ray(normalize(rayDir), camera.position);
}
float rayTriangle(Ray ray, Triangle triangle) {
    // https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm#C++_implementation translated to glsl
    const float epsilon = 1e-10;

    vec3 edge1 = triangle.B - triangle.A;
    vec3 edge2 = triangle.C - triangle.A;
    vec3 ray_cross_e2 = cross(normalize(ray.direction), edge2);
    float det = dot(edge1, ray_cross_e2);

    if (det > -epsilon && det < epsilon)
        return -1;    // This ray is parallel to this triangle.

    float inv_det = 1.0 / det;
    vec3 s = ray.origin - triangle.A;
    float u = inv_det * dot(s, ray_cross_e2);

    if ((u < 0 && abs(u) > epsilon) || (u > 1 && abs(u-1) > epsilon))
        return -1;

    vec3 s_cross_e1 = cross(s, edge1);
    float v = inv_det * dot(normalize(ray.direction), s_cross_e1);

    if ((v < 0 && abs(v) > epsilon) || (u + v > 1 && abs(u + v - 1) > epsilon))
        return -1;

    // At this stage we can compute t to find out where the intersection point is on the line.
    float t = inv_det * dot(edge2, s_cross_e1);

    // t > epsilon: ray intersection
    // else: This means that there is a line intersection but not a ray intersection.
    return float(t > epsilon) * t + float(t <= epsilon) * -1;
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
float rayAABB(Ray ray, AABB aabb) {
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
    if (tmax < 0 || tmin > tmax) return -1;
    if(tmin < 0) return tmax;
    return tmin;
}
bool rayAABBb(Ray ray, AABB aabb) {
    vec3 dirfrac = 1.0f / ray.direction;
    float t1 = (aabb.min.x - ray.origin.x)*dirfrac.x;
    float t2 = (aabb.max.x - ray.origin.x)*dirfrac.x;
    float t3 = (aabb.min.y - ray.origin.y)*dirfrac.y;
    float t4 = (aabb.max.y - ray.origin.y)*dirfrac.y;
    float t5 = (aabb.min.z - ray.origin.z)*dirfrac.z;
    float t6 = (aabb.max.z - ray.origin.z)*dirfrac.z;

    float tmin = max(max(min(t1, t2), min(t3, t4)), min(t5, t6));
    float tmax = min(min(max(t1, t2), max(t3, t4)), max(t5, t6));

    return tmin < tmax;
}
