//
// Created by xuhongxu on 16/3/13.
//

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

const int WIDTH = 800; // Screen Width
const int HEIGHT = 600; // Screen Height
const int EDGE_N = 3; // Polygon's Edge count
const GLfloat EDGE_LENGTH = .5f; // Length of edges
const GLfloat PI = 3.1415926f; // Pi

GLfloat vertices[EDGE_N][2]; // Position of verticesSun
bool isFill = true; // Fill or Stroke mode
GLfloat scaleSpeed = 1.0f;
GLfloat rotateSpeed = 0.0f;
GLfloat rotateCenter[3] = {0.0f, 0.0f, 1.0f};
GLfloat angle = 0.0f;

void error_callback(int error, const char* description)
{
    std::cerr << description << std::endl;
}

void outputTip() {
    std::cout << "Enter: Switch stroke/fill mode" << std::endl;
    std::cout << "Left/Right: Rotate" << std::endl;
    std::cout << "+/-: Scale" << std::endl;
    std::cout << "Move cursor: Move the rotation center point" << std::endl;
}

// Calculate verticesSun' position of the polygon
void initPolygon() {
    for (int i = 0; i < EDGE_N; ++i) {
        vertices[i][0] = EDGE_LENGTH * (GLfloat)sin(2 * PI / EDGE_N * i);
        vertices[i][1] = EDGE_LENGTH * (GLfloat)cos(2 * PI / EDGE_N * i);
    }
}

// On mouse move
void cursor_pos_callback(GLFWwindow *window, double x, double y) {
    static double oldX = x, oldY = y; // record old position
    double dx = (x - oldX) / 1000.0; // calculate offset
    double dy = (oldY - y) / 1000.0;
    // Because of the rotation, we need to rotate against the polygon to get the correct offset vector
    glm::vec4 vec(dx, dy, 0.0f, 1.0f);
    glm::mat4 trans;
    trans = glm::rotate(trans, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
    vec = trans * vec;
    rotateCenter[0] += vec.x; // Move Rotate Center Point through the offset vector
    rotateCenter[1] += vec.y;
    oldX = x;
    oldY = y;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ENTER) { // Enter to change Fill/Stroke mode
            isFill = !isFill;
            glPolygonMode(GL_BACK, isFill ? GL_FILL : GL_LINE);
        } else if (key == GLFW_KEY_LEFT) { // Rotate anticlockwise
            rotateSpeed += 1.0f;
        } else if (key == GLFW_KEY_RIGHT) { // Rotate clockwise
            rotateSpeed -= 1.0f;
        } else if (key == GLFW_KEY_EQUAL) { // Zoom in
            scaleSpeed += .01f;
        } else if (key == GLFW_KEY_MINUS) { // Zoom out
            scaleSpeed -= .01f;
        }
    }
}

void display() {

    glClear(GL_COLOR_BUFFER_BIT); // Clear the screen
    glColor3f(1.0f, 0.3f, 0.3f); // Set current color for polygon


    glScalef(scaleSpeed, scaleSpeed, scaleSpeed); // Scale
    glTranslatef(rotateCenter[0], rotateCenter[1], 0.0f); // Translate rotate center
    angle -= rotateSpeed; // Record the current rotating angle
    glRotatef(rotateSpeed, 0.0f, 0.0f, 1.0f); // Rotate
    glTranslatef(-rotateCenter[0], -rotateCenter[1], 0.0f); // Translate back


    glBegin(GL_POLYGON); // Draw polygon
    {
        for (int i = 0; i < EDGE_N; ++i) {
            glVertex3f(vertices[i][0], vertices[i][1], 0.0f);
        }
    }
    glEnd();


    glColor3f(1.0f, 1.0f, 0.0f); // Set current color for Rotate Center Point

    glBegin(GL_POINTS); // Draw Rotate Center Point
    {
        glVertex2f(rotateCenter[0], rotateCenter[1]);
    }
    glEnd();
}

int main(int argc, char **argv) {
    glfwInit();

    glfwSetErrorCallback(error_callback);

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "XHX OpenGL 2", nullptr, nullptr);

    glViewport(0, 0, WIDTH, HEIGHT);

    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Event Callback
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    outputTip();
    initPolygon();

    // Rotate Center Point's size
    glPointSize(5);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        display();

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

