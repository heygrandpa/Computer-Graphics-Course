//
// Created by xuhongxu on 16/3/20.
//
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <string>
#include <GL/GLEW.h>

#include "shader.h"

GLuint loadShaders(const GLchar *vertex_shader_file, const GLchar *fragment_shader_file) {

    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    std::string vertexShaderCode;
    std::ifstream vertexShaderStream(vertex_shader_file, std::ios::in);
    if (vertexShaderStream.is_open()) {
        std::string line = "";
        while (getline(vertexShaderStream, line))
            vertexShaderCode += "\n" + line;
        vertexShaderStream.close();
    } else {
        std::cerr << "Failed to open vertex shader source file." << vertex_shader_file << std::endl;
        return 0;
    }

    std::string fragmentShaderCode;
    std::ifstream fragmentShaderStream(fragment_shader_file, std::ios::in);
    if (fragmentShaderStream.is_open()) {
        std::string line = "";
        while (getline(fragmentShaderStream, line))
            fragmentShaderCode += "\n" + line;
        fragmentShaderStream.close();
    } else {
        std::cerr << "Failed to open fragment shader source file." << std::endl;
        return 0;
    }

    GLint result = GL_FALSE;
    GLint infoLogLength;

    const GLchar *vertexSource = vertexShaderCode.c_str();
    glShaderSource(vertexShaderID, 1, &vertexSource, nullptr);
    glCompileShader(vertexShaderID);

    glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &result);
    glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 0) {
        std::cout << "VertexShader Error Message:" << std::endl;
        GLchar *vertexShaderErrorMessage = new GLchar[infoLogLength + 1];
        glGetShaderInfoLog(vertexShaderID, infoLogLength, nullptr, vertexShaderErrorMessage);
        std::cout << vertexShaderErrorMessage << std::endl;
        delete [] vertexShaderErrorMessage;
    }

    if (!result) {
        std::cerr << "Failed to compile vertex shader." << std::endl;
        return 0;
    }

    const GLchar *fragmentSource = fragmentShaderCode.c_str();
    glShaderSource(fragmentShaderID, 1, &fragmentSource, nullptr);
    glCompileShader(fragmentShaderID);

    glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &result);
    glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 0) {
        std::cout << "FragmentShader Error Message:" << std::endl;
        GLchar *fragmentShaderErrorMessage = new GLchar[infoLogLength + 1];
        glGetShaderInfoLog(fragmentShaderID, infoLogLength, nullptr, fragmentShaderErrorMessage);
        std::cout << fragmentShaderErrorMessage << std::endl;
        delete [] fragmentShaderErrorMessage;
    }

    if (!result) {
        std::cerr << "Failed to compile fragment shader." << std::endl;
        return 0;
    }

    GLuint programID = glCreateProgram();
    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragmentShaderID);
    glLinkProgram(programID);

    glGetProgramiv(programID, GL_LINK_STATUS, &result);
    glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 0) {
        std::cout << "Program Error Message:" << std::endl;
        GLchar *programErrorMessage = new GLchar[infoLogLength + 1];
        glGetProgramInfoLog(programID, infoLogLength, nullptr, programErrorMessage);
        std::cout << programErrorMessage << std::endl;
        delete [] programErrorMessage;
    }

    if (!result) {
        std::cerr << "Failed to link program." << std::endl;
        return 0;
    }

    glDetachShader(programID, vertexShaderID);
    glDetachShader(programID, fragmentShaderID);

    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);

    return programID;
}
