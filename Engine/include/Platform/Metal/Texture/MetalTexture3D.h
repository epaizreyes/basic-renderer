#pragma once

#include "Common/Renderer/Texture/Texture3D.h"
#include "Platform/Metal/Texture/MetalTexture.h"

/**
 * Concrete implementation of a three-dimensional texture for OpenGL.
 *
 * The `MetalGLTexture3D` class specializes the `Texture3D` class to provide a concrete
 * implementation using the Metal API. It handles the creation, binding, unbinding, and resource
 * management of one-dimensional texture data within a Metal context.
 *
 * @note Copying and moving `MetalTexture3D` objects is disabled o prevent unintended
 * resource ownership issues.
 */
class MetalTexture3D : public Texture3D, public MetalTexture
{
public:
    // Constructor(s)/Destructor
    // ----------------------------------------
    MetalTexture3D();
    MetalTexture3D(const TextureSpecification& spec);
    
    MetalTexture3D(const void *data);
    MetalTexture3D(const void *data, const TextureSpecification& spec);
    
    DEFINE_MTL_TEXTURE_DESTRUCTOR(Texture3D)
    
    // Usage
    // ----------------------------------------
    DEFINE_MTL_TEXTURE_BINDING_METHODS()
    
protected:
    DEFINE_MTL_TEXTURE_RESOURCE_METHODS()
    
    // Disable the copying or moving of this resource
    // ----------------------------------------
public:
    DISABLE_COPY_AND_MOVE(MetalTexture3D);
};
