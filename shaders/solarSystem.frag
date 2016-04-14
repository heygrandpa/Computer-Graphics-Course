#version 330 core
out vec4 color;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

uniform sampler2D solarTexture;

void main()
{
    vec3 objectColor = vec3(1.0f, 1.0f, 1.0f); // Object Color

    float ambientStrength = 0.15f; // Ambient Strength

    vec3 lightPos = vec3(0.0f, 0.0f, 0.0f); // Light Position
    vec3 lightColor = vec3(1.0f, 1.0f, 1.0f); // Light Color

    vec3 viewPos = vec3(0.0f, 0.0f, -2.0f);
    float shininess = 20.0f; // Shininess
    float specularStrength = 0.5f; // Ambient Strength

    // Ambient
    vec3 ambient = ambientStrength * vec3(1.0f);

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0f);
    vec3 diffuse = diff * lightColor;

    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = min((ambient + diffuse + specular) * objectColor, vec3(1.0f));
    color = texture(solarTexture, TexCoord) * vec4(result, 1.0f);
}