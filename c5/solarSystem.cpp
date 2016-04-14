#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <gli/gli.hpp>

#include <fstream>

#include "utils/shader.h"
#include "utils/tgaLoader.h"

struct Vertex {
    GLfloat positions[3];
};

struct Index {
    GLuint indices[3];
};

class SolarSystemApp {

private:
    const GLdouble PI = 3.1415926535;
    const GLuint WIDTH = 800, HEIGHT = 600;

    const GLchar *VERTEX_SHADER_FILE = "shaders/solarSystem.vert";
    const GLchar *FRAGMENT_SHADER_FILE = "shaders/solarSystem.frag";

    GLFWwindow *window;
    GLuint shaderProgram;
    GLuint VBO_Sun, VAO_Sun, EBO_Sun;
    std::vector<Vertex> verticesSun;
    std::vector<Index> indicesSun;
    GLuint texSun;

    static void error_callback(int error, const char *description) {
        std::cerr << description << std::endl;
    }

    Vertex makeVertex(GLfloat x, GLfloat y, GLfloat z) {
        Vertex v;
        v.positions[0] = x;
        v.positions[1] = y;
        v.positions[2] = z;
        return v;
    }

    Index makeIndex(GLuint x, GLuint y, GLuint z) {
        Index v;
        v.indices[0] = x;
        v.indices[1] = y;
        v.indices[2] = z;
        return v;
    }

    void init() {

        glfwSetErrorCallback(error_callback);
        glfwInit();

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
        glfwWindowHint(GLFW_SAMPLES, 4);

        window = glfwCreateWindow(WIDTH, HEIGHT, "XHX OpenGL 4 - Solar System", nullptr, nullptr);
        glfwMakeContextCurrent(window);

        glewExperimental = GL_TRUE;
        glewInit();

        glViewport(0, 0, WIDTH, HEIGHT);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_MULTISAMPLE);

        shaderProgram = loadShaders(VERTEX_SHADER_FILE, FRAGMENT_SHADER_FILE);
    }

    void createSphere(GLfloat radius, GLfloat resolution) {
        resolution = (GLfloat) (PI / resolution);
        GLdouble offset = 0;
        GLuint i = 1;
        int n = 0;
        verticesSun.push_back(makeVertex(0, radius, 0));
        verticesSun.push_back(makeVertex(0, 0, 0));
        for (GLdouble fi = resolution; fi <= PI; fi += resolution) {
            int tn = 0;
            for (GLdouble theta = 0; theta <= 2 * PI; theta += resolution) {
                ++tn;
                GLfloat delta = resolution / 2;

                GLfloat x = (GLfloat) (radius * cos(theta + offset) * sin(fi));
                GLfloat z = (GLfloat) (radius * sin(theta + offset) * sin(fi));
                GLfloat y = (GLfloat) (radius * cos(fi));

                verticesSun.push_back(makeVertex(x, y, z));

                GLfloat U = (GLfloat) (theta / 2 / PI);
                GLfloat V = (GLfloat) (cos(fi) + 1) / 2;

                //std::cout << x << " " << y << " " << z << std::endl;
                std::cout << U << " " << V << std::endl;

                verticesSun.push_back(makeVertex(U, V, 0));

                if (theta != 0) {
                    if (fi == resolution) {
                        indicesSun.push_back(makeIndex(i - 1, i, 0));
                    } else {
                        indicesSun.push_back(makeIndex(i - 1, i, i - n - 1));
                        indicesSun.push_back(makeIndex(i, i - n - 1, i - n));
                    }
                }
                ++i;
            }
            if (fi == resolution) {
                indicesSun.push_back(makeIndex(i - tn, i - 1, 0));
            } else {
                indicesSun.push_back(makeIndex(i - tn, i - 1, i - n - 1));
                indicesSun.push_back(makeIndex(i - tn, i - n - tn, i - n - 1));
            }
            n = tn;
            offset += resolution / 2;
        }
        for (GLuint j = i - 1; j > i - n; --j) {
            indicesSun.push_back(makeIndex(j, j - 1, i));
        }
        indicesSun.push_back(makeIndex(i - 1, i - n, i));
        verticesSun.push_back(makeVertex(0, 0, -radius));
        verticesSun.push_back(makeVertex(0, 0, -radius));

        glGenVertexArrays(1, &VAO_Sun);
        glGenBuffers(1, &VBO_Sun);

        glBindVertexArray(VAO_Sun);

        glBindBuffer(GL_ARRAY_BUFFER, VBO_Sun);
        glBufferData(GL_ARRAY_BUFFER, verticesSun.size() * sizeof(Vertex), verticesSun.data(), GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex) * 2, (GLvoid *) 0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex) * 2, (GLvoid *) 0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex) * 2, (GLvoid *) (sizeof(Vertex)));
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &EBO_Sun);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_Sun);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSun.size() * sizeof(Index), indicesSun.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glBindVertexArray(0);

    }

    GLuint createTexture(char const *filePath) {

        return loadtga(filePath);
    }


    void display() {

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);


        // Matrix
        glm::mat4 view = glm::lookAt(
                glm::vec3(0, 0, -5.0),
                glm::vec3(0, 0, 0),
                glm::vec3(0, 1, 0)
        );
        glm::mat4 projection = glm::perspective(45.0f, (GLfloat) WIDTH / (GLfloat) HEIGHT, 0.1f, 100.0f);
        GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texSun);
        glUniform1i(glGetUniformLocation(shaderProgram, "solarTexture"), 0);

        // Vertex Array
        glBindVertexArray(VAO_Sun);

        // Model Matrix
        glm::mat4 model(1.0f);
        model = glm::rotate(model, (GLfloat) glfwGetTime(), glm::vec3(0, 1, 0));
        model = glm::translate(model, glm::vec3(2.0f, 0, 0));
        model = glm::rotate(model, (GLfloat) glfwGetTime(), glm::vec3(1, 1, 1));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        // Array Buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_Sun);

        // Draw

        //
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        //

        glDrawElements(GL_TRIANGLES, (GLsizei) (indicesSun.size() * 3), GL_UNSIGNED_INT, 0);

        // Reset
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

    }

public:
    SolarSystemApp() : window(NULL) {
    }

    void run() {

        init();
        glPointSize(2);

        createSphere(.8, 50);
        texSun = createTexture("resources/Earth.tga");

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            display();

            glfwSwapBuffers(window);
        }

    }

    ~SolarSystemApp() {

        glDeleteVertexArrays(1, &VAO_Sun);
        glDeleteBuffers(1, &VBO_Sun);
        glDeleteBuffers(1, &EBO_Sun);
        glfwTerminate();

    }
};


int main() {
    SolarSystemApp app;

    app.run();

    return 0;
}
