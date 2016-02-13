#include "custom.h"
#include <AL/al.h>
#include <AL/alc.h>

#ifdef LIBAUDIO
#include <audio/wave.h>
#define BACKEND "libaudio"
#else
#include <AL/alut.h>
#define BACKEND "alut"
#endif

static void list_audio_devices(const ALCchar *devices)
{
  const ALCchar *device = devices, *next = devices + 1;
  size_t len = 0;

  fprintf(stdout, "Devices list:\n");
  fprintf(stdout, "----------\n");
  while (device && *device != '\0' && next && *next != '\0') {
    fprintf(stdout, "%s\n", device);
    len = strlen(device);
    device += (len + 1);
    next += (len + 2);
  }
  fprintf(stdout, "----------\n");
}

#define TEST_ERROR(_msg)    \
  error = alGetError();   \
  if (error != AL_NO_ERROR) { \
    fprintf(stderr, _msg "\n"); \
    return -1;    \
  }



static inline ALenum to_al_format(short channels, short samples)
{
  bool stereo = (channels > 1);

  switch (samples) {
  case 16:
    if (stereo)
      return AL_FORMAT_STEREO16;
    else
      return AL_FORMAT_MONO16;
  case 8:
    if (stereo)
      return AL_FORMAT_STEREO8;
    else
      return AL_FORMAT_MONO8;
  default:
    return -1;
  }
}


/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

  // Create the shaders
  GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

  // Read the Vertex Shader code from the file
  std::string VertexShaderCode;
  std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
  if(VertexShaderStream.is_open())
  {
    std::string Line = "";
    while(getline(VertexShaderStream, Line))
      VertexShaderCode += "\n" + Line;
    VertexShaderStream.close();
  }

  // Read the Fragment Shader code from the file
  std::string FragmentShaderCode;
  std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
  if(FragmentShaderStream.is_open()){
    std::string Line = "";
    while(getline(FragmentShaderStream, Line))
      FragmentShaderCode += "\n" + Line;
    FragmentShaderStream.close();
  }

  GLint Result = GL_FALSE;
  int InfoLogLength;

  // Compile Vertex Shader
  cout << "Compiling shader : " <<  vertex_file_path << endl;
  char const * VertexSourcePointer = VertexShaderCode.c_str();
  glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
  glCompileShader(VertexShaderID);

  // Check Vertex Shader
  glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> VertexShaderErrorMessage( max(InfoLogLength, int(1)) );
  glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
  cout << VertexShaderErrorMessage.data() << endl;

  // Compile Fragment Shader
  cout << "Compiling shader : " << fragment_file_path << endl;
  char const * FragmentSourcePointer = FragmentShaderCode.c_str();
  glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
  glCompileShader(FragmentShaderID);

  // Check Fragment Shader
  glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> FragmentShaderErrorMessage( max(InfoLogLength, int(1)) );
  glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
  cout << FragmentShaderErrorMessage.data() << endl;

  // Link the program
  cout << "Linking program" << endl;
  GLuint ProgramID = glCreateProgram();
  glAttachShader(ProgramID, VertexShaderID);
  glAttachShader(ProgramID, FragmentShaderID);
  glLinkProgram(ProgramID);

  // Check the program
  glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
  glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
  glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
  cout << ProgramErrorMessage.data() << endl;

  glDeleteShader(VertexShaderID);
  glDeleteShader(FragmentShaderID);

  return ProgramID;
}

static void error_callback(int error, const char* description)
{
  cout << "Error: " << description << endl;
}

void quit(GLFWwindow *window)
{
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}

glm::vec3 getRGBfromHue (int hue)
{
  float intp;
  float fracp = modff(hue/60.0, &intp);
  float x = 1.0 - abs((float)((int)intp%2)+fracp-1.0);

  if (hue < 60)
    return glm::vec3(1,x,0);
  else if (hue < 120)
    return glm::vec3(x,1,0);
  else if (hue < 180)
    return glm::vec3(0,1,x);
  else if (hue < 240)
    return glm::vec3(0,x,1);
  else if (hue < 300)
    return glm::vec3(x,0,1);
  else
    return glm::vec3(1,0,x);
}

/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
  struct VAO* vao = new struct VAO;
  vao->PrimitiveMode = primitive_mode;
  vao->NumVertices = numVertices;
  vao->FillMode = fill_mode;

  // Create Vertex Array Object
  // Should be done after CreateWindow and before any other GL calls
  glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
  glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
  glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

  glBindVertexArray (vao->VertexArrayID); // Bind the VAO
  glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
  glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
  glVertexAttribPointer(
      0,                  // attribute 0. Vertices
      3,                  // size (x,y,z)
      GL_FLOAT,           // type
      GL_FALSE,           // normalized?
      0,                  // stride
      (void*)0            // array buffer offset
      );

  glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors
  glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
  glVertexAttribPointer(
      1,                  // attribute 1. Color
      3,                  // size (r,g,b)
      GL_FLOAT,           // type
      GL_FALSE,           // normalized?
      0,                  // stride
      (void*)0            // array buffer offset
      );

  return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
  GLfloat* color_buffer_data = new GLfloat [3*numVertices];
  for (int i=0; i<numVertices; i++) {
    color_buffer_data [3*i] = red;
    color_buffer_data [3*i + 1] = green;
    color_buffer_data [3*i + 2] = blue;
  }

  return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

struct VAO* create3DTexturedObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* texture_buffer_data, GLuint textureID, GLenum fill_mode=GL_FILL)
{
  struct VAO* vao = new struct VAO;
  vao->PrimitiveMode = primitive_mode;
  vao->NumVertices = numVertices;
  vao->FillMode = fill_mode;
  vao->TextureID = textureID;

  // Create Vertex Array Object
  // Should be done after CreateWindow and before any other GL calls
  glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
  glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
  glGenBuffers (1, &(vao->TextureBuffer));  // VBO - textures

  glBindVertexArray (vao->VertexArrayID); // Bind the VAO
  glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
  glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
  glVertexAttribPointer(
      0,                  // attribute 0. Vertices
      3,                  // size (x,y,z)
      GL_FLOAT,           // type
      GL_FALSE,           // normalized?
      0,                  // stride
      (void*)0            // array buffer offset
      );

