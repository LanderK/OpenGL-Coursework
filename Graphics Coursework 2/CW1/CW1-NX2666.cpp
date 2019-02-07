#include "stdafx.h"
#include <windows.h>   // Standard Header For Most Programs
#include <gl/gl.h>     // The GL Header File
#include <gl/glut.h>   // The GL Utility Toolkit (Glut) Header
#include <math.h>      // For mathematic operation


char rendermode;// global variable for current rendering mode
float RotateY, RotateX, RotateZ;
GLuint Texture;
int leftButtonState, rightButtonState;
GLfloat eye[] = {5.0f,5.0f,10.0f};
GLfloat center[] = {0.0f,0.0f,0.0f};
GLfloat up[] = { 0.0f,1.0f,0.0f,1.0f};
/*
 * Scene initialisation
 */

void InitGL(GLvoid)
{
    glShadeModel(GL_SMOOTH);						// Enable smooth shading
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);			// Black background
    glClearDepth(1.0f);								// Depth buffer setup
    glEnable(GL_DEPTH_TEST);						// Enables depth testing
    glDepthFunc(GL_LEQUAL);						    // The type of depth testing to do
    glEnable(GL_COLOR_MATERIAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	
}

GLuint loadBMPToTexture(const char * imagepath) {
	//Code From http://www.opengl-tutorial.org/beginners-tutorials/tutorial-5-a-textured-cube/
	// Data read from the header of the BMP file
	unsigned char header[54]; // Each BMP file begins by a 54-bytes header
	unsigned int dataPos;     // Position in the file where the actual data begins
	unsigned int width, height;
	unsigned int imageSize;   // = width*height*3
							  // Actual RGB data
	unsigned char * data;
	// Open the file
	FILE * file = fopen(imagepath, "rb");
	if (!file) {
		printf("Image could not be opened\n");
		return 0;
	}

	if (fread(header, 1, 54, file) != 54) { // If not 54 bytes read : problem
		printf("Not a correct BMP file\n");
		return false;
	}

	if (header[0] != 'B' || header[1] != 'M') {
		printf("Not a correct BMP file\n");
		return 0;
	}

	// Read ints from the byte array
	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);

	// Some BMP files are misformatted, guess missing information
	if (imageSize == 0)    imageSize = width*height * 3; // 3 : one byte for each Red, Green and Blue component
	if (dataPos == 0)      dataPos = 54; // The BMP header is done that way

	 // Create a buffer
	data = new unsigned char[imageSize];

	// Read the actual data from the file into the buffer
	fread(data, 1, imageSize, file);

	//Everything is in memory now, the file can be closed
	fclose(file);
	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);
	
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);
	
	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, data);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	return 1;
	
}

void mouseZoom()
{
	if (leftButtonState){
		eye[0]= eye[0]/1.001f;
		eye[1]= eye[1]/1.001f;
		eye[2]= eye[2]/1.001f;

	}

	if (rightButtonState) {
		eye[0]=eye[0] * 1.001f;
		eye[1]=eye[1] * 1.001f;
		eye[2]=eye[2] * 1.001f;

	} 

}

