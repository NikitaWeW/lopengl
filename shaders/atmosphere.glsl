/*
Copyright (c) 2025 Nikita Martynau (MIT license: https://opensource.org/license/mit)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#shader vertex
#version 330 core

layout (location = 0) in vec4 a_position;

out VS_OUT {
    flat vec3 camPosLocalSpace;
    flat vec3 sunPosLocalSpace;
    vec3 fragPosLocalSpace;
} vs_out;

uniform mat4 u_modelMat;
uniform mat4 u_viewMat;
uniform mat4 u_projectionMat;

uniform vec3 u_camPos;
uniform vec3 u_sunPos;

void main() 
{
    gl_Position = u_projectionMat * u_viewMat * u_modelMat * a_position;
    mat4 invModelMat = inverse(u_modelMat);
    vs_out.camPosLocalSpace = vec3(invModelMat * vec4(u_camPos, 1));
    vs_out.sunPosLocalSpace = vec3(invModelMat * vec4(u_sunPos, 1));
    vs_out.fragPosLocalSpace = vec3(a_position);
}

#shader fragment
#version 330 core

in VS_OUT {
    flat vec3 camPosLocalSpace;
    flat vec3 sunPosLocalSpace;
    vec3 fragPosLocalSpace;
} fs_in;

out vec4 o_color;

uniform float u_planetSize;
uniform float u_atmosphereSize; // relative to planetSize
uniform float u_sunIntensity;

struct Ray 
{
    vec3 origin;
    vec3 direction;
};
struct AABB 
{
    vec3 min;
    vec3 max;
};
// returns (close; far) ray intersection points or values < 0 if no intersection
vec2 rayAABB(Ray ray, AABB aabb) 
{
    if(ray.direction == vec3(0)) return vec2(-1);
    vec3 rayInvDir = 1 / ray.direction;
    vec3 tMin = (aabb.min - ray.origin) * rayInvDir;
    vec3 tMax = (aabb.max - ray.origin) * rayInvDir;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);

    return tFar >= tNear && tFar > 0 ?
        (tNear < 0  ?
            vec2(0, tFar) :
            vec2(tNear, tFar)) :
        vec2(-1, -2);
}
vec3 at(Ray ray, float t)
{
    return t * ray.direction + ray.origin;
}
float getHeight(vec3 point, float planetRadius)
{
    vec3 q = abs(vec3(0) - point) - planetRadius;
    float sd = length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
    return max(sd, 0);
}
float dencityAtHeight(float height, float scaleHeight)
{
    return exp(-height / scaleHeight) * 0.1;
}

// thanks to https://github.com/wwwtyro/glsl-atmosphere
// constants
// =========
const float PI = 3.14159265;
const float numLightSamples = 2;
const float numViewSamples = 2;
const float scaleHeight_R = 0.8;
const float scaleHeight_M = 0.1;
const float g = 0.88;
const vec3 beta_R = vec3(0.055, 0.13, 0.3);
const float beta_M = 0.021;
// =========
// viewRay and sunDir are in local space!
vec4 atmosphere(Ray viewRay, vec3 sunDir, float sunIntensity, float planetSize, float atmosphereSize, float meshRadius) {
    // Normalize the sun and view directions.
    sunDir = normalize(sunDir);
    viewRay.direction = normalize(viewRay.direction);

    // Calculate atmosphere and planet mathematical models
    AABB atmosphereBox;
    atmosphereBox.min = -vec3(meshRadius);
    atmosphereBox.max =  vec3(meshRadius);

    float planetRadius = planetSize * meshRadius / (planetSize + atmosphereSize);
    AABB planetBox;
    planetBox.min = -vec3(planetRadius);
    planetBox.max =  vec3(planetRadius);

    // Calculate the step size of the primary ray.
    vec2 viewRayAtmosphereIntersection = rayAABB(viewRay, atmosphereBox);
    if(viewRayAtmosphereIntersection.x < 0) return vec4(0);
    vec2 viewRayPlanetIntersection = rayAABB(viewRay, planetBox);
    viewRayAtmosphereIntersection.y = viewRayPlanetIntersection.x < 0 ? viewRayAtmosphereIntersection.y : viewRayPlanetIntersection.x;

    float viewRayStepSize = (viewRayAtmosphereIntersection.y - viewRayAtmosphereIntersection.x) / float(numViewSamples);

    // Calculate shadow
    // maybe shadow mapping?
    Ray viewRayLightRay = Ray(at(viewRay, viewRayAtmosphereIntersection.y - 1e-4), sunDir);
    vec2 viewRayLightRayPlanetIntersection = rayAABB(viewRayLightRay, planetBox);
    float shadow_R = clamp(1 - dot(normalize(viewRayLightRay.origin), sunDir) - 0.9, 0, 0.9);
    float shadow_M = float(viewRayLightRayPlanetIntersection.x >= 0);

    // Initialize accumulators for Rayleigh and Mie scattering.
    vec3 total_R = vec3(0);
    vec3 total_M = vec3(0);

    // Initialize optical depth accumulators for the primary ray.
    float viewRayOpticalDepth_R = 0.0;
    float viewRayOpticalDepth_M = 0.0;

    // Calculate the Rayleigh and Mie phases.
    float mu = dot(viewRay.direction, sunDir);
    float mu2 = mu * mu;
    float g2 = g * g;
    float phase_R = 3.0 / (16.0 * PI) * (1.0 + mu2);
    float phase_M = 3.0 / (8.0 * PI) * ((1.0 - g2) * (mu2 + 1.0)) / (pow(1.0 + g2 - 2.0 * mu * g, 1.5) * (2.0 + g2));

    // Sample the primary ray.
    for (int i = 0; i < numViewSamples; i++) {
        // Calculate the primary ray sample position.
        vec3 viewRaySample = at(viewRay, viewRayAtmosphereIntersection.x + i * viewRayStepSize + viewRayStepSize * 0.5);

        // Calculate the height of the sample.
        float viewRaySampleHeight = getHeight(viewRaySample, planetRadius);

        // Calculate the optical depth of the Rayleigh and Mie scattering for this step.
        float viewRaySampleOpticalDepth_R = dencityAtHeight(viewRaySampleHeight, scaleHeight_R) * viewRayStepSize;
        float viewRaySampleOpticalDepth_M = dencityAtHeight(viewRaySampleHeight, scaleHeight_M) * viewRayStepSize;

        // Accumulate optical depth.
        viewRayOpticalDepth_R += viewRaySampleOpticalDepth_R;
        viewRayOpticalDepth_M += viewRaySampleOpticalDepth_M;

        // Calculate the step size of the secondary ray.
        Ray lightRay;
        lightRay.origin = viewRaySample;
        lightRay.direction = sunDir;
        vec2 lightRayAtmosphereIntersection = rayAABB(lightRay, atmosphereBox);
        float lightRayStepSize = (lightRayAtmosphereIntersection.y - lightRayAtmosphereIntersection.x) / float(numLightSamples);

        // Initialize optical depth accumulators for the secondary ray.
        float lightRayOpticalDepth_R = 0.0;
        float lightRayOpticalDepth_M = 0.0;

        // Sample the secondary ray.
        for (int j = 0; j < numLightSamples; j++) {
            // Calculate the secondary ray sample position.
            vec3 lightRaySample = at(lightRay, lightRayAtmosphereIntersection.x + j * lightRayStepSize + lightRayStepSize * 0.5);

            // Calculate the height of the sample.
            float lightRaySampleHeight = getHeight(lightRaySample, planetRadius);

            // Accumulate the optical depth.
            lightRayOpticalDepth_R += dencityAtHeight(lightRaySampleHeight, scaleHeight_R) * lightRayStepSize;
            lightRayOpticalDepth_M += dencityAtHeight(lightRaySampleHeight, scaleHeight_M) * lightRayStepSize;
        }

        // Calculate attenuation.
        vec3 attenuation = exp(-(beta_M * (viewRayOpticalDepth_M + lightRayOpticalDepth_M) + beta_R * (viewRayOpticalDepth_R + lightRayOpticalDepth_R)));

        // Accumulate scattering.
        total_R += viewRaySampleOpticalDepth_R * attenuation ;
        total_M += viewRaySampleOpticalDepth_M * attenuation ;
    }
    return mix(
        vec4(
            sunIntensity * (phase_R * beta_R * total_R * (1 - shadow_R) + phase_M * beta_M * total_M * (1 - shadow_M)),
            (1.0 - shadow_R) * 0.9
        ),
        vec4(
            normalize(sunIntensity * (phase_R * beta_R * total_R * (1 - shadow_R) + phase_M * beta_M * total_M * (1 - shadow_M))),
            (1.0 - shadow_R) * (1.0 - exp(-(viewRayAtmosphereIntersection.y - viewRayAtmosphereIntersection.x))) * 0.9
        ),
        min(viewRayAtmosphereIntersection.x * 2, 1)
    );
}

void main()
{
    Ray viewRay; // in the atmosphere local space
    viewRay.direction = normalize(fs_in.fragPosLocalSpace - fs_in.camPosLocalSpace);
    viewRay.origin = fs_in.camPosLocalSpace;
    const float meshRadius = 0.5; // the radius of the atmosphere mesh in local space. in my case, the atmosphere is being rendered as a 1x1x1 cube, which means the radius is 0.5

    o_color = atmosphere(
        viewRay, 
        normalize(fs_in.sunPosLocalSpace), 
        u_sunIntensity, 
        u_planetSize, 
        u_atmosphereSize, 
        meshRadius
    );
    o_color.rgb = 1.0 - exp(-1.0 * o_color.rgb); // apply exposure
    o_color.rgb = pow(o_color.rgb, vec3(1/2.2)); // apply gamma correction
}
