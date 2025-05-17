#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;    // Normal in world space
in vec3 FragPos;   // Fragment position in world space

uniform sampler2D diffuseMap;

// Lighting uniforms (will be set from C++)
uniform vec3 lightPos;    // Light position in world space
uniform vec3 lightColor;
uniform vec3 viewPos;     // Camera/View position in world space

// Material properties (can be passed from Model class later, or hardcoded for now)
// uniform vec3 objectColor; // If not using texture, or for mixing

void main()
{
    // For now, just texture color
    vec4 texColor = texture(diffuseMap, TexCoords);
    
    // Ambient lighting
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
  
    // Diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Combine results (ambient + diffuse for now)
    // The texColor.rgb acts as the object's base color (albedo)
    vec3 result = (ambient + diffuse) * texColor.rgb;
    FragColor = vec4(result, texColor.a);

}