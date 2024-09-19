#pragma once

#include "Common/Renderer/GraphicsContext.h"

struct GLFWwindow;

/**
 *  Manages a Metal graphics context.
 *
 *  The `MetalContext` class is responsible for initializing, managing, and interacting
 *  with a Metal rendering context. It uses GLFW to interface with the windowing system
 *   and interacts with Metal for rendering on Apple platforms.
 */
class MetalContext : public GraphicsContext
{
public:
    struct MetalState;
    
public:
    // Constructor(s)
    // ----------------------------------------
    MetalContext(GLFWwindow* windowHandle);

    // Initialization
    // ----------------------------------------
    virtual void Init() override;
    
    // Getter(s)
    // ----------------------------------------
    void* GetDevice() const;
    void* GetEncoder() const;
    
    void* GetResourceQueue() const;
    
    // Setter(s)
    // ----------------------------------------
    static void SetWindowHints();
    void SetVerticalSync(bool enabled) override;
    
    // Draw
    // ----------------------------------------
    void Clear(const glm::vec4& color = glm::vec4(0.0f));
    void SwapBuffers() override;
    
    // Graphics context variables
    // ----------------------------------------
private:
    ///< Native window (GLFW).
    GLFWwindow* m_WindowHandle;
    
    ///< Holds the core Metal objects required for rendering.
    std::shared_ptr<MetalState> m_State;
};
