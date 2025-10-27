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


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow *window);
std::vector<std::vector<float>> generateWorldVertices(int width, int height, int seed);

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
    std::cout << fragment;
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
    int width = 200;
    int height = 120;
    int seed = 294852343; // 294852343 //BUGGED SEED: 145134800; CELL 235
    std::vector<std::vector<float>> worldVertices = generateWorldVertices(width, height, seed);


    // BUFFERS AND SUCH
    GLuint VBO, EBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    //glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
        static_cast<void *>(nullptr));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
        reinterpret_cast<void *>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);


    // THE RENDER LOOP
    std::cout << "Entering render loop\n";

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

            worldVertices = generateWorldVertices(width, height, seed);

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> diff = end - start;
            std::cout << "Generation took " << diff << "s\n";

            lastSeedTime = currentTime;
        }

        //glClearColor(0.39f, 0.58f, 0.93f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set projection matrices
        glm::mat4 model         = glm::mat4(1.0f);
        glm::mat4 view          = glm::mat4(1.0f);
        glm::mat4 projection    = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -20.0f));
        model = glm::scale(model, glm::vec3(1.0f));
        view = glm::translate(glm::mat4(1.0f), glm::vec3(-width / 2, height / 2, -150.0f));
        projection = glm::perspective(glm::radians(45.0f),
            static_cast<float>(screenWidth) / static_cast<float>(screenHeight), 0.1f, 200.0f);

        // Use shader uniforms
        glUseProgram(shaderProgram);
        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLuint viewLoc  = glGetUniformLocation(shaderProgram, "view");
        GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Draw stuff on screen
        for (int i = 0; i < worldVertices.size(); i++) {
            std::vector<float>& vertices = worldVertices[i];

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                worldVertices[i].data(), GL_STATIC_DRAW);
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size() / 6);
        }

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

std::vector<std::vector<float>> generateWorldVertices(int width, int height, int seed) {
    // VORONOI STUFF
    const int macroWidth = 20;
    const int macroHeight = 12;
    const int minPoints = 2;
    const int maxPoints = 3;
    VoronoiGrid grid = createVoronoiGrid(width, height, macroWidth, macroHeight);
    generateVoronoiCells(grid, seed, minPoints, maxPoints);
    //printVoronoiGrid(grid);

    // Get a list of all bitmasks
    std::vector<VoronoiBitmask> bitmasks;
    bitmasks.reserve(grid.numFeaturePoints);
    for (int i  = 0; i < grid.numFeaturePoints; i++) {
        VoronoiBitmask currentBitmask = generateVoronoiBitmask(grid, i);
        bitmasks.push_back(currentBitmask);
    }
    std::cout << "Bitmask data complete\n";

    // Get a list of all vertex positions for each voronoi cell
    std::vector<std::vector<float>> cellsVertices;
    cellsVertices.reserve(grid.numFeaturePoints);
    for (int i = 0; i < bitmasks.size(); i++) {
        VoronoiBitmask& currentBitmask = bitmasks[i];
        std::vector<float> currentEdgeVertices = getEdgeVertices(currentBitmask);
        std::vector<float> currentFanVertices = getCenterVertex(currentBitmask,
            currentEdgeVertices);
        cellsVertices.push_back(currentFanVertices);
    }

    // Get color data for each vertex
    std::mt19937 colorGenerator(seed);
    std::uniform_real_distribution<float> randomRed(0.1f, 1.0f);
    std::uniform_real_distribution<float> randomGreen(0.1f, 1.0f);
    std::uniform_real_distribution<float> randomBlue(0.1f, 1.0f);
    std::vector<std::vector<float>> vertexColors;
    vertexColors.reserve(grid.numFeaturePoints * 3);
    for (int i = 0; i < grid.numFeaturePoints; i++) {
        float red = randomRed(colorGenerator);
        float green = randomGreen(colorGenerator);
        float blue = randomBlue(colorGenerator);
        std::vector<float> currentColor = {red, green, blue};
        vertexColors.push_back(currentColor);
    }

    // Add color data to position data
    for (int i = 0; i < cellsVertices.size(); i++) {
        std::vector<float> currentColor = vertexColors[i];
        for (int j = 3; j < cellsVertices[i].size(); j += 3) {
            cellsVertices[i].insert(cellsVertices[i].begin() + j, currentColor[0]);
            cellsVertices[i].insert(cellsVertices[i].begin() + j + 1, currentColor[1]);
            cellsVertices[i].insert(cellsVertices[i].begin() + j + 2, currentColor[2]);
            j += 3;
        }
        cellsVertices[i].insert(cellsVertices[i].end(), currentColor[0]);
        cellsVertices[i].insert(cellsVertices[i].end(), currentColor[1]);
        cellsVertices[i].insert(cellsVertices[i].end(), currentColor[2]);
    }
    std::cout << "Vertex data complete\n";

    return cellsVertices;
}
