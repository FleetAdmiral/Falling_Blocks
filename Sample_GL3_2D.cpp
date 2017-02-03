#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <time.h>
#include <stdlib.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
float left_move = 0;
glm::vec3 b1_pos, b2_pos, rect_pos;
double xpos, ypos, xposGun, yposGun;
double angleTan, angleTanGun;
float triangle_rotation = 0;
float bulletStatus=0;
int bulletindex=-1;
float score=0;
double bullet_time, current_time;
struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;
GLuint programID;
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
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
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

/**************************
 * Customizable functions *
 **************************/

/*float rectangle_rot_dir = 1;
bool rectangle_rot_status = true;*/

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
  glfwGetCursorPos(window,&xposGun,&yposGun);
     // Function is called first on GLFW_PRESS.

  /*  if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_C:
                rectangle_rot_status = !rectangle_rot_status;
                break;
            case GLFW_KEY_P:
                triangle_rot_status = !triangle_rot_status;
                break;
            case GLFW_KEY_X:
                // do something ..
                break;
            default:
                break;
        }
    }*/
    if (action == GLFW_PRESS) {
                switch(key){
                  case GLFW_KEY_SPACE:
                  angleTanGun = atan(yposGun/xposGun);
                  bulletStatus = 1;
                default:
                  break;
                }
          }
    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))
      {
        if(glfwGetKey(window, GLFW_KEY_LEFT))
        {
        if(b2_pos.x>-2.8)
        {
          b2_pos.x -= 0.2;
        }
        }
        else if(glfwGetKey(window, GLFW_KEY_RIGHT))
        {
          if(b2_pos.x<1.8)
          {
          b2_pos.x += 0.2;
        }
        }
      }
      if(glfwGetKey(window, GLFW_KEY_LEFT_ALT))
        {
          if(glfwGetKey(window, GLFW_KEY_LEFT))
          {
          if(b1_pos.x>-2.8)
          {
            b1_pos.x -= 0.2;
          }
          }
          else if(glfwGetKey(window, GLFW_KEY_RIGHT))
          {
            if(b1_pos.x<2.8)
            {
            b1_pos.x += 0.2;
          }
          }
        }
        if(glfwGetKey(window, GLFW_KEY_F))
          {
              if(rect_pos.y>-1)
              rect_pos.y -= 0.2;

          }
          if(glfwGetKey(window, GLFW_KEY_S))
            {
                if(rect_pos.y<2.2)
                rect_pos.y += 0.2;
            }
            if(glfwGetKey(window, GLFW_KEY_A))
              {
                  //if(rect_pos.y<1.4)
                  if(triangle_rotation==0)
                  triangle_rotation = 1;
                  else
                  ++triangle_rotation;
              }
              if(glfwGetKey(window, GLFW_KEY_D))
                {
                    //if(rect_pos.y<1.4)
                    triangle_rotation = -1;
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
    /*switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_RELEASE)
                triangle_rot_dir *= -1;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_RELEASE) {
                rectangle_rot_dir *= -1;
            }
            break;
        default:
            break;
    }*/

  }


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}
/* Variable declarations are here*/
VAO *rectangle;
VAO *gun;
VAO *base;
VAO *basket1;
VAO *basket2;
VAO *mirror;
//VAO *greenBrick;
VAO *greenBrick;
VAO *redBrick;
VAO *blackBrick;
VAO *bullet;
int greenBrickIndex = 0, redBrickIndex = 0, blackBrickIndex = 0;
float randaf,randafred,randafblack;
float greenx[1000];
float greeny[1000];
float greenrand[1000];
float redx[1000];
float redy[1000];
float redrand[1000];
float blackx[1000];
float blacky[1000];
float bulletx[100];
float bullety[100];
double bulletrot[100];
float time_now,time_before, time_beforered, time_beforeblack;
int indexc = 0,indexred = 0, indexblack =  0;
float gbx = 2.5;

