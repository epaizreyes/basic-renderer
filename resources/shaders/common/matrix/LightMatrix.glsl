/**
 * Represents transformation matrices for rendering.
 */
struct Transform {
    mat4 Model;         ///< Model matrix for transforming object vertices to world space.
    mat4 View;          ///< View matrix for transforming world space to camera space.
    mat4 Projection;    ///< Projection matrix for transforming camera space to clip space.
    
    mat3 Normal;        ///< Normal matrix for transforming normals to world space.
    mat4 Light;         ///< Light matrix for transforming vertices to light space.
};
