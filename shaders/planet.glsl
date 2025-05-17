#shader vertex
#version 430 core
layout(location = 0) in vec4 a_position;
layout(location = 0) in vec4 a_normal;

out vec3 v_fragPos;
out vec3 v_normal;

uniform mat4 u_modelMat;
uniform mat4 u_viewMat;
uniform mat4 u_projectionMat;

void main() {
    gl_Position = u_projectionMat * u_viewMat * u_modelMat * a_position;
    v_fragPos = vec3(u_modelMat * a_position);
    mat4 normalMat = transpose(inverse(u_modelMat));
    v_normal = vec3(normalMat * a_normal);
}

#shader fragment
#version 430 core
out vec4 o_color;

in vec3 v_fragPos;
in vec3 v_normal;

uniform vec4 u_color;
uniform vec3 u_camPos;
uniform vec3 u_sunPos;

vec4 light(vec3 lightDir, vec3 lightColor, vec3 norm, vec3 viewDir) {
    lightDir = normalize(lightDir);
    norm = normalize(norm);
    viewDir = normalize(viewDir);

    vec3 ambient = lightColor * 0.05;
    vec3 diffuse = 
        lightColor * 
        vec3(max(dot(norm, lightDir), 0.0));
    vec3 specular = vec3(0);
    float shadow = 0;

    return vec4(ambient + (1 - shadow) * (diffuse + specular), 1.0);
}

void main() {
    o_color = light(normalize(u_sunPos), vec3(1), v_normal, normalize(u_camPos - v_fragPos)) * u_color;
    o_color.a = 1;
    o_color.rgb = 1.0 - exp(-1.0 * o_color.rgb); // apply exposure
    o_color.rgb = pow(o_color.rgb, vec3(1/2.2)); // apply gamma correction
}