void idle (void)
{
    glutPostRedisplay();   // trigger display callback
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	// set the camera
	mouseZoom();
	gluLookAt(eye[0], eye[1], eye[2],
            center[0], center[1], center[2],
			up[0], up[1], up[2]);
	GLfloat light_position[] = { 0.0f,1.0f,1.5f,0.0f };
	glLightfv(GL_LIGHT1, GL_POSITION, light_position);
	glPushMatrix();
	//Rotate Cube
	glRotatef(RotateY, 0.0f, 1.0f, 0.0f);
	glRotatef(RotateX, 1.0f, 0.0f, 0.0f);
	glRotatef(RotateZ, 0.0f, 0.0f, 1.0f);
	glEnable(GL_TEXTURE_2D);
	
    // different render mode
    switch ( rendermode ) {
            
        case 'f': // to display faces
		    //Front
			glBegin(GL_POLYGON);
			glColor3f(1.0f,0.0f,0.0f);
			glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f, 1.0f, 1.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 1.0f);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,-1.0f, 1.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,-1.0f, 1.0f);
			glEnd();
			//Top - Green
			glBegin(GL_POLYGON);
			glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, 1.0f, 1.0f);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1.0f, 1.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
			glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);
			glEnd();
			//Right 
			glBegin(GL_POLYGON);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, 1.0f, 1.0f);
			glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, -1.0f);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f);
			glEnd();
			//Bottom
			glBegin(GL_POLYGON);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, -1.0f, 1.0f);
			glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, -1.0f);
			glEnd();
			//Back
			glBegin(GL_POLYGON);
			glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, -1.0f, -1.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, 1.0f, -1.0f);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
			glEnd();
			//Left
			glBegin(GL_POLYGON);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
			glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 1.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
			glEnd();
            break;
            
        case 'v': // to display points
            glBegin(GL_POINTS);
            glColor3f(0.0f,1.0f,0.0f);
            glVertex3f( 1.0f, 1.0f, 1.0f);
            glVertex3f(-1.0f, 1.0f, 1.0f);
            glVertex3f(-1.0f,-1.0f, 1.0f);
            glVertex3f( 1.0f,-1.0f, 1.0f);
			glVertex3f(1.0f, 1.0f, -1.0f);
			glVertex3f(-1.0f, 1.0f, -1.0f);
			glVertex3f(-1.0f, -1.0f, -1.0f);
			glVertex3f(1.0f, -1.0f, -1.0f);
            glEnd();
            
            glPointSize(5);
            break;
            
        case 'e': // to display edges
            glBegin(GL_LINES);
            glColor3f(0.0f,0.0f,1.0f);
            //Front
            glVertex3f( -1.0f, 1.0f,1.0f);
            glVertex3f( -1.0f, -1.0f,1.0f);
            
            glVertex3f( -1.0f, 1.0f,1.0f);
            glVertex3f( 1.0f, 1.0f,1.0f);
            
            glVertex3f( -1.0f, -1.0f,1.0f);
            glVertex3f( 1.0f, -1.0f,1.0f);
            
            glVertex3f( 1.0f, -1.0f,1.0f);
            glVertex3f( 1.0f, 1.0f,1.0f);
            //Right
			glVertex3f(1.0f,1.0f,1.0f);
			glVertex3f(1.0f,1.0f,-1.0f);

			glVertex3f(1.0f,-1.0f,1.0f);
			glVertex3f(1.0f, -1.0f, -1.0f);

			glVertex3f(1.0f, -1.0f, -1.0f);
			glVertex3f(1.0f, 1.0f, -1.0f);
			//Top
			glVertex3f(1.0f, 1.0f, -1.0f);
			glVertex3f(-1.0f, 1.0f, -1.0f);

			glVertex3f(-1.0f, 1.0f, -1.0f);
			glVertex3f(-1.0f, 1.0f, 1.0f);
			//Left
			glVertex3f(-1.0f, 1.0f, -1.0f);
			glVertex3f(-1.0f, -1.0f, -1.0f);

			glVertex3f(-1.0f, -1.0f, 1.0f);
			glVertex3f(-1.0f, -1.0f, -1.0f);
			//Back
			glVertex3f(-1.0f, -1.0f, -1.0f);
			glVertex3f(1.0f, -1.0f, -1.0f);
            glEnd();
            break;
		case 'p'://Faces with phong Illumination
			glPushMatrix();// Push Matrix so Light isnt rotated with the cube
			glLoadIdentity();
			glEnable(GL_LIGHTING);
			glEnable(GL_LIGHT1);

		
			
			GLfloat light_amb[] = { 0.1f,0.4f ,0.1f, 1.0f };
			GLfloat light_diff[]= { 0.5f,0.5f ,0.5f, 1.0f };
			GLfloat light_spec[] = { 0.2f,0.8f ,0.2f, 1.0f };

			GLfloat refl_amb[] = { 0.1f,0.8f,0.1f,1.0f };
			GLfloat refl_diff[] = { 0.1f,0.8f,0.1f,1.0f };
			GLfloat refl_spec[] = { 1.0f,1.0f,1.0f,1.0f };
			
			glLightfv(GL_LIGHT1, GL_AMBIENT, light_amb);
			glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diff);
			glLightfv(GL_LIGHT1, GL_SPECULAR, light_spec);
			
			glPopMatrix();
			
			glBegin(GL_POLYGON);
			//Front
			glMaterialfv(GL_FRONT, GL_AMBIENT, refl_amb);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, refl_diff);
			glMaterialfv(GL_FRONT, GL_SPECULAR, refl_spec);
			
			glNormal3f(0.0f, 0.0f, 1.0f);
			glVertex3f(1.0f, 1.0f, 1.0f);
			glVertex3f(-1.0f, 1.0f, 1.0f);
			glVertex3f(-1.0f, -1.0f, 1.0f);
			glVertex3f(1.0f, -1.0f, 1.0f);
			glEnd();
			glBegin(GL_POLYGON);
			//Top 
			glNormal3f(0.0f,1.0f,0.0f);
			glVertex3f(1.0f, 1.0f, 1.0f);
			glVertex3f(-1.0f, 1.0f, 1.0f);
			glVertex3f(-1.0f, 1.0f, -1.0f);
			glVertex3f(1.0f, 1.0f, -1.0f);
			glEnd();
			//Right 
			glBegin(GL_POLYGON);
			glNormal3f(1.0f, 0.0f, 0.0f);
			glVertex3f(1.0f, 1.0f, 1.0f);
			glVertex3f(1.0f, 1.0f, -1.0f);
			glVertex3f(1.0f, -1.0f, -1.0f);
			glVertex3f(1.0f, -1.0f, 1.0f);
			glEnd();
			//Bottom-
			glBegin(GL_POLYGON);
			glNormal3f(0.0f, -1.0f, 0.0f);
			glVertex3f(1.0f, -1.0f, 1.0f);
			glVertex3f(-1.0f, -1.0f, 1.0f);
			glVertex3f(-1.0f, -1.0f, -1.0f);
			glVertex3f(1.0f, -1.0f, -1.0f);
			glEnd();
			//Back 
			glBegin(GL_POLYGON);
			glNormal3f(0.0f, 0.0f, -1.0f);
			glVertex3f(1.0f, -1.0f, -1.0f);
			glVertex3f(1.0f, 1.0f, -1.0f);
			glVertex3f(-1.0f, 1.0f, -1.0f);
			glVertex3f(-1.0f, -1.0f, -1.0f);
			glEnd();
			//Left-
			glBegin(GL_POLYGON);
			glNormal3f(-1.0f, 0.0f, 0.0f);
			glVertex3f(-1.0f, -1.0f, -1.0f);
			glVertex3f(-1.0f, 1.0f, -1.0f);
			glVertex3f(-1.0f, 1.0f, 1.0f);
			glVertex3f(-1.0f, -1.0f, 1.0f);
			glEnd();
			break;
    }

	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT1);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
    //Draw Dotted Cartesian coordinate system as lines
	glPushAttrib(GL_ENABLE_BIT);
	glLineStipple(2, 0xAAAA); 
	glEnable(GL_LINE_STIPPLE);
	glBegin(GL_LINES);
	glColor3f(1.0f, 1.0f, 1.0f);
	//X-axis
	glVertex3f(-3.0f,0.0f,0.0f);
	glVertex3f(3.0f, 0.0f, 0.0f);
	//Y-axis
	glVertex3f(0.0f, -3.0f, 0.0f);
	glVertex3f(0.0f, 3.0f, 0.0f);
	//Z-axis
	glVertex3f(0.0f, 0.0f, -3.0f);
	glVertex3f(0.0f, 0.0f, 3.0f);
	glEnd();
	glPopAttrib();
   
	glutSwapBuffers();
}