  glBindBuffer (GL_ARRAY_BUFFER, vao->TextureBuffer); // Bind the VBO textures
  glBufferData (GL_ARRAY_BUFFER, 2*numVertices*sizeof(GLfloat), texture_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
  glVertexAttribPointer(
      2,                  // attribute 2. Textures
      2,                  // size (s,t)
      GL_FLOAT,           // type
      GL_FALSE,           // normalized?
      0,                  // stride
      (void*)0            // array buffer offset
      );

  return vao;
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
  // Change the Fill Mode for this object
  glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

  // Bind the VAO to use
  glBindVertexArray (vao->VertexArrayID);

  // Enable Vertex Attribute 0 - 3d Vertices
  glEnableVertexAttribArray(0);
  // Bind the VBO to use
  glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

  // Enable Vertex Attribute 1 - Color
  glEnableVertexAttribArray(1);
  // Bind the VBO to use
  glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

  // Draw the geometry !
  glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

void draw3DTexturedObject (struct VAO* vao)
{
  // Change the Fill Mode for this object
  glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

  // Bind the VAO to use
  glBindVertexArray (vao->VertexArrayID);

  // Enable Vertex Attribute 0 - 3d Vertices
  glEnableVertexAttribArray(0);
  // Bind the VBO to use
  glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

  // Bind Textures using texture units
  glBindTexture(GL_TEXTURE_2D, vao->TextureID);

  // Enable Vertex Attribute 2 - Texture
  glEnableVertexAttribArray(2);
  // Bind the VBO to use
  glBindBuffer(GL_ARRAY_BUFFER, vao->TextureBuffer);

  // Draw the geometry !
  glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle

  // Unbind Textures to be safe
  glBindTexture(GL_TEXTURE_2D, 0);
}



/* Create an OpenGL Texture from an image */
GLuint createTexture (const char* filename)
{
  GLuint TextureID;
  // Generate Texture Buffer
  glGenTextures(1, &TextureID);
  // All upcoming GL_TEXTURE_2D operations now have effect on our texture buffer
  glBindTexture(GL_TEXTURE_2D, TextureID);
  // Set our texture parameters
  // Set texture wrapping to GL_REPEAT
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // Set texture filtering (interpolation)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  // Load image and create OpenGL texture
  int twidth, theight;
  unsigned char* image = SOIL_load_image(filename, &twidth, &theight, 0, SOIL_LOAD_RGB);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, twidth, theight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
  glGenerateMipmap(GL_TEXTURE_2D); // Generate MipMaps to use
  SOIL_free_image_data(image); // Free the data read from file after creating opengl texture
  glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess it up

  return TextureID;
}

void scrollCallback(GLFWwindow* window, double x, double y) {
  if(y > 0) zoom_flag -= 0.5;
  else if(y < 0) zoom_flag += 0.5;
}


void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
  // Function is called first on GLFW_PRESS.

  if (action == GLFW_RELEASE) {
    if(key == GLFW_KEY_R) {
      viewPtr[currentView] = (viewPtr[currentView]+1) % numSubViews[currentView]; 
    }
    if(key == GLFW_KEY_L) lightOn = !lightOn;
    if(key == GLFW_KEY_UP) {
      if(currentView == 2 or currentView == 3) {

      if(playerDirection == 1) playerMoveUp = false;
      else if(playerDirection == 2) playerMoveDown = false;
      else if(playerDirection == 3) playerMoveRight = false;
      else if(playerDirection == 4) playerMoveLeft = false;
    }
    else playerMoveUp = false;

    }
    else if(key == GLFW_KEY_DOWN) {
      if(currentView == 2 or currentView == 3) {

       if(playerDirection == 1) playerMoveUp = false;
      else if(playerDirection == 2) playerMoveDown = false;
      else if(playerDirection == 3) playerMoveRight = false;
      else if(playerDirection == 4) playerMoveLeft = false;
    }
  else playerMoveDown = false;
          }
    else if(key == GLFW_KEY_RIGHT) {
      if(currentView == 2 or currentView == 3) {

       if(playerDirection == 1) playerMoveUp = false;
      else if(playerDirection == 2) playerMoveDown = false;
      else if(playerDirection == 3) playerMoveRight = false;
      else if(playerDirection == 4) playerMoveLeft = false;
    }
    else playerMoveRight = false;
    }
    else if(key == GLFW_KEY_LEFT) {
      if(currentView == 2 or currentView == 3) {

       if(playerDirection == 1) playerMoveUp = false;
      else if(playerDirection == 2) playerMoveDown = false;
      else if(playerDirection == 3) playerMoveRight = false;
      else if(playerDirection == 4) playerMoveLeft = false;
    }
    else playerMoveLeft = false;
    }
    else if(key == GLFW_KEY_F && speed<7) speed += 1;
    else if(key == GLFW_KEY_S && speed>0) speed -= 1;
   
  }
  else if (action == GLFW_PRESS) {
     switch (key) {
      case GLFW_KEY_ESCAPE:
        quit(window);
        break;
      default:
        break;
    }
    if(key == GLFW_KEY_V) {
      currentView = (currentView + 1) % numViews;
    }
    if(playerFall or playerJumpUp or playerJumpDown or playerJumpRight or playerJumpLeft or playerWin) return;
     if(key == GLFW_KEY_UP && glfwGetKey(window, GLFW_KEY_SPACE) && !playerFall && !playerJumpUp && !playerJumpDown && !playerJumpRight && !playerJumpLeft) {
      prevPlayerX = playerX;
      prevPlayerZ = playerZ;
       if(currentView == 2 or currentView == 3) {
        if(playerDirection == 1) playerJumpUp = true;
      else if(playerDirection == 3) playerJumpRight = true;
      else if(playerDirection == 2) playerJumpDown = true;
      else if(playerDirection == 4) playerJumpLeft = true;
    }
    else playerJumpUp = true, playerDirection = 1;
    }
    else if(key == GLFW_KEY_DOWN && glfwGetKey(window, GLFW_KEY_SPACE) && !playerFall && !playerJumpUp && !playerJumpDown && !playerJumpRight && !playerJumpLeft) {
      prevPlayerX = playerX;
      prevPlayerZ = playerZ;     
       if(currentView == 2 or currentView == 3) {

      if(playerDirection == 1) playerJumpDown = true, playerDirection = 2;
      else if(playerDirection == 3) playerJumpLeft = true, playerDirection = 4;
      else if(playerDirection == 4) playerJumpRight = true, playerDirection = 3;
      else if(playerDirection == 2) playerJumpUp = true, playerDirection = 1;
    }
    else playerJumpDown = true, playerDirection = 2;
    }
    else if(key == GLFW_KEY_RIGHT && glfwGetKey(window, GLFW_KEY_SPACE) && !playerFall && !playerJumpUp && !playerJumpDown && !playerJumpRight && !playerJumpLeft) {
      prevPlayerX = playerX;
      prevPlayerZ = playerZ; 
       if(currentView == 2 or currentView == 3) {

       if(playerDirection == 3) playerJumpDown = true, playerDirection = 2;
      else if(playerDirection == 1) playerJumpRight = true, playerDirection = 3;
      else if(playerDirection == 4) playerJumpUp = true, playerDirection = 1;
      else if(playerDirection == 2) playerJumpLeft = true, playerDirection = 4;    
    }
    else playerJumpRight = true, playerDirection = 3;

    }
    else if(key == GLFW_KEY_LEFT && glfwGetKey(window, GLFW_KEY_SPACE) && !playerFall && !playerJumpUp && !playerJumpDown && !playerJumpRight && !playerJumpLeft) {
      prevPlayerX = playerX;
      prevPlayerZ = playerZ; 
      if(currentView == 2 or currentView == 3) {

       if(playerDirection == 1) playerJumpLeft = true, playerDirection = 4;
      else if(playerDirection == 2) playerJumpRight = true, playerDirection = 3;
      else if(playerDirection == 3) playerJumpUp = true, playerDirection = 1;
      else if(playerDirection == 4) playerJumpDown = true, playerDirection = 2;    
    }
    else playerJumpLeft = true, playerDirection = 4;
    }
    else if(key == GLFW_KEY_UP) {
      if(currentView == 2 or currentView == 3) {
        if(playerDirection == 1) playerMoveUp = true;
      else if(playerDirection == 3) playerMoveRight = true;
      else if(playerDirection == 2) playerMoveDown = true;
      else if(playerDirection == 4) playerMoveLeft = true;
    }
    else playerMoveUp = true, playerDirection = 1;
    }
    else if(key == GLFW_KEY_DOWN) {
      if(currentView == 2 or currentView == 3) {

      if(playerDirection == 1) playerMoveDown = true, playerDirection = 2;
      else if(playerDirection == 3) playerMoveLeft = true, playerDirection = 4;
      else if(playerDirection == 4) playerMoveRight = true, playerDirection = 3;
      else if(playerDirection == 2) playerMoveUp = true, playerDirection = 1;
    }
    else playerMoveDown = true, playerDirection = 2;

    }
    else if(key == GLFW_KEY_RIGHT) {
      if(currentView == 2 or currentView == 3) {

       if(playerDirection == 3) playerMoveDown = true, playerDirection = 2;
      else if(playerDirection == 1) playerMoveRight = true, playerDirection = 3;
      else if(playerDirection == 4) playerMoveUp = true, playerDirection = 1;
      else if(playerDirection == 2) playerMoveLeft = true, playerDirection = 4;    
    }
    else playerMoveRight = true, playerDirection = 3;

    }
    else if(key == GLFW_KEY_LEFT) {
      if(currentView == 2 or currentView == 3) {

       if(playerDirection == 1) playerMoveLeft = true, playerDirection = 4;
      else if(playerDirection == 2) playerMoveRight = true, playerDirection = 3;
      else if(playerDirection == 3) playerMoveUp = true, playerDirection = 1;
      else if(playerDirection == 4) playerMoveDown = true, playerDirection = 2;    
    }
    else playerMoveLeft = true, playerDirection = 4;

    }   
  }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
  switch (key) {
    case 'Q':
    case 'q':
      quit(window);
      break;
    default:
      break;
  }
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
  switch (button) {
    case GLFW_MOUSE_BUTTON_LEFT:
      if (action == GLFW_PRESS) {
                glfwGetCursorPos(window, &xpos, &ypos);
        prevXpos = xpos;
        prevYpos = ypos;
      }


      if (action == GLFW_RELEASE) {

        glfwGetCursorPos(window, &xpos, &ypos);
        if(xpos > prevXpos) {
          viewPtr[currentView]--;
          if(viewPtr[currentView] < 0) viewPtr[currentView] = numSubViews[currentView] - 1;
         
        }
        else if(xpos < prevXpos) {
      viewPtr[currentView] = (viewPtr[currentView] + 1) % numSubViews[currentView]; 
  
        }

      }
      break;
    default:
      break;
  }
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
  int fbwidth=width, fbheight=height;
  /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
  glfwGetFramebufferSize(window, &fbwidth, &fbheight);

  GLfloat fov = 45.0f;

  // sets the viewport of openGL renderer
  glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

  // set the projection matrix as perspective
  /* glMatrixMode (GL_PROJECTION);
     glLoadIdentity ();
     gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
  // Store the projection matrix in a variable for future use
  // Perspective projection for 3D views
  Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

  // Ortho projection for 2D views
  //Matrices.projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 500.0f);
}


GLFWwindow* initGLFW (int width, int height)
{
  GLFWwindow* window; // window desciptor/handle

  glfwSetErrorCallback(error_callback);
  if (!glfwInit()) {
    exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(width, height, "Valar Morghulis", NULL, NULL);

  if (!window) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);
  gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
  glfwSwapInterval( 1 );

  /* --- register callbacks with GLFW --- */

  /* Register function to handle window resizes */
  /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
  glfwSetFramebufferSizeCallback(window, reshapeWindow);
  glfwSetWindowSizeCallback(window, reshapeWindow);

  /* Register function to handle window close */
  glfwSetWindowCloseCallback(window, quit);

  /* Register function to handle keyboard input */
  glfwSetKeyCallback(window, keyboard);      // general keyboard input
  glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

  /* Register function to handle mouse click */
  glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
  glfwSetScrollCallback(window, scrollCallback);

  return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */


void createShapes() {

  static const GLfloat vertex_buffer_data_block [] = {
    -1.0f,-3.0f,-1.0f, // triangle 1 : begin
    -1.0f,-3.0f, 1.0f,
    -1.0f, 3.0f, 1.0f, // triangle 1 : end
    1.0f, 3.0f,-1.0f, // triangle 2 : begin
    -1.0f,-3.0f,-1.0f,
    -1.0f, 3.0f,-1.0f, // triangle 2 : end
    1.0f,-3.0f, 1.0f,
    -1.0f,-3.0f,-1.0f,
    1.0f,-3.0f,-1.0f,
    1.0f, 3.0f,-1.0f,
    1.0f,-3.0f,-1.0f,
    -1.0f,-3.0f,-1.0f,
    -1.0f,-3.0f,-1.0f,
    -1.0f, 3.0f, 1.0f,
    -1.0f, 3.0f,-1.0f,
    1.0f,-3.0f, 1.0f,
    -1.0f,-3.0f, 1.0f,
    -1.0f,-3.0f,-1.0f,
    -1.0f, 3.0f, 1.0f,
    -1.0f,-3.0f, 1.0f,
    1.0f,-3.0f, 1.0f,
    1.0f, 3.0f, 1.0f,
    1.0f,-3.0f,-1.0f,
    1.0f, 3.0f,-1.0f,
    1.0f,-3.0f,-1.0f,
    1.0f, 3.0f, 1.0f,
    1.0f,-3.0f, 1.0f,
    1.0f, 3.0f, 1.0f,
    1.0f, 3.0f,-1.0f,
    -1.0f,3.0f,-1.0f,
    1.0f, 3.0f, 1.0f,
    -1.0f, 3.0f,-1.0f,
    -1.0f, 3.0f, 1.0f,
    1.0f, 3.0f, 1.0f,
    -1.0f, 3.0f, 1.0f,
    1.0f,-3.0f, 1.0f
  };

  float l, b, h;
  l = 18;
  b = 1;
  h = 18;
  static const GLfloat vertex_buffer_data_water [] = {
    -l,-b,-h, // triangle 1 : begin
    -l,-b, h,
    -l, b, h, // triangle 1 : end
    l, b,-h, // triangle 2 : begin
    -l,-b,-h,
    -l, b,-h, // triangle 2 : end
    l,-b, h,
    -l,-b,-h,
    l,-b,-h,
    l, b,-h,
    l,-b,-h,
    -l,-b,-h,
    -l,-b,-h,
    -l, b, h,
    -l, b,-h,
    l,-b, h,
    -l,-b, h,
    -l,-b,-h,
    -l, b, h,
    -l,-b, h,
    l,-b, h,
    l, b, h,
    l,-b,-h,
    l, b,-h,
    l,-b,-h,
    l, b, h,
    l,-b, h,
    l, b, h,
    l, b,-h,
    -l,b,-h,
    l, b, h,
    -l, b,-h,
    -l, b, h,
    l, b, h,
    -l, b, h,
    l,-b, h
  };
    l = 0.2;
  b = 0.9;
  h = 0.2;
  static const GLfloat vertex_buffer_data_limbs [] = {
    -l,-b,-h, // triangle 1 : begin
    -l,-b, h,
    -l, b, h, // triangle 1 : end
    l, b,-h, // triangle 2 : begin
    -l,-b,-h,
    -l, b,-h, // triangle 2 : end
    l,-b, h,
    -l,-b,-h,
    l,-b,-h,
    l, b,-h,
    l,-b,-h,
    -l,-b,-h,
    -l,-b,-h,
    -l, b, h,
    -l, b,-h,
    l,-b, h,
    -l,-b, h,
    -l,-b,-h,
    -l, b, h,
    -l,-b, h,
    l,-b, h,
    l, b, h,
    l,-b,-h,
    l, b,-h,
    l,-b,-h,
    l, b, h,
    l,-b, h,
    l, b, h,
    l, b,-h,
    -l,b,-h,
    l, b, h,
    -l, b,-h,
    -l, b, h,
    l, b, h,
    -l, b, h,
    l,-b, h
  };

  static const GLfloat color_buffer_data_block[] = {
    0.583f,  0.771f,  0.014f,
    0.609f,  0.115f,  0.436f,
    0.327f,  0.483f,  0.844f,
    0.822f,  0.569f,  0.201f,
    0.435f,  0.602f,  0.223f,
    0.310f,  0.747f,  0.185f,
    0.597f,  0.770f,  0.761f,
    0.559f,  0.436f,  0.730f,
    0.359f,  0.583f,  0.152f,
    0.483f,  0.596f,  0.789f,
    0.559f,  0.861f,  0.639f,
    0.195f,  0.548f,  0.859f,
    0.014f,  0.184f,  0.576f,
    0.771f,  0.328f,  0.970f,
    0.406f,  0.615f,  0.116f,
    0.676f,  0.977f,  0.133f,
    0.971f,  0.572f,  0.833f,
    0.140f,  0.616f,  0.489f,
    0.997f,  0.513f,  0.064f,
    0.945f,  0.719f,  0.592f,
    0.543f,  0.021f,  0.978f,
    0.279f,  0.317f,  0.505f,
    0.167f,  0.620f,  0.077f,
    0.347f,  0.857f,  0.137f,
    0.055f,  0.953f,  0.042f,
    0.714f,  0.505f,  0.345f,
    0.783f,  0.290f,  0.734f,
    0.722f,  0.645f,  0.174f,
    0.302f,  0.455f,  0.848f,
    0.225f,  0.587f,  0.040f,
    0.517f,  0.713f,  0.338f,
    0.053f,  0.959f,  0.120f,
    0.393f,  0.621f,  0.362f,
    0.673f,  0.211f,  0.457f,
    0.820f,  0.883f,  0.371f,
    0.982f,  0.099f,  0.879f
  };

  float r = 0.5, t = 1;
  static const GLfloat vertex_buffer_data_coin [] = {
    0,t,0,
    -r,0,r,
    r,0,r,
    0,t,0,
    r,0,r,
    r,0,-r,
    0,t,0,
    r,0,-r,
    -r,0,-r,
    0,r,0,
    -r,0,-r,
    -r,0,r,
    0,-t,0,
    -r,0,r,
    r,0,r,
    0,-t,0,
    r,0,r,
    r,0,-r,
    0,-t,0,
    r,0,-r,
    -r,0,-r,
    0,-t,0,
    -r,0,-r,
    -r,0,r,
  };

  r = 0.9;

  static const GLfloat vertex_buffer_data_spikes [] = {
    0,r,0,
    -r,0,r,
    r,0,r,
    0,r,0,
    r,0,r,
    r,0,-r,
    0,r,0,
    r,0,-r,
    -r,0,-r,
    0,r,0,
    -r,0,-r,
    -r,0,r,
    0,-r,0,
    -r,0,r,
    r,0,r,
    0,-r,0,
    r,0,r,
    r,0,-r,
    0,-r,0,
    r,0,-r,
    -r,0,-r,
    0,-r,0,
    -r,0,-r,
    -r,0,r,
  };

  static GLfloat color_buffer_data_coin[24*3];
  for(int i=0; i<4*3; i+=3) {
    color_buffer_data_coin[i] = 1;
    color_buffer_data_coin[i+1] = 1;
    color_buffer_data_coin[i+2] = 1;

  }

  static GLfloat color_buffer_data_spikes[24*3];
  for(int i=0; i<4*3; i+=3) {
    color_buffer_data_spikes[i] = 0;
    color_buffer_data_spikes[i+1] = 0;
    color_buffer_data_spikes[i+2] = 0;

  }

  /*  float cs = 1.5*cos(M_PI/6), sn = 1.5*sin(M_PI/6);

      static const GLfloat vertex_buffer_data5 [] = {
      0,0,0,
      cs,sn,0,
      cs,-sn,0,
      0,0,0,
      sn,cs,0,
      -sn,cs,0,
      0,0,0,
      -cs,sn,0,
      -cs,-sn,0,
      0,0,0,
      sn,-cs,0,
      -sn, -cs,0
      };

      static GLfloat color_buffer_data5[12*3];
      for(int i=0; i<12*3; i+=3) {
      color_buffer_data5[i] = 0.0/255.0;
      color_buffer_data5[i+1] = 102.0/255.0;
      color_buffer_data5[i+2] = 102.0/255.0;

      }*/
      l = 0.5, b = 1, h = 0.5;
  static const GLfloat vertex_buffer_data_player [] = {
    -l,-b,-h, // triangle 1 : begin
    -l,-b, h,
    -l, b, h, // triangle 1 : end
    l, b,-h, // triangle 2 : begin
    -l,-b,-h,
    -l, b,-h, // triangle 2 : end
    l,-b, h,
    -l,-b,-h,
    l,-b,-h,
    l, b,-h,
    l,-b,-h,
    -l,-b,-h,
    -l,-b,-h,
    -l, b, h,
    -l, b,-h,
    l,-b, h,
    -l,-b, h,
    -l,-b,-h,
    -l, b, h,
    -l,-b, h,
    l,-b, h,
    l, b, h,
    l,-b,-h,
    l, b,-h,
    l,-b,-h,
    l, b, h,
    l,-b, h,
    l, b, h,
    l, b,-h,
    -l,b,-h,
    l, b, h,
    -l, b,-h,
    -l, b, h,
    l, b, h,
    -l, b, h,
    l,-b, h
  };

  static GLfloat color_buffer_data_player[36*3];
  for(int i=0; i<36*3; i+=3) {
    color_buffer_data_player[i] = 0;
        color_buffer_data_player[i+1] = 0;
    color_buffer_data_player[i+2] = 0;
}
  
    static GLfloat color_buffer_data_limbs[36*3];
  for(int i=0; i<36*3; i+=3) {
    color_buffer_data_limbs[i] = 1;
    color_buffer_data_limbs[i+1] =0;
    color_buffer_data_limbs[i+2] = 0;

  }
    static GLfloat color_buffer_data_temp[36*3];
  for(int i=0; i<36*3; i+=3) {
    color_buffer_data_temp[i] = 0;
    color_buffer_data_temp[i+1] =0;
    color_buffer_data_temp[i+2] =0;

  }
  cube = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data_block, color_buffer_data_temp, GL_FILL);
  cube2 = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data_block, color_buffer_data_temp, GL_LINE);

