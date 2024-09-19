#pragma once

#include "Common/Renderer/Texture/Texture.h"
#include "Common/Renderer/Texture/TextureUtils.h"

#include "Platform/Metal/MetalContext.h"

/**
 * Represents a texture in Metal.
 *
 * This class encapsulates the creation, configuration, and anagement of texture data specifically
 * for Metal, Apple's graphics API. It provides methods to define texture parameters, upload data
 * to the GPU, generate mipmaps, and access the underlying Metal texture object.
 */
class MetalTexture
{
public:
    // Constructor(s)/Destructor
    // ----------------------------------------
    /// @brief Delete the metal texture.
    virtual ~MetalTexture() = default;
    
    // Getter(s)
    // ----------------------------------------
    void* MTLGetTexture() const;
    void* MTLGetSampler() const;
    
protected:
    // Constructor(s)/Destructor
    // ----------------------------------------
    MetalTexture();
    
    // Texture Generator
    // ----------------------------------------
    void MTLCreateTexture(const void *data, const TextureSpecification& spec,
                          unsigned int samples = 1, unsigned int slice = 0);
    
    void MTLDefineTexture(const TextureSpecification& spec,
                          unsigned int samples = 1);
    void MTLDefineSampler(const TextureSpecification& spec);
    
    void MTLGenerateMipMaps();
    
private:
    // Conversion(s)
    // ----------------------------------------
    void static ConvertRGBToRGBA(const void* rgb, void* rgba,
                                 const TextureSpecification& spec);
    
    // Texture variables
    // ----------------------------------------
private:
    ///< Internal Metal texture.
    struct MetalTextureData;
    std::shared_ptr<MetalTextureData> m_TextureData;
    
    ///< Metal context.
    MetalContext* m_Context;
};

// Destructor
// ----------------------------------------
#define DEFINE_MTL_TEXTURE_DESTRUCTOR(TextureType)\
    /** @brief Destructor to release the OpenGL texture resource. */\
    ~Metal##TextureType() override { ReleaseTexture(); }

// Usage
// ----------------------------------------
#define DEFINE_MTL_TEXTURE_BINDING_METHODS()\
    /** @brief Binds the texture to the active texture unit. */\
    void Bind() const override {}\
    /** @brief Binds the texture to a specific texture unit. */\
    /** @param slot The index of the texture unit to bind to. */\
    void BindToTextureUnit(uint32_t slot) const override {}\
    /** @brief Unbinds the texture from the active texture unit. */\
    void Unbind() const override {}

// Creation & Release
// ----------------------------------------
#define DEFINE_MTL_TEXTURE_RESOURCE_METHODS()\
    /** @brief Releases the underlying OpenGL texture resource. */\
    void ReleaseTexture() override {}\
    /** @brief Creates the OpenGL texture. */\
    void CreateTexture(const void* data) override;
