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

vec3 rayColor(Ray ray);
ShpereRayCollision raySphere(Ray ray, Sphere sphere);

Sphere testSphere = Sphere(vec3(0,0,-4), 1);

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    vec2 texCoords = vec2(texelCoord) / (gl_NumWorkGroups.xy*gl_WorkGroupSize.xy);

    vec2 texCoordsNDC = texCoords * 2 - 1;
    float tangentFOV = tan(radians(u_camera.fov) * 0.5);
    vec2 offset = vec2(texCoordsNDC.x * u_camera.aspect * tangentFOV, texCoordsNDC.y * tangentFOV);
    Ray ray = Ray(normalize(u_camera.forward + offset.x * u_camera.right + offset.y * u_camera.up), u_camera.position);

    imageStore(u_output, texelCoord, vec4(rayColor(ray), 1.0));
}

vec3 rayColor(Ray ray) {
    ShpereRayCollision sphereCollision = raySphere(ray, testSphere);
    if(sphereCollision.isCollide) {
        return normalize(sphereCollision.location - testSphere.center);
    } else {
        return (1 - normalize(ray.direction).y) * vec3(0.3, 0.5, 0.7);
    }
}
ShpereRayCollision raySphere(Ray ray, Sphere sphere) {
    vec3 OC = sphere.center - ray.origin;
    float a = dot(ray.direction, ray.direction);
    float b = -2.0 * dot(ray.direction, OC);
    float c = dot(OC, OC) - sphere.radius * sphere.radius;
    float discriminant = b*b - 4*a*c;
    if(discriminant < 0) return ShpereRayCollision(false, vec3(0));
    return ShpereRayCollision(true, ray.origin + ((-b - sqrt(discriminant)) / 2*a) * normalize(ray.direction));
}