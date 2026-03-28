// Original source: https://www.shadertoy.com/view/Ml2cWG

#version 330 core

in vec3 vPos;
out vec4 oColor;

const float pi = 3.14159265359;
const float invPi = 1.0 / pi;

uniform float uZenithOffset = 0.0;
uniform float uMultiScatterPhase = 0.1;
uniform float uDensity = 0.7;
uniform vec3  uSkyColor = vec3(0.39, 0.57, 1.0); // Make sure none of the components is never 0.0
uniform vec3  uLightPos;

float smoothing(float x) 
{ 
    return x*x*(3.0-2.0*x);
}
float zenithDensity(float x) 
{
    return uDensity / pow(max(x - uZenithOffset, 0.35e-2), 0.75);
}

vec3 getSkyAbsorption(vec3 x, float y)
{
	vec3 absorption = x * -y;
    absorption = exp2(absorption) * 2.0;
	
	return absorption;
}

// probably want it to be a square
float getSunPoint(vec3 p, vec3 lightPos)
{
	return smoothstep(0.03, 0.026, distance(p, lightPos)) * 50.0;
}

float getRayleigMultiplier(vec3 p, vec3 lightPos)
{
	return 1.0 + pow(1.0 - clamp(distance(p, lightPos), 0.0, 1.0), 2.0) * pi * 0.5;
}

float getMie(vec3 p, vec3 lightPos)
{
	float disk = clamp(1.0 - pow(distance(p, lightPos), 0.1), 0.0, 1.0);
	
	return disk*disk*(3.0 - 2.0 * disk) * 2.0 * pi;
}

vec3 getAtmosphericScattering(vec3 p, vec3 lightPos)
{
	float zenith = zenithDensity(p.y);
	float sunPointDistMult = clamp(length(max(lightPos.y + uMultiScatterPhase - uZenithOffset, 0.0)), 0.0, 1.0);
	
	float rayleighMult = getRayleigMultiplier(p, lightPos);
	
	vec3 absorption = getSkyAbsorption(uSkyColor, zenith);
    vec3 sunAbsorption = getSkyAbsorption(uSkyColor, zenithDensity(lightPos.y + uMultiScatterPhase));
	vec3 sky = uSkyColor * zenith * rayleighMult;
	vec3 sun = getSunPoint(p, lightPos) * absorption;
	vec3 mie = getMie(p, lightPos) * sunAbsorption;
	
	vec3 totalSky = mix(sky * absorption, sky / (sky + 0.5), sunPointDistMult);
         totalSky += sun + mie;
	     totalSky *= sunAbsorption * 0.5 + 0.5 * length(sunAbsorption);
	
	return totalSky;
}

vec3 jodieReinhardTonemap(vec3 c)
{
    float l = dot(c, vec3(0.2126, 0.7152, 0.0722));
    vec3 tc = c / (c + 1.0);

    return mix(c / (l + 1.0), tc, tc);
}

void main() {

	vec3 color = getAtmosphericScattering(normalize(vPos), normalize(uLightPos));
    
    // Tone mapping and gamma correction
	color = jodieReinhardTonemap(color);
    color = pow(color, vec3(2.2)); 

    oColor = vec4(color, 1);
}