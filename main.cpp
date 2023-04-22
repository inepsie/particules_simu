
//#include <exception>
#include <fstream>
#include <glm/fwd.hpp>
#include <glm/matrix.hpp>
#include <iostream>
//#include <string>
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include <chrono>
#include <sstream>
#include <vector>
using namespace std;

GLFWwindow *window = NULL;

const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;
const int N = 70000;
//const int N = 90000;
const int LEVEL_MAX_MIPMAP = 9;

static GLuint _vert_shader, _geom_shader, _frag_shader, _cs_shader;
static GLuint _bloom_vert_shader, _bloom_frag_shader;
static GLuint _blending_vert_shader, _blending_frag_shader;
static GLuint _program, _cs_program, _bloom_program, _blending_program;
static GLuint _vao = 0;
static GLuint _buffer[3] = {0};
static GLuint _modelLoc, _viewLoc, _projLoc;
static GLuint _dt;
static GLuint _fbo = 0;
static GLuint _blur_fbo[2] = {0, 0};
static GLuint _rbo = 0;
static GLuint _bloom_tex[2] = {0, 0};
static GLuint _tex_color_buffer = 0;
static GLuint _tex_bright_buffer = 0;
static GLuint _attachements[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
static GLuint _quad = 0;
static GLuint _quad_vbo = 0;
static GLuint _quad_vao = 0;
static GLuint _MVP_id = 0;
static GLuint _inv_view_mat_id = 0;

std::string read(const char *filename) {
  std::stringbuf source;
  std::ifstream in(filename);
  // verifie que le fichier existe
  if (in.good() == false)
    // affiche une erreur, si le fichier n'existe pas ou n'est pas accessible
    cout << "[error] loading program : " << filename << endl;
  else
    printf("loading program '%s'...\n", filename);
  // lire le fichier, jusqu'au premier separateur,
  // le caractere '\0' ne peut pas se trouver dans un fichier texte, donc lit
  // tout le fichier d'un seul coup
  in.get(source, 0);
  // renvoyer la chaine de caracteres
  return source.str();
}

void printWorkGroupsCapabilities() {
  GLint workgroup_count[3] = {0};
  GLint workgroup_size[3];
  GLint workgroup_invocations;
  glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &workgroup_count[0]);
  glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &workgroup_count[1]);
  glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &workgroup_count[2]);
  printf("Taille maximale des workgroups:\n\tx:%u\n\ty:%u\n\tz:%u\n",
         workgroup_size[0], workgroup_size[1], workgroup_size[2]);
  glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &workgroup_size[0]);
  glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &workgroup_size[1]);
  glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &workgroup_size[2]);
  printf("Nombre maximal d'invocation locale:\n\tx:%u\n\ty:%u\n\tz:%u\n",
         workgroup_size[0], workgroup_size[1], workgroup_size[2]);
  glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &workgroup_invocations);
  printf("Nombre maximum d'invocation de workgroups:\n\t%u\n",
         workgroup_invocations);
}

void compil_shader(const char *filename, GLuint *id_shader,
                   GLuint shader_type) {
  GLint status;
  std::string source = read(filename);
  // cout << source << endl;
  const char *strings[] = {source.c_str()};
  switch (shader_type) {
  case 0:
    (*id_shader) = glCreateShader(GL_VERTEX_SHADER);
    break;
  case 1:
    (*id_shader) = glCreateShader(GL_GEOMETRY_SHADER);
    break;
  case 2:
    (*id_shader) = glCreateShader(GL_FRAGMENT_SHADER);
    break;
  case 3:
    (*id_shader) = glCreateShader(GL_COMPUTE_SHADER);
    break;
  default:
    break;
  }
  glShaderSource(*id_shader, 1, strings, NULL);
  glCompileShader(*id_shader);
  glGetShaderiv(*id_shader, GL_COMPILE_STATUS, &status);
  if (status == GL_TRUE) {
    cout << "compilation du program : " << filename << " -> ok" << endl;
  } else {
    cout << "*********************************** ERROR : erreur de compilation de : " << filename << endl;
    GLsizei length;
    glGetShaderiv(*id_shader, GL_INFO_LOG_LENGTH, &length);
    char *message = new char[1024];
    glGetShaderInfoLog(*id_shader, 1024, &length, message);
    cout << "erreurs de compilation :" << endl << message << endl;
    delete message;
  }
}

