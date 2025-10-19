/*
Copyright (c) 2025 Nikita Martynau (MIT license: https://opensource.org/license/mit)
*/

#version 330

uniform float PlanetSize = 100;
uniform float AtmosphereSize = 15; // relative to planetSize

in vec3 camPosLocalSpace;
in vec3 sunPosLocalSpace;
in vec3 fragPosLocalSpace;

out vec4 fragColor;

const float numLightSamples = 10;
const float numViewSamples = 20;

uniform vec3 WaveLength = vec3(700,540,440);

uniform float distribu = 3.0;
uniform float sunsetcof = 19.0;

uniform vec4 skycorrpowday = vec4(0.7,0.9,0.3,0.5);
uniform vec4 skycorrmulday = vec4(1.2,0.9,2.5,1.2);

uniform vec4 skycorrpowset = vec4(1.2,1.0,0.9,1.1);
uniform vec4 skycorrmulset = vec4(0.8,0.6,1.3,1.2);

uniform vec4 skycorrmulspace = vec4(1.0, 1.0, 1.0,1.0);

uniform float cornerRadius = 0.1;

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

vec2 rayAABB(Ray ray, float cubeside)
{
	AABB aabb;
	aabb.max = vec3(cubeside);
	aabb.min = -aabb.max;

	// ray.origin = vec3(mm * vec4(ray.origin, 1));
	// ray.direction = vec3(mm * vec4(ray.direction, 0));

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
	vec2(-1, -1);
}
float max3(vec3 v)
{
	return max(max(v.x, v.y), v.z);
}
vec3 at(Ray ray, float t)
{
	return t * ray.direction + ray.origin;
}
vec2 intersect(Ray ray, float side)
{
	vec2 box = rayAABB(ray, side);
	// TODO: maybe rounded cube?
	return box;
}

vec3 cubeshellintersect(Ray ray,float innercube,float outercube) {
	float innerclamp = intersect(ray,innercube).x;
	innerclamp -= min(innerclamp,0.) * 1000.;
	vec2 k = intersect(ray,outercube);
	return ray.direction * (min(k.y,innerclamp) - k.x) * 0.9999;
}
// float box(vec3 position, vec3 halfSize, float cornerRadius) {
//    position = abs(position) - halfSize + cornerRadius;
//    return length(max(position, 0.0)) + min(max(max(position.x, position.y), position.z), 0.0) - cornerRadius;
// }
float height(vec3 p) {
	return max3(abs(p));
	// float r = 0.5;
    // float sd = box(p, vec3(1), r);
    // float sd0 = box(vec3(0), vec3(1), r);
    // return sd - sd0;
}

float densityat(vec3 p,float irad,float f) {
	float height = (height(p) - irad)  / (0.5 - irad);
	return exp(-height * f) * (1. - height);
}

float opticaldepth(vec3 p,vec3 dp,float irad, float f) {
	float tdenst = 0.;
	for(float j = 0.; j < numLightSamples; j++) {
		tdenst += densityat(p,irad,f);
		p += dp;
	}
	return tdenst;
}

float planeHeight(vec3 planePos, vec3 planeNorm, vec3 p)
{
	return dot(planeNorm, p - planePos);
}

vec4 atmosphere(Ray ray, vec3 dir,float DensityFalloff,float ScatterStrength) {
	float outcube = 0.5;

	float h = PlanetSize / (PlanetSize + AtmosphereSize) * 0.5;

	float incube = h;

	vec3 indir = cubeshellintersect(ray,incube,outcube);

	float invsamp = 1. / (numViewSamples - 1.);

	vec3 step = indir * invsamp;
	vec3 lightin = vec3(0);
	vec3 inpoint = at(ray,intersect(ray,outcube).x);

	vec3 scattervals = 400. / WaveLength;
	scattervals *= scattervals;
	scattervals *= scattervals * ScatterStrength;

	float veiwoptdepth = 0;

	for(float i = 0.; i < numViewSamples; i++) {
		Ray sunray;

		float stupidLittleCutoffThatHeReallyNeeds = smoothstep(-0.5, 0.1, planeHeight(vec3(0), normalize(sunPosLocalSpace), inpoint));

		float densityhere = densityat(inpoint,h,DensityFalloff) * length(step);
		veiwoptdepth += densityhere;

		sunray.origin = inpoint;
		sunray.direction = dir;
		vec2 k = intersect(sunray,outcube);
		vec3 sunrayin = dir * (k.y - k.x) * invsamp;
		float sunoptdepth = opticaldepth(inpoint,sunrayin,h,DensityFalloff) * length(sunrayin);
		vec3 transmittance = exp(-(sunoptdepth + veiwoptdepth) * scattervals);

		lightin += densityhere * transmittance * stupidLittleCutoffThatHeReallyNeeds;
		inpoint += step;
	}
	lightin *= scattervals * 1.6;
	return vec4(lightin,max(lightin.x,max(lightin.y,lightin.z)));
}

vec2 settings(vec3 p, vec3 l) {
	float h = PlanetSize / (PlanetSize + AtmosphereSize) * 0.5;
	float a = 1. / pow(max(length(p) - h,0.) * 20. + 1., 2);
	float b = clamp(dot(normalize(p),normalize(l)) * 5.3,0.,1.);
	return vec2(a,b);
}

void main()
{
	Ray ray; // in the atmosphere local space
	vec3 raydir = normalize(fragPosLocalSpace - camPosLocalSpace);
	vec3 raypos = camPosLocalSpace;
	ray.direction = raydir;
	ray.origin = raypos;

	vec2 s = settings(ray.origin,sunPosLocalSpace);

	fragColor = atmosphere(
	ray,
	normalize(sunPosLocalSpace),
	distribu,
	sunsetcof
	);
	fragColor.xyz /= fragColor.a;
	fragColor = mix(
		fragColor,
		mix(
			pow(fragColor,skycorrpowset),
			pow(fragColor,skycorrpowday),
			s.y
		),
		s.x
	) * mix(
		skycorrmulspace,
		mix(skycorrmulset,skycorrmulday,s.y),
		s.x
	);

	fragColor = 1 - exp(-fragColor); // hdr tonemapping
}