  coin = create3DObject(GL_TRIANGLES, 24, vertex_buffer_data_coin, color_buffer_data_coin, GL_FILL);
  spikes = create3DObject(GL_TRIANGLES, 24, vertex_buffer_data_spikes, color_buffer_data_spikes, GL_FILL);
  player = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data_player, color_buffer_data_player, GL_FILL);
  limbs = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data_limbs, color_buffer_data_limbs, GL_FILL);
}

void createRectangle ()
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -0.5,-0.5,0, // vertex 1
    0.5,-0.5,0, // vertex 2
    0.5, 0.5,0, // vertex 3

    0.5, 0.5,0, // vertex 3
    -0.5, 0.5,0, // vertex 4
    -0.5,-0.5,0  // vertex 1
  };
  static const GLfloat vertex_buffer_data2 [] = {
    -0.35,-0.35,0, // vertex 1
    0.35,-0.35,0, // vertex 2
    0.35, 0.35,0, // vertex 3

    0.35, 0.35,0, // vertex 3
    -0.35, 0.35,0, // vertex 4
    -0.35,-0.35,0  // vertex 1
  };

  static const GLfloat vertex_buffer_data3 [] = {
    -18,-18,0, // vertex 1
    18,-18,0, // vertex 2
    18, 18,0, // vertex 3

    18, 18,0, // vertex 3
    -18, 18,0, // vertex 4
    -18,-18,0  // vertex 1
  };
  

   static const GLfloat vertex_buffer_data4 [] = {
    -1,-1,0, // vertex 1
    1,-1,0, // vertex 2
    1, 1,0, // vertex 3

    1, 1,0, // vertex 3
    -1, 1,0, // vertex 4
    -1,-1,0  // vertex 1
  };
  float  h=0.25, w=0.5;
  GLfloat vertex_buffer_data_speedy [] = {
        -w/2,-h/2,0, // vertex 1
        w/2,-h/2,0, // vertex 2
        w/2, h/2,0, // vertex 3


        -w/2,-h/2,0, // vertex 1
        w/2, h/2,0, // vertex 3
        -w/2, h/2,0 // vertex 4
      };
      w = 8, h = 8;

      GLfloat vertex_buffer_data5 [] = {
         -w/2,-h/2,0, // vertex 1
        w/2,-h/2,0, // vertex 2
        w/2, h/2,0, // vertex 3


      
        w/2, h/2,0, // vertex 3
        -w/2, h/2,0, // vertex 4
          -w/2,-h/2,0 // vertex 1
      };

          w = 14, h = 4;

      GLfloat vertex_buffer_data6 [] = {
         -w/2,-h/2,0, // vertex 1
        w/2,-h/2,0, // vertex 2
        w/2, h/2,0, // vertex 3


      
        w/2, h/2,0, // vertex 3
        -w/2, h/2,0, // vertex 4
          -w/2,-h/2,0 // vertex 1
      };

      GLfloat color_buffer_data_speedy [] = {
        1,0,0, // color 1
        1,0,0, // color 2
        1,0,0, // color 3


        1,0,0,  // color 1
        1,0,0, // color 3
        1,0,0, // color 4
      };

      GLfloat color_buffer_data_speedy_null [] = {
       1, 1, 1, // color 1
       1, 1, 1 ,// color 1
        1, 1, 1 ,// color 1
         1, 1, 1 ,// color 1
          1, 1, 1 ,// color 1
           1, 1, 1 // color 1
      };

      speedy[0] = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data_speedy, color_buffer_data_speedy, GL_FILL);
      speedy[1] = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data_speedy, color_buffer_data_speedy_null, GL_FILL);
  // create3DObject creates and returns a handle to a VAO that can be used later
  for(int i = 0; i<16; i++)
    timer[i] = create3DTexturedObject(GL_TRIANGLES, 6, vertex_buffer_data, texture_buffer_data, textureID[i], GL_FILL);


  for(int i = 0; i<3; i++) 
    stars[i] = create3DTexturedObject(GL_TRIANGLES, 6, vertex_buffer_data2, texture_buffer_data, textureID[16+i], GL_FILL);
  for(int i=0; i<22; i++) 
    hearts[i] = create3DTexturedObject(GL_TRIANGLES, 6, vertex_buffer_data, texture_buffer_data, textureID[19+i], GL_FILL);
   for(int i = 0; i<25; i++) {
    water[i] = create3DTexturedObject(GL_TRIANGLES, 6, vertex_buffer_data3, texture_buffer_data, textureID[41+i], GL_FILL);
  }
  for(int i=0; i<120; i++)
    soldier[i] = create3DTexturedObject(GL_TRIANGLES, 6, vertex_buffer_data5, texture_buffer_data, textureID[72+i], GL_FILL);
  for(int i = 0; i<12; i++) 
    dragon[i] = create3DTexturedObject(GL_TRIANGLES, 6, vertex_buffer_data5, texture_buffer_data, textureID[192+i], GL_FILL);

  square[0] = create3DTexturedObject(GL_TRIANGLES, 6, vertex_buffer_data4, texture_buffer_data, textureID[66], GL_FILL);
  square[1] = create3DTexturedObject(GL_TRIANGLES, 6, vertex_buffer_data4, texture_buffer_data, textureID[67], GL_FILL);
  grass = create3DTexturedObject(GL_TRIANGLES, 6, vertex_buffer_data4, texture_buffer_data, textureID[204], GL_FILL);
  wood = create3DTexturedObject(GL_TRIANGLES, 6, vertex_buffer_data4, texture_buffer_data, textureID[205], GL_FILL);

  tree = create3DTexturedObject(GL_TRIANGLES, 6, vertex_buffer_data4, texture_buffer_data, textureID[68], GL_FILL);
  arya = create3DTexturedObject(GL_TRIANGLES, 6, vertex_buffer_data, texture_buffer_data, textureID[204], GL_FILL);


  throne = create3DTexturedObject(GL_TRIANGLES, 6, vertex_buffer_data6, texture_buffer_data, textureID[71], GL_FILL);

  star = create3DTexturedObject(GL_TRIANGLES, 6, vertex_buffer_data2, texture_buffer_data, textureID[69], GL_FILL);
  heart = create3DTexturedObject(GL_TRIANGLES, 6, vertex_buffer_data2, texture_buffer_data, textureID[70], GL_FILL);

   
}