void check_attach_and_link(GLuint program) {
  GLint status;
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if (status == GL_TRUE) {
    cout << "Attach and Link program : ok " << endl;
  } else {
    // GLsizei length;
    GLchar errorLog[512];
    glGetProgramInfoLog(_program, 512, NULL, errorLog);
    cout << "*************************************** ERROR : Attach and link program" << errorLog << endl;
  }
}

void init() {
  GLfloat data_0[4 * N];
  GLfloat data_1[4 * N];
  GLfloat data_2[4 * N];
  for (int j = 0, k = 0; j < N; ++j) {
    data_0[k] = (float)rand() / (float)(RAND_MAX / 20.0) - 10.0;
    //data_1[k] = (float)rand() / (float)(RAND_MAX / 20.0) - 10.0;
    data_1[k] = 0.0f;
    data_2[k++] = (float)rand() / (float)(RAND_MAX / 20.0) - 10.0;
    data_0[k] = (float)rand() / (float)(RAND_MAX / 20.0) - 10.0;
    //data_1[k] = (float)rand() / (float)(RAND_MAX / 20.0) - 10.0;
    data_1[k] = 0.0f;
    data_2[k++] = (float)rand() / (float)(RAND_MAX / 20.0) - 10.0;
    data_0[k] = (float)rand() / (float)(RAND_MAX / 20.0) - 10.0;
    //data_1[k] = (float)rand() / (float)(RAND_MAX / 20.0) - 10.0;
    data_1[k] = 0.0f;
    data_2[k++] = (float)rand() / (float)(RAND_MAX / 20.0) - 10.0;
    data_0[k] = 0.0f;
    data_1[k] = 0.0f;
    data_2[k++] = 0.0f;
  }

  glGenFramebuffers(1, &_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

  glGenTextures(1, &_tex_color_buffer);
  glBindTexture(GL_TEXTURE_2D, _tex_color_buffer);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WINDOW_WIDTH, WINDOW_HEIGHT, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         _tex_color_buffer, 0);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    cout << "******************************************** ERROR fbo status is "
            "not ok"
         << endl;
    cout << glCheckFramebufferStatus(GL_FRAMEBUFFER) << endl;
  } else {
    cout << "fbo 1 status -> ok" << endl;
  }

  glGenTextures(1, &_tex_bright_buffer);
  glBindTexture(GL_TEXTURE_2D, _tex_bright_buffer);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, LEVEL_MAX_MIPMAP);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WINDOW_WIDTH, WINDOW_HEIGHT, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glGenerateMipmap(GL_TEXTURE_2D);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + 1, GL_TEXTURE_2D,
                         _tex_bright_buffer, 0);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    cout << "******************************************** ERROR fbo status is "
            "not ok"
         << endl;
    cout << glCheckFramebufferStatus(GL_FRAMEBUFFER) << endl;
  } else {
    cout << "fbo 1 status -> ok" << endl;
  }

  glDrawBuffers(2, _attachements);
  glGenRenderbuffers(1, &_rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, _rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WINDOW_WIDTH, WINDOW_HEIGHT);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbo);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
    cout << "************************************** ERROR::FRAMEBUFFER:: Framebuffer is not complete!"
              << endl;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glGenFramebuffers(2, _blur_fbo);
  glGenTextures(2, _bloom_tex);
  for (int j = 0; j < 2; ++j) {
    glBindFramebuffer(GL_FRAMEBUFFER, _blur_fbo[j]);
    glBindTexture(GL_TEXTURE_2D, _bloom_tex[j]);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 5);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WINDOW_WIDTH, WINDOW_HEIGHT, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _bloom_tex[j], 0);

    //glGenerateMipmap(GL_TEXTURE_2D);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      cout << "******************************************** ERROR fbo status is not ok" << endl;
      cout << glCheckFramebufferStatus(GL_FRAMEBUFFER) << endl;
    } else {
      cout << "fbo 1 status -> ok" << endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  //CS SHADER
  compil_shader("./shaders/basic.cs", &_cs_shader, 3);
  _cs_program = glCreateProgram();
  glAttachShader(_cs_program, _cs_shader);
  glLinkProgram(_cs_program);
  cout << "Check CS program :" << endl;
  check_attach_and_link(_cs_program);
  _dt = glGetUniformLocation(_cs_program, "dt");

  //RENDER SHADERS
  compil_shader("./shaders/basic.vert", &_vert_shader, 0);
  compil_shader("./shaders/basic.geom", &_geom_shader, 1);
  compil_shader("./shaders/basic.frag", &_frag_shader, 2);
  _program = glCreateProgram();
  glAttachShader(_program, _vert_shader);
  glAttachShader(_program, _geom_shader);
  glAttachShader(_program, _frag_shader);
  glLinkProgram(_program);
  check_attach_and_link(_program);

  //BLOOM SHADERS
  compil_shader("./shaders/bloom.vert", &_bloom_vert_shader, 0);
  compil_shader("./shaders/bloom.frag", &_bloom_frag_shader, 2);
  _bloom_program = glCreateProgram();
  glAttachShader(_bloom_program, _bloom_vert_shader);
  glAttachShader(_bloom_program, _bloom_frag_shader);
  glLinkProgram(_bloom_program);
  check_attach_and_link(_bloom_program);

  //BLENDING SHADERS
  compil_shader("./shaders/blending.vert", &_blending_vert_shader, 0);
  compil_shader("./shaders/blending.frag", &_blending_frag_shader, 2);
  _blending_program = glCreateProgram();
  glAttachShader(_blending_program, _blending_vert_shader);
  glAttachShader(_blending_program, _blending_frag_shader);
  glLinkProgram(_blending_program);
  check_attach_and_link(_blending_program);

  //BLOOM
  GLfloat quad[] = {
  -1.0f, 1.0f, 0.0f, 1.0f,//
  -1.0f, -1.0f, 0.0f, 0.0f,//
  1.0f, -1.0f, 1.0f,  0.0f,//
  -1.0f, 1.0f, 0.0f, 1.0f,//
  1.0f,  -1.0f, 1.0f,  0.0f,//
  1.0f, 1.0f,  1.0f,  1.0f//
};
  glGenBuffers(1, &_quad_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, _quad_vbo);
  glGenVertexArrays(1, &_quad_vao);
  glBindVertexArray(_quad_vao);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad), &quad, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));

  // BUFFER 0 : vertex position de base
  glGenBuffers(3, _buffer);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, _buffer[0]);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof data_0, data_0, GL_STATIC_DRAW);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, _buffer[1]);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof data_1, data_1, GL_STATIC_DRAW);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, _buffer[2]);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof data_2, data_2, GL_STATIC_DRAW);
  // VAO PARTICULES
  glGenVertexArrays(1, &_vao);
  glBindVertexArray(_vao);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, _buffer[0]);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof *data_0,
                        (const void *)0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);


}

