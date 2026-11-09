#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <compile_shader_program.h>
#include <iostream>

const char *vertexShaderSource = R"(#version 330 core

layout (location = 0) in vec3 aPos;

void main()
{
   gl_Position = vec4(aPos/2, 1.0);
}
)";

const char *fragmentShaderTealSource = R"(#version 330 core

out vec4 FragColor;

void main()
{
   int cx = int(gl_FragCoord.x) / 20;
   int cy = int(gl_FragCoord.y) / 20;
   if ((cx + cy) % 2 == 0) FragColor = vec4(1.0, 0, 0, 1.0);
   else                    FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}

)";

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main()
{
    if (!glfwInit())
    {
        std::cerr << "CRITICAL ERROR: Failed to initialize GLFW framework" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "CSCI 4620", NULL, NULL);

    if (!window)
    {
        std::cerr << "CRITICAL ERROR: Failed to create OS Window via GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "CRITICAL ERROR: GLAD failed to map system OpenGL driver pointers" << std::endl;
        return -1;
    }

    unsigned int shaderProgramTriangle = compileShaderProgram(vertexShaderSource, fragmentShaderTealSource);

    float triangleVertices[] = {
        -.9f, -.9f, 0.0f, // Vertex one
        .9f,  -.9f, 0.0f, // Vertex two
        .9f,  .9f,  0.0f,  // Vertex three
    };

    unsigned int VAO_Triangle, VBO_Triangle;
    glGenVertexArrays(1, &VAO_Triangle);
    glGenBuffers(1, &VBO_Triangle);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_Triangle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);
    glBindVertexArray(VAO_Triangle);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgramTriangle);
        glBindVertexArray(VAO_Triangle);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO_Triangle);
    glDeleteBuffers(1, &VBO_Triangle);
    glDeleteProgram(shaderProgramTriangle);

    glfwTerminate();
    return 0;
}