void createSphere(int slices, int stacks) {

  int i=0, numPoints = 2 * (slices + 1) * stacks;

  GLfloat *points = new GLfloat[3*numPoints];
  GLfloat *color = new GLfloat[3*numPoints];

  for (float theta = -M_PI / 2; theta < M_PI / 2 - 0.0001; theta += M_PI / stacks) {
    for (float phi = -M_PI; phi <= M_PI + 0.0001; phi += 2 * M_PI / slices) {

      points[3*i] = 1*(cos(theta) * sin(phi));
      points[3*i + 1] = 1*(-sin(theta));
      points[3*i + 2] = 1*(cos(theta) * cos(phi));

      color[3*i] = 1;
      color[3*i + 1] = 0;
      color[3*i + 2] = 0;

      i++;

      points[3*i] = 1*(cos(theta + M_PI / stacks) * sin(phi));
      points[3*i + 1] = 1*( -sin(theta + M_PI / stacks));
      points[3*i + 2] = 1*( cos(theta + M_PI / stacks) * cos(phi));

      color[3*i] = 0;
      color[3*i + 1] = 0;
      color[3*i + 2] = 1;

      i++;
    }
  }
  sphere = create3DObject(GL_TRIANGLE_STRIP, numPoints, points, color, GL_FILL);


  slices = 10, stacks = 10;
  float r = 0.5;
  i=0, numPoints = 2 * (slices + 1) * stacks;


  for (float theta = -M_PI / 2; theta < M_PI / 2 - 0.0001; theta += M_PI / stacks) {
    for (float phi = -M_PI; phi <= M_PI + 0.0001; phi += 2 * M_PI / slices) {

      points[3*i] = r*(cos(theta) * sin(phi));
      points[3*i + 1] = r*(-sin(theta));
      points[3*i + 2] = r*(cos(theta) * cos(phi));

      color[3*i] = 1;
      color[3*i + 1] = 1;
      color[3*i + 2] = 0;

      i++;

      points[3*i] = r*(cos(theta + M_PI / stacks) * sin(phi));
      points[3*i + 1] = r*( -sin(theta + M_PI / stacks));
      points[3*i + 2] = r*( cos(theta + M_PI / stacks) * cos(phi));

      color[3*i] = 1;
      color[3*i + 1] = 1;
      color[3*i + 2] = 0;

      i++;
    }
  }


  head = create3DObject(GL_TRIANGLE_STRIP, numPoints, points, color, GL_FILL);


}

