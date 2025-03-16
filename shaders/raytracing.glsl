#shader compute
#version 430 core
layout(local_size_x = 20, local_size_y = 20, local_size_z = 1) in;
layout(rgba16f) uniform image2D u_output;

struct Camera {
    float fov;
    float aspect;
    vec3 position;
    vec3 forward;
    vec3 right;
    vec3 up;
};
uniform Camera u_camera;
struct Ray {
    vec3 direction;
    vec3 origin;
};
struct Sphere {
    vec3 center;
    vec3 color;
    float radius;
};
struct SphereRayCollision {
    bool exists;
    vec3 location;
};
SphereRayCollision raySphere(Ray ray, Sphere sphere);

struct Triangle {
    vec3 A;
    vec3 B;
    vec3 C;
};
struct RayTriangleIntersection {
    bool exists;
    vec3 location;
};
RayTriangleIntersection rayTriangle(Ray ray, Triangle triangle);
vec3 getBarycentric(vec3 p, vec3 a, vec3 b, vec3 c);

uniform float u_time;

vec3 rayColor(Ray ray);
Ray calculateRay(vec2 texCoords, Camera camera);

Sphere testSphere = Sphere(
    vec3(0,0,-4), 
    vec3(0.7, 0.4, 0.2), 
    1
);
Triangle testTriangle = Triangle(
    vec3(-3,  1, -4),
    vec3( 1,  5, -3),
    vec3( 4, -1, -5)
);

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    vec2 texCoords = vec2(texelCoord) / (gl_NumWorkGroups.xy*gl_WorkGroupSize.xy);

    Ray ray = calculateRay(texCoords, u_camera);
    imageStore(u_output, texelCoord, vec4(rayColor(ray), 1.0));
}

vec3 rayColor(Ray ray) {
    RayTriangleIntersection triangleIntersection = rayTriangle(ray, testTriangle);
    if(triangleIntersection.exists) {
        return normalize(triangleIntersection.location);
    } else {
        return vec3(0.2, 0.3, 0.7) * normalize(1 - ray.direction).y;
    }
}
SphereRayCollision raySphere(Ray ray, Sphere sphere) {
    const vec3 OC = sphere.center - ray.origin;
    const float a = dot(ray.direction, ray.direction);
    const float h = dot(ray.direction, OC);
    const float c = dot(OC, OC) - sphere.radius * sphere.radius;
    const float discriminant = h*h - a*c;
    if(discriminant < 0) return SphereRayCollision(false, vec3(0));
    
    const float sqrtDiscriminant = sqrt(discriminant);
    float t = min((h - sqrtDiscriminant) / a, (h + sqrtDiscriminant) / a);

    if(t < 0) return SphereRayCollision(false, vec3(0));

    return SphereRayCollision(true, ray.origin + t * normalize(ray.direction));
}
Ray calculateRay(vec2 texCoords, Camera camera) {
    vec2 NDCcoords = texCoords * 2.0 - 1.0;
    float nearPlaneScale = tan(radians(camera.fov) * 0.5);
    vec2 viewPortCoords = vec2(NDCcoords.x * camera.aspect * nearPlaneScale, NDCcoords.y * nearPlaneScale);

    const vec3 rayDir = camera.forward + viewPortCoords.x * camera.right + viewPortCoords.y * camera.up;

    return Ray(normalize(rayDir), camera.position);
}
/*
plane equation: n * (a - p) = 0
ray equation: o + d * t
n * (o + d * t - p) = 0
solve for t:
t = (n * (p - o)) / (n*d)
*/
RayTriangleIntersection rayTriangle(Ray ray, Triangle triangle) {
    const vec3 normal = normalize(cross(triangle.B - triangle.A, triangle.C - triangle.A)); // change
    const float denominator = dot(normal, ray.direction);
    if(denominator == 0) return RayTriangleIntersection(false, vec3(0));
    const float t = dot(normal, triangle.A - ray.origin) / denominator;
    if(t < 0) return RayTriangleIntersection(false, vec3(0));
    const vec3 planePoint = ray.origin + ray.direction * t;

    vec3 barycentric = getBarycentric(planePoint, triangle.A, triangle.B, triangle.C);

    if (
        0 <= barycentric.x && barycentric.x <= 1 && 
        0 <= barycentric.y && barycentric.y <= 1 &&
        0 <= barycentric.z && barycentric.z <= 1
    ) return RayTriangleIntersection(true, planePoint);
    return RayTriangleIntersection(false, vec3(0));
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
