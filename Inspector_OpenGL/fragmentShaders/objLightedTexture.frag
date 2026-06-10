#version 410 core

in vec3 frag_pos;
in vec3 frag_norm;
in vec2 frag_uv;

out vec4 frag_col;

// Texture
uniform sampler2D tex0;

// Light uniforms 
uniform vec3 lightPos;
uniform vec3 lightCol;
uniform vec3 camPos;

// light uniforms from imgui
uniform float ambientStrength;
uniform float diffuseStrength;
uniform float specularStrength;
uniform float k1;
uniform float k2;
uniform float k3;

// show texture
uniform bool showTexture;

// show light
uniform bool isLit;

// mesh color 
uniform vec3 meshColor;

void main()
{
    // texture color
    vec3 tex_color;

    // lightning final
    vec3 lighting = vec3(1.0, 1.0, 1.0);

    // mesh color
    vec3 mesh_color = meshColor;

    if (showTexture)
    {
        tex_color = texture(tex0, frag_uv).rgb;
    }
    else
    {
        tex_color = vec3(1.0, 1.0, 1.0);
    }

    if (isLit)      // if lit we do lighting calculations
    {
        // Ambient
        float ambient = ambientStrength;

        // Diffuse
        vec3 norm = normalize(frag_norm);
        vec3 ldir = normalize(lightPos - frag_pos);
        float diffuse = diffuseStrength * max(dot(norm, ldir), 0.0);

        // Specular
        vec3 view_dir = normalize(camPos - frag_pos);
        vec3 refl_dir = reflect(-ldir, norm);
        float specular = specularStrength * pow(max(dot(view_dir, refl_dir), 0.0), 128.0);

        // Attenuation
        float light_dist = length(lightPos - frag_pos);
        float atten = 1.0 / (k1 + k2 * light_dist + k3 * light_dist * light_dist);

        // Final color
        lighting = (ambient + diffuse + specular) * lightCol * atten;
    }

    // final fragment color
    frag_col = vec4(mesh_color * tex_color * lighting, 1.0);
}