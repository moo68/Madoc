#include <iostream>
#include <random>
#include <chrono>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <madoc/shader_utils.h>
#include <madoc/log_utils.h>
#include <madoc/voronoi.h>
#include <madoc/voronoi_mesh.h>
#include "madoc/biome_generator.h"
#include "madoc/perlin_noise.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow *window);

bool isWireframe = false;


int main() {
    std::cout << "Start of main\n";

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

    std::cout << "GLFW boilerplate complete\n";

    // SHADERS
    // Load shader files
    std::string vertex;
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
    }

    std::cout << "Shader complete\n";


    // WORLD GENERATION
    int width = 1000;
    int height = 600;
    int seed = 99342094;

    // VORONOI STUFF
    const int macroWidth = 20;
    const int macroHeight = 12;
    const int minPoints = 2;
    const int maxPoints = 2;
    VoronoiGrid grid = createVoronoiGrid(width, height, macroWidth, macroHeight);
    generateVoronoiCells(grid, seed, minPoints, maxPoints);

    // Get a list of all bitmasks
    std::vector<VoronoiBitmask> bitmasks;
    bitmasks.reserve(grid.numFeaturePoints);
    for (int i  = 0; i < grid.numFeaturePoints; i++) {
        VoronoiBitmask currentBitmask = generateVoronoiBitmask(grid, i);
        bitmasks.push_back(currentBitmask);
    }
    std::cout << "Bitmask data complete\n";

    // VERTEX DATA
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    unsigned int numPreviousIndices = 0;

    // For each bitmask, get the vertex and index data for that polygon
    for (int i = 0; i < bitmasks.size(); i++) {
        VoronoiBitmask& currentBitmask = bitmasks[i];
        std::vector<float> currentVertices = getEdgeVertices(currentBitmask);
        std::vector<unsigned int> currentIndices = getEarClippedIndices(currentVertices);
        if (numPreviousIndices != 0) {
            for (int j = 0; j < currentIndices.size(); j++) {
                currentIndices[j] += numPreviousIndices;
            }
        }
        numPreviousIndices += currentVertices.size() / 3;

        // Get the centroid coordinate of each polygon, and plug it into the
        // Perlin noise function to get its color value
        std::vector<float> centroid = getCenterVertex(currentVertices);
        std::vector<float> currentColor = generateBiomeColor(centroid[0], centroid[1], seed);

        // Add the generated color value to the list of vertex data
        for (int j = 3; j < currentVertices.size(); j += 6) {
            currentVertices.insert(currentVertices.begin() + j, currentColor[0]);
            currentVertices.insert(currentVertices.begin() + j + 1, currentColor[1]);
            currentVertices.insert(currentVertices.begin() + j + 2, currentColor[2]);
        }
        currentVertices.insert(currentVertices.end(), currentColor[0]);
        currentVertices.insert(currentVertices.end(), currentColor[1]);
        currentVertices.insert(currentVertices.end(), currentColor[2]);

        vertices.insert(vertices.end(), currentVertices.begin(), currentVertices.end());
        indices.insert(indices.end(), currentIndices.begin(), currentIndices.end());
    }
    std::cout << "Vertex data complete\n";


    // BUFFERS AND SUCH
    GLuint VBO, EBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, (vertices.size() * sizeof(float)),
        vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (indices.size() * sizeof(unsigned int)),
        indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
        static_cast<void *>(nullptr));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
        reinterpret_cast<void *>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glUseProgram(shaderProgram);


    // THE RENDER LOOP
    std::cout << "Entering render loop\n\n";

    std::mt19937 seedGenerator(314159265);
    std::uniform_int_distribution<int> randomSeed(0, 999999999);
    double lastSeedTime = glfwGetTime();
    double seedInterval = 2.0f;

    while(!glfwWindowShouldClose(window))
    {
        processInput(window);
        double currentTime = glfwGetTime();

        // Generate a new world every few seconds
        if (currentTime - lastSeedTime >= seedInterval) {
            seed = randomSeed(seedGenerator);
            std::cout << "Current Seed: " << seed << "\n";

            auto start = std::chrono::high_resolution_clock::now();

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> diff = end - start;
            std::cout << "Generation took " << diff << "\n\n";

            lastSeedTime = currentTime;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set projection matrices
        glm::mat4 model         = glm::mat4(1.0f);
        glm::mat4 view          = glm::mat4(1.0f);
        glm::mat4 projection    = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -20.0f));
        model = glm::scale(model, glm::vec3(1.0f));
        view = glm::translate(glm::mat4(1.0f), glm::vec3(-width / 2, height / 2, -750.0f));
        projection = glm::perspective(glm::radians(45.0f),
            static_cast<float>(screenWidth) / static_cast<float>(screenHeight), 0.1f, 1000.0f);

        // Use shader uniforms
        glUseProgram(shaderProgram);
        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLuint viewLoc  = glGetUniformLocation(shaderProgram, "view");
        GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Draw stuff on screen
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
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
