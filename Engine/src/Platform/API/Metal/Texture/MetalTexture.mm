#include "enginepch.h"
#include "Platform/Metal/Texture/MetalTexture.h"

#include "Platform/Metal/Texture/MetalTextureUtils.h"

#include <Metal/Metal.h>

/**
 * Internal representation of a Metal texture.
 */
struct MetalTexture::MetalTextureData
{
    ///< The actual Metal texture object.
    id<MTLTexture> Texture;
    ///< The sampler state associated with the texture.
    id<MTLSamplerState> Sampler;
};

/**
 * Creates a Metal texture.
 */
MetalTexture::MetalTexture()
{
    // Get the Metal graphics context and save it
    MetalContext& context = dynamic_cast<MetalContext&>(GraphicsContext::Get());
    CORE_ASSERT(&context, "Graphics context is not Metal!");
    m_Context = &context;
    
    // Initialize the texture data
    m_TextureData = std::make_shared<MetalTextureData>();
}

/**
 * Gets the Metal texture.
 * 
 * @return A pointer to the texture as a void pointer, or nullptr if the texture has not been defined.
 */
void* MetalTexture::MTLGetTexture() const
{
    return reinterpret_cast<void*>(m_TextureData->Texture);
}

/**
 * Gets the Metal texture sampler.
 *
 * @return A pointer to the texture sampler as a void pointer, or nullptr if the texture has not been defined.
 */
void* MetalTexture::MTLGetSampler() const
{
    return reinterpret_cast<void*>(m_TextureData->Sampler);
}

/**
 * Creates and configures a Metal texture.
 *
 * @param data The texture data.
 * @param spec The texture specifications.
 * @param samples The number of samples to use for the texture.
 */
void MetalTexture::MTLCreateTexture(const void *data, 
                                    const TextureSpecification& spec,
                                    unsigned int samples, unsigned int slice)
{
    // Create the internal Metal texture object if it doesn't exist
    if (!m_TextureData->Texture)
    {
        MTLDefineTexture(spec, samples);
        MTLDefineSampler(spec);
    }
    
    // If data not defined, only define the texture and return
    if (!data)
        return;

    // Get a reference to the internal Metal texture
    id<MTLTexture> texture = reinterpret_cast<id<MTLTexture>>(m_TextureData->Texture);

    // Define the region of the texture to be updated
    MTLRegion region = utils::textures::mtl::GetMetalRegion(spec);

    // Calculate the number of channels, size per channel, and stride
    const size_t channels = utils::textures::mtl::GetMetalChannelCount(spec.Format);
    const size_t bytes = utils::textures::GetBytesPerChannel(spec.Format);
    const size_t stride = spec.Width * channels * bytes;
    const size_t imageSize = spec.Width * spec.Height * channels * bytes;

    // Pointer to hold the data that will be uploaded to the GPU
    const void* uploadData = data;

    // Perform RGB to RGBA conversion if the format requires it
    if (utils::textures::IsRGBFormat(spec.Format))
    {
        // Allocate memory for the converted RGBA data
        size_t size = bytes * channels * spec.Width * spec.Height;
        void* rgba = malloc(size);

        // Convert the RGB data to RGBA
        ConvertRGBToRGBA(data, rgba, spec);
        // Update the upload data pointer to the converted data
        uploadData = rgba;
    }

    // Upload the texture data to the GPU
    [texture replaceRegion:region
             mipmapLevel:0
             slice:slice
             withBytes:uploadData
             bytesPerRow:stride
             bytesPerImage:imageSize];
    
    // Free the allocated memory if conversion took place
    if (uploadData != data)
        free(const_cast<void*>(uploadData));
    
    // Generate mipmaps if specified in the TextureSpecification
    if (spec.MipMaps)
        MTLGenerateMipMaps();
}

/**
 * Generate the Metal texture based on the input specifications.
 *
 * @param spec The texture specifications.
 * @param samples The number of samples to use for the texture.
 */
void MetalTexture::MTLDefineTexture(const TextureSpecification& spec,
                                    unsigned int samples)
{
    // Get the Metal device from the context
    id<MTLDevice> device = reinterpret_cast<id<MTLDevice>>(m_Context->GetDevice());
    
    // Create a new texture descriptor
    MTLTextureDescriptor *descriptor = [[MTLTextureDescriptor alloc] init];
    
    // Set texture properties based on the provided specification
    descriptor.textureType = utils::textures::mtl::ToMetalTextureType(spec.Type);
    descriptor.pixelFormat = utils::textures::mtl::ToMetalPixelFormat(spec.Format);
    descriptor.width = spec.Width;
    
    // Set optional texture dimensions if provided
    if (spec.Height > 0)
        descriptor.height = spec.Height;
    if (spec.Depth > 0)
        descriptor.depth = spec.Depth;
    
    // Set optional multi-sampling size
    if (samples > 1)
        descriptor.sampleCount = samples;
    
    // Calculate and set the number of mipmap levels
    descriptor.mipmapLevelCount = spec.MipMaps
        ? floor(log2f((float)MAX(MAX(spec.Width, spec.Height), spec.Depth))) + 1
        : 1;
    
    // Set the texture's storage and usage modes
    descriptor.storageMode = MTLStorageModeShared;
    descriptor.usage = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite |
                       MTLTextureUsageRenderTarget;
    
    // Create the texture
    m_TextureData->Texture = [device newTextureWithDescriptor:descriptor];
    CORE_ASSERT(m_TextureData->Texture, "Error creating texture!");
    
    // Release the memory of the descriptor
    [descriptor release];
}

