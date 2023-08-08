#shader vertex
#version 330 core

struct Transform {
    mat4 Model;
    mat4 View;
    mat4 Projection;
    
    mat3 Normal;
};

layout (location = 0) in vec4 a_Position;
layout (location = 1) in vec3 a_Normal;

uniform Transform u_Transform;

out vec3 v_WorldPosition;
out vec3 v_Normal;

void main()
{
    vec4 worldPosition = u_Transform.Model * a_Position;
    
    v_WorldPosition = worldPosition.xyz / worldPosition.w;  // Perspective divide to get world space position
    v_Normal = u_Transform.Normal * a_Normal;               // Transform normal to world space
    
    // Calculate final vertex position in clip space
    gl_Position = u_Transform.Projection * u_Transform.View * u_Transform.Model * a_Position;
}

#shader fragment
#version 330 core

struct Material {
    vec3 Ka;
    vec3 Kd;
    vec3 Ks;
    
    float Shininess;
    float Alpha;
};

struct View {
    vec3 Position;
};

struct Light {
    vec3 Position;
    
    vec3 Color;
    
    float La;
    float Ld;
    float Ls;
};

layout (location = 0) out vec4 color;

uniform View u_View;
uniform Light u_Light;

uniform Material u_Material;

in vec3 v_WorldPosition;
in vec3 v_Normal;

float saturate(float x) {
    return clamp(x, 0.0f, 1.0f);
}

vec3 calculateSpecular(vec3 V, vec3 R, vec3 ks, float shininess)
{
    float cosPhi = saturate(dot(V, R));
    return pow(cosPhi, shininess) * ks;
}

void main()
{
    // Calculate direction vectors
    vec3 normal = normalize(v_Normal);
    
    vec3 lightDirection = normalize(u_Light.Position - v_WorldPosition);
    vec3 viewDirection = normalize(u_View.Position - v_WorldPosition);
    
    vec3 reflectionDirection = normalize(2.0 * dot(lightDirection, normal) * normal - lightDirection);
    
    // Ambient component
    vec3 ambient = u_Light.La * u_Material.Ka;
    
    // Diffuse component
    float cosTheta = saturate(dot(normal, lightDirection));
    vec3 diffuse = u_Light.Ld * cosTheta * u_Material.Kd;
    
    // Specular component
    vec3 specular = cosTheta > 0.0f ? calculateSpecular(viewDirection, reflectionDirection,
        u_Material.Ks, u_Material.Shininess) : vec3(0.0f);
    
    // Define fragment color using Phong shading
    vec3 result = u_Light.Color * (ambient + diffuse + specular);
    color = vec4(result, u_Material.Alpha);
}
