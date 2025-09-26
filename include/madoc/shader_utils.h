#pragma once

#include <string>

#include <glad/glad.h>


enum ShaderType {
    VERTEX,
    FRAGMENT
};

// Take a file path as input, and output a string of the text in that file
std::string readShaderFile(const std::string& filePath);

// Generate a shader based on the source code and shader type
GLuint createShader(const char* shaderSource, ShaderType shaderType);

// Generate a shaderProgram based on the given lists of individual shaders
GLuint createShaderProgram(GLuint shaderList[]);