void rotateCamera(float angle, char dir) {


	float h = 1.0f; //homogeneous Variable
	GLfloat tempEx = eye[0];
	GLfloat tempEy = eye[1];
	GLfloat tempEz = eye[2];
	GLfloat tempx = up[0];
	GLfloat tempy = up[1];
	GLfloat tempz = up[2];
	GLfloat tempw = up[3];
		
	if (dir == 'x') {

		//Rotational Matrix Around X
		GLfloat Rx[4][4] = { { 1.0f , 0.0f, 0.0f, 0.0f },
							 { 0.0f, cosf(angle), -sinf(angle), 0.0f },
		                     { 0.0f, sinf(angle), cosf(angle), 0.0f },
		                     { 0.0f, 0.0f, 0.0f, 1.0f } };

		tempEx = eye[0] * Rx[0][0] + eye[1] * Rx[0][1] + eye[2] * Rx[0][2] + h*Rx[0][3];
		tempEy = eye[0] * Rx[1][0] + eye[1] * Rx[1][1] + eye[2] * Rx[1][2] + h*Rx[1][3];
		tempEz = eye[0] * Rx[2][0] + eye[1] * Rx[2][1] + eye[2] * Rx[2][2] + h*Rx[2][3];
		    h = eye[0] * Rx[3][0] + eye[1] * Rx[3][1] + eye[2] * Rx[3][2] + h*Rx[3][3];

		//tempx = up[0] * Rx[0][0] + up[1] * Rx[0][1] + up[2] * Rx[0][2] + up[3] * Rx[0][3];
		//tempy = up[0] * Rx[1][0] + up[1] * Rx[1][1] + up[2] * Rx[1][2] + up[3] * Rx[1][3];
		//tempz = up[0] * Rx[2][0] + up[1] * Rx[2][1] + up[2] * Rx[2][2] + up[3] * Rx[2][3];
		//tempw = up[0] * Rx[3][0] + up[1] * Rx[3][1] + up[2] * Rx[3][2] + up[3] * Rx[3][3];

	}

	else if (dir == 'y') {

		//Rotationall Matrix Around Y
		GLfloat Ry[4][4] = { { cosf(angle), 0.0f, sinf(angle), 0.0f },
		                     { 0.0f, 1.0f, 0.0f, 0.0f },
		                     { -sinf(angle), 0.0f, cosf(angle), 0.0f },
		                     { 0.0f, 0.0f, 0.0f, 1.0f } };

		tempEx = eye[0] * Ry[0][0] + eye[1] * Ry[0][1] + eye[2] * Ry[0][2] + h*Ry[0][3];
		tempEy = eye[0] * Ry[1][0] + eye[1] * Ry[1][1] + eye[2] * Ry[1][2] + h*Ry[1][3];
		tempEz = eye[0] * Ry[2][0] + eye[1] * Ry[2][1] + eye[2] * Ry[2][2] + h*Ry[2][3];
		    h = eye[0] * Ry[3][0] + eye[1] * Ry[3][1] + eye[2] * Ry[3][2] + h*Ry[3][3];

		//tempx = up[0] * Ry[0][0] + up[1] * Ry[0][1] + up[2] * Ry[0][2] + up[3] * Ry[0][3];
		//tempy = up[0] * Ry[1][0] + up[1] * Ry[1][1] + up[2] * Ry[1][2] + up[3] * Ry[1][3];
		//tempz = up[0] * Ry[2][0] + up[1] * Ry[2][1] + up[2] * Ry[2][2] + up[3] * Ry[2][3];
		//tempw = up[0] * Ry[3][0] + up[1] * Ry[3][1] + up[2] * Ry[3][2] + up[3] * Ry[3][3];
	}

	else if (dir == 'z') {

		GLfloat Rz[4][4] = { { cosf(angle), -sinf(angle), 0.0f, 0.0f },
							 { sinf(angle), cosf(angle), 0.0f, 0.0f },
							 { 0.0f, 0.0f, 1.0f, 0.0f },
							 { 0.0f, 0.0f, 0.0f, 1.0f } };

		tempEx = eye[0] * Rz[0][0] + eye[1] * Rz[0][1] + eye[2] * Rz[0][2] + h*Rz[0][3];
		tempEy = eye[0] * Rz[1][0] + eye[1] * Rz[1][1] + eye[2] * Rz[1][2] + h*Rz[1][3];
		tempEz = eye[0] * Rz[2][0] + eye[1] * Rz[2][1] + eye[2] * Rz[2][2] + h*Rz[2][3];
		    h = eye[0] * Rz[3][0] + eye[1] * Rz[3][1] + eye[2] * Rz[3][2] + h*Rz[3][3];

		tempx = up[0] * Rz[0][0] + up[1] * Rz[0][1] + up[2] * Rz[0][2] + up[3] * Rz[0][3];
		tempy = up[0] * Rz[1][0] + up[1] * Rz[1][1] + up[2] * Rz[1][2] + up[3] * Rz[1][3];
		tempz = up[0] * Rz[2][0] + up[1] * Rz[2][1] + up[2] * Rz[2][2] + up[3] * Rz[2][3];
		tempw = up[0] * Rz[3][0] + up[1] * Rz[3][1] + up[2] * Rz[3][2] + up[3] * Rz[3][3];

	}
	
	eye[0] = tempEx / h;
	eye[1] = tempEy / h;
	eye[2] = tempEz / h;

	up[0] = tempx / up[3];
	up[1] = tempy / up[3];
	up[2] = tempz / up[3];
	up[3] = tempw / up[3];
	
}
/*
 * The reshape function sets up the viewport and projection
 */
