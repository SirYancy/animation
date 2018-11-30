#include <iostream>
#include <fstream>
#include "collada/colladaloader.h"
#include "glad/glad.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_opengl.h"

#define GLM_FORCE_RADIANS

void CreateShaderProgram(GLuint shader, GLuint fragShader);

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

GLuint shaderProgram;
GLuint vao, vertexvbo, indexvbo;

const char *vertfn = "../shaders/vert.glsl";
const char *fragfn = "../shaders/frag.glsl";

int main() {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_version comp;
    SDL_version linked;
    SDL_VERSION(&comp);
    SDL_GetVersion(&linked);
    printf("SDL: %d.%d.%d\n", comp.major, comp.minor, comp.patch);
    printf("Linked: %d.%d.%d\n", linked.major, linked.minor, linked.patch);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    SDL_Window *window = SDL_CreateWindow("Cowboy", 100, 100, 640, 480, SDL_WINDOW_OPENGL);

    if(!window) {
        printf("Window failed\n");
        return EXIT_FAILURE;
    }

    float aspect = 640 / (float)480;

    SDL_GLContext context = SDL_GL_CreateContext(window);

    if(gladLoadGLLoader(SDL_GL_GetProcAddress)){
        printf("Vendor: %s\n", glGetString(GL_VENDOR));
        printf("Renderer: %s\n", glGetString(GL_RENDERER));
        printf("Version: %s\n", glGetString(GL_VERSION));
    }else{
        printf("OPENGL CONTEXT NOT LOADED\n");
        return -1;
    }

    GLuint vertShader;
    GLuint fragShader;
    CreateShaderProgram(vertShader, fragShader);

    printf("Loading Model\n");
    ColladaLoader *loader = new ColladaLoader("../models/cowboy.dae");
    auto *geometries = new std::vector<Geometry>();
    loader->ReadGeometries(geometries);
    loader->FreeGeometries(geometries);
    return 0;
}

int LoadShader(const char*filename, GLuint shaderID){
    ifstream file;
    file.open(filename, std::ios::in);
    if (!file) return -1;

    file.tellg();
    file.seekg(0, std::ios::end);
    int len = static_cast<int>(file.tellg());
    file.seekg(std::ios::beg);

    if (len == 0) return -1; //empty file

    GLchar *shaderSource = new GLchar[len + 1];
    shaderSource[len] = 0;

    int i = 0;
    while (file.good()) {
        shaderSource[i] = static_cast<GLchar>(file.get());
        if (!file.eof())
            i++;
    }

    shaderSource[i] = 0;
    file.close();

    glShaderSource(shaderID, 1, &shaderSource, NULL);
    glCompileShader(shaderID);

    GLint status;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);
    if (!status) {
        char buffer[512];
        glGetShaderInfoLog(shaderID, 512, NULL, buffer);
        printf("Shader Compile Failed. Info:\n\n%s\n", buffer);
        return -1;
    }
    return 0;
}
void CreateShaderProgram(GLuint *vertShader, GLuint &fragShader) {
    vertShader = glCreateShader(GL_VERTEX_SHADER);
    fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    LoadShader(vertfn, vertShader);
    LoadShader(fragfn, fragShader);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertShader);
    glAttachShader(shaderProgram, fragShader);
//     glBindFragDataLocation(shaderProgram, 0, "outColor");
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);


}