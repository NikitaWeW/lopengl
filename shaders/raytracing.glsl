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
    float radius;
};
struct ShpereRayCollision {
    bool isCollide;
    vec3 location;
};
ShpereRayCollision raySphere(Ray ray, Sphere sphere);

vec3 rayColor(Ray ray);
Ray calculateRay(vec2 texCoords, Camera camera);

Sphere testSphere = Sphere(vec3(0,0,-4), 1);

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    vec2 texCoords = vec2(texelCoord) / (gl_NumWorkGroups.xy*gl_WorkGroupSize.xy);

    Ray ray = calculateRay(texCoords, u_camera);
    imageStore(u_output, texelCoord, vec4(rayColor(ray), 1.0));
}

vec3 rayColor(Ray ray) {
    // return ray.origin;
    ShpereRayCollision sphereCollision = raySphere(ray, testSphere);
    if(sphereCollision.isCollide) {
        vec3 normal = normalize(sphereCollision.location - testSphere.center);
        return sphereCollision.location;
    } else {
        return (1 - normalize(ray.direction).y) * vec3(0.3, 0.5, 0.7);
    }
}
ShpereRayCollision raySphere(Ray ray, Sphere sphere) {
    const vec3 OC = sphere.center - ray.origin;
    const float a = dot(ray.direction, ray.direction);
    const float b = -2.0 * dot(ray.direction, OC);
    const float c = dot(OC, OC) - sphere.radius * sphere.radius;
    const float discriminant = b*b - 4*a*c;
    if(discriminant < 0) return ShpereRayCollision(false, vec3(0));
    
    const float sqrtDiscriminant = sqrt(discriminant);
    float t = min((-b - sqrtDiscriminant) / (2*a), (-b + sqrtDiscriminant) / (2*a));

    if(t < 0) return ShpereRayCollision(false, vec3(0));

    return ShpereRayCollision(true, ray.origin + t * normalize(ray.direction));
}
Ray calculateRay(vec2 texCoords, Camera camera) {
    vec2 NDCcoords = texCoords * 2.0 - 1.0;
    float nearPlaneScale = tan(radians(camera.fov) * 0.5);
    vec2 viewPortCoords = vec2(NDCcoords.x * camera.aspect * nearPlaneScale, NDCcoords.y * nearPlaneScale);

    const vec3 rayDir = camera.forward + viewPortCoords.x * camera.right + viewPortCoords.y * camera.up;

    return Ray(normalize(rayDir), camera.position);
}