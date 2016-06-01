#version 330 core
out vec4 color;

in vec3 Normal;
in vec3 FragPos;

uniform int point;
uniform vec3 viewPos;
uniform int light;

void main()
{
    vec3 objectColor;
    if (point == 0) {
        objectColor = vec3(1.0f, 0.0f, 0.0f);
    } else {
        objectColor = vec3(0.0f, 1.0f, 1.0f);
    }
    if (light == 1) {

        vec3 result = vec3(1.0f);
        float ambientStrength = 0.15f; // Ambient Strength

        vec3 lightPos = vec3(0.0f, 1.0f, -2.0f); // Light Position
        vec3 lightColor = vec3(1.0f, 1.0f, 1.0f); // Light Color

        float shininess = 200.0f; // Shininess
        float specularStrength = 0.01f; // Specular Strength

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
        color = vec4(result, 1.0f);
    } else {
        color = vec4(objectColor, 1.0f);
    }
}
