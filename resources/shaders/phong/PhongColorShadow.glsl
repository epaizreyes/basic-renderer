#shader vertex
#version 330 core

// Include transformation matrices
#include "Resources/shaders/common/matrix/LightMatrix.glsl"

// Include vertex shader
#include "Resources/shaders/common/vertex/PN-S.vs.glsl"

#shader fragment
#version 330 core

// Include material, view and light properties
#include "Resources/shaders/common/material/PhongColorMaterial.glsl"
#include "Resources/shaders/common/view/SimpleView.glsl"
#include "Resources/shaders/common/light/ShadowLight.glsl"

// Include fragment inputs
#include "Resources/shaders/common/fragment/PN.fs.glsl"
#include "Resources/shaders/common/fragment/L.fs.glsl"

// Include additional functions
#include "Resources/shaders/common/utils/Saturate.glsl"
#include "Resources/shaders/common/shading/PhongSpecular.glsl"
#include "Resources/shaders/common/shading/Phong.glsl"

#include "Resources/shaders/common/shadow/PCF.glsl"
#include "Resources/shaders/common/shadow/SimpleSM.glsl"

// Entry point of the fragment shader
void main()
{
    // Calculate the normalized surface normal
    vec3 normal = normalize(v_Normal);
    
    // Calculate the normalized light direction vector
    vec3 lightDirection = normalize(u_Light.Position - v_Position);
    
    // Calculate shadow factor using the calculateShadow function
    // Parameters: Normal vector (normal), Light direction vector (lightDirection),
    // Light space position (v_LightSpacePosition)
    float shadow = calculateShadow(normal, lightDirection, v_LightSpacePosition);
    
    // Calculate shading result using Phong shading model with shadows
    // Parameters: Ambient reflection (ka), Diffuse reflection (kd),
    // Specular reflection (ks), Shadow factor (calculated above)
    vec3 result = calculateColor(u_Material.Ka, u_Material.Kd, u_Material.Ks, shadow);
    
    // Set the fragment color with the calculated result and material's alpha
    color = vec4(result, u_Material.Alpha);
}
