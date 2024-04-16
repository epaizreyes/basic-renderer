#pragma once

#include "Renderer/Texture/Texture3D.h"

#include "Renderer/Light/Light.h"

#include "Renderer/Material/SimpleMaterial.h"

#include "Renderer/Mesh/MeshUtils.h"
#include "Renderer/Model/ModelUtils.h"

#include <glm/glm.hpp>

/**
 * Represents an environment light source in 3D rendering.
 *
 * The `EnvironmentLight` class extends the `Light` base class to define an environment light source.
 * It provides methods to set and retrieve the light's position, environment map, and additional properties
 * such as the shadow camera and 3D model representation.
 *
 * Copying or moving `EnvironmentLight` objects is disabled to ensure single ownership and prevent
 * unintended duplication of light resources.
 */
class EnvironmentLight : public BaseLight
{
public:
    // Constructor(s)/Destructor
    // ----------------------------------------
    EnvironmentLight(const unsigned int width, const unsigned int height);
    
    /// @brief Destructor for the environment light.
    ~EnvironmentLight() = default;
    
    // Setter(s)
    // ----------------------------------------
    /// @brief Set the strength of the ambient light.
    /// @param s The strength of the ambient component (a value between 0 and 1).
    void SetAmbientStrength(float s) { m_AmbientStrength = s; }
    
    void SetEnvironmentMap(const std::shared_ptr<Texture>& texture);
    
    // Getter(s)
    // ----------------------------------------
    /// @brief Get the ambient light strength.
    /// @return The ambient strength.
    float GetAmbientStrength() const { return m_AmbientStrength; }
    
    /// @brief Get the environment map.
    /// @return The texture describing the environment.
    const std::shared_ptr<Texture>& GetEnvironmentMap() { return m_EnvironmentMap; }
    
    const std::shared_ptr<Texture>& GetIrradianceMap();
    const std::shared_ptr<Texture>& GetPreFilterMap();
    
    // Properties
    // ----------------------------------------
    void DefineLightProperties(const std::shared_ptr<Shader>& shader,
                               const LightFlags& flags,
                               unsigned int& slot) override;
    
    // Render
    // ----------------------------------------
    /// @brief Renders the 3D model that represents the light source.
    void DrawLight() override
    {
        if (m_Model && m_EnvironmentMap)
        {
            Renderer::SetDepthFunction(DepthFunction::LEqual);
            m_Model->DrawModel();
            Renderer::SetDepthFunction(DepthFunction::Less);
        }
    }
    
private:
    // Initialization
    // ----------------------------------------
    void InitEnvironmentFramebuffers(const unsigned int cubeSize);
    void InitEnvironmentMaterials();
    
    // Update
    // ----------------------------------------
    void UpdateEnvironment();
    void UpdateLight();
    
    // Render
    // ----------------------------------------
    void RenderCubeMap(const std::array<glm::mat4, 6> &views, const glm::mat4 &projection,
                       const std::shared_ptr<Material> &material,
                       const std::shared_ptr<FrameBuffer> &framebuffer,
                       const unsigned int& viewportWidth = 0, const unsigned int& viewportHeight = 0,
                       const unsigned int &level = 0, const bool& genMipMaps = true);
    
    // Environment variables
    // ----------------------------------------
private:
    ///< The ambient light intensity.
    float m_AmbientStrength = 0.2f;
    ///< The environment map.
    std::shared_ptr<Texture> m_EnvironmentMap;
    
    ///< Framebuffer(s) for pre-processing.
    FrameBufferLibrary m_Framebuffers;
    ///< Material(s) for pre-processing.
    MaterialLibrary m_Materials;
    
    // Disable the copying or moving of this resource
    // ----------------------------------------
public:
    EnvironmentLight(const EnvironmentLight&) = delete;
    EnvironmentLight(EnvironmentLight&&) = delete;

    EnvironmentLight& operator=(const EnvironmentLight&) = delete;
    EnvironmentLight& operator=(EnvironmentLight&&) = delete;
};
