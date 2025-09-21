#pragma once

#include <string>

#include <glad/glad.h>


enum ShaderType {
    VERTEX,
    FRAGMENT
};

std::string readShaderFile(const std::string& filePath);
GLuint createShader(const char* shaderSource, ShaderType shaderType);
GLuint createShaderProgram(GLuint shaderList[]);
