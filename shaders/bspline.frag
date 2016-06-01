#version 330 core
out vec4 color;

uniform int bezier;

void main()
{
    if (bezier == 0) {
        color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    } else {
        color = vec4(0.0f, 1.0f, 1.0f, 1.0f);
    }
}