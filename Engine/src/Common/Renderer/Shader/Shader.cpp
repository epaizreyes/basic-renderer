#include "enginepch.h"
#include "Common/Renderer/Shader/Shader.h"

#include "Common/Renderer/Renderer.h"

#include "Platform/OpenGL/Shader/OpenGLShader.h"
#include "Platform/Metal/Shader/MetalShader.h"

// ----------------------------------------
// Shader
// ----------------------------------------

/**
 * Create a shader based on the active rendering API.
 *
 * @param name The name to assign to the shader.
 * @param filePath The path to the shader source file.
 *
 * @return A shared pointer to the created shader, or nullptr if the API
 *         is not supported or an error occurs.
 */
std::shared_ptr<Shader> Shader::Create(const std::string &name,
                                       const std::filesystem::path &filePath)
{
    CREATE_RENDERER_OBJECT(Shader, name, GetFullFilePath(filePath))
}


/**
 * Create a shader based on the active rendering API.
 *
 * @param filePath The path to the shader source file.
 *
 * @return A shared pointer to the created Shader object, or nullptr if the API
 *         is not supported or an error occurs.
 */
std::shared_ptr<Shader> Shader::Create(const std::filesystem::path &filePath)
{
    CREATE_RENDERER_OBJECT(Shader, GetFullFilePath(filePath))
}

/**
 * @brief Reads the contents of a file and returns it as a string.
 *
 * @param filePath The path to the file to be read.
 *
 * @return The contents of the file as a string.
 */
std::string Shader::ReadFile(const std::filesystem::path& filePath)
{
    std::ifstream fileStream(filePath);
    CORE_ASSERT(fileStream.is_open(), "Failed to open file: " + filePath.string());

    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    return buffer.str();
}

/**
 * Verify if the uniform is defined in the shader program.
 *
 * @param name Name of the uniform.
 *
 * @return 'True' if the uniform is defined in the shader.
 */
bool Shader::IsUniform(const std::string& name) const
{
    auto [group, member] = utils::SplitString(name);
    if (m_Uniforms.Exists(group, member))
        return true;
        
    CORE_WARN("Uniform " + name + " doesn't exist!");
    return false;
}

/**
 * Constructs the full file path for a shader, including the correct extension.
 *
 * This function takes a base file path for a shader (e.g., "shaders/MyShader")
 * and automatically appends the appropriate file extension based on the currently
 * active rendering API (".glsl" for OpenGL, ".Metal" for Metal).
 *
 * @param filePath The base file path for the shader.
 *
 * @return The complete file path with the appropriate shader file extension.
 */
std::filesystem::path Shader::GetFullFilePath(const std::filesystem::path& filePath)
{
    // Define a copy of the filepath to be modify
    std::filesystem::path fullFilePath = filePath;
    
    // Get the extension of the shader based on the graphics API that is currently in use
    std::string extension;
    switch (Renderer::GetAPI()) {
        case RendererAPI::API::OpenGL:
            extension = ".glsl";
            break;
            
#ifdef __APPLE__
        case RendererAPI::API::Metal:
            extension = ".Metal";
            break;
#endif
            
        default:
            CORE_ASSERT(false, "Unknown Renderer API!");
            return filePath;
    }

    // Update the extension of the file path if necessary
    if (filePath.extension() != extension)
    {
        CORE_ASSERT(filePath.extension().empty(), "Shader extension not supported for the current graphics API");
        fullFilePath += extension;
    }

    return fullFilePath;
}

// ----------------------------------------
// Shader Library
// ----------------------------------------

/**
 * Loads a shader from a file and adds it to the library.
 *
 * @param filePath The path to the file containing the shader.
 *
 * @return The loaded shader.
 */
std::shared_ptr<Shader> ShaderLibrary::Load(const std::filesystem::path& filePath)
{
    return ShaderLibrary::Load(filePath.stem().string(), filePath);
}

/**
 * Loads a shader from a file with a specified name and adds it to the library.
 *
 * @param name The name to associate with the loaded shader.
 * @param filePath The path to the file containing the shader.
 *
 * @return The loaded shader.
 */
std::shared_ptr<Shader> ShaderLibrary::Load(const std::string& name,
                                            const std::filesystem::path& filePath)
{
    auto shader = Shader::Create(name, filePath);
    Add(name, shader);
    return shader;
}
