#include <iostream>
#include <random>

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

    // VORONOI STUFF
    const int width = 200;
    const int height = 120;
    const int macroWidth = 20;
    const int macroHeight = 12;
    const int seed = 54296452; //Other seed: 314159265 //Missing cell 17: 60294323
    const int minPoints = 2;
    const int maxPoints = 3;
    VoronoiGrid grid = createVoronoiGrid(width, height, macroWidth, macroHeight);
    generateVoronoiCells(grid, seed, minPoints, maxPoints);
    //printVoronoiGrid(grid);

    // Get a list of all bitmask data
    std::vector<VoronoiBitmask> bitmasks;
    bitmasks.reserve(grid.numFeaturePoints);
    for (int i  = 0; i < grid.numFeaturePoints; i++) {
        VoronoiBitmask currentBitmask = generateVoronoiBitmask(grid, i);
        bitmasks.push_back(currentBitmask);
    }
    std::cout << "Bitmask data complete\n";

    // Get a list of all vertex data for each voronoi cell
    std::vector<std::vector<float>> cellsVertices;
    cellsVertices.reserve(grid.numFeaturePoints);
    for (int i = 0; i < bitmasks.size(); i++) {
        VoronoiBitmask& currentBitmask = bitmasks[i];
        std::vector<float> currentEdgeVertices = getEdgeVertices(currentBitmask);
        std::vector<float> currentFanVertices = getCenterVertex(currentBitmask,
            currentEdgeVertices);
        cellsVertices.push_back(currentFanVertices);
    }
    std::cout << "Vertex data complete\n";


    // BUFFERS AND SUCH
    GLuint VBO, EBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    //glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
        static_cast<void *>(nullptr));
    glEnableVertexAttribArray(0);

    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);


    std::mt19937 generator(seed);
    std::uniform_real_distribution<float> randomRed(0.0f, 1.0f);
    std::uniform_real_distribution<float> randomGreen(0.0f, 1.0f);
    std::uniform_real_distribution<float> randomBlue(0.0f, 1.0f);

    // THE RENDER LOOP
    std::cout << "Entering render loop\n";

    while(!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.39f, 0.58f, 0.93f, 1.0f);
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

        //glm::vec4 color = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);

        // Use shader uniforms
        glUseProgram(shaderProgram);
        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLuint viewLoc  = glGetUniformLocation(shaderProgram, "view");
        GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        GLuint colorLoc = glGetUniformLocation(shaderProgram, "color");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3f(colorLoc, 1.0f, 0.0f, 0.0f);

        // Draw stuff on screen
        for (int i = 0; i < cellsVertices.size(); i++) {
            std::vector<float>& vertices = cellsVertices[i];

            //float red = randomRed(generator);
            //float green = randomGreen(generator);
            //float blue = randomBlue(generator);
            float colorValue = static_cast<float>(i) / static_cast<float>(grid.numFeaturePoints);
            glUniform3f(colorLoc, colorValue, 0.0f, 0.0f);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                cellsVertices[i].data(), GL_STATIC_DRAW);
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size() / 3);
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