void initGL (GLFWwindow* window, int width, int height)
{
  // Load Textures
  // Enable Texture0 as current texture memory
  glActiveTexture(GL_TEXTURE0);
  // load an image file directly as a new OpenGL texture
  // GLuint texID = SOIL_load_OGL_texture ("beach.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_TEXTURE_REPEATS); // Buggy for OpenGL

  textureID[0] = createTexture("images/timer/frame_0_delay-0.jpg");
  textureID[1] = createTexture("images/timer/frame_1_delay-0.jpg");
  textureID[2] = createTexture("images/timer/frame_2_delay-0.jpg");
  textureID[3] = createTexture("images/timer/frame_3_delay-0.jpg");
  textureID[4] = createTexture("images/timer/frame_4_delay-0.jpg");
  textureID[5] = createTexture("images/timer/frame_5_delay-0.jpg");
  textureID[6] = createTexture("images/timer/frame_6_delay-0.jpg");
  textureID[7] = createTexture("images/timer/frame_7_delay-0.jpg");
  textureID[8] = createTexture("images/timer/frame_8_delay-0.jpg");
  textureID[9] = createTexture("images/timer/frame_9_delay-0.jpg");
  textureID[10] = createTexture("images/timer/frame_10_delay-0.jpg");
  textureID[11] = createTexture("images/timer/frame_11_delay-0.jpg");
  textureID[12] = createTexture("images/timer/frame_12_delay-0.jpg");
  textureID[13] = createTexture("images/timer/frame_13_delay-0.jpg");
  textureID[14] = createTexture("images/timer/frame_14_delay-0.jpg");
  textureID[15] = createTexture("images/timer/frame_15_delay-0.jpg");

  textureID[16] = createTexture("images/stars/frame_0_delay-0.jpg");
  textureID[17] = createTexture("images/stars/frame_1_delay-0.jpg");
  textureID[18] = createTexture("images/stars/frame_2_delay-0.jpg");

  textureID[19] = createTexture("images/lives/frame-001.jpg");
  textureID[20] = createTexture("images/lives/frame-002.jpg");
  textureID[21] = createTexture("images/lives/frame-003.jpg");
  textureID[22] = createTexture("images/lives/frame-004.jpg");
  textureID[23] = createTexture("images/lives/frame-005.jpg");
  textureID[24] = createTexture("images/lives/frame-006.jpg");
  textureID[25] = createTexture("images/lives/frame-007.jpg");
  textureID[26] = createTexture("images/lives/frame-008.jpg");
  textureID[27] = createTexture("images/lives/frame-009.jpg");
  textureID[28] = createTexture("images/lives/frame-010.jpg");
  textureID[29] = createTexture("images/lives/frame-011.jpg");
  textureID[30] = createTexture("images/lives/frame-012.jpg");
  textureID[31] = createTexture("images/lives/frame-013.jpg");
  textureID[32] = createTexture("images/lives/frame-014.jpg");
  textureID[33] = createTexture("images/lives/frame-015.jpg");
  textureID[34] = createTexture("images/lives/frame-016.jpg");
  textureID[35] = createTexture("images/lives/frame-017.jpg");
  textureID[36] = createTexture("images/lives/frame-018.jpg");
  textureID[37] = createTexture("images/lives/frame-019.jpg");
  textureID[38] = createTexture("images/lives/frame-020.jpg");
  textureID[39] = createTexture("images/lives/frame-021.jpg");
  textureID[40] = createTexture("images/lives/frame-022.jpg");
  
  textureID[41] = createTexture("images/water/frame-001.jpg");
  textureID[42] = createTexture("images/water/frame-002.jpg");
  textureID[43] = createTexture("images/water/frame-003.jpg");
  textureID[44] = createTexture("images/water/frame-004.jpg");
  textureID[45] = createTexture("images/water/frame-005.jpg");
  textureID[46] = createTexture("images/water/frame-006.jpg");
  textureID[47] = createTexture("images/water/frame-007.jpg");
  textureID[48] = createTexture("images/water/frame-008.jpg");
  textureID[49] = createTexture("images/water/frame-009.jpg");
  textureID[50] = createTexture("images/water/frame-010.jpg");
  textureID[51] = createTexture("images/water/frame-011.jpg");
  textureID[52] = createTexture("images/water/frame-012.jpg");
  textureID[53] = createTexture("images/water/frame-013.jpg");
  textureID[54] = createTexture("images/water/frame-014.jpg");
  textureID[55] = createTexture("images/water/frame-015.jpg");
  textureID[56] = createTexture("images/water/frame-016.jpg");
  textureID[57] = createTexture("images/water/frame-017.jpg");
  textureID[58] = createTexture("images/water/frame-018.jpg");
  textureID[59] = createTexture("images/water/frame-019.jpg");
  textureID[60] = createTexture("images/water/frame-020.jpg");
  textureID[61] = createTexture("images/water/frame-021.jpg");
  textureID[62] = createTexture("images/water/frame-022.jpg");
  textureID[63] = createTexture("images/water/frame-023.jpg");
  textureID[64] = createTexture("images/water/frame-024.jpg");
  textureID[65] = createTexture("images/water/frame-025.jpg");

  textureID[66] = createTexture("images/square.jpg");
  textureID[67] = createTexture("images/square1.jpg");

  textureID[68] = createTexture("images/tree.png");
  textureID[69] = createTexture("images/star.jpg");
  textureID[70] = createTexture("images/heart.png");
  textureID[71] = createTexture("images/throne.jpg");
  char ibuf[100];
  for(int i = 0; i<120; i++) {
  snprintf(ibuf, sizeof(ibuf), "images/soldier/o_a71eebd8de89d627-%d.jpg", i);
    textureID[72+i] = createTexture(ibuf);
  }
  for(int i = 0; i<12; i++) {
  snprintf(ibuf, sizeof(ibuf), "images/dragon/o_b90b52b369699b6e-%d.jpg", i);
    textureID[192+i] = createTexture(ibuf);
  }

  textureID[204] = createTexture("images/grass.jpg");
  textureID[205] = createTexture("images/wood.jpg");









  // check for an error during the load process
  if(textureID == 0 )
    cout << "SOIL loading error: '" << SOIL_last_result() << "'" << endl;

  // Create and compile our GLSL program from the texture shaders
  textureProgramID = LoadShaders( "TextureRender.vert", "TextureRender.frag" );
  // Get a handle for our "MVP" uniform
  Matrices.TexMatrixID = glGetUniformLocation(textureProgramID, "MVP");


  /* Objects should be created before any other gl function and shaders */
  // Create the modelsrray buffer
  createShapes();
  createRectangle();
  createSphere(10, 10);

  // Create and compile our GLSL program from the shaders
  programID = LoadShaders( "Sample_GL3.vert", "Sample_GL3.frag" );
  // Get a handle for our "MVP" uniform
  Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


  reshapeWindow (window, width, height);

  // Background color of the scene
  glClearColor (0.0f, 0.0f, 0.0f, 0.0f); // R, G, B, A
  glClearDepth (1.0f);

  glEnable (GL_DEPTH_TEST);
  glDepthFunc (GL_LESS);
  //glEnable(GL_BLEND);
  //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Initialise FTGL stuff
  const char* fontfile = "arial.ttf";
  GL3Font.font = new FTExtrudeFont(fontfile); // 3D extrude style rendering

  if(GL3Font.font->Error())
  {
    cout << "Error: Could not load font `" << fontfile << "'" << endl;
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  // Create and compile our GLSL program from the font shaders
  fontProgramID = LoadShaders( "fontrender.vert", "fontrender.frag" );
  GLint fontVertexCoordAttrib, fontVertexNormalAttrib, fontVertexOffsetUniform;
  fontVertexCoordAttrib = glGetAttribLocation(fontProgramID, "vertexPosition");
  fontVertexNormalAttrib = glGetAttribLocation(fontProgramID, "vertexNormal");
  fontVertexOffsetUniform = glGetUniformLocation(fontProgramID, "pen");
  GL3Font.fontMatrixID = glGetUniformLocation(fontProgramID, "MVP");
  GL3Font.fontColorID = glGetUniformLocation(fontProgramID, "fontColor");

  GL3Font.font->ShaderLocations(fontVertexCoordAttrib, fontVertexNormalAttrib, fontVertexOffsetUniform);
  GL3Font.font->FaceSize(1);
  GL3Font.font->Depth(0);
  GL3Font.font->Outset(0, 0);
  GL3Font.font->CharMap(ft_encoding_unicode);

  cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
  cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
  cout << "VERSION: " << glGetString(GL_VERSION) << endl;
  cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

}

void drawSpeedy() {
  float h = 0.25, w = 0.5, x = -10;
      glUseProgram (programID);
      float tempx=0;
      for(float i=1;i<=7;i++){
        tempx+=w;
        glm::mat4 translateRectangle;
        glm::mat4 VP = Matrices.projection * Matrices.view;
        // Send our transformation to the currently bound shader, in the "MVP" uniform
        // For each model you render, since the MVP will be different (at least the M part)
        //  Don't change unless you are sure!!
        glm::mat4 MVP;  // MVP = Projection * View * Model
        // Load identity to model matrix
        Matrices.model = glm::mat4(1.0f);
        translateRectangle = glm::translate (glm::vec3(tempx+x, 5, 0));        // glTranslatef
        Matrices.model *= (translateRectangle );
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        // draw3DObject draws the VAO given to it using current MVP matrix
        if (i<=speed)
          draw3DObject(speedy[0]);
        else
          draw3DObject(speedy[1]);
      }
}

void drawTimer() {


  static int i = 0, frames = 0;
  if(frames == 3) frames = 0;
  if(frames == 0) {
    i++;
    if(i==16) i = 0;
  }

  frames++;

  glUseProgram (textureProgramID);
  glm::mat4 translateRectangle;
  glm::mat4 VP = Matrices.projection * Matrices.view;
  glm::mat4 MVP;  // MVP = Projection * View * Model
  Matrices.model = glm::mat4(1.0f);
  translateRectangle = glm::translate (glm::vec3(6.8,5,0));        // glTranslatef
  Matrices.model *= (translateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);     
  draw3DTexturedObject(timer[i]);
}

void drawBackground() {
  static int i = 0, frames = 0;
  if(frames == 15) frames = 0;
  if(frames == 0) {
    i++;
    if(i==25) i = 0;
  }

  frames++;
glUseProgram (textureProgramID);
  glm::mat4 translateRectangle;
  glm::mat4 VP = Matrices.projection * Matrices.view;
  glm::mat4 MVP;  // MVP = Projection * View * Model
  Matrices.model = glm::mat4(1.0f);
  if(currentView == 0) translateRectangle = glm::translate (glm::vec3(0,-3,0));
  else if(currentView == 1 || currentView == 2 || currentView == 3) translateRectangle = glm::translate (glm::vec3(0,-6,0));        // glTranslatef
  glm::mat4 rotateRectangle = glm::rotate((float)(-90*M_PI/180.0f), glm::vec3(1,0,0)); // glTranslatef
  Matrices.model *= (translateRectangle * rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);     
  draw3DTexturedObject(water[i]);
}

void drawStars() {

  static int i = 0, frames = 0;
  if(frames == 5) frames = 0;
  if(frames == 0) {
    i++;
    if(i==3) i = 0;
  }
  frames++;
  glUseProgram (textureProgramID);
  glm::mat4 translateRectangle;
  glm::mat4 VP = Matrices.projection * Matrices.view;
  glm::mat4 MVP;  // MVP = Projection * View * Model
  Matrices.model = glm::mat4(1.0f);
  translateRectangle = glm::translate (glm::vec3(1.8,5,0));        // glTranslatef
  Matrices.model *= (translateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);     
  draw3DTexturedObject(stars[i]);


}
void drawSoldier() {
  if(currentView == 0 and (viewPtr[currentView] == 0 or viewPtr[currentView] == 1 or viewPtr[currentView] == 2)) {


  static int i = 0, frames = 0;
  if(frames == 2) frames = 0;
  if(frames == 0) {
    i++;
    if(i==120) i = 0;
  }
  frames++;
  glUseProgram (textureProgramID);
  glm::mat4 translateRectangle, rotateRectangle;
  glm::mat4 VP = Matrices.projection * Matrices.view;
  glm::mat4 MVP;  // MVP = Projection * View * Model
  Matrices.model = glm::mat4(1.0f);
  translateRectangle = glm::translate (glm::vec3(13,4,18));    
  rotateRectangle = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,1,0)); // glTranslatef
  Matrices.model *= (translateRectangle * rotateRectangle);    // glTranslatef
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);     
  draw3DTexturedObject(soldier[i]);
}

}