void draw(float dt) {
  int i=0;
  int horizontal = 0;
  int first_pass = 1;
  GLuint in = _tex_bright_buffer;
  GLfloat pas[2] = {1.0f / WINDOW_WIDTH, 1.0f / WINDOW_HEIGHT};
  static GLfloat angle = 0.0f;
  static GLfloat campos[3] = {0.0f, 9.0f, 40.0f};
  angle = 0.1 * dt;
  campos[0] = 100.0f * -cos(angle);
  campos[2] = 100.0f * -sin(angle);
  //campos[0] = 0.0f;
  //campos[2] = -100.0f;

  // Compute Shader
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _buffer[0]);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _buffer[1]);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, _buffer[2]);
  glUseProgram(_cs_program);
  glUniform1f(_dt, dt);
  glDispatchCompute(N, 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  glUseProgram(0);

  // Premier rendu
  glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glUseProgram(_program);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, in);

  glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
  glClearColor(0.0f, 0.001f, 0.0f, 1.0f);

  glm::mat4 proj = glm::perspective(glm::radians(45.0f),
                                    (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT,
                                    0.1f, 100.0f);
  glm::mat4 view = glm::lookAt(
      glm::vec3(campos[0],
                campos[1],
                campos[2]), // Camera is at (4,3,3), in World Space
      glm::vec3(0, 0, 0),   // and looks at the origin
      glm::vec3(0, 1, 0)    // Head is up (set to 0,-1,0 to look upside-down)
  );
  glm::mat4 inv_view = glm::inverse(view);
  glm::mat4 model = glm::mat4(1.0f);
  glm::mat4 mvp =
      proj * view *
      model; // Remember, matrix multiplication is the other way around
  _MVP_id = glGetUniformLocation(_program, "MVP");
  glUniformMatrix4fv(_MVP_id, 1, GL_FALSE, &mvp[0][0]);
  _inv_view_mat_id = glGetUniformLocation(_program, "inv_view_mat");
  glUniformMatrix4fv(_inv_view_mat_id, 1, GL_FALSE, &inv_view[0][0]);

  //On effectue le rendu dans un framebuffer à part
  glBindBuffer(GL_ARRAY_BUFFER, _buffer[0]);
  glBindVertexArray(_vao);
  glDrawArrays(GL_POINTS, 0, N);
  glBindVertexArray(0);
  glUseProgram(0);

  do {
    glBindFramebuffer(GL_FRAMEBUFFER, _blur_fbo[i%2]);
    glGenerateMipmap(GL_TEXTURE_2D);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(_bloom_program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, in);
    glUniform1i(glGetUniformLocation(_bloom_program, "tex"), 0);
    glUniform2fv(glGetUniformLocation(_bloom_program, "pas"), 1, pas);
    glUniform1i(glGetUniformLocation(_bloom_program, "horizontal"), horizontal);
    glUniform1i(glGetUniformLocation(_bloom_program, "first_pass"), first_pass);
    glUniform1i(glGetUniformLocation(_bloom_program, "level_max"), LEVEL_MAX_MIPMAP);
    glBindVertexArray(_quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
    in = _bloom_tex[(i % 2)];
    ++i;
    horizontal = (horizontal + 1) % 2;
    first_pass = 0;
  } while (i < 80);

  //BLENDING
  //glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
  //glClear(GL_COLOR_BUFFER_BIT);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(_blending_program);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _tex_color_buffer);
  glUniform1i(glGetUniformLocation(_blending_program, "scene"), 0);
  glActiveTexture(GL_TEXTURE0 + 1);
  glBindTexture(GL_TEXTURE_2D, _bloom_tex[0]);
  glUniform1i(glGetUniformLocation(_blending_program, "bloom"), 1);
  glBindVertexArray(_quad_vao);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glUseProgram(0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
                    0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void quit() {
  glDeleteShader(_vert_shader);
  glDeleteShader(_geom_shader);
  glDeleteShader(_frag_shader);
}

int main() {
  // Init GLFW (need to do this first)
  glfwInit();
  // Use OpenGL 4.3 Core
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  // Create window and context
  window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
                            "Particules test 01", NULL, NULL);
  glfwShowWindow(window);
  glfwMakeContextCurrent(window);

  // Initialise GLEW (needs to happen after we have a context)
  if (glewInit() != GLEW_OK) {
    throw std::runtime_error("Failed to initialise GLEW");
  }
  // Affichage des dimensions des groupes de travail GPU
  printWorkGroupsCapabilities();
  init();
  auto lastTime = std::chrono::high_resolution_clock::now();
  do {
    // Clear the screen. It's not mentioned before Tutorial 02, but it can cause
    // flickering, so it's there nonetheless.
    auto currentTime = std::chrono::high_resolution_clock::now();
    float dt = std::chrono::duration<float>(currentTime - lastTime).count();
    draw(dt);
    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
  } // Check if the ESC key was pressed or the window was closed
  while (glfwGetKey(window, GLFW_KEY_L) != GLFW_PRESS &&
         glfwWindowShouldClose(window) == 0);

  quit();
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
