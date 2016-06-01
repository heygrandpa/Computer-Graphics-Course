#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <vector>
#include <map>
#include <utility>

#include "utils/shader.h"

struct Vertex {
    GLfloat positions[3];
};

class ObjViewerApp {

private:
    const float d = .2f;
    const int dir[8][2][2] = {
            {{0,  1},  {1,  1}},
            {{1,  1},  {1,  0}},
            {{1,  0},  {1,  -1}},
            {{0,  -1}, {1,  -1}},
            {{0,  -1}, {-1, -1}},
            {{-1, -1}, {-1, 0}},
            {{-1, 0},  {-1, 1}},
            {{-1, 1}, {0, 1}}
    };
    const GLdouble PI = 3.1415926535;
    const GLuint WIDTH = 640, HEIGHT = 640;

    const GLchar *VERTEX_SHADER_FILE = "shaders/b_surface.vert";
    const GLchar *FRAGMENT_SHADER_FILE = "shaders/b_surface.frag";

    GLFWwindow *window;
    GLuint shaderProgram;
    GLuint VBO = 0, VAO = 0;
    GLuint VBO_b = 0, VAO_b = 0;

    std::vector<std::vector<Vertex>> rows;
    std::vector<Vertex> vertices;
    std::vector<std::vector<Vertex>> bezierRows;
    std::vector<Vertex> bezierVertices;

    std::vector<double> B1, B2;

    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    GLfloat cameraSpeed = 0.01f;
    bool keyW = false;
    bool keyA = false;
    bool keyS = false;
    bool keyD = false;
    bool keyUp = false;
    bool keyLeft = false;
    bool keyRight = false;
    bool keyDown = false;

    static std::map<GLFWwindow *, ObjViewerApp *> appMap;

    static void error_callback(int error, const char *description) {
        std::cerr << description << std::endl;
    }

