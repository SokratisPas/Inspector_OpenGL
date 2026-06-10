#version 410 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec3 aNormal;

out vec3 frag_pos;
out vec3 frag_norm;
out vec2 frag_uv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat3 transInvModel;

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);

    frag_pos = worldPos.xyz;

    // normal transformation to world coords
    frag_norm = transInvModel * aNormal;

    frag_uv = aUV;

    gl_Position = projection * view * worldPos;
}