void drawDragon() {
  if(currentView == 0 and (viewPtr[currentView] == 0 or viewPtr[currentView] == 1 or viewPtr[currentView] == 7)) {

  static int i = 0, frames = 0;
  if(frames == 6) frames = 0;
  if(frames == 0) {
    i++;
    if(i==12) i = 0;
  }
  frames++;
  glUseProgram (textureProgramID);
  glm::mat4 translateRectangle, rotateRectangle;
  glm::mat4 VP = Matrices.projection * Matrices.view;
  glm::mat4 MVP;  // MVP = Projection * View * Model
  Matrices.model = glm::mat4(1.0f);
  translateRectangle = glm::translate (glm::vec3(11,6,10));    
  rotateRectangle = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,1,0)); // glTranslatef
  Matrices.model *= (translateRectangle * rotateRectangle);    // glTranslatef
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);     
  draw3DTexturedObject(dragon[i]);

}
}

void drawAnimate(VAO *object) {
  animateX += 0.1;
  if(animateX >= 10) {
    if(starAnimate) starAnimate = false;
    if(heartAnimate) heartAnimate = false;
    animateX = -10, animateY = 3;
    return;
  }
  if(animateX<0) animateY += 0.015;
  else animateY -= 0.015;
  glUseProgram (textureProgramID);
  glm::mat4 translateRectangle;
  glm::mat4 VP = Matrices.projection * Matrices.view;
  glm::mat4 MVP;  // MVP = Projection * View * Model
  Matrices.model = glm::mat4(1.0f);
  translateRectangle = glm::translate (glm::vec3(animateX,animateY,0));        // glTranslatef
  Matrices.model *= (translateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);     
  draw3DTexturedObject(object);


}


void drawLives() {

  static int i = 0, frames = 0;
  if(frames == 5) frames = 0;
  if(frames == 0) {
    i++;
    if(i==22) i = 0;
  }

  frames++;

  glUseProgram (textureProgramID);
  glm::mat4 translateRectangle;
  glm::mat4 VP = Matrices.projection * Matrices.view;
  glm::mat4 MVP;  // MVP = Projection * View * Model
  Matrices.model = glm::mat4(1.0f);
  translateRectangle = glm::translate (glm::vec3(-3.2,5,0));        // glTranslatef
  Matrices.model *= (translateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);     
  draw3DTexturedObject(hearts[i]);
}

void writeTexts() {
  glm::mat4 MVP;
  static int fontScale = 0;
  float fontScaleValue = 0.75 + 0.25*sinf(fontScale*M_PI/180.0f);
  glm::vec3 fontColor = getRGBfromHue (fontScale);

  glm::mat4 translateText, scaleText;
  // Use font Shaders for next part of code
  glUseProgram(fontProgramID);

  Matrices.model = glm::mat4(1.0f);
  translateText = glm::translate(glm::vec3(2.6, 4.8,0));
  scaleText = glm::scale(glm::vec3(fontScaleValue,fontScaleValue,fontScaleValue));
  Matrices.model *= (translateText);
  MVP = Matrices.projection * Matrices.view * Matrices.model;

  // send font's MVP and font color to fond shaders
  glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);
  snprintf(buffer, sizeof(buffer), "%d", points);
  GL3Font.font->Render(buffer);

  Matrices.model = glm::mat4(1.0f);
  translateText = glm::translate(glm::vec3(-2, 4,0));
  scaleText = glm::scale(glm::vec3(fontScaleValue,fontScaleValue,fontScaleValue));
  Matrices.model *= (translateText);
  MVP = Matrices.projection * Matrices.view * Matrices.model;

  // send font's MVP and font color to fond shaders
  glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);
  snprintf(buffer, sizeof(buffer), "%s", "Congratulations!");
  if(playerWin) GL3Font.font->Render(buffer);
  Matrices.model = glm::mat4(1.0f);
  translateText = glm::translate(glm::vec3(-2, 4,0));
  scaleText = glm::scale(glm::vec3(fontScaleValue,fontScaleValue,fontScaleValue));
  Matrices.model *= (translateText);
  MVP = Matrices.projection * Matrices.view * Matrices.model;

  // send font's MVP and font color to fond shaders
  glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);
  snprintf(buffer, sizeof(buffer), "%s", "You Lose!");
  if(playerLose) GL3Font.font->Render(buffer);

  Matrices.model = glm::mat4(1.0f);
  translateText = glm::translate(glm::vec3(-2.4,4.8,0));
  scaleText = glm::scale(glm::vec3(fontScaleValue,fontScaleValue,fontScaleValue));
  Matrices.model *= (translateText);
  MVP = Matrices.projection * Matrices.view * Matrices.model;

  // send font's MVP and font color to fond shaders
  glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);
  snprintf(buffer, sizeof(buffer), "%d", lives);
  GL3Font.font->Render(buffer);

  Matrices.model = glm::mat4(1.0f);
  translateText = glm::translate(glm::vec3(7.6,4.8,0));
  scaleText = glm::scale(glm::vec3(fontScaleValue,fontScaleValue,fontScaleValue));
  Matrices.model *= (translateText);
  MVP = Matrices.projection * Matrices.view * Matrices.model;

  // send font's MVP and font color to fond shaders
  glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);
  int hours, minutes, seconds;
  if(!playerWin) currentTime = glfwGetTime() - gameStart;
  hours = currentTime / 3600;
  minutes = currentTime / 60;
  seconds = currentTime % 60;
  snprintf(buffer, sizeof(buffer), "%d:%d:%d", hours, minutes, seconds);
  GL3Font.font->Render(buffer);
  fontScale = (fontScale + 1) % 360;

}



float camera_rotation_angle = 90;

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
  if(glfwGetTime() - loseTime > 5 && playerLose) gameReset();

  static int frames = 0;
  drawFall();
  drawJump();
  updateObstacles();
  updateBlockMotion();
  drawMove();
  float alpha = 0, beta = 0;

  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // Matrices.view = glm::lookAt(glm::vec3(0,0,10), glm::vec3(0,0,0), glm::vec3(0,1,0));
  // drawBackground();

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);
      viewsX[3][0] = playerCoordX, viewsZ[3][0] = playerCoordZ, viewsY[3][0] = playerCoordY + 4;
      viewsY[2][0] = playerCoordY + 10;


    if(playerDirection == 1) {
      viewsX[2][0] = playerCoordX - 4, viewsZ[2][0] = playerCoordZ;
      alpha = 2;
    }
    else if(playerDirection == 2) {
      viewsX[2][0] = playerCoordX + 4, viewsZ[2][0] = playerCoordZ;
      alpha = -2;
    }

    if(playerDirection == 3) {
      viewsZ[2][0] = playerCoordZ - 4, viewsX[2][0] = playerCoordX;
      beta = 2;
    }
    if(playerDirection == 4) {
      viewsZ[2][0] = playerCoordZ + 4, viewsX[2][0] = playerCoordX;
      beta = -2;
    }
    float eyeX, eyeY, eyeZ;
    eyeX = viewsX[currentView][viewPtr[currentView]];
    eyeY = viewsY[currentView][viewPtr[currentView]];
    eyeZ = viewsZ[currentView][viewPtr[currentView]];
    if(currentView == 0 or currentView == 1) eyeY += zoom_flag;
    else if(currentView == 2 or currentView == 3 and zoom_flag < 4 and zoom_flag >-4) {
      if(playerDirection == 1) eyeX -= zoom_flag;
      if(playerDirection == 2) eyeX += zoom_flag;
      if(playerDirection == 3) eyeZ -= zoom_flag;
      if(playerDirection == 4) eyeZ += zoom_flag;

    }

  // Eye - Location of camera. Don't change unless you are sure!!
  //glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  glm::vec3 eye ( eyeX, eyeY, eyeZ);

  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
    glm::vec3 target2 (playerCoordX, 0, playerCoordZ);
    glm::vec3 target3 (playerCoordX + alpha, playerCoordY + 1.8, playerCoordZ + beta);




  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  if(currentView == 2) 

  Matrices.view = glm::lookAt( eye, target2, up ); // Rotating Camera for 3D
