#include <iostream>

#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "utils/shader.h"

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

const GLuint WIDTH = 800, HEIGHT = 600;

const GLchar* vertexShaderFile = "shaders/animation.vert";
const GLchar* fragmentShaderFile = "shaders/animation.frag";

void error_callback(int error, const char* description)
{
    std::cerr << description << std::endl;
}

int main()
{
    glfwSetErrorCallback(error_callback);
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "XHX OpenGL 3", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);

    glewExperimental = GL_TRUE;
    glewInit();

    glViewport(0, 0, WIDTH, HEIGHT);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_MULTISAMPLE);

    GLuint shaderProgram = loadShaders(vertexShaderFile, fragmentShaderFile);

    static const GLfloat vertices[] = {
            -0.5f,  -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
            -0.5f, 0.5f, -0.5f, 0.0f, 0.5f, 0.0f,
            0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 0.5f,
            0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.0f,
            -0.5f,  -0.5f, 0.5f, 0.0f, 0.5f, 0.5f,
            -0.5f, 0.5f, 0.5f, 0.5f, 0.0f, 0.5f,
            0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
            0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 1.0f,
    };

    static const GLuint indices[] = {
            0, 1, 2,
            0, 2, 3,
            1, 2, 5,
            2, 5, 6,
            5, 6, 7,
            4, 5, 7,
            0, 4, 7,
            0, 3, 7,
            2, 3, 7,
            2, 6, 7,
            0, 1, 5,
            0, 4, 5,
    };

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glUseProgram(shaderProgram);

    GLint cubeMVPid = glGetUniformLocation(shaderProgram, "transform");

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (GLfloat) WIDTH / HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(
        glm::vec3(0, 0, 5),
        glm::vec3(0, 0, 0),
        glm::vec3(0, 1, 0)
    );

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Render
        static int r = 0;
        static int g = 128;
        static int b = 255;
        static int dr = 1;
        static int dg = 1;
        static int db = 1;
        glClearColor(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
        r += dr;
        g += dg;
        b += db;
        if (r > 255 || r < 0) dr = -dr;
        if (g > 255 || g < 0) dg = -dg;
        if (b > 255 || b < 0) db = -db;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 cubeMat = glm::mat4(1.0f);
        cubeMat = glm::translate(cubeMat, glm::vec3(0.5f * cos(glfwGetTime()), 0.5f * sin(glfwGetTime()), 0.5f * cos(glfwGetTime()) * sin(glfwGetTime())));
        cubeMat = glm::rotate(cubeMat, (GLfloat)glfwGetTime(), glm::vec3(1.0f, 1.0f, 1.0f));
        cubeMat = glm::scale(cubeMat, glm::vec3(1.0f + .5f * cos(glfwGetTime() * 3), 1.0f + .5f * cos(glfwGetTime() * 3), 1.0f + .5f * cos(glfwGetTime() * 3)));
        glm::mat4 cubeMVP = projection * view * cubeMat;
        glUniformMatrix4fv(cubeMVPid, 1, GL_FALSE, glm::value_ptr(cubeMVP));

        glDrawElements(GL_TRIANGLES, 12 * 3, GL_UNSIGNED_INT, nullptr);

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();
    return 0;
}

