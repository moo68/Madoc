#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <madoc/shader_utils.h>
#include <madoc/log_utils.h>
#include <madoc/voronoi.h>
#include <madoc/voronoi_mesh.h>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow *window);

bool isWireframe = false;


int main() {
    // BOILERPLATE NONSENSE
    if (!glfwInit()) {
        std::cerr << "GLFW failed to initialize!" << std::endl;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Fetch monitor information in order to properly create a fullscreen window
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* videoMode = glfwGetVideoMode(primaryMonitor);
    int screenWidth = videoMode->width;
    int screenHeight = videoMode->height;
    // Actually create the window
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Madoc",
        glfwGetPrimaryMonitor(), nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);

    // Load GLAD and set the viewport
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cout << "Failed to initialize GLAD!" << std::endl;
        return -1;
    }
    glViewport(0, 0, screenWidth, screenHeight);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    // SHADERS
    // Load shader files
    /*std::string vertex;
    std::string fragment;
    try {
        vertex = readShaderFile("assets/shaders/vertex.glsl");
        fragment = readShaderFile("assets/shaders/fragment.glsl");
    }
    catch (const std::runtime_error& error) {
        logError("shader_utils", error.what());
        return -1;
    }
    const char* vertexShaderSource = vertex.c_str();
    const char* fragmentShaderSource = fragment.c_str();
    // Set up the shader program
    GLuint shaderProgram;
    try {
        const GLuint vertexShader = createShader(vertexShaderSource, VERTEX);
        const GLuint fragmentShader = createShader(fragmentShaderSource, FRAGMENT);
        GLuint shaderList[] = {vertexShader, fragmentShader};
        shaderProgram = createShaderProgram(shaderList);
    }
    catch (const std::runtime_error& error) {
        logError("shader_utils", error.what());
        return -1;
    }*/
    // --- Minimal inline shader sanity test ---
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    const char* vsSrc = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";
    glShaderSource(vs, 1, &vsSrc, nullptr);
    glCompileShader(vs);

    // check vertex compile
    GLint success;
    char infoLog[512];
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vs, 512, nullptr, infoLog);
        std::cerr << "Vertex shader error:\n" << infoLog << std::endl;
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fsSrc = R"(
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0, 0.5, 0.2, 1.0);
}
)";
    glShaderSource(fs, 1, &fsSrc, nullptr);
    glCompileShader(fs);

    // check fragment compile
    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fs, 512, nullptr, infoLog);
        std::cerr << "Fragment shader error:\n" << infoLog << std::endl;
    }

    // link program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);

    // check link
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Program link error:\n" << infoLog << std::endl;
    }

    // count active uniforms
    GLint numUniforms = 0;
    glGetProgramiv(shaderProgram, GL_ACTIVE_UNIFORMS, &numUniforms);
    std::cout << "Active uniforms: " << numUniforms << std::endl;

    // cleanup (optional for this test)
    glDeleteShader(vs);
    glDeleteShader(fs);


    int activeUniforms = 0;
    glGetProgramiv(shaderProgram, GL_ACTIVE_UNIFORMS, &activeUniforms);
    std::cout << "Active uniforms: " << activeUniforms << std::endl;

    for (int i = 0; i < activeUniforms; ++i) {
        char name[256];
        int length = 0;
        glGetActiveUniformName(shaderProgram, i, sizeof(name), &length, name);
        std::cout << "Uniform " << i << ": " << name << std::endl;
    }

    // VORONOI STUFF
    const int width = 80;
    const int height = 30;
    const int macroWidth = 20;
    const int macroHeight = 10;
    const int seed = 314159265;
    const int minPoints = 2;
    const int maxPoints = 3;
    VoronoiGrid grid = createVoronoiGrid(width, height, macroWidth, macroHeight);
    generateVoronoiCells(grid, seed, minPoints, maxPoints);
    //printVoronoiGrid(grid);

    const u_int16_t voronoiID = 19; // 12: weird isolated edge case; 19: fan no-workey
    VoronoiBitmask bitmask = generateVoronoiBitmask(grid, voronoiID);
    printBitmask(bitmask, voronoiID);

    std::vector<float> edgeVertices = getEdgeVertices(bitmask);
    std::vector<float> fanVertices = getCenterVertex(edgeVertices);
    std::vector<float> centroid = {fanVertices[0], fanVertices[1], fanVertices[2]};
    /*for (int i = 0; i < fanVertices.size(); i += 3) {
        std::cout << "(" << fanVertices[i] << ", " << fanVertices[i + 1] << ", " << fanVertices[i + 2] << ")\n";
    }*/

    //glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(width),
        //static_cast<float>(height), 0.0f, -1.0f, 1.0f);

    // DATA
    /*float vertices[] = {
        0.5f,  0.5f, 0.0f,  // top right
         0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f   // top left
    };
    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };*/


    // BUFFERS AND SUCH
   /* GLuint VBO, EBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    //glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, (fanVertices.size() * sizeof(float)), fanVertices.data(), GL_STATIC_DRAW);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
        static_cast<void *>(nullptr));
    glEnableVertexAttribArray(0);*/
    float tri[] = {
        0.0f,  0.5f, 0.0f,
       -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f
   };

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    for (int i = 0; i < fanVertices.size(); i += 3) {
       std::cout << "(" << fanVertices[i] << ", " << fanVertices[i + 1] << ", " << fanVertices[i + 2] << ")\n";
    }

    glBufferData(GL_ARRAY_BUFFER, fanVertices.size() * sizeof(float),
        fanVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);

    // THE RENDER LOOP
    while(!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.39f, 0.58f, 0.93f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 model         = glm::mat4(1.0f);
        glm::mat4 view          = glm::mat4(1.0f);
        glm::mat4 projection    = glm::mat4(1.0f);
        //model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::translate(model, glm::vec3(-centroid[0], -centroid[1], -20.0f));
        model = glm::scale(model, glm::vec3(1.0f));
        view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
        projection = glm::perspective(glm::radians(45.0f),
            static_cast<float>(screenWidth) / static_cast<float>(screenHeight), 0.1f, 100.0f);

        glUseProgram(shaderProgram);
        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLuint viewLoc  = glGetUniformLocation(shaderProgram, "view");
        GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        //if (modelLoc == -1) std::cerr << "Uniform 'model' not found in shader!" << std::endl;
        //if (viewLoc == -1) std::cerr << "Uniform 'view' not found in shader!" << std::endl;
        //if (projectionLoc == -1) std::cerr << "Uniform 'projection' not found in shader!" << std::endl;

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        //glDrawArrays(GL_TRIANGLES, 0, 3);
        glDrawArrays(GL_TRIANGLE_FAN, 0, fanVertices.size() / 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    //glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}


/*
 * Dynamically change the glViewport's width and/or height if the user changes
 * the window size.
 */
void framebuffer_size_callback(GLFWwindow* window, const int width, const int height)
{
    glViewport(0, 0, width, height);
}

/*
 * Get user input from the keyboard one time when they press a key
 * key_callback() should be used for single button presses and toggles
 */
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // ESC to exit the program
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    // TAB to toggle wireframe mode
    if (!isWireframe && key == GLFW_KEY_TAB && action == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        isWireframe = true;
    }
    else if (isWireframe && key == GLFW_KEY_TAB && action == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        isWireframe = false;
    }
}

/*
 * Get user input from the keyboard continuously as the key gets pressed
 * processInput() should be used for inputs that are continuous in nature
 */
void processInput(GLFWwindow *window)
{

}