else if(currentView == 3) 
  Matrices.view = glm::lookAt( eye, target3, up ); // Rotating Camera for 3D

else
  Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D

  //  Don't change unless you are sure!!

  //static float c = 0;
  //c++;
  //Matrices.view = glm::lookAt(glm::vec3(0,0,10), glm::vec3(0,0,0), glm::vec3(0,1,0));
  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!

  drawBackground();
     glm::mat4 translateCube;
        glm::mat4 rotateCube;

        glm::mat4 CubeTransform;
        glm::mat4 MVP;
        glm::mat4 translateRectangle;
         glm::mat4 rotateRectangle;
  


    CubeTransform = translateCube * rotateCube;

    Matrices.model *= CubeTransform;
    MVP = VP * Matrices.model; // MVP = p * V * M

    //  Don't change unless you are sure!!
    // Copy MVP to normal shaders
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(coin);

         for(int i = 0; i<12; i++) {
          Matrices.model = glm::mat4(1.0f);
  if(currentView == 0 && viewPtr[0] <5) translateRectangle = glm::translate (glm::vec3(-15 + 2*i, min(5.0, 4.8 + i*0.1), -10));
  
  Matrices.model *= (translateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);     
  draw3DTexturedObject(tree);
}


drawSoldier();
drawDragon();
glUseProgram (programID);
  for(int i=0; i<10; i++) {
    for(int j = 0; j<10; j++) {
      Matrices.model = glm::mat4(1.0f);
      /* Render your scene */
      if(!isMoving[i][j])
        translateCube = glm::translate (glm::vec3(i*2+shiftX, 0, j*2+shiftZ)); // glTranslatef
      else
        translateCube = glm::translate (glm::vec3(i*2+shiftX, blockCoordY, j*2+shiftZ)); // glTranslatef

      glm::mat4 CubeTransform = translateCube;

      Matrices.model *= CubeTransform;
      MVP = VP * Matrices.model; // MVP = p * V * M

      //  Don't change unless you are sure!!
      // Copy MVP to normal shaders
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

      // draw3DObject draws the VAO given to it using current MVP matrix
      if(isPresent[i][j]) {
        draw3DObject(cube);

      glUseProgram (textureProgramID);
  translateRectangle;
  VP = Matrices.projection * Matrices.view;
  MVP;  // MVP = Projection * View * Model
  Matrices.model = glm::mat4(1.0f);
  if(!isMoving[i][j]) translateRectangle = glm::translate (glm::vec3(i*2+shiftX, 3.05, j*2+shiftZ));
  else translateRectangle = glm::translate (glm::vec3(i*2+shiftX, blockCoordY+3.05, j*2+shiftZ));
  rotateRectangle = glm::rotate((float)(-90*M_PI/180.0f), glm::vec3(1,0,0)); // glTranslatef
  Matrices.model *= (translateRectangle * rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0); 

  if(lightOn == true) {
    if(playerX >= i-1 && playerX<=i+1 && playerZ >= j-1 && playerZ <= j+1) {
    if((i+j )% 2) 
  draw3DTexturedObject(grass);
else
  draw3DTexturedObject(wood);
}
    else draw3DTexturedObject(square[(i+j)%2]);
  }
  else draw3DTexturedObject(square[(i+j)%2]);

 
}
  glUseProgram(programID);
      if((playerX == i && playerZ == j) || playerZ>9 || playerX>9 || playerX<0 || playerZ<0) {
         
      
      
        Matrices.model = glm::mat4(1.0f);
        translateCube = glm::translate (glm::vec3(playerCoordX, playerCoordY+playerWin*2+1.5, playerCoordZ)); // glTranslatef
        rotateCube = glm::rotate((float)(sphereRotation*M_PI/180.0f), glm::vec3(0,0,1)); // glTranslatef
        glm::mat4 rotateCube2 = glm::rotate((float)(sphereRotation*M_PI/180.0f), glm::vec3(0,1,0)); // glTranslatef

        glm::mat4 CubeTransform = translateCube;
        if(playerAnimate || playerFallOff || playerJumpUp || playerJumpRight || playerJumpLeft || playerJumpDown) CubeTransform *= rotateCube;
        if(playerWin) CubeTransform *= rotateCube2;

        Matrices.model *= CubeTransform;
        MVP = VP * Matrices.model; // MVP = p * V * M

        //  Don't change unless you are sure!!
        // Copy MVP to normal shaders
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        if(!playerAnimate) {
          draw3DObject(player); 
          Matrices.model = glm::mat4(1.0f);
    translateCube = glm::translate (glm::vec3(playerCoordX, playerCoordY+3, playerCoordZ)); // glTranslatef
   

    CubeTransform = translateCube;

    Matrices.model *= CubeTransform;
    MVP = VP * Matrices.model; // MVP = p * V * M

    //  Don't change unless you are sure!!
    // Copy MVP to normal shaders
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(head);
        Matrices.model = glm::mat4(1.0f);
    if(playerDirection == 3 or playerDirection == 4) {
    translateCube = glm::translate (glm::vec3(playerCoordX-0.5, playerCoordY, playerCoordZ)); // glTranslatef

    rotateCube = glm::rotate((float)(-20*M_PI/180.0f), glm::vec3(0,0,1)); // glTranslatef
  }
     else {
     translateCube = glm::translate (glm::vec3(playerCoordX, playerCoordY, playerCoordZ-0.5)); // glTranslatef
    rotateCube = glm::rotate((float)(30*M_PI/180.0f), glm::vec3(1,0,0)); // glTranslatef

   }
   

    CubeTransform = translateCube * rotateCube;

    Matrices.model *= CubeTransform;
    MVP = VP * Matrices.model; // MVP = p * V * M

    //  Don't change unless you are sure!!
    // Copy MVP to normal shaders
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(limbs);
      Matrices.model = glm::mat4(1.0f);
    if(playerDirection == 3 or playerDirection == 4) {
    translateCube = glm::translate (glm::vec3(playerCoordX+0.5, playerCoordY, playerCoordZ)); // glTranslatef
    rotateCube = glm::rotate((float)(20*M_PI/180.0f), glm::vec3(0,0,1)); // glTranslatef
  }
   else {
     translateCube = glm::translate (glm::vec3(playerCoordX, playerCoordY, playerCoordZ+0.5)); // glTranslatef
    rotateCube = glm::rotate((float)(150*M_PI/180.0f), glm::vec3(1,0,0)); // glTranslatef

   }

    CubeTransform = translateCube * rotateCube;

    Matrices.model *= CubeTransform;
    MVP = VP * Matrices.model; // MVP = p * V * M

    //  Don't change unless you are sure!!
    // Copy MVP to normal shaders
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(limbs);
        Matrices.model = glm::mat4(1.0f);
    if(playerDirection == 3 or playerDirection == 4) {
    translateCube = glm::translate (glm::vec3(playerCoordX+1, playerCoordY+2, playerCoordZ)); // glTranslatef
   
    rotateCube = glm::rotate((float)(50*M_PI/180.0f), glm::vec3(0,0,1)); // glTranslatef
  }
   else {
     translateCube = glm::translate (glm::vec3(playerCoordX, playerCoordY+2, playerCoordZ+1)); // glTranslatef
    rotateCube = glm::rotate((float)(110*M_PI/180.0f), glm::vec3(1,0,0)); // glTranslatef

   }
    CubeTransform = translateCube * rotateCube;

    Matrices.model *= CubeTransform;
    MVP = VP * Matrices.model; // MVP = p * V * M

    //  Don't change unless you are sure!!
    // Copy MVP to normal shaders
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(limbs);
         Matrices.model = glm::mat4(1.0f);
   if(playerDirection == 3 or playerDirection == 4)  {
   translateCube = glm::translate (glm::vec3(playerCoordX-1, playerCoordY+2, playerCoordZ)); // glTranslatef
   
    rotateCube = glm::rotate((float)(-50*M_PI/180.0f), glm::vec3(0,0,1)); // glTranslatef
  }
     else {
     translateCube = glm::translate (glm::vec3(playerCoordX, playerCoordY+2, playerCoordZ-1)); // glTranslatef
    rotateCube = glm::rotate((float)(-110*M_PI/180.0f), glm::vec3(1,0,0)); // glTranslatef

   }

    CubeTransform = translateCube * rotateCube;

    Matrices.model *= CubeTransform;
    MVP = VP * Matrices.model; // MVP = p * V * M

    //  Don't change unless you are sure!!
    // Copy MVP to normal shaders
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(limbs);
  }

        else {
          frames++;
          if(frames == 10) {
            draw3DObject(player);
            Matrices.model = glm::mat4(1.0f);
    translateCube = glm::translate (glm::vec3(playerCoordX, playerCoordY+3, playerCoordZ)); // glTranslatef
   

    CubeTransform = translateCube;

    Matrices.model *= CubeTransform;
    MVP = VP * Matrices.model; // MVP = p * V * M

    //  Don't change unless you are sure!!
    // Copy MVP to normal shaders
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(head);
        Matrices.model = glm::mat4(1.0f);
    if(playerDirection == 3 or playerDirection == 4) {
    translateCube = glm::translate (glm::vec3(playerCoordX-0.5, playerCoordY, playerCoordZ)); // glTranslatef

    rotateCube = glm::rotate((float)(-20*M_PI/180.0f), glm::vec3(0,0,1)); // glTranslatef
  }
     else {
     translateCube = glm::translate (glm::vec3(playerCoordX, playerCoordY, playerCoordZ-0.5)); // glTranslatef
    rotateCube = glm::rotate((float)(30*M_PI/180.0f), glm::vec3(1,0,0)); // glTranslatef

   }
   

    CubeTransform = translateCube * rotateCube;

    Matrices.model *= CubeTransform;
    MVP = VP * Matrices.model; // MVP = p * V * M

    //  Don't change unless you are sure!!
    // Copy MVP to normal shaders
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(limbs);
      Matrices.model = glm::mat4(1.0f);
    if(playerDirection == 3 or playerDirection == 4) {
    translateCube = glm::translate (glm::vec3(playerCoordX+0.5, playerCoordY, playerCoordZ)); // glTranslatef
    rotateCube = glm::rotate((float)(20*M_PI/180.0f), glm::vec3(0,0,1)); // glTranslatef
  }
   else {
     translateCube = glm::translate (glm::vec3(playerCoordX, playerCoordY, playerCoordZ+0.5)); // glTranslatef
    rotateCube = glm::rotate((float)(150*M_PI/180.0f), glm::vec3(1,0,0)); // glTranslatef

   }

    CubeTransform = translateCube * rotateCube;

    Matrices.model *= CubeTransform;
    MVP = VP * Matrices.model; // MVP = p * V * M

    //  Don't change unless you are sure!!
    // Copy MVP to normal shaders
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(limbs);
        Matrices.model = glm::mat4(1.0f);
    if(playerDirection == 3 or playerDirection == 4) {
    translateCube = glm::translate (glm::vec3(playerCoordX+1, playerCoordY+2, playerCoordZ)); // glTranslatef
   
    rotateCube = glm::rotate((float)(50*M_PI/180.0f), glm::vec3(0,0,1)); // glTranslatef
  }
   else {
     translateCube = glm::translate (glm::vec3(playerCoordX, playerCoordY+2, playerCoordZ+1)); // glTranslatef
    rotateCube = glm::rotate((float)(110*M_PI/180.0f), glm::vec3(1,0,0)); // glTranslatef

   }
    CubeTransform = translateCube * rotateCube;

    Matrices.model *= CubeTransform;
    MVP = VP * Matrices.model; // MVP = p * V * M

    //  Don't change unless you are sure!!
    // Copy MVP to normal shaders
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(limbs);
         Matrices.model = glm::mat4(1.0f);
   if(playerDirection == 3 or playerDirection == 4)  {
   translateCube = glm::translate (glm::vec3(playerCoordX-1, playerCoordY+2, playerCoordZ)); // glTranslatef
   
    rotateCube = glm::rotate((float)(-50*M_PI/180.0f), glm::vec3(0,0,1)); // glTranslatef
  }
     else {
     translateCube = glm::translate (glm::vec3(playerCoordX, playerCoordY+2, playerCoordZ-1)); // glTranslatef
    rotateCube = glm::rotate((float)(-110*M_PI/180.0f), glm::vec3(1,0,0)); // glTranslatef

   }

    CubeTransform = translateCube * rotateCube;

    Matrices.model *= CubeTransform;
    MVP = VP * Matrices.model; // MVP = p * V * M

    //  Don't change unless you are sure!!
    // Copy MVP to normal shaders
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(limbs);
            frames = 0;
            if(glfwGetTime() - timeStamp > 2.5) {
              playerAnimate = false;
              playerReset(2);
            }
          }
        }
      }
    }
  }


  for(int l=0; l<obstacles.size(); l++) {
    int i=obstacles[l].first;
    int j=obstacles[l].second;

    Matrices.model = glm::mat4(1.0f);
    translateCube = glm::translate (glm::vec3(i*2+shiftX, 4, j*2+shiftZ)); // glTranslatef
    rotateCube = glm::rotate((float)(sphereRotation*M_PI/180.0f), glm::vec3(0,0,1)); // glTranslatef

    glm::mat4 CubeTransform = translateCube * rotateCube;

    Matrices.model *= CubeTransform;
    MVP = VP * Matrices.model; // MVP = p * V * M

    //  Don't change unless you are sure!!
    // Copy MVP to normal shaders
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(sphere);
    draw3DObject(spikes);
  }

  for(int l=0; l<coins.size(); l++) {
    int i=coins[l].first;
    int j=coins[l].second;

    Matrices.model = glm::mat4(1.0f);
    translateCube = glm::translate (glm::vec3(i*2+shiftX, 4.2, j*2+shiftZ));
    rotateCube = glm::rotate((float)((-sphereRotation+90)*M_PI/180.0f), glm::vec3(0,0,1)); // glTranslatef


    glm::mat4 CubeTransform = translateCube * rotateCube;

    Matrices.model *= CubeTransform;
    MVP = VP * Matrices.model; // MVP = p * V * M

    //  Don't change unless you are sure!!
    // Copy MVP to normal shaders
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(coin);
  }
  Matrices.view = glm::lookAt(glm::vec3(0,0,10), glm::vec3(0,0,0), glm::vec3(0,1,0));
  drawSpeedy();
  drawTimer();
  drawStars();
  drawLives();

  if(starAnimate) drawAnimate(star);
  if(heartAnimate) drawAnimate(heart);

  sphereRotation++;
  writeTexts();
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */


int main (int argc, char** argv)
{
  ALboolean enumeration;
  const ALCchar *devices;
  const ALCchar *defaultDeviceName = argv[1];
  int ret;
#ifdef LIBAUDIO
  WaveInfo *wave;
#endif
  char *bufferData;
  ALCdevice *device;
  ALvoid *data;
  ALCcontext *context;
  ALsizei size, freq;
  ALenum format;
  ALuint buffer, source;
  ALfloat listenerOri[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
  ALboolean loop = AL_FALSE;
  ALCenum error;
  ALint source_state;

  fprintf(stdout, "Using " BACKEND " as audio backend\n");

  enumeration = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
  if (enumeration == AL_FALSE)
    fprintf(stderr, "enumeration extension not available\n");

  list_audio_devices(alcGetString(NULL, ALC_DEVICE_SPECIFIER));

  if (!defaultDeviceName)
    defaultDeviceName = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);

  device = alcOpenDevice(defaultDeviceName);
  if (!device) {
    fprintf(stderr, "unable to open default device\n");
    return -1;
  }

  fprintf(stdout, "Device: %s\n", alcGetString(device, ALC_DEVICE_SPECIFIER));

  alGetError();

  context = alcCreateContext(device, NULL);
  if (!alcMakeContextCurrent(context)) {
    fprintf(stderr, "failed to make default context\n");
    return -1;
  }
  TEST_ERROR("make default context");

  /* set orientation */
  alListener3f(AL_POSITION, 0, 0, 1.0f);
  TEST_ERROR("listener position");
      alListener3f(AL_VELOCITY, 0, 0, 0);
  TEST_ERROR("listener velocity");
  alListenerfv(AL_ORIENTATION, listenerOri);
  TEST_ERROR("listener orientation");

  alGenSources((ALuint)1, &source);
  TEST_ERROR("source generation");

  alSourcef(source, AL_PITCH, 1);
  TEST_ERROR("source pitch");
  alSourcef(source, AL_GAIN, 1);
  TEST_ERROR("source gain");
  alSource3f(source, AL_POSITION, 0, 0, 0);
  TEST_ERROR("source position");
  alSource3f(source, AL_VELOCITY, 0, 0, 0);
  TEST_ERROR("source velocity");
  alSourcei(source, AL_LOOPING, AL_TRUE);
  TEST_ERROR("source looping");

  alGenBuffers(1, &buffer);
  TEST_ERROR("buffer generation");

#ifdef LIBAUDIO
  /* load data */
  wave = WaveOpenFileForReading("music.wav");
  if (!wave) {
    fprintf(stderr, "failed to read wave file\n");
    return -1;
  }

  ret = WaveSeekFile(0, wave);
  if (ret) {
    fprintf(stderr, "failed to seek wave file\n");
    return -1;
  }

  bufferData = malloc(wave->dataSize);
  if (!bufferData) {
    perror("malloc");
    return -1;
  }

  ret = WaveReadFile(bufferData, wave->dataSize, wave);
  if (ret != wave->dataSize) {
    fprintf(stderr, "short read: %d, want: %d\n", ret, wave->dataSize);
    return -1;
  }

  alBufferData(buffer, to_al_format(wave->channels, wave->bitsPerSample),
      bufferData, wave->dataSize, wave->sampleRate);
  TEST_ERROR("failed to load buffer data");
#else
  alutLoadWAVFile((ALbyte* )"music.wav", &format, &data, &size, &freq, &loop);
  TEST_ERROR("loading wav file");

  alBufferData(buffer, format, data, size, freq);
  TEST_ERROR("buffer copy");
#endif

  alSourcei(source, AL_BUFFER, buffer);
  TEST_ERROR("buffer binding");

  alSourcePlay(source);
  TEST_ERROR("source playing");

  int width = 1400;
  int height = 714;


  GLFWwindow* window = initGLFW(width, height);

  initGL (window, width, height);

  double last_update_time = glfwGetTime(), current_time;

  /* Draw in loop */
  while (!glfwWindowShouldClose(window)) {

    // OpenGL Draw commands
    draw();


    // Swap Frame Buffer in double buffering
    glfwSwapBuffers(window);

    // Poll for Keyboard and mouse events
    glfwPollEvents();

    // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
    current_time = glfwGetTime(); // Time in seconds
    if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
      // do something every 0.5 seconds ..
      last_update_time = current_time;
    }
  }
  alGetSourcei(source, AL_SOURCE_STATE, &source_state);
  TEST_ERROR("source state get");
  while (source_state == AL_PLAYING) {
    alGetSourcei(source, AL_SOURCE_STATE, &source_state);
    TEST_ERROR("source state get");
  }

  


  /* exit context */
  alDeleteSources(1, &source);
  alDeleteBuffers(1, &buffer);
  device = alcGetContextsDevice(context);
  alcMakeContextCurrent(NULL);
  alcDestroyContext(context);
  alcCloseDevice(device);

  glfwTerminate();
  exit(EXIT_SUCCESS);
}
