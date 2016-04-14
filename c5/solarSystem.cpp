#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <gli/gli.hpp>

#include <fstream>
#include <map>

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
    GLuint VBO_Sphere, VAO_Sphere, EBO_Sphere;
    GLulong sphereIndexSize;

    glm::vec3 cameraPos   = glm::vec3(0.0f, 4.0f, -40.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, -0.2f,  1.0f);
    glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

    static std::map<GLFWwindow*, SolarSystemApp*> appMap;

    static void error_callback(int error, const char *description) {
        std::cerr << description << std::endl;
    }

    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        SolarSystemApp &app = *appMap[window];
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);
        GLfloat cameraSpeed = 1.0f;
        if(key == GLFW_KEY_W)
            app.cameraPos += cameraSpeed * app.cameraFront;
        if(key == GLFW_KEY_S)
            app.cameraPos -= cameraSpeed * app.cameraFront;
        if(key == GLFW_KEY_LEFT)
            app.cameraPos -= glm::normalize(glm::cross(app.cameraFront, app.cameraUp)) * cameraSpeed;
        if(key == GLFW_KEY_RIGHT)
            app.cameraPos += glm::normalize(glm::cross(app.cameraFront, app.cameraUp)) * cameraSpeed;
        if(key == GLFW_KEY_UP)
            app.cameraPos += glm::normalize(app.cameraUp) * cameraSpeed;
        if(key == GLFW_KEY_DOWN)
            app.cameraPos -= glm::normalize(app.cameraUp) * cameraSpeed;
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
        appMap[window] = this;
        glfwMakeContextCurrent(window);

        glewExperimental = GL_TRUE;
        glewInit();

        glViewport(0, 0, WIDTH, HEIGHT);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_MULTISAMPLE);

        shaderProgram = loadShaders(VERTEX_SHADER_FILE, FRAGMENT_SHADER_FILE);
    }

    GLulong createSphere(GLfloat radius, GLfloat resolution, GLuint &VAO, GLuint &VBO, GLuint &EBO) {
        resolution = (GLfloat) (PI / resolution);
        GLdouble offset = 0;
        GLuint i = 1;
        int n = 0;
        std::vector<Vertex> vertices;
        std::vector<Index> indices;
        vertices.push_back(makeVertex(0, radius, 0));
        vertices.push_back(makeVertex(0.5f, 1.0f, 0));
        for (GLdouble fi = resolution; fi <= PI; fi += resolution) {
            int tn = 0;
            for (GLdouble theta = 0; theta <= 2 * PI; theta += resolution) {
                ++tn;
                GLfloat delta = resolution / 2;

                GLfloat x = (GLfloat) (radius * cos(theta + offset) * sin(fi));
                GLfloat z = (GLfloat) (radius * sin(theta + offset) * sin(fi));
                GLfloat y = (GLfloat) (radius * cos(fi));

                vertices.push_back(makeVertex(x, y, z));

                GLfloat U = (GLfloat) (theta / 2 / PI);
                GLfloat V = (GLfloat) (cos(fi) + 1) / 2;

                vertices.push_back(makeVertex(U, V, 0));

                if (theta != 0) {
                    if (fi == resolution) {
                        indices.push_back(makeIndex(i - 1, i, 0));
                    } else {
                        indices.push_back(makeIndex(i - 1, i, i - n - 1));
                        indices.push_back(makeIndex(i, i - n - 1, i - n));
                    }
                }
                ++i;
            }
            if (fi == resolution) {
                indices.push_back(makeIndex(i - tn, i - 1, 0));
            } else {
                indices.push_back(makeIndex(i - tn, i - 1, i - n - 1));
                indices.push_back(makeIndex(i - tn, i - n - tn, i - n - 1));
            }
            n = tn;
            offset += resolution / 2;
        }
        for (GLuint j = i - 1; j > i - n; --j) {
            indices.push_back(makeIndex(j, j - 1, i));
        }
        indices.push_back(makeIndex(i - 1, i - n, i));
        vertices.push_back(makeVertex(0, -radius, 0));
        vertices.push_back(makeVertex(0.5f, 0.0f, 0));

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex) * 2, (GLvoid *) 0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex) * 2, (GLvoid *) 0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex) * 2, (GLvoid *) (sizeof(Vertex)));
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(Index), indices.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glBindVertexArray(0);

        return indices.size();

    }

    GLuint createTexture(char const *filePath) {
        return loadtga(filePath);
    }

    void setTexture(char const *filePath) {
        GLuint tex= createTexture(filePath);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);
        glUniform1i(glGetUniformLocation(shaderProgram, "solarTexture"), 0);
    }

    void drawSphere(glm::mat4 &model, bool light = true) {
        // Model Matrix
        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        // Light
        GLint lightLoc = glGetUniformLocation(shaderProgram, "light");
        glUniform1i(lightLoc, light);

        // Draw
        glDrawElements(GL_TRIANGLES, (GLsizei) (sphereIndexSize * 3), GL_UNSIGNED_INT, 0);
    }

    void drawSun() {
        // Texture
        setTexture("resources/Sun.tga");

        // Model Matrix
        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        glm::mat4 model(1.0f);
        model = glm::scale(model, glm::vec3(10, 10, 10));

        // Draw
        drawSphere(model, false);
   }

   void drawOthers(const char *name, GLfloat radius, GLfloat timeFactor, GLfloat scaleFactor) {

       char filePath[200] = "resources/";
       strcat(filePath, name);
       strcat(filePath, ".tga");

       // Texture
       setTexture(filePath);

       // Model Matrix
       glm::mat4 model(1.0f);
       radius += 20;
       timeFactor /= 300;
       model = glm::scale(model, glm::vec3(scaleFactor, scaleFactor, scaleFactor));
       model = glm::translate(model, glm::vec3(radius * cos(glfwGetTime() / timeFactor),
                                               0,
                                               radius * sin(glfwGetTime() / timeFactor)
       ));
       model = glm::rotate(model, (GLfloat) glfwGetTime() , glm::vec3(1, 1, 0));

       // Draw
       drawSphere(model);
   }

    void display() {

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Matrix
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(45.0f, (GLfloat) WIDTH / (GLfloat) HEIGHT, 0.1f, 100.0f);

        GLint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
        GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniform3f(viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Vertex Array
        glBindVertexArray(VAO_Sphere);

        // Array Buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_Sphere);

        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        drawSun();

        drawOthers("Mercury", 1.0f, 80, 0.5f);
        drawOthers("Venus", 1.1f, 100, 0.95f);
        drawOthers("Earth", 1.2f, 120, 1.0f);
        drawOthers("Mars", 1.4f, 160, 1.0f);
        drawOthers("Jupiter", 1.6f, 200, 2.0f);
        drawOthers("Saturn", 2.0f, 260, 1.4f);
        drawOthers("Uranus", 2.3f, 300, 1.2f);
        drawOthers("Neptune", 2.5f, 400, 1.2f);

        // Reset
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

    }

public:
    SolarSystemApp() : window(NULL) {
    }

    void run() {

        init();

        glfwSetKeyCallback(window, key_callback);

        sphereIndexSize = createSphere(.8, 30, VAO_Sphere, VBO_Sphere, EBO_Sphere);

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            display();

            glfwSwapBuffers(window);
        }

    }

    ~SolarSystemApp() {

        glDeleteVertexArrays(1, &VAO_Sphere);
        glDeleteBuffers(1, &VBO_Sphere);
        glDeleteBuffers(1, &EBO_Sphere);
        glfwTerminate();

    }
};

std::map<GLFWwindow*, SolarSystemApp*> SolarSystemApp::appMap;

int main() {
    SolarSystemApp app;

    app.run();

    return 0;
}
