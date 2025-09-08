/*
Copyright (c) 2025 Nikita Martynau (https://opensource.org/license/mit)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#version 430

in vec2 v_texCoord;
out vec4 o_color;

uniform uint u_face;
uniform uint u_seed;
uniform bool u_spherical;

vec3 getPoint() {
    // https://www.reddit.com/r/opengl/comments/1kuayos/convert_cubemap_face_uv_to_xyz/
    // u_face is face
    vec3 ret = vec3(0.0, 0.0, 0.0);
    vec2 uv = v_texCoord * 2 - 1;

    // 0  GL_TEXTURE_CUBE_MAP_POSITIVE_X
    if (u_face == 0) {
    ret = vec3(1.0, -uv.y, -uv.x);
    }
    // 1  GL_TEXTURE_CUBE_MAP_NEGATIVE_X
    else if (u_face == 1) {
    ret = vec3(-1.0, -uv.y, uv.x);
    }
    // 2  GL_TEXTURE_CUBE_MAP_POSITIVE_Y
    else if (u_face == 2) {
    ret = vec3(uv.x, 1.0, uv.y);
    }
    // 3  GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
    else if (u_face == 3) {
    ret = vec3(uv.x, -1.0, -uv.y);
    }
    // 4  GL_TEXTURE_CUBE_MAP_POSITIVE_Z
    else if (u_face == 4) {
    ret = vec3(uv.x, -uv.y, 1.0);
    }
    // 5  GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    else {
    ret = vec3(-uv.x, -uv.y, -1.0);
    }
    return ret;
} 

//
// GLSL textureless classic 4D noise "cnoise",
// with an RSL-style periodic variant "pnoise".
// Author:  Stefan Gustavson (stefan.gustavson@gmail.com)
// Version: 20241-11-07
//
// Many thanks to Ian McEwan of Ashima Arts for the
// ideas for permutation and gradient selection.
//
// Copyright (c) 2011 Stefan Gustavson. All rights reserved.
// Distributed under the MIT license. See LICENSE file.
// https://github.com/stegu/webgl-noise
//

vec4 mod289(vec4 x)
{
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}
vec4 permute(vec4 x)
{
    return mod289(((x*34.0)+10.0)*x);
}
vec4 taylorInvSqrt(vec4 r)
{
    return 1.79284291400159 - 0.85373472095314 * r;
}
vec4 fade(vec4 t) {
    return t*t*t*(t*(t*6.0-15.0)+10.0);
}
// Classic Perlin noise
float cnoise(vec4 P)
{
    vec4 Pi0 = floor(P); // Integer part for indexing
    vec4 Pi1 = Pi0 + 1.0; // Integer part + 1
    Pi0 = mod289(Pi0);
    Pi1 = mod289(Pi1);
    vec4 Pf0 = fract(P); // Fractional part for interpolation
    vec4 Pf1 = Pf0 - 1.0; // Fractional part - 1.0
    vec4 ix = vec4(Pi0.x, Pi1.x, Pi0.x, Pi1.x);
    vec4 iy = vec4(Pi0.yy, Pi1.yy);
    vec4 iz0 = vec4(Pi0.zzzz);
    vec4 iz1 = vec4(Pi1.zzzz);
    vec4 iw0 = vec4(Pi0.wwww);
    vec4 iw1 = vec4(Pi1.wwww);

    vec4 ixy = permute(permute(ix) + iy);
    vec4 ixy0 = permute(ixy + iz0);
    vec4 ixy1 = permute(ixy + iz1);
    vec4 ixy00 = permute(ixy0 + iw0);
    vec4 ixy01 = permute(ixy0 + iw1);
    vec4 ixy10 = permute(ixy1 + iw0);
    vec4 ixy11 = permute(ixy1 + iw1);

    vec4 gx00 = ixy00 * (1.0 / 7.0);
    vec4 gy00 = floor(gx00) * (1.0 / 7.0);
    vec4 gz00 = floor(gy00) * (1.0 / 6.0);
    gx00 = fract(gx00) - 0.5;
    gy00 = fract(gy00) - 0.5;
    gz00 = fract(gz00) - 0.5;
    vec4 gw00 = vec4(0.75) - abs(gx00) - abs(gy00) - abs(gz00);
    vec4 sw00 = step(gw00, vec4(0.0));
    gx00 -= sw00 * (step(0.0, gx00) - 0.5);
    gy00 -= sw00 * (step(0.0, gy00) - 0.5);

    vec4 gx01 = ixy01 * (1.0 / 7.0);
    vec4 gy01 = floor(gx01) * (1.0 / 7.0);
    vec4 gz01 = floor(gy01) * (1.0 / 6.0);
    gx01 = fract(gx01) - 0.5;
    gy01 = fract(gy01) - 0.5;
    gz01 = fract(gz01) - 0.5;
    vec4 gw01 = vec4(0.75) - abs(gx01) - abs(gy01) - abs(gz01);
    vec4 sw01 = step(gw01, vec4(0.0));
    gx01 -= sw01 * (step(0.0, gx01) - 0.5);
    gy01 -= sw01 * (step(0.0, gy01) - 0.5);

    vec4 gx10 = ixy10 * (1.0 / 7.0);
    vec4 gy10 = floor(gx10) * (1.0 / 7.0);
    vec4 gz10 = floor(gy10) * (1.0 / 6.0);
    gx10 = fract(gx10) - 0.5;
    gy10 = fract(gy10) - 0.5;
    gz10 = fract(gz10) - 0.5;
    vec4 gw10 = vec4(0.75) - abs(gx10) - abs(gy10) - abs(gz10);
    vec4 sw10 = step(gw10, vec4(0.0));
    gx10 -= sw10 * (step(0.0, gx10) - 0.5);
    gy10 -= sw10 * (step(0.0, gy10) - 0.5);

    vec4 gx11 = ixy11 * (1.0 / 7.0);
    vec4 gy11 = floor(gx11) * (1.0 / 7.0);
    vec4 gz11 = floor(gy11) * (1.0 / 6.0);
    gx11 = fract(gx11) - 0.5;
    gy11 = fract(gy11) - 0.5;
    gz11 = fract(gz11) - 0.5;
    vec4 gw11 = vec4(0.75) - abs(gx11) - abs(gy11) - abs(gz11);
    vec4 sw11 = step(gw11, vec4(0.0));
    gx11 -= sw11 * (step(0.0, gx11) - 0.5);
    gy11 -= sw11 * (step(0.0, gy11) - 0.5);

    vec4 g0000 = vec4(gx00.x,gy00.x,gz00.x,gw00.x);
    vec4 g1000 = vec4(gx00.y,gy00.y,gz00.y,gw00.y);
    vec4 g0100 = vec4(gx00.z,gy00.z,gz00.z,gw00.z);
    vec4 g1100 = vec4(gx00.w,gy00.w,gz00.w,gw00.w);
    vec4 g0010 = vec4(gx10.x,gy10.x,gz10.x,gw10.x);
    vec4 g1010 = vec4(gx10.y,gy10.y,gz10.y,gw10.y);
    vec4 g0110 = vec4(gx10.z,gy10.z,gz10.z,gw10.z);
    vec4 g1110 = vec4(gx10.w,gy10.w,gz10.w,gw10.w);
    vec4 g0001 = vec4(gx01.x,gy01.x,gz01.x,gw01.x);
    vec4 g1001 = vec4(gx01.y,gy01.y,gz01.y,gw01.y);
    vec4 g0101 = vec4(gx01.z,gy01.z,gz01.z,gw01.z);
    vec4 g1101 = vec4(gx01.w,gy01.w,gz01.w,gw01.w);
    vec4 g0011 = vec4(gx11.x,gy11.x,gz11.x,gw11.x);
    vec4 g1011 = vec4(gx11.y,gy11.y,gz11.y,gw11.y);
    vec4 g0111 = vec4(gx11.z,gy11.z,gz11.z,gw11.z);
    vec4 g1111 = vec4(gx11.w,gy11.w,gz11.w,gw11.w);

    vec4 norm00 = taylorInvSqrt(vec4(dot(g0000, g0000), dot(g0100, g0100), dot(g1000, g1000), dot(g1100, g1100)));
    vec4 norm01 = taylorInvSqrt(vec4(dot(g0001, g0001), dot(g0101, g0101), dot(g1001, g1001), dot(g1101, g1101)));
    vec4 norm10 = taylorInvSqrt(vec4(dot(g0010, g0010), dot(g0110, g0110), dot(g1010, g1010), dot(g1110, g1110)));
    vec4 norm11 = taylorInvSqrt(vec4(dot(g0011, g0011), dot(g0111, g0111), dot(g1011, g1011), dot(g1111, g1111)));

    float n0000 = norm00.x * dot(g0000, Pf0);
    float n0100 = norm00.y * dot(g0100, vec4(Pf0.x, Pf1.y, Pf0.zw));
    float n1000 = norm00.z * dot(g1000, vec4(Pf1.x, Pf0.yzw));
    float n1100 = norm00.w * dot(g1100, vec4(Pf1.xy, Pf0.zw));
    float n0010 = norm10.x * dot(g0010, vec4(Pf0.xy, Pf1.z, Pf0.w));
    float n0110 = norm10.y * dot(g0110, vec4(Pf0.x, Pf1.yz, Pf0.w));
    float n1010 = norm10.z * dot(g1010, vec4(Pf1.x, Pf0.y, Pf1.z, Pf0.w));
    float n1110 = norm10.w * dot(g1110, vec4(Pf1.xyz, Pf0.w));
    float n0001 = norm01.x * dot(g0001, vec4(Pf0.xyz, Pf1.w));
    float n0101 = norm01.y * dot(g0101, vec4(Pf0.x, Pf1.y, Pf0.z, Pf1.w));
    float n1001 = norm01.z * dot(g1001, vec4(Pf1.x, Pf0.yz, Pf1.w));
    float n1101 = norm01.w * dot(g1101, vec4(Pf1.xy, Pf0.z, Pf1.w));
    float n0011 = norm11.x * dot(g0011, vec4(Pf0.xy, Pf1.zw));
    float n0111 = norm11.y * dot(g0111, vec4(Pf0.x, Pf1.yzw));
    float n1011 = norm11.z * dot(g1011, vec4(Pf1.x, Pf0.y, Pf1.zw));
    float n1111 = norm11.w * dot(g1111, Pf1);

    vec4 fade_xyzw = fade(Pf0);
    vec4 n_0w = mix(vec4(n0000, n1000, n0100, n1100), vec4(n0001, n1001, n0101, n1101), fade_xyzw.w);
    vec4 n_1w = mix(vec4(n0010, n1010, n0110, n1110), vec4(n0011, n1011, n0111, n1111), fade_xyzw.w);
    vec4 n_zw = mix(n_0w, n_1w, fade_xyzw.z);
    vec2 n_yzw = mix(n_zw.xy, n_zw.zw, fade_xyzw.y);
    float n_xyzw = mix(n_yzw.x, n_yzw.y, fade_xyzw.x);
    return 2.2 * n_xyzw;
}
// white noise
float wnoise(float x){
    return fract(sin(dot(vec2(x, fract((x + 14) * 12.24 * x)), vec2(12.9898, 78.233))) * 43758.5453);
}
float fbm(vec4 st, float amplitude, int octaves) {
    float value = 0.0;
    for (int i = 0; i < octaves; i++) {
        value += amplitude * cnoise(st + i);
        st.xyz *= 2.;
        amplitude *= .5;
    }
    return value;
}
float fbm(vec4 st)
{
    return fbm(st, 1, 6);
}

const vec3 tilt = normalize(vec3(0,1,0));
const float waterLevel = 0.5;
const float mountainSnowLevel = 0.9;
const float PI = 3.14159265359;
const float MOISTURE_DROP_RATE = 0.7;
const float CLIMATE_NOISE_SCALE = 0.25;

// biome colors
const vec3 stoneColor = vec3(0.56, 0.53, 0.47);

const vec3 desertColor = vec3(0.85, 0.76, 0.49); const vec3 desertWaterColor = vec3(0.20, 0.45, 0.40);
const vec3 jungleColor = vec3(0.13, 0.45, 0.18); const vec3 jungleWaterColor = vec3(0.18, 0.41, 0.36);
const vec3 tundraColor = vec3(0.50, 0.55, 0.45); const vec3 tundraWaterColor = vec3(0.25, 0.40, 0.50);
const vec3 plainsColor = vec3(0.36, 0.65, 0.20); const vec3 plainsWaterColor = vec3(0.15, 0.45, 0.55);
const vec3 arcticColor = vec3(0.85, 0.88, 0.92); const vec3 arcticWaterColor = vec3(0.55, 0.75, 0.85);

// biomes
const int NUM_BIOMES = 5;
const int DESERT = 0;
const int JUNGLE = 1;
const int TUNDRA = 2;
const int PLAINS = 3;
const int ARCTIC = 4;
const vec2 biomes[NUM_BIOMES] = vec2[NUM_BIOMES](
    //  temp   moist
    vec2(0.85, 0.15), // DESERT: hot, dry
    vec2(0.85, 0.90), // JUNGLE: hot, wet
    vec2(0.25, 0.35), // TUNDRA: cold, dry-ish
    vec2(0.55, 0.55), // PLAINS: temperate, medium moist
    vec2(0.10, 0.10)  // ARCTIC: cold, dry
);

float[NUM_BIOMES] getBiomeMask(float temp, float moist)
{
    vec2 point = vec2(temp, moist);

    float weights[NUM_BIOMES];
    float sumW = 0.0;

    for (int i = 0; i < NUM_BIOMES; ++i) {
        float d = distance(point, biomes[i]);
        float w = exp(- (d * d) / 0.05);
        weights[i] = w;
        sumW += w;
    }

    float mask[NUM_BIOMES];
    for (int i = 0; i < NUM_BIOMES; ++i) {
        mask[i] = float(weights[i] / sumW);
    }
    return mask;
}
vec3 mountain(vec3 mainCol, float h)
{
    if(h > mountainSnowLevel)
        return arcticColor;
    return mix(
        mix(mainCol, mainCol * 1.25, smoothstep(0.4, 0.9, h)),
        stoneColor, 
        smoothstep(0.7, 0.9, h)
    );
}
vec3 waterDepth(vec3 waterCol, float h)
{
    return mix(
        waterCol * 0.75,
        waterCol * 1.25, 
        smoothstep(0, waterLevel, h)
    );
}
float getMoisture(float lat01, float height01, float noiseSample) {
    float mLat = (1.1 - lat01);
    float mAlt = exp(-height01 * MOISTURE_DROP_RATE);
    float m = mLat * mAlt + noiseSample;
    return clamp(m, 0.0, 1.0);
}

vec3 debugBiome(float weights[NUM_BIOMES])
{
    int best = 0;
    float bestW = weights[0];
    for (int i = 1; i < NUM_BIOMES; ++i) {
        if (weights[i] > bestW) {
            bestW = weights[i];
            best = i;
        }
    }
 
    if(best == DESERT) return vec3(1.0, 0.9, 0.4);   // yellowish
    if(best == JUNGLE) return vec3(0.0, 0.6, 0.0);   // green
    if(best == TUNDRA) return vec3(0.6, 0.6, 0.5);   // grey
    if(best == PLAINS) return vec3(0.4, 0.8, 0.2);   // light green
    if(best == ARCTIC) return vec3(0.8, 0.9, 1.0);   // pale blue
    return vec3(1.0, 0.0, 1.0); // magenta for "error"   
}

void main() {
    // boring stuff
    float seed = u_seed;

    vec3 point = getPoint();
    vec3 dir = normalize(point);
    vec3 xyz = u_spherical ? dir : point;
    float xyzID = xyz.z * xyz.y * xyz.x + xyz.y * xyz.x * xyz.x;

    float noise_w = wnoise(xyzID * float(seed) / 0xffffffffu);
    float noise_t = fbm(vec4(xyz, seed), 0.25, 6);
    float noise_h = fbm(vec4(xyz, seed+1));
    float noise_m = fbm(vec4(xyz * 0.25, seed+2));

    float latitude;
    if(u_spherical) 
        latitude = abs(dot(tilt, dir));
    else
        latitude = (abs(point.y) + 1 - max(abs(point.x), abs(point.z))) * 0.5;

    ////////////////////////////////////////////////////////////////////////////////////////////////////

    // set the height
    float height = noise_h * 0.5 + 0.5;

    // set the temperature and moisture
    float temp = smoothstep(0.1, 1, 1 - latitude) * smoothstep(1, 0.8, height) + CLIMATE_NOISE_SCALE * noise_t;
    float moist = getMoisture(latitude, height, CLIMATE_NOISE_SCALE * noise_m);

    // set the biome
    float[NUM_BIOMES] biome = getBiomeMask(temp, moist);

    temp *= smoothstep(1, 0.9, height);

    ////////////////////////////////////////////////////////////////////////////////////////////////////

    // color the biome

    vec3 color = mountain(
        desertColor * biome[DESERT] +
        jungleColor * biome[JUNGLE] +
        tundraColor * biome[TUNDRA] +
        plainsColor * biome[PLAINS] +
        arcticColor * biome[ARCTIC],
        height
    );

    if(height < waterLevel)
    {
        vec3 water = waterDepth(
            desertWaterColor * biome[DESERT] +
            jungleWaterColor * biome[JUNGLE] +
            tundraWaterColor * biome[TUNDRA] +
            plainsWaterColor * biome[PLAINS] +
            arcticWaterColor * biome[ARCTIC],
            height
        );
        
        color = water;

        if(height >= waterLevel - 0.012) // beaches
        {
            color = 
                vec3(0.74, 0.65, 0.38) * biome[DESERT] +
                vec3(0.47, 0.41, 0.24) * biome[JUNGLE] +
                vec3(0.83, 0.74, 0.47) * biome[PLAINS] +
                tundraColor            * biome[TUNDRA] +
                arcticColor            * biome[ARCTIC];
        }
    }

    color += 0.03 * noise_w;
    // color = debugBiome(biome);
    // color = vec3(noise_m);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    o_color = vec4(color, 1);
}