#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <vector>
#include <map>

#include "utils/shader.h"

struct Vertex {
    GLfloat positions[3];
};

class ObjViewerApp {

private:
    const GLdouble PI = 3.1415926535;
    const GLuint WIDTH = 640, HEIGHT = 640;

    const GLchar *VERTEX_SHADER_FILE = "shaders/bspline.vert";
    const GLchar *FRAGMENT_SHADER_FILE = "shaders/bspline.frag";

    GLFWwindow *window;
    GLuint shaderProgram;
    GLuint VBO = 0, VAO = 0;
    GLuint VBO_b = 0, VAO_b = 0;
    double mouseX, mouseY;
    Vertex *chosenV = nullptr;

    std::vector<Vertex> vertices;
    std::vector<Vertex> bezierVertices;

    std::vector<double> B;

    static std::map<GLFWwindow *, ObjViewerApp *> appMap;

    static void error_callback(int error, const char *description) {
        std::cerr << description << std::endl;
    }

    static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos) {
        ObjViewerApp &app = *appMap[window];
        xpos /= app.WIDTH / 2;
        ypos /= app.HEIGHT / 2;
        xpos -= 1;
        ypos = 1 - ypos;
        app.mouseX = xpos;
        app.mouseY = ypos;
        if (app.chosenV) {
            app.chosenV->positions[0] = (GLfloat) xpos;
            app.chosenV->positions[1] = (GLfloat) ypos;
            app.updatePoints();
        }
    }

    static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
        ObjViewerApp &app = *appMap[window];
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);
    }

    static double dis(double dx, double dy, double dz = 0) {
        return sqrt(dx * dx + dy * dy + dz * dz);
    }

    int n() {
        return (int) vertices.size() - 1;
    }

    int k() {
        return 4;
    }

    int m() {
        return k() + n();
    }

    std::vector<Vertex>::iterator findChosen() {
        chosenV = nullptr;
        double minD = -1;
        auto p = vertices.begin();
        for (auto it = vertices.begin(); it != vertices.end(); ++it) {
            auto v = *it;
            double d = dis(v.positions[0] - mouseX, v.positions[1] - mouseY, v.positions[2]);
            if (d < 0.03 && (minD == -1 || d < minD)) {
                minD = d;
                p = it;
            }
        }
        if (minD != -1) {
            chosenV = &(*p);
            return p;
        }
        return vertices.end();
    }

    static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
        ObjViewerApp &app = *appMap[window];
        if (action == GLFW_RELEASE) {
            app.chosenV = nullptr;
            app.createBezier();
            app.createPoints(app.VAO, app.VBO, app.vertices, true);
        }

        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            app.findChosen();
        }
        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
            auto it = app.findChosen();
            if (app.chosenV) {
                app.vertices.erase(it);
                app.chosenV = nullptr;
            } else {
                app.vertices.push_back(makeVertex((GLfloat) app.mouseX, (GLfloat) app.mouseY, 0));
            }
            app.updatePoints();
        }
    }

    void updatePoints() {
        createPoints(VAO, VBO, vertices, true);
        createBezier();
        createPoints(VAO_b, VBO_b, bezierVertices, true);
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

        window = glfwCreateWindow(WIDTH, HEIGHT, "XHX OpenGL 6 - B Spline Curve", nullptr, nullptr);
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

    void createBezier(double d = 0.01) {
        B.resize((unsigned long) (m() + 1));
        bezierVertices.clear();
        for (double t = knots(k() - 1); t <= knots(n() + 1); t += d) {
            calcBasis(t);
            Vertex newV;
            newV.positions[0] = newV.positions[1] = newV.positions[2] = 0;
            for (int i = 0; i <= n(); ++i) {
                for (int j = 0; j < 3; ++j) {
                    newV.positions[j] += (GLfloat) (B[i] * vertices[i].positions[j]);
                }
            }
            bezierVertices.push_back(newV);
        }
    }

    double knots(int i) {
        /* if first k-1 knots are 0 and others are 1,
         * the curve will be bezier curve which controlled only by first k points
         *
        if (i <= k() - 1) return 0;
        return 1;
         */
        return (double)i;
    }

    void calcBasis(double t) { // assume knot[i] = i
        for (int i = 0; i < m(); ++i) {
            if (t < knots(i + 1) && t >= knots(i)) {
                B[i] = 1;
            } else {
                B[i] = 0;
            }
        }
        for (int p = 2; p <= k(); ++p) {
            for (int i = 0; i <= n(); ++i) {
                double r1, r2;
                if (knots(i + p - 1) == knots(i)) r1 = 0;
                else r1 = (t - knots(i)) / (knots(i + p - 1) - knots(i)) * B[i];
                if (knots(i + p) - knots(i + 1) == 0) r2 = 0;
                else r2 = (knots(i + p) - t) / (knots(i + p) - knots(i + 1)) * B[i + 1];
                B[i] = r1 + r2;
            }
        }

    }

    void createPoints(GLuint &VAO, GLuint &VBO, std::vector<Vertex> &vertices, bool second = false) {

        if (second) {
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
        }

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *) 0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindVertexArray(0);
    }

    void display() {

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Vertex Array
        glBindVertexArray(VAO);

        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        GLint bezierLoc = glGetUniformLocation(shaderProgram, "bezier");
        glUniform1i(bezierLoc, 0);
        glDrawArrays(GL_POINTS, 0, (GLsizei) vertices.size());

        bezierLoc = glGetUniformLocation(shaderProgram, "bezier");
        glUniform1i(bezierLoc, 0);
        glDrawArrays(GL_LINE_STRIP, 0, (GLsizei) vertices.size());

        glBindVertexArray(VAO_b);

        bezierLoc = glGetUniformLocation(shaderProgram, "bezier");
        glUniform1i(bezierLoc, 1);
        glDrawArrays(GL_LINE_STRIP, 0, (GLsizei) bezierVertices.size());


        // Reset
        glBindVertexArray(0);

    }

public:
    ObjViewerApp() : window(NULL) {
    }

    void run() {

        init();

        glPointSize(5);

        vertices.push_back(makeVertex(-0.8f, 0, 0));
        vertices.push_back(makeVertex(0, 0, 0));
        vertices.push_back(makeVertex(0.8f, 0, 0));

        createPoints(VAO, VBO, vertices);
        createBezier();
        createPoints(VAO_b, VBO_b, bezierVertices);

        glfwSetKeyCallback(window, key_callback);
        glfwSetCursorPosCallback(window, cursor_position_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);

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
