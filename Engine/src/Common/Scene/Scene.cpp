#include "enginepch.h"
#include "Common/Scene/Scene.h"

#include "Common/Renderer/Material/LightedMaterial.h"
#include "Common/Renderer/Material/SimpleMaterial.h"
#include "Common/Renderer/Light/PositionalLight.h"

/**
 * Define a scene to be rendered.
 *
 * @param width Viewport width.
 * @param height Viewport height.
 * @param viewportShader The shader file path to be used for shading the viewport.
 */
Scene::Scene(int width, int height,
             const std::filesystem::path& viewportShader)
{
    // Define the rendering camera
    m_Camera = std::make_shared<PerspectiveCamera>(width, height);
    
    // Define the lights
    m_Lights.Create<EnvironmentLight>("Environment", width, height);
    
    // Define the viewport
    m_Viewport = std::make_shared<Viewport>(width, height, viewportShader);
    
    Renderer::GetMaterialLibrary().Add("Viewport", m_Viewport->m_Material);
    m_FramebufferLibrary.Add("Viewport", m_Viewport->m_Framebuffer);
    m_Models.Add("Viewport", m_Viewport->m_Geometry);
}

/**
 * Draws the scene using the provided render pass specification.
 *
 * @param pass The render pass specification containing the parameters for drawing the scene.
 */
void Scene::Draw(const RenderPassSpecification &pass)
{
    // Run the post-rendering code
    if (pass.PreRenderCode)
        pass.PreRenderCode();
    
    // Bind the framebuffer if it is provided
    if (pass.Framebuffer)
        pass.Framebuffer->Bind();
    
    // Begin the scene with the provided camera, or without a camera if none is provided
    if (pass.Camera)
        Renderer::BeginScene(pass.Camera);
    else
        Renderer::BeginScene();
    
    if(pass.Size.has_value())
        Renderer::SetViewport(0, 0, pass.Size.value().x, pass.Size.value().y);
    
    // Clear the framebuffer with the specified color (if provided), or clear it with the active buffers
    bool clear = pass.SkipClear.has_value() ? !*pass.SkipClear : true;
    if (clear)
    {
        if (pass.Framebuffer && pass.Color.has_value())
            Renderer::Clear(pass.Color.value(), pass.Framebuffer->GetActiveBuffers());
        else if (pass.Framebuffer)
            Renderer::Clear(pass.Framebuffer->GetActiveBuffers());
        else if (pass.Color.has_value())
            Renderer::Clear(pass.Color.value());
        else
            Renderer::Clear();
    }
    
    // Render each model with its associated material
    for (auto& pair : pass.Models)
    {
        // Check if the model is the light sources and render it separately
        if (pair.first == "Light")
        {
            DrawLight(); // Render the light source
            continue;   // Move to the next iteration of the loop
        }
            
        // Retrieve the model associated with the current pair
        auto& model = m_Models.Get(pair.first);
        
        // Check if the model is valid
        if (model)
        {
            // If a material is specified for the model, set it
            if (!pair.second.empty())
            {
                auto& material = Renderer::GetMaterialLibrary().Get(pair.second);
                DefineShadowProperties(material);
                model->SetMaterial(material);
            }
            
            // Draw the model
            model->DrawModel();
        }
    }
        
    // End the scene
    Renderer::EndScene();
    
    // Unbind the framebuffer if it was provided
    if (pass.Framebuffer)
        pass.Framebuffer->Unbind();
    
    // Run the post-rendering code
    if (pass.PostRenderCode)
        pass.PostRenderCode();
}

/**
 * Draws the scene lights.
 */
void Scene::DrawLight()
{
    for (auto& pair : m_Lights)
    {
        auto& light = pair.second;
        light->DrawLight();
    }
}

/**
 * Draws the scene according to the specified render passes.
 */
void Scene::Draw()
{
    for (auto& name : m_RenderPasses.m_Order)
    {
        auto& pass = m_RenderPasses.Get(name);
        if (pass.Active)
            Draw(pass);
        else
        {
            if (pass.Framebuffer)
                pass.Framebuffer->Bind();
            Renderer::Clear(glm::vec4(0.0f));
        }
    }
}

/**
 * Define shadow properties for a given material.
 *
 * @param baseMaterial The base material to define shadow properties for.
 * @note If the "Shadow" render pass does not exist in the scene, no shadows maps will be defined.
 */
void Scene::DefineShadowProperties(const std::shared_ptr<Material>& baseMaterial)
{
    // Attempt to cast the base material to a LightedMaterial
    auto material = std::dynamic_pointer_cast<LightedMaterial>(baseMaterial);
    if (!material)
        return; // Base material is not a LightedMaterial, so return early

    // Define the properties of the material related to the light sources
    material->DefineLightProperties(m_Lights);
}
