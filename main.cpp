#include <iostream>
#include <fstream>
#include "collada/colladaloader.h"
#include "glad/glad.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_opengl.h"

using namespace std;

#define GLM_FORCE_RADIANS

void CreateShaderProgram(GLuint &vertShader, GLuint &fragShader);

void CreateBuffers(Geometry *g);

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

GLuint shaderProgram;
GLuint vao, vbos;

const char *model = "../models/sphere.dae";
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
    ColladaLoader *loader = new ColladaLoader(model);
    Geometry *geometry = new Geometry();

    loader->ReadGeometry(geometry);

    CreateBuffers(geometry);

    glEnable(GL_DEPTH_TEST);

    bool quit = false;

    SDL_Event windowEvent;

    while(!quit){
        while(SDL_PollEvent(&windowEvent)){
            if(windowEvent.type == SDL_QUIT) quit = true;;
            if(windowEvent.type == SDL_KEYUP){
                switch(windowEvent.key.keysym.sym){
                    case SDLK_ESCAPE:
                    case SDLK_q:
                        quit = true;
                        break;
                    default:
                        break;
                }
            }
        }

        glClearColor(0.f, 0.f, 1.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        GLint unicolor = glGetUniformLocation(shaderProgram, "inColor");
        glm::vec3 colVec(0.f , 0.7f, 0.f);
        glUniform4fv(unicolor, 1, glm::value_ptr(colVec));

        glm::mat4 model;
        GLint uniModel = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

        glm::mat4 view = glm::lookAt(
                glm::vec3(0.0f, 0.0f, -5.0f),
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 1.0f, 0.0f));
        GLint uniView = glGetUniformLocation(shaderProgram, "view");
        glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

        glm::mat4 proj = glm::perspective(3.14f/4, aspect, 0.01f, 100.0f);
        GLint uniProj = glGetUniformLocation(shaderProgram, "proj");
        glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

        glUseProgram(shaderProgram);
        glBindVertexArray(vao);

        glDrawArrays(GL_TRIANGLES, 0, geometry->primitiveCount/3);

        SDL_GL_SwapWindow(window);

    }


    loader->FreeGeometry(geometry);
    return 0;
}

void CreateBuffers(Geometry *g) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbos);

    glBindVertexArray(vao);

    // bind position data
    glBindBuffer(GL_ARRAY_BUFFER, vbos);

    int size = g->position.size + g->normals.size;

    glBufferData(GL_ARRAY_BUFFER, size, g->data, GL_STATIC_DRAW);

    // Bind Position Data
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glVertexAttribPointer(posAttrib, g->position.stride, g->position.type, GL_FALSE, size, 0);
    glEnableVertexAttribArray(posAttrib);

    // bind Normal data
    GLint normAttrib = glGetAttribLocation(shaderProgram, "inNormal");
    glVertexAttribPointer(normAttrib, g->normals.stride, g->normals.type, GL_FALSE, size, (void *)g->position.size);
    glEnableVertexAttribArray(normAttrib);

    // TODO TEXTURE

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

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
void CreateShaderProgram(GLuint &vertShader, GLuint &fragShader) {
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