void reshape ( int width, int height )
{
    // Prevent a divide by zero error by making height equal 1
    if (height==0)
    {
        height=1;
    }
    
    glViewport(0,0,width,height);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    // Need to calculate the aspect ratio of the window for gluPerspective
    gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);
    
    // Return to ModelView mode for future operations
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/*
 * Callback for standard keyboard presses
 */
void keyboard ( unsigned char key, int x, int y )
{
    switch(key) {
        // Exit the program when escape is pressed
        case 27:
            exit(0);
            break;
        
        // Switch render mode for v,e,f
        case 'v':
            rendermode='v';
            break;
            
        case 'l':
            rendermode='e';
            break;
            
        case 'f':
            rendermode='f';
            break;
		case 'p':
			rendermode = 'p';
			break;
		case 'd':
			RotateY++;
			break;
		case 'a':
			RotateY--;
			break;
		case 'w':
			RotateX--;
			break;
		case 's':
			RotateX++;
			break;
		case 'q':
			RotateZ++;
			break;
		case 'e':
			RotateZ--;
			break;
		case'W':
			rotateCamera(-0.05f,'x');
			break;
		case'S':
			rotateCamera(0.05f, 'x');
			break;
		case'A':
			rotateCamera(0.05f, 'y');
			break;
		case'D':
			rotateCamera(-0.05f, 'y');
			break;
		case'Q':
			rotateCamera(0.05f, 'z');
			break;
		case'E':
			rotateCamera(-0.05f, 'z');
			break;
		default:
            break;
    }
    
    glutPostRedisplay();
}