bool chckcollision(float ax,float bx, float ay, float by, float aw, float bw, float ah, float bh)
{
	return fabs(ax - bx) < (aw + bw)/2 && fabs(ay - by) < (ah + bh)/2;
}
void createblackBrick()
{
  // GL3 accepts only Triangles. Quads are not supported
  /*static const GLfloat vertex_buffer_data [] = {
    -1.2,-1,0, // vertex 1
    1.2,-1,0, // vertex 2
    1.2, 1,0, // vertex 3

    1.2, 1,0, // vertex 3
    -1.2, 1,0, // vertex 4
    -1.2,-1,0  // vertex 1


  };*/
  static const GLfloat vertex_buffer_data [] = {
    0,0,0, // vertex 1
    0.3,0,0, // vertex 2
    0,0.4,0,// vertex 3

    0.3, 0,0, // vertex 3
    0.3, 0.4,0, // vertex 4
    0,0,0,  // vertex 1
  };
  static const GLfloat color_buffer_data [] = {
    0,0,0, // color 1
    0,0,0, // color 2
    0,0,0, // color 3

    0,0,0, // color 3
    0,0,0, // color 4
    0,0,0,  // color 1*/
  };
//  for(int i=0;i<1000;i++)
  blackBrick = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createBullet()
{
  // GL3 accepts only Triangles. Quads are not supported
  /*static const GLfloat vertex_buffer_data [] = {
    -1.2,-1,0, // vertex 1
    1.2,-1,0, // vertex 2
    1.2, 1,0, // vertex 3

    1.2, 1,0, // vertex 3
    -1.2, 1,0, // vertex 4
    -1.2,-1,0  // vertex 1


  };*/
  static const GLfloat vertex_buffer_data [] = {
    0,0.07,0, // vertex 1
    0,0,0, // vertex 2
    0.15,0,0,// vertex 3

    0, 0.07,0, // vertex 3
    0.15, 0,0, // vertex 4
    0.15,0.07,0  // vertex 1
  };
  static const GLfloat color_buffer_data [] = {
    1,1,0, // color 1
    1,1,0, // color 2
    1,1,0, // color 3

    1,1,0, // color 3
    1,1,0, // color 4
    1,1,0,  // color 1*/
  };
  //for(int i=0;i<100;i++)
  bullet = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void creategreenBrick()
{
  // GL3 accepts only Triangles. Quads are not supported
  /*static const GLfloat vertex_buffer_data [] = {
    -1.2,-1,0, // vertex 1
    1.2,-1,0, // vertex 2
    1.2, 1,0, // vertex 3

    1.2, 1,0, // vertex 3
    -1.2, 1,0, // vertex 4
    -1.2,-1,0  // vertex 1


  };*/
  static const GLfloat vertex_buffer_data [] = {
    0,0,0, // vertex 1
    0.3,0,0, // vertex 2
    0,0.4,0,// vertex 3

    0.3, 0,0, // vertex 3
    0.3, 0.4,0, // vertex 4
    0,0,0,  // vertex 1
  };
  static const GLfloat color_buffer_data [] = {
    0,1,0, // color 1
    0,1,0, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0,1,0, // color 4
    0,1,0  // color 1*/
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
//  for(int i=0;i<1000;i++)
  greenBrick = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createredBrick()
{
  // GL3 accepts only Triangles. Quads are not supported
  /*static const GLfloat vertex_buffer_data [] = {
    -1.2,-1,0, // vertex 1
    1.2,-1,0, // vertex 2
    1.2, 1,0, // vertex 3

    1.2, 1,0, // vertex 3
    -1.2, 1,0, // vertex 4
    -1.2,-1,0  // vertex 1


  };*/
  static const GLfloat vertex_buffer_data [] = {
    0,0,0, // vertex 1
    0.3,0,0, // vertex 2
    0,0.4,0,// vertex 3

    0.3, 0,0, // vertex 3
    0.3, 0.4,0, // vertex 4
    0,0,0,  // vertex 1
  };
  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    1,0,0, // color 2
    1,0,0, // color 3

    1,0,0, // color 3
    1,0,0, // color 4
    1,0,0  // color 1*/
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  //for(int i=0;i<1000;i++)
  redBrick = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createBasket2 ()
{
  // GL3 accepts only Triangles. Quads are not supported
  /*static const GLfloat vertex_buffer_data [] = {
    -1.2,-1,0, // vertex 1
    1.2,-1,0, // vertex 2
    1.2, 1,0, // vertex 3

    1.2, 1,0, // vertex 3
    -1.2, 1,0, // vertex 4
    -1.2,-1,0  // vertex 1


  };*/
  static const GLfloat vertex_buffer_data [] = {
  /*  2.5,-3.6,0, // vertex 1
    1.8,-3.6,0, // vertex 2
    2.5,-2.6,0,// vertex 3

    2.5, -2.6,0, // vertex 3
    1.8, -3.6,0, // vertex 4
    1.8,-2.6,0  // vertex 1*/
    0,0,0,
    0.7,0,0,
    0.7,1,0,

    0,1,0,
    0.7,1,0,
    0,0,0,
  };

  static const GLfloat color_buffer_data [] = {
    0,1,0, // color 1
    0,1,0, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0,1,0, // color 4
    0,1,0  // color 1*/
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  basket2 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createBasket1 ()
{
  // GL3 accepts only Triangles. Quads are not supported
  /*static const GLfloat vertex_buffer_data [] = {
    -1.2,-1,0, // vertex 1
    1.2,-1,0, // vertex 2
    1.2, 1,0, // vertex 3

    1.2, 1,0, // vertex 3
    -1.2, 1,0, // vertex 4
    -1.2,-1,0  // vertex 1


  };*/
  static const GLfloat vertex_buffer_data [] = {
    0,0,0,
    0.7,0,0,
    0.7,1,0,

    0,1,0,
    0.7,1,0,
    0,0,0,  // vertex 1
  };
  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    1,0,0, // color 2
    1,0,0, // color 3

    1,0,0, // color 3
    1,0,0, // color 4
    1,0,0  // color 1*/
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  basket1 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createBase ()
{
  // GL3 accepts only Triangles. Quads are not supported
  /*static const GLfloat vertex_buffer_data [] = {
    -1.2,-1,0, // vertex 1
    1.2,-1,0, // vertex 2
    1.2, 1,0, // vertex 3

    1.2, 1,0, // vertex 3
    -1.2, 1,0, // vertex 4
    -1.2,-1,0  // vertex 1


  };*/
  static const GLfloat vertex_buffer_data [] = {
    /*-4,-3,0, // vertex 1
    4,-3,0, // vertex 2
    -4,-4,0,// vertex 3

    -4, -4,0, // vertex 3
    4, -4,0, // vertex 4
    4,-3,0  // vertex 1*/
    0,0,0,
    8,0,0,
    8,1,0,

    0,0,0,
    0,1,0,
    8,1,0,
  };
  static const GLfloat color_buffer_data [] = {
    0,0,0, // color 1
    0,0,0, // color 2
    0,0,0, // color 3

    0,0,0, // color 3
    0,0,0, // color 4
    0,0,0  // color 1*/
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  base = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createGun ()
{
  // GL3 accepts only Triangles. Quads are not supported
  /*static const GLfloat vertex_buffer_data [] = {
    -1.2,-1,0, // vertex 1
    1.2,-1,0, // vertex 2
    1.2, 1,0, // vertex 3

    1.2, 1,0, // vertex 3
    -1.2, 1,0, // vertex 4
    -1.2,-1,0  // vertex 1


  };*/
  static const GLfloat vertex_buffer_data [] = {
  /*  0.75,0.15,0, // vertex 1
    0,0.15,0, // vertex 2
    0.75,-0.15,0,// vertex 3

    0.75, -0.15,0, // vertex 3
    0, -0.15,0, // vertex 4
    0,0.15,0  // vertex 1*/
    0,0,0,
    0.75,0,0,
    0,0.4,0,

    0,0.4,0,
    0.75,0,0,
    0.75,0.4,0,
  };
  static const GLfloat color_buffer_data [] = {
    0,0,0, // color 1
    0,0,0, // color 2
    0,0,0, // color 3

    0,0,0, // color 3
    0,0,0, // color 4
    0,0,0, // color 1*/
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  gun = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

// Creates the triangle object used in this sample code
// Creates the rectangle object used in this sample code
void createRectangle ()
{
  // GL3 accepts only Triangles. Quads are not supported
  /*static const GLfloat vertex_buffer_data [] = {
    -1.2,-1,0, // vertex 1
    1.2,-1,0, // vertex 2
    1.2, 1,0, // vertex 3

    1.2, 1,0, // vertex 3
    -1.2, 1,0, // vertex 4
    -1.2,-1,0  // vertex 1


  };*/
  static const GLfloat vertex_buffer_data [] = {
    /*-4,0.5,0, // vertex 1
    -3.5,0.5,0, // vertex 2
    -4,-0.5,0,// vertex 3

    -4, -0.5,0, // vertex 3
    -3.5, -0.5,0, // vertex 4
    -3.5,0.5,0  // vertex 1*/
    0,0,0,
    0.5,0,0,
    0,1,0,

    0,1,0,
    0.5,0,0,
    0.5,1,0,
  };
  static const GLfloat color_buffer_data [] = {
    0,0,0, // color 1
    0,0,0, // color 2
    0,0,0, // color 3

    0,0,0, // color 3
    0,0,0, // color 4
    0,0,0  // color 1*/
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createMirror ()
{
  // GL3 accepts only Triangles. Quads are not supported
  /*static const GLfloat vertex_buffer_data [] = {
    -1.2,-1,0, // vertex 1
    1.2,-1,0, // vertex 2
    1.2, 1,0, // vertex 3

    1.2, 1,0, // vertex 3
    -1.2, 1,0, // vertex 4
    -1.2,-1,0  // vertex 1


  };*/
  static const GLfloat vertex_buffer_data [] = {
    0,0,0,
    0.5,0,0,
    0.5,0.5,0,// vertex 1
  };
  static const GLfloat color_buffer_data [] = {
    0,1,1, // color 1
    0,1,1, // color 2
    0,1,1, // color 3 // color 1*/
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  mirror = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
float camera_rotation_angle = 90;
float rectangle_rotation = 4;
float rectangle_rot_dir = 1;
bool rectangle_rot_status = true;
/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{

cout<<"The score is "<<score<<endl;
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model - all matrices. Modelling : converting to world coordinates. View: Conversion from objectvcoord to

  // Load identity to model matrix
  Matrices.model = glm::mat4(1.0f);

  /* Render your scene */

  //glm::mat4 translateTriangle = glm::translate (glm::vec3(-2.0f, 0.0f, 0.0f)); // glTranslatef
  // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
  // glPopMatrix ();
  Matrices.model = glm::mat4(1.0f);

  //glm::mat4 translateRectangle = glm::translate (glm::vec3(2, 0, 0));        // glTranslatef



  MVP = VP * Matrices.model;
  glm::mat4 translateBase = glm::translate (glm::vec3(-4,-4,0));
  Matrices.model *= translateBase;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(base);
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateMirror = glm::translate(glm::vec3(2.5,0,0));
  Matrices.model *= (translateMirror);
  MVP = VP * Matrices.model;
  for(int j=0;j<bulletindex;j++)
  {
      bool tryandl = chckcollision(2.5, bulletx[j], 0, bullety[j], 0.4, 0.15, 0.4, 0.07);
      if(tryandl)
      {
        bulletrot[j] = bulletrot[j]*(M_PI);
        cout<<"Reflect"<<endl;
      }
  }
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(mirror);
  Matrices.model = glm::mat4(1.0f);
  // draw3DObject draws the VAO given to it using current MVP matrix

  /*glm::mat4 translateRectangle12 = glm::translate (rect_pos);        // glTranslatef
  glm::mat4 rotateRectangle12 = glm::translate(rect_pos); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectangle12 * rotateRectangle12);*/
  glm::mat4 translateRectangle124 = glm::translate (rect_pos);
  Matrices.model *= (translateRectangle124);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(rectangle);
  Matrices.model = glm::mat4(1.0f);      // glTranslatef

  glm::mat4 translateRectangle124n = glm::translate (glm::vec3(rect_pos.x+0.2,rect_pos.y+0.3,rect_pos.z));
  glm::mat4 rotateRectangle12n = glm::rotate((float)(-angleTan+0.5), glm::vec3(glm::vec3(0,0,1))); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectangle124n * rotateRectangle12n);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(gun);
  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateRectangle = glm::translate (b1_pos);
  //cout<<"basket at "<<b1_pos.x<<" and "<<b1_pos.y<<endl;      // glTranslatef // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(basket1);
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateRectangle1 = glm::translate (b2_pos);        // glTranslatef
  Matrices.model *= (translateRectangle1 );
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(basket2);
  for (int i=0;i<(bulletindex);i++)
  {
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateRectangle = glm::translate (glm::vec3(bulletx[i],bullety[i], 0));        // glTranslatef
  glm::mat4 bulletRotate = glm::rotate((float)(-bulletrot[i]+0.5), glm::vec3(0,0,1));
  bulletx[i] = bulletx[i] + 0.1;
  if (bulletx[i]>4)
  bulletx[i] = 200;
  bullety[i] = bullety[i] + ((cos(-bulletrot[i] - 1.085))/4);
  Matrices.model *= (translateRectangle * bulletRotate);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(bullet);
  for(int j=0;j<blackBrickIndex;j++)
  {
    bool aok = chckcollision(bulletx[i], blackx[j], bullety[i], blacky[j], 0.15, 0.4, 0.07,  0.4);
    if(aok)
    {
      ++score;
      blackx[j] = 100;
      bulletx[i] = 200;
    }
  }
}

if(bulletStatus == 1)
{
  bulletrot[bulletindex] = angleTanGun;
//cout << "The time now is "<<time_now<<endl;
bulletStatus = 0;
if(current_time-bullet_time>=1)
{
bullet_time = current_time;
++bulletindex;
Matrices.model = glm::mat4(1.0f);
glm::mat4 translateRectangle1246n = glm::translate (glm::vec3(rect_pos.x+0.2,rect_pos.y+0.3,rect_pos.z));
glm::mat4 rotateRectangle126n = glm::rotate((float)(-bulletrot[bulletindex]+0.5), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
bulletx[bulletindex] = rect_pos.x+0.4;
bullety[bulletindex] = rect_pos.y+0.2 + 0.75*sin(-angleTanGun+0.5);
bulletrot[bulletindex] = angleTanGun;
Matrices.model *= (translateRectangle1246n * rotateRectangle126n);
MVP = VP * Matrices.model;
glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
}
//draw3DObject(bullet[bulletindex]);
}
for (int i=0;i<(redBrickIndex);i++)
{
  bool tryna = chckcollision(redx[i],b1_pos.x, redy[i], b1_pos.y, 0.7, 0.3, 1, 0.4);
  if(tryna)
  {
    redx[i] = 200;
    ++score;
  }
Matrices.model = glm::mat4(1.0f);
glm::mat4 translateRectangle = glm::translate (glm::vec3(redx[i],redy[i], 0));        // glTranslatef
redy[i] = redy[i]-0.01;
glm::mat4 rotateRectangle = glm::translate(glm::vec3(redx[i],redy[i],0)); // rotate about vector (-1,1,1)
Matrices.model *= (translateRectangle * rotateRectangle);
MVP = VP * Matrices.model;
glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
draw3DObject(redBrick);
/*if(fabs(((redy[i]-4)-b1_pos.y))<0.5)
{
  if(fabs((redx[i]-redrand[i])-b1_pos.x)<0.4)
  {
    //++score;
    //cout<<"Basket is "<<b1_pos.x<<" and "<<b1_pos.y<<endl;
    //cout<<"Brick is "<<redx[i]-redrand[i]<<" and "<<redy[i]-4<<endl;
    //cout<<"Score is "<<score<<endl;
  }
}*/
// Increment angle

//camera_rotation_angle++; // Simulating camera rotation
}
if(time_now - time_beforered >= 1.5)
{
time_beforered = time_now;
Matrices.model = glm::mat4(1.0f);
++redBrickIndex;
redx[redBrickIndex] = randafred;
redrand[redBrickIndex] = randafred;
//cout<<"The random variable is: "<<randafred<<endl;
redy[redBrickIndex] = 4;
glm::mat4 translateRectangle = glm::translate (glm::vec3(redx[redBrickIndex],redy[redBrickIndex], 0));        // glTranslatef
glm::mat4 rotateRectangle = glm::translate(glm::vec3(redx[redBrickIndex],redy[redBrickIndex],0)); // rotate about vector (-1,1,1)
Matrices.model *= (translateRectangle * rotateRectangle);
MVP = VP * Matrices.model;
glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
draw3DObject(redBrick);
}

for (int i=0;i<(greenBrickIndex);i++)
{
  bool tryna = chckcollision(greenx[i],b2_pos.x, greeny[i], b2_pos.y,  0.7 , 0.3,1, 0.4);
  if(tryna)
  {
    greenx[i] = 200;
    ++score;
  }
Matrices.model = glm::mat4(1.0f);
glm::mat4 translateRectangle = glm::translate (glm::vec3(greenx[i],greeny[i], 0));        // glTranslatef
greeny[i] = greeny[i]-0.01;
glm::mat4 rotateRectangle = glm::translate(glm::vec3(greenx[i],greeny[i],0)); // rotate about vector (-1,1,1)
Matrices.model *= (translateRectangle * rotateRectangle);
MVP = VP * Matrices.model;

glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

draw3DObject(greenBrick);

// Increment angle

//camera_rotation_angle++; // Simulating camera rotation
}
if(time_now - time_before >= 1.5)
{
time_before = time_now;
Matrices.model = glm::mat4(1.0f);
++greenBrickIndex;
greenx[greenBrickIndex] = randaf;
greenrand[greenBrickIndex] = randaf;
//cout<<"The random variable is: "<<randafred<<endl;
greeny[greenBrickIndex] = 4;
glm::mat4 translateRectangle = glm::translate (glm::vec3(greenx[greenBrickIndex],greeny[greenBrickIndex], 0));        // glTranslatef
glm::mat4 rotateRectangle = glm::translate(glm::vec3(greenx[greenBrickIndex],greeny[greenBrickIndex],0)); // rotate about vector (-1,1,1)
Matrices.model *= (translateRectangle * rotateRectangle);
MVP = VP * Matrices.model;
glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
draw3DObject(greenBrick);
}

for (int i=0;i<(blackBrickIndex);i++)
{
Matrices.model = glm::mat4(1.0f);
glm::mat4 translateRectangle = glm::translate (glm::vec3(blackx[i],blacky[i], 0));        // glTranslatef
blacky[i] = blacky[i]-0.01;
glm::mat4 rotateRectangle = glm::translate(glm::vec3(blackx[i],blacky[i],0)); // rotate about vector (-1,1,1)
Matrices.model *= (translateRectangle * rotateRectangle);
MVP = VP * Matrices.model;
glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
draw3DObject(blackBrick);
// Increment angle

//camera_rotation_angle++; // Simulating camera rotation
}
if(time_now - time_beforeblack >= 1.5)
{
time_beforeblack = time_now;
//cout << "The time now is "<<time_now<<endl;
Matrices.model = glm::mat4(1.0f);
++blackBrickIndex;
blackx[blackBrickIndex] = randafblack;
//cout<<"The random variable is: "<<randafblack<<endl;
blacky[blackBrickIndex] = 4;
glm::mat4 translateRectangle = glm::translate (glm::vec3(blackx[blackBrickIndex],blacky[blackBrickIndex], 0));        // glTranslatef
glm::mat4 rotateRectangle = glm::translate(glm::vec3(blackx[blackBrickIndex],blacky[blackBrickIndex],0)); // rotate about vector (-1,1,1)
Matrices.model *= (translateRectangle * rotateRectangle);
MVP = VP * Matrices.model;
glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
draw3DObject(blackBrick);
}
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
//        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        glfwTerminate();
//        exit(EXIT_FAILURE);
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

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
	// Create the models
  createGun();
  createRectangle ();
  createBase();
  createBasket1();
  createBasket2();
  creategreenBrick();
  createredBrick();
  createblackBrick();
  createBullet();
  createMirror();
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (255, 255, 255, 255); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
  glEnable ( GL_LINE);
	glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 1000;
	int height = 1000;
  b2_pos = glm::vec3(1.8,-3.3,0);
  b1_pos = glm::vec3(-2.3,-3.3,0);
  rect_pos = glm::vec3(-4,-0.2,0);
  if(indexc==0)
  {
  time_before = glfwGetTime();
  indexc++;
}
if(indexred ==0 )
{
  time_beforered = glfwGetTime();
  indexred++;
}
if(indexblack == 0)
{
  time_beforeblack = glfwGetTime();
  indexblack++;
}
    GLFWwindow* window = initGLFW(width, height); // makes the window

	initGL (window, width, height); // intializes the window

    double last_update_time = glfwGetTime();
srand(time(NULL));
    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) { // as long as window is open?

        // OpenGL Draw commands
        glfwGetCursorPos	(window,&xpos,&ypos);
        angleTan = atan(ypos/xpos);
        randaf = (rand()%7)-1;
        randafred = (rand()%7)-1;
        randafblack = (rand()%7)-1;
        draw();

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window); // Swaps the front and back buffers of the specified window.

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        time_now = current_time;
        if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
            last_update_time = current_time; // every 0.5 seconds, time is updated
        }
    }

    glfwTerminate();
//    exit(EXIT_SUCCESS);
}