/**
 * @brief Defines a Metal sampler state object based on the TextureSpecification.
 *
 * @param spec The texture specifications.
 */
void MetalTexture::MTLDefineSampler(const TextureSpecification &spec)
{
    // Get the Metal device from the context
    id<MTLDevice> device = reinterpret_cast<id<MTLDevice>>(m_Context->GetDevice());
    
    // Create a new texture descriptor
    MTLSamplerDescriptor *descriptor = [[MTLSamplerDescriptor alloc] init];
    
    // Define the filter apply when sampling
    descriptor.minFilter = utils::textures::mtl::ToMetalFilter(spec.Filter);
    descriptor.magFilter = utils::textures::mtl::ToMetalFilter(spec.Filter);
    
    // Define the wrapping mode
    descriptor.sAddressMode = utils::textures::mtl::ToMetalWrap(spec.Wrap);
    
    if (spec.Height > 0)
        descriptor.tAddressMode = utils::textures::mtl::ToMetalWrap(spec.Wrap);
    if (spec.Depth > 0)
        descriptor.rAddressMode = utils::textures::mtl::ToMetalWrap(spec.Wrap);
    
    // Create the sampler
    m_TextureData->Sampler = [device newSamplerStateWithDescriptor:descriptor];
    CORE_ASSERT(m_TextureData->Sampler, "Error creating texture sampler!");
    
    // Release the memory of the descriptor
    [descriptor release];
}

/**
 * Generates the mip-maps of the internal Metal texture.
 */
void MetalTexture::MTLGenerateMipMaps()
{
    // Get the command queue dedicated for the resources
    id<MTLCommandQueue> queue = reinterpret_cast<id<MTLCommandQueue>>(m_Context->GetResourceQueue());
    
    // Create the command buffer
    id<MTLCommandBuffer> commandBuffer = [queue commandBuffer];
    // Create a blit command encoder to handle mipmap generation
    id<MTLBlitCommandEncoder> blitEncoder = [commandBuffer blitCommandEncoder];
    
    // Get the internal Metal texture object
    id<MTLTexture> texture = reinterpret_cast<id<MTLTexture>>(m_TextureData->Texture);

    // Synchronize the texture data before generating mipmaps
    [blitEncoder synchronizeResource:texture];
    // Generate mipmaps
    [blitEncoder generateMipmapsForTexture:texture];

    // End encoding and submit the command buffer for execution
    [blitEncoder endEncoding];
    [commandBuffer commit];
    [commandBuffer waitUntilCompleted];
}

/**
 * Converts raw RGB texture data to RGBA format.
 *
 * @param rgb A pointer to the source RGB texture data.
 * @param rgba A pointer to the destination memory where the RGBA data will be written.
 * @param spec The specification of the texture.
 */
void MetalTexture::ConvertRGBToRGBA(const void *rgb, void *rgba,
                                    const TextureSpecification &spec)
{
    // Verify the type of data
    if (!utils::textures::IsRGBFormat(spec.Format))
    {
        CORE_WARN("Data conversion not possible!");
        return;
    }
        
    // Get the number of components and data type size
    const size_t srcChannels = 3;
    const size_t dstChannels = 4;
    const size_t size = utils::textures::GetBytesPerChannel(spec.Format);

    const size_t srcStride = srcChannels * size;
    const size_t dstStride = dstChannels * size;
    
    // (using char* for byte-wise access)
    const char* src = reinterpret_cast<const char*>(rgb);
    char* dst = reinterpret_cast<char*>(rgba);
    
    // Convert RGB to RGBA
    for (size_t i = 0; i < spec.Width * spec.Height; ++i) 
    {
        // Copy RGB data (byte-by-byte)
        for (size_t c = 0; c < srcStride; ++c) {
            dst[i * dstStride + c] = src[i * srcStride + c];
        }

        // Set alpha to 1.0f based on data type
        size_t alphaOffset = i * dstStride + (dstChannels - 1) * size;
        if (size == 1)
            dst[alphaOffset] = 255;
        else if (size == 2)
            reinterpret_cast<uint16_t*>(dst + alphaOffset)[0] = 0x3C00;
        else if (size == 4)
            reinterpret_cast<float*>(dst + alphaOffset)[0] = 1.0f;
        else if (size == 8)
            reinterpret_cast<double*>(dst + alphaOffset)[0] = 1.0;
        else
        {
            CORE_WARN("Unsupported type during data conversion!");
            return;
        }
    }
}