// Arrow keys need to be handled in a separate function from other keyboard presses
void arrow_keys ( int a_keys, int x, int y )
{
    switch ( a_keys ) {
        case GLUT_KEY_UP:
		//	eye[1] = eye[1] + 0.1f;
		//	center[1] = center[1] + 0.1f;
            break;
        case GLUT_KEY_DOWN:
			//eye[1] = eye[1] - 0.1f;
			//center[1] = center[1] - 0.1f;
            break;
		case GLUT_KEY_RIGHT:
			eye[0] = eye[0] + 0.1f;
			center[0] = center[0] + 0.1f;
			break;
		case GLUT_KEY_LEFT:
			eye[0] = eye[0] - 0.1f;
			center[0] = center[0] - 0.1f;
			break;
        default:
            break;
    }
}

void mouseButton(int button, int state, int x, int y)
{
	switch (button) {
		case GLUT_LEFT_BUTTON :
			leftButtonState = (state == GLUT_DOWN);
			
			break;
		
		case GLUT_RIGHT_BUTTON :
			rightButtonState = (state == GLUT_DOWN);
			break;
		default:
			break;

	}
}

void mouseMove(int x, int y)
{
}



//*** Note: You may wish to add interactivity like clicking and dragging to move the camera.
//***       If so, use the above functions.


/*
 * Entry point to the application
 */
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitWindowSize(500, 500);
    glutCreateWindow("CW1 OpenGL Framework");
//    glutFullScreen();          // Uncomment to start in full screen
    InitGL();
    rendermode='f';
	Texture = loadBMPToTexture("bricks.bmp");
	
    
    // Callback functions
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(arrow_keys);  // For special keys
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMove);
    glutIdleFunc(idle);
    
    glutMainLoop();
}