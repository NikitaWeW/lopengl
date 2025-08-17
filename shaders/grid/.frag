#version 330 core
out vec4 o_color;
in vec2 v_texCoord;

uniform vec4 u_color = vec4(1);

in VS_OUT {
    vec3 fragmentPosition;
    vec3 cameraPosition;
    vec2 texCoord;
} fs_in;

const float maxFadeDistance = 50;

// thx to https://bgolus.medium.com/the-best-darn-grid-shader-yet-727f9278b9d8
float grid(float lineWidth, vec2 texCoord) {
    vec4 uvDDXY = vec4(dFdx(texCoord), dFdy(texCoord)); 
    vec2 uvDeriv = vec2(length(uvDDXY.xz), length(uvDDXY.yw));
    bool invertLine = lineWidth > 0.5;
    vec2 targetWidth = vec2(invertLine ? 1.0 - lineWidth : lineWidth);
    vec2 drawWidth = clamp(targetWidth, uvDeriv, vec2(0.5));
    vec2 lineAA = uvDeriv * 1.5;
    vec2 gridUV = abs(fract(texCoord) * 2.0 - 1.0);
    gridUV = invertLine ? gridUV : 1.0 - gridUV;
    vec2 grid2 = smoothstep(drawWidth + lineAA, drawWidth - lineAA, gridUV);
    grid2 *= clamp(targetWidth / drawWidth, 0.0, 1.0);
    grid2 = mix(grid2, targetWidth, clamp(uvDeriv * 2.0 - 1.0, 0.0, 1.0));
    grid2 = invertLine ? 1.0 - grid2 : grid2;
    float grid = mix(grid2.x, 1.0, grid2.y);
    return grid;
}

void main() {
    o_color = u_color;

    float falloff = smoothstep(1.0, 0.0, length(fs_in.fragmentPosition - fs_in.cameraPosition) / maxFadeDistance); // fade the grid
    o_color.a = (grid(0.01, 1000 * fs_in.texCoord) + grid(0.003, 100 * fs_in.texCoord)) * falloff;
}