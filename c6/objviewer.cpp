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
#include "HalfEdge.h"

struct Vertex {
    GLfloat positions[3];
};

class ObjViewerApp {

private:
    Mesh *mesh = nullptr;

    const GLdouble PI = 3.1415926535;
    const GLuint WIDTH = 800, HEIGHT = 600;

    const GLchar *VERTEX_SHADER_FILE = "shaders/objviewer.vert";
    const GLchar *FRAGMENT_SHADER_FILE = "shaders/objviewer.frag";

    GLFWwindow *window;
    GLuint shaderProgram;
    GLuint VBO, VAO;

    std::vector<Vertex> vertices;

    glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f,  1.0f);
    glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

    GLfloat cameraSpeed = 0.01f;
    bool keyW = false;
    bool keyA = false;
    bool keyS = false;
    bool keyD = false;
    bool keyUp = false;
    bool keyLeft = false;
    bool keyRight = false;
    bool keyDown = false;

    static std::map<GLFWwindow*, ObjViewerApp*> appMap;

    static void error_callback(int error, const char *description) {
        std::cerr << description << std::endl;
    }

    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        ObjViewerApp &app = *appMap[window];
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);
        bool set = action != GLFW_RELEASE;
        if(key == GLFW_KEY_W)
            app.keyW = set;
        if(key == GLFW_KEY_S)
            app.keyS = set;
        if(key == GLFW_KEY_D)
            app.keyD = set;
        if(key == GLFW_KEY_A)
            app.keyA = set;
        if(key == GLFW_KEY_LEFT)
            app.keyLeft = set;
        if(key == GLFW_KEY_RIGHT)
            app.keyRight = set;
        if(key == GLFW_KEY_UP)
            app.keyUp = set;
        if(key == GLFW_KEY_DOWN)
            app.keyDown = set;
    }

    Vertex makeVertex(GLfloat x, GLfloat y, GLfloat z) {
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

        window = glfwCreateWindow(WIDTH, HEIGHT, "XHX OpenGL 5 - Obj Viewer", nullptr, nullptr);
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

    void createModel(Mesh *m, GLuint &VAO, GLuint &VBO) {
        auto meshVertices = m->getVertices();
        auto meshFaces = m->getFaces();

        for (auto it = meshFaces.begin(); it != meshFaces.end(); ++it) {
            Face *f = *it;
            HalfEdge *he1 = f->getHalfEdge();
            HalfEdge *he2 = he1->getNext();
            Vert *v = he1->getSource();
            glm::vec3 vn = v->getNormal();
            vertices.push_back(makeVertex(v->x, v->y, v->z));
            vertices.push_back(makeVertex(vn.x, vn.y, vn.z));
            v = he1->getTarget();
            vn = v->getNormal();
            vertices.push_back(makeVertex(v->x, v->y, v->z));
            vertices.push_back(makeVertex(vn.x, vn.y, vn.z));
            v = he2->getTarget();
            vn = v->getNormal();
            vertices.push_back(makeVertex(v->x, v->y, v->z));
            vertices.push_back(makeVertex(vn.x, vn.y, vn.z));
        }

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex) * 2, (GLvoid *) 0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex) * 2, (GLvoid *) (sizeof(Vertex)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindVertexArray(0);

    }

    void drawModel(glm::mat4 &model, bool light = true) {
        // Model Matrix
        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        // Light
        GLint lightLoc = glGetUniformLocation(shaderProgram, "light");
        glUniform1i(lightLoc, light);

        // Draw
        //glDrawElements(GL_TRIANGLES, (GLsizei) (indexSize * 3), GL_UNSIGNED_INT, 0);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei) vertices.size());
    }

    void draw() {
        // Model Matrix
        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        glm::mat4 model(1.0f);
        //model = glm::translate(model, glm::vec3(0.3, 0, 0));
        model = glm::scale(model, glm::vec3(4,4,4));

        // Draw
        drawModel(model);
    }

    void display() {


        if (keyW)
            cameraPos += cameraSpeed * cameraFront;
        if (keyS)
            cameraPos -= cameraSpeed * cameraFront;
        if (keyD)
            cameraFront += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (keyA)
            cameraFront -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (keyLeft)
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (keyRight)
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (keyUp)
            cameraPos += glm::normalize(cameraUp) * cameraSpeed;
        if (keyDown)
            cameraPos -= glm::normalize(cameraUp) * cameraSpeed;

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
        glBindVertexArray(VAO);

        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        draw();

        // Reset
        glBindVertexArray(0);

    }

public:
    ObjViewerApp() : window(NULL) {
        mesh = new Mesh("resources/bunny.obj");
    }

    void run() {

        init();

        glfwSetKeyCallback(window, key_callback);

        createModel(mesh, VAO, VBO);

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            display();

            glfwSwapBuffers(window);
        }

    }

    ~ObjViewerApp() {

        delete mesh;
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glfwTerminate();

    }
};

std::map<GLFWwindow*, ObjViewerApp*> ObjViewerApp::appMap;

int main() {
    ObjViewerApp app;

    app.run();

    return 0;
}
