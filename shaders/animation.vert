#version 330 core
layout (location = 0) in vec3 position; // The position variable has attribute position 0
layout (location = 1) in vec3 color;	// The color variable has attribute position 1
uniform mat4 transform;

out vec3 ourColor; // Output a color to the fragment shader

void main()
{
    ourColor = color; // Set ourColor to the input color we got from the vertex data
    gl_Position = transform * vec4(position, 1.0);
}