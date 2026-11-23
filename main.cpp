#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <compile_shader_program.h>
#include <iostream>
#include <cmath>
#include <obj_loader.h>

using namespace std;

const char *vertexShaderSource = R"(#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec3 aNormal;

uniform mat4 modelToWorld; //model
uniform mat4 worldToCamera; //view
uniform mat4 projection;
uniform mat4 toClippingSpace; //clip

out vec3 pos;
out vec2 uv;
out vec3 normal;

void main()
{
   gl_Position = toClippingSpace * projection * worldToCamera * modelToWorld * vec4(aPos, 1.0);
   pos = aPos;
   uv = aUV;
   // Rotate the normal into world space (mat3 drops the translation part)
   normal = mat3(modelToWorld) * aNormal;
}
)";

const char *fragmentShaderTealSource = R"(#version 330 core

in vec3 pos;
in vec2 uv;
in vec3 normal;

out vec4 FragColor;

void main()
{
   vec3 N = normalize(normal);                      
   vec3 L = normalize(vec3(0.5, 1.0, 0.8));         
   float diffuse = max(dot(N, L), 0.0);             
   float ambient = 0.15;                            

   vec3 baseColor = vec3(0.3, 0.8, 0.4);            
   FragColor = vec4(baseColor * (ambient + diffuse), 1.0);
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

    GLFWwindow *window = glfwCreateWindow(1600, 1280, "CSCI 4620", NULL, NULL);

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
    

    std::vector<float> meshVertices = loadOBJ("Monster.obj");
    int meshVertexCount = (int)(meshVertices.size() / 8);
    

    unsigned int VAO_Triangle, VBO_Triangle;
    glGenVertexArrays(1, &VAO_Triangle);
    glGenBuffers(1, &VBO_Triangle);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_Triangle);
    glBufferData(GL_ARRAY_BUFFER, meshVertices.size() * sizeof(float), meshVertices.data(), GL_STATIC_DRAW);
    glBindVertexArray(VAO_Triangle);
    //positions
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    //uvs
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    //normals
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(5*sizeof(float)));
    glEnableVertexAttribArray(2);

    int modelToWorldPointer = glGetUniformLocation(shaderProgramTriangle, "modelToWorld");
    int worldToCameraPointer = glGetUniformLocation(shaderProgramTriangle, "worldToCamera");
    int projectionPointer = glGetUniformLocation(shaderProgramTriangle, "projection");
    int toClippingSpacePointer = glGetUniformLocation(shaderProgramTriangle, "toClippingSpace");


    float nearPlane = .1f;
    float farPlane = 100.0f;

    float FOV = 45;
    float radians = FOV/360 * 2 * 3.14159265f;
    float aspectRatio = 1600/1280.0f;
    float top = nearPlane * tan(radians/2);
    float right = top * aspectRatio;

    float modelToWorld[] = {
        1, 0, 0, 10, //
        0, 1, 0, -10, //
        0, 0, 1, -30, //
        0, 0, 0, 1, //
    };

    float worldToCamera[] = {
        1, 0, 0, -10, //
        0, 1, 0, 0, //
        0, 0, 1, 0, //
        0, 0, 0, 1, //
    };

    float projection[] = {
        1, 0, 0, 0, //
        0, 1, 0, 0, //
        0, 0, (nearPlane + farPlane), nearPlane*farPlane, //
        0, 0, -1, 0, //
    };

    float toClippingSpace[] = {
        nearPlane/right, 0, 0, 0, //
        0, nearPlane/top, 0, 0, //
        0, 0, -2/(farPlane-nearPlane), -(farPlane+nearPlane)/(farPlane-nearPlane), //
        0, 0, 0, 1, //
    };

    // Depth test: keep only the closest fragment at each pixel
    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.f, 0.f, 0.f, 1.0f);
        // Clear the depth buffer too, or last frame's depth values block this frame
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
       

        glUseProgram(shaderProgramTriangle);

        glUniformMatrix4fv(modelToWorldPointer, 1, GL_TRUE, modelToWorld);
        glUniformMatrix4fv(worldToCameraPointer, 1, GL_TRUE, worldToCamera);
        glUniformMatrix4fv(projectionPointer, 1, GL_TRUE, projection);
        glUniformMatrix4fv(toClippingSpacePointer, 1, GL_TRUE, toClippingSpace);


        glBindVertexArray(VAO_Triangle);
        glDrawArrays(GL_TRIANGLES, 0, meshVertexCount);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO_Triangle);
    glDeleteBuffers(1, &VBO_Triangle);
    glDeleteProgram(shaderProgramTriangle);

    glfwTerminate();
    return 0;
}