    static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
        ObjViewerApp &app = *appMap[window];
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);
        bool set = action != GLFW_RELEASE;
        if (key == GLFW_KEY_W)
            app.keyW = set;
        if (key == GLFW_KEY_S)
            app.keyS = set;
        if (key == GLFW_KEY_D)
            app.keyD = set;
        if (key == GLFW_KEY_A)
            app.keyA = set;
        if (key == GLFW_KEY_LEFT)
            app.keyLeft = set;
        if (key == GLFW_KEY_RIGHT)
            app.keyRight = set;
        if (key == GLFW_KEY_UP)
            app.keyUp = set;
        if (key == GLFW_KEY_DOWN)
            app.keyDown = set;
    }

    void updatePoints() {

        for (auto it = rows.begin(); it != rows.end(); ++it) {
            for (auto it2 = it->begin(); it2 != it->end(); ++it2) {
                auto &v = *it2;
                v.positions[2] += cos(glfwGetTime() + (v.positions[0]) / d * PI / 2) / 200.0f;
            }
        }

        createVertices();
        createPoints(VAO, VBO, vertices, false, true);
        createBezier();
        createPoints(VAO_b, VBO_b, bezierVertices, true, true);
    }

    static Vertex makeVertex(GLfloat x, GLfloat y, GLfloat z) {
        Vertex v;
        v.positions[0] = x;
        v.positions[1] = y;
        v.positions[2] = z;
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

        window = glfwCreateWindow(WIDTH, HEIGHT, "XHX OpenGL 6.2 - B Spline Surface", nullptr, nullptr);
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

    void createBezier(double d = 0.4) {
        int m = (int) (rows.size() - 1);
        int n = (int) (rows[0].size() - 1);
        int k = 3;
        int h = 3;
        B1.resize((unsigned long) (m + k + 1));
        B2.resize((unsigned long) (n + h + 1));
        bezierVertices.clear();
        bezierRows.clear();
        for (double u = knots(k - 1); u <= knots(m + 1); u += d) {
            for (double v = knots(h - 1); v <= knots(n + 1); v += d) {
                calcBasis(B1, u, m, k);
                calcBasis(B2, v, n, h);
                Vertex newV;
                newV.positions[0] = newV.positions[1] = newV.positions[2] = 0;
                for (int i = 0; i <= m; ++i) {
                    for (int j = 0; j <= n; ++j) {

                        for (int p = 0; p < 3; ++p) {
                            newV.positions[p] += (GLfloat) (B1[i] * B2[j] * rows[i][j].positions[p]);
                        }
                    }
                }
                bezierVertices.push_back(newV);
            }
            bezierRows.push_back(bezierVertices);
            bezierVertices.clear();
        }

        std::map<std::pair<int, int>, Vertex> norms;
        for (int i = 0; i < bezierRows.size(); ++i) {
            for (int j = 0; j < bezierRows[i].size(); ++j) {
                glm::vec3 norm(0.0f);
                for (int f = 0; f < 8; ++f) {
                    int i1 = i + dir[f][0][0], j1 = j + dir[f][0][1];
                    int i2 = i + dir[f][1][0], j2 = j + dir[f][2][1];
                    if (i1 < 0 || i1 >= bezierRows.size() || i2 < 0 || i2 >= bezierRows.size()
                            || j1 < 0 || j1 >= bezierRows[i1].size() || j2 < 0 || j2 >= bezierRows[i2].size()) continue;
                    glm::vec3 v1(bezierRows[i][j].positions[0],
                                 bezierRows[i][j].positions[1],
                                 bezierRows[i][j].positions[2]);
                    glm::vec3 v2(bezierRows[i1][j1].positions[0],
                                 bezierRows[i1][j1].positions[1],
                                 bezierRows[i1][j1].positions[2]);
                    glm::vec3 v3(bezierRows[i2][j2].positions[0],
                                 bezierRows[i2][j2].positions[1],
                                 bezierRows[i2][j2].positions[2]);
                    norm += glm::cross(v2 - v1, v3 - v1);
                }
                norm = glm::normalize(norm);
                norms[std::make_pair(i, j)] = makeVertex(norm.x, norm.y, norm.z);
            }
        }
        for (int i = 0; i < bezierRows.size() - 1; ++i) {
            for (int j = 0; j < bezierRows[i].size() - 1; ++j) {
                bezierVertices.push_back(bezierRows[i][j]);
                bezierVertices.push_back(norms[std::make_pair(i, j)]);
                bezierVertices.push_back(bezierRows[i + 1][j]);
                bezierVertices.push_back(norms[std::make_pair(i + 1, j)]);
                bezierVertices.push_back(bezierRows[i][j + 1]);
                bezierVertices.push_back(norms[std::make_pair(i, j + 1)]);
                bezierVertices.push_back(bezierRows[i + 1][j]);
                bezierVertices.push_back(norms[std::make_pair(i + 1, j)]);
                bezierVertices.push_back(bezierRows[i][j + 1]);
                bezierVertices.push_back(norms[std::make_pair(i, j + 1)]);
                bezierVertices.push_back(bezierRows[i + 1][j + 1]);
                bezierVertices.push_back(norms[std::make_pair(i + 1, j + 1)]);
            }
        }
    }

    double knots(int i) {
        return (double) i;
    }

    void calcBasis(std::vector<double> &B, double t, double n, double k) {
        double m = k + n;
        for (int i = 0; i < m; ++i) {
            if (t < knots(i + 1) && t >= knots(i)) {
                B[i] = 1;
            } else {
                B[i] = 0;
            }
        }
        for (int p = 2; p <= k; ++p) {
            for (int i = 0; i <= n; ++i) {
                double r1, r2;
                if (knots(i + p - 1) == knots(i)) r1 = 0;
                else r1 = (t - knots(i)) / (knots(i + p - 1) - knots(i)) * B[i];
                if (knots(i + p) - knots(i + 1) == 0) r2 = 0;
                else r2 = (knots(i + p) - t) / (knots(i + p) - knots(i + 1)) * B[i + 1];
                B[i] = r1 + r2;
            }
        }

    }

    void createVertices() {

        vertices.clear();
        for (auto it = rows.begin(); it != rows.end(); ++it) {
            for (auto it2 = it->begin(); it2 != it->end(); ++it2) {
                vertices.push_back(*it2);
            }
        }

    }

    void createPoints(GLuint &VAO, GLuint &VBO, std::vector<Vertex> &vertices, bool normal, bool second = false) {

        if (second) {
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
        }

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        if (normal) {
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex) * 2, (GLvoid *) 0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex) * 2, (GLvoid *) (sizeof(Vertex)));
            glEnableVertexAttribArray(1);
        } else {
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *) 0);
            glEnableVertexAttribArray(0);
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindVertexArray(0);
    }

    void display() {

        if (keyW)
            cameraPos += cameraSpeed * cameraFront;
        if (keyS)
            cameraPos -= cameraSpeed * cameraFront;
        if (keyLeft)
            cameraFront -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (keyRight)
            cameraFront += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (keyA)
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (keyD)
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (keyUp)
            cameraPos += glm::normalize(cameraUp) * cameraSpeed;
        if (keyDown)
            cameraPos -= glm::normalize(cameraUp) * cameraSpeed;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

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
        glBindVertexArray(VAO);


        /*
         *
         * Control Points
         *
         */
        glm::mat4 model(1.0f);
        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        GLint lightLoc = glGetUniformLocation(shaderProgram, "light");
        glUniform1i(lightLoc, 0);
        GLint pointLoc = glGetUniformLocation(shaderProgram, "point");
        glUniform1i(pointLoc, 0);

        glDrawArrays(GL_POINTS, 0, (GLsizei) vertices.size());

        /*
         *
         * Surface
         *
         */
        glBindVertexArray(VAO_b);

        modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        lightLoc = glGetUniformLocation(shaderProgram, "light");
        glUniform1i(lightLoc, 1);
        pointLoc = glGetUniformLocation(shaderProgram, "point");
        glUniform1i(pointLoc, 1);

        glDrawArrays(GL_TRIANGLES, 0, (GLsizei) bezierVertices.size());

        // Reset
        glBindVertexArray(0);

        updatePoints();

    }

public:
    ObjViewerApp() : window(NULL) {
    }

    void run() {

        init();

        glPointSize(5);

        srand((unsigned int) time(0));
        for (float i = -0.8f; i <= 0.8f; i += d) {
            for (float j = -0.8f; j <= 0.8f; j += d) {
                float r1 = 0;//(rand() % 1000 - 500) / 10000.0f;
                float r2 = 0;//(rand() % 1000 - 500) / 10000.0f;
                float r3 = 0;//(rand() % 1000 - 500) / 1000.0f;
                vertices.push_back(makeVertex(i + r1, j + r2, r3 + 0.4f));
            }
            rows.push_back(vertices);
            vertices.clear();
        }

        createVertices();
        createPoints(VAO, VBO, vertices, false);
        createBezier();
        createPoints(VAO_b, VBO_b, bezierVertices, true);

        glfwSetKeyCallback(window, key_callback);

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            display();

            glfwSwapBuffers(window);
        }

    }

    ~ObjViewerApp() {

        glDeleteVertexArrays(1, &VAO_b);
        glDeleteBuffers(1, &VBO_b);
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glfwTerminate();

    }
};

std::map<GLFWwindow *, ObjViewerApp *> ObjViewerApp::appMap;

int main() {
    ObjViewerApp app;

    app.run();

    return 0;
}
