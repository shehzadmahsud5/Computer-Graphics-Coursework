#version 330 core
layout (location = 0) in vec3 aPos;    // Vertex positions
layout (location = 1) in vec2 aTexCoords; // Texture coordinates (UVs)
layout (location = 2) in vec3 aNormal;   // Vertex normals

// Uniforms
uniform mat4 model;      // Model matrix (object's world transformation)
uniform mat4 view;       // View matrix (camera)
uniform mat4 projection; // Projection matrix

// Outputs to Fragment Shader (we'll use these later)
out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos; // Vertex position in world space

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    
    // Pass data to fragment shader
    FragPos = vec3(model * vec4(aPos, 1.0)); // Vertex position in world space
    Normal = mat3(transpose(inverse(model))) * aNormal; // Transform normals correctly
    TexCoords = aTexCoords;
}