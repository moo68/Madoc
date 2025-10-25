#include <fstream>
#include <stdexcept>
#include <iostream>
#include <filesystem>

#include <madoc/shader_utils.h>
#include <madoc/log_utils.h>


std::string readShaderFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open shader file: " + filePath);
    }
    if (filePath.size() < 5 || filePath.substr(filePath.size() - 5) != ".glsl") {
        logWarning("shader_utils", filePath + " is not a .glsl file");
    }

    std::string line;
    std::string outputString;
    while (std::getline(file, line)) {
        outputString += line;
        outputString += "\n";
    }

    return outputString;
}

GLuint createShader(const char* shaderSource, const ShaderType shaderType) {
    GLuint shader = 0;
    switch (shaderType) {
        case VERTEX:
            shader = glCreateShader(GL_VERTEX_SHADER);
            break;
        case FRAGMENT:
            shader = glCreateShader(GL_FRAGMENT_SHADER);
            break;
        default:
            throw std::runtime_error("Unknown shader type. Cannot create shader.");
    }
    glShaderSource(shader, 1, &shaderSource, nullptr);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        switch (shaderType) {
            case VERTEX:
                throw std::runtime_error("Vertex shader compilation failed; "
                                         "OpenGL's log: " + std::string(infoLog));
            case FRAGMENT:
                throw std::runtime_error("Fragment shader compilation failed; "
                                         "OpenGL's log: " + std::string(infoLog));
        }
    }

    return shader;
}

GLuint createShaderProgram(GLuint shaderList[]) {
    const GLuint shaderProgram = glCreateProgram();
    for (int i = 0; i < sizeof(shaderList) / sizeof(shaderList[0]); i++) {
        glAttachShader(shaderProgram, shaderList[i]);
    }
    glLinkProgram(shaderProgram);

    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        throw std::runtime_error("Shader program linking failed; "
                                 "OpenGL's log: " + std::string(infoLog));
    }

    for (int i = 0; i < sizeof(shaderList) / sizeof(shaderList[0]); i++) {
        glDeleteShader(shaderList[i]);
    }

    return shaderProgram;
}
