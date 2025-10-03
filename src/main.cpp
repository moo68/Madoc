#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

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


    // DATA
    float vertices[] = {
        0.5f,  0.5f, 0.0f,  // top right
         0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f   // top left
    };
    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };


    // BUFFERS AND SUCH
    GLuint VBO, EBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
        static_cast<void *>(nullptr));
    glEnableVertexAttribArray(0);


    // VORONOI STUFF
    const int width = 80;
    const int height = 30;
    const int macroWidth = 20;
    const int macroHeight = 10;
    const int seed = 709248357;
    const int minPoints = 2;
    const int maxPoints = 3;
    VoronoiGrid grid = createVoronoiGrid(width, height, macroWidth, macroHeight);
    generateVoronoiCells(grid, seed, minPoints, maxPoints);
    printVoronoiGrid(grid);

    const u_int16_t voronoiID = 0;
    VoronoiBitmask bitmask = generateVoronoiBitmask(grid, voronoiID);
    printBitmask(bitmask, voronoiID);


    // THE RENDER LOOP
    while(!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.39f, 0.58f, 0.93f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

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
