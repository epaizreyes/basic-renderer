#pragma once

/**
 * An index buffer used to store indices used to decide what vertices to be drawn.
 */
class IndexBuffer
{
public:
    /// Constructors/Destructor
    IndexBuffer(const unsigned int *indices, const unsigned int count);
    ~IndexBuffer();
    /// Usage
    void Bind() const;
    void Unbind() const;
    /// Gets
    unsigned int GetCount() const;
    
private:
    /// ID of the index buffer
    unsigned int m_ID;
    /// Number of indices (element count)
    unsigned int m_Count;
};
