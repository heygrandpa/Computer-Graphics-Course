#version 330 core
out vec4 color;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

uniform sampler2D solarTexture;
uniform int light;
uniform vec3 viewPos;

void main()
{
    vec3 result = vec3(1.0f);
    if (light == 1) {
        vec3 objectColor = vec3(1.0f, 1.0f, 1.0f); // Object Color

        float ambientStrength = 0.15f; // Ambient Strength

        vec3 lightPos = vec3(0.0f, 0.0f, 0.0f); // Light Position
        vec3 lightColor = vec3(1.0f, 1.0f, 1.0f); // Light Color

        float shininess = 10.0f; // Shininess
        float specularStrength = 0.5f; // Specular Strength

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
        float spec = pow(clamp(dot(viewDir, reflectDir), 0, 1), shininess);
        vec3 specular = specularStrength * spec * lightColor;

        result = min((ambient + diffuse + specular) * objectColor, vec3(1.0f));
    }
    color = texture(solarTexture, TexCoord) * vec4(result, 1.0f);
}