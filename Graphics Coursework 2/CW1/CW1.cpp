#include "stdafx.h"
#include <windows.h>   // Standard Header For Most Programs
#include <gl/gl.h>     // The GL Header File
#include <gl/glut.h>   // The GL Utility Toolkit (Glut) Header
#include <math.h>      // For mathematic operation
#include <vector>
#include <iostream>
#include "Eigen/Core"
#include "Eigen/Eigenvalues"
#include "objloader.h"
#include "OpenGLSupport"


char rendermode, meshmode, boundmode;// global variable for current rendering mode
float RotateY, RotateX, RotateZ, alpha = 1.0f, planeZ= 0.0f;
int plane = 0;
int leftButtonState, rightButtonState; //Global variable to Store the Current mouse button being pressed
//Camera Variables
GLfloat eye[] = {5.0f,5.0f,10.0f};
GLfloat center[] = {0.0f,0.0f,0.0f};
GLfloat up[] = { 0.0f,1.0f,0.0f,1.0f};
//Eigen Variables
Eigen::Vector3d abbMin, abbMax, obbMin, obbMax; // Min max values
Eigen::Matrix3d eigV; //Eigen Vectors of Cov Matrix
Eigen::Vector3d obbVertices[8]; //Obb Vertex's
std::vector<Eigen::Vector3d> vertices; //Mesh
std::vector<Eigen::Vector3d> faceIndices; //Mesh
/*
 * Scene initialisation
 */
void rotateMesh(float,char);
void InitGL(GLvoid)
{
    glShadeModel(GL_SMOOTH);						// Enable smooth shading
    glClearColor(0.1f, 0.1f, 0.1f, 0.5f);			// Black background
    glClearDepth(1.0f);								// Depth buffer setup
    glEnable(GL_DEPTH_TEST);						// Enables depth testing
    glDepthFunc(GL_LEQUAL);						    // The type of depth testing to do
    glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_NORMALIZE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
	//Decrements or Increments the Eye postion on the camera
	if (leftButtonState){
		eye[0]= eye[0]/1.01f;
		eye[1]= eye[1]/1.01f;
		eye[2]= eye[2]/1.01f;

	}

	if (rightButtonState) {
		eye[0]=eye[0] * 1.01f;
		eye[1]=eye[1] * 1.01f;
		eye[2]=eye[2] * 1.01f;

	} 

}

void generateAbb(void) {
	
	abbMin = vertices[0];
	abbMax = vertices[0];
	//Check Min Max
	for (unsigned int i = 0; i < vertices.size(); i++) {
		if (vertices[i](0) < abbMin(0)) abbMin(0) = vertices[i](0); //Min X
		if (vertices[i](1) < abbMin(1)) abbMin(1) = vertices[i](1);	//Min y
		if (vertices[i](2) < abbMin(2)) abbMin(2) = vertices[i](2); //Min z 
		if (vertices[i](0) > abbMax(0)) abbMax(0) = vertices[i](0); //Max x
		if (vertices[i](1) > abbMax(1)) abbMax(1) = vertices[i](1); //Max y
		if (vertices[i](2) > abbMax(2)) abbMax(2) = vertices[i](2); //Max Z
	}
}

void drawAbb(void) {

	generateAbb(); //Only do this at start and when the mesh changes 
	alpha = 0.2f;

	glBegin(GL_QUADS);
	glColor4f(0.0f, 1.0f, 1.0f, alpha);
	//Back
	glVertex3f(abbMax(0), abbMin(1), abbMin(2));
	glVertex3f(abbMax(0), abbMax(1), abbMin(2));
	glVertex3f(abbMin(0), abbMax(1), abbMin(2));
	glVertex3f(abbMin(0), abbMin(1), abbMin(2));
	//Bottom
	glVertex3f(abbMax(0), abbMin(1), abbMax(2));
	glVertex3f(abbMin(0), abbMin(1), abbMax(2));
	glVertex3f(abbMin(0), abbMin(1), abbMin(2));
	glVertex3f(abbMax(0), abbMin(1), abbMin(2));
	//Left
	glVertex3f(abbMin(0), abbMax(1), abbMin(2));
	glVertex3f(abbMin(0), abbMax(1), abbMax(2));
	glVertex3f(abbMin(0), abbMin(1), abbMax(2));
	glVertex3f(abbMin(0), abbMin(1), abbMin(2));
	//Right 
	glVertex3f(abbMax(0), abbMax(1), abbMax(2));
	glVertex3f(abbMax(0), abbMax(1), abbMin(2));
	glVertex3f(abbMax(0), abbMin(1), abbMin(2));
	glVertex3f(abbMax(0), abbMin(1), abbMax(2));
	//Top
	glVertex3f(abbMax(0), abbMax(1), abbMax(2));
	glVertex3f(abbMin(0), abbMax(1), abbMax(2));
	glVertex3f(abbMin(0), abbMax(1), abbMin(2));
	glVertex3f(abbMax(0), abbMax(1), abbMin(2));
	//Front
	glVertex3f(abbMax(0), abbMax(1), abbMax(2));
	glVertex3f(abbMax(0), abbMin(1), abbMax(2));
	glVertex3f(abbMin(0), abbMin(1), abbMax(2));
	glVertex3f(abbMin(0), abbMax(1), abbMax(2));


	glEnd();
}

int variable(char v) {
	//returns the vertex number of a given axis
	int var;
	if (v == 'x') {
		var = 0;
	}
	else if (v == 'y') {
		var = 1;
	}
	else if (v == 'z') {
		var = 2;
	}

	return var;

}

float mean(char v) {

	float mean = 0.0f;
	for (unsigned int i = 0; i < vertices.size(); i++) {
		mean += vertices[i](variable(v));
	}
	
	mean = mean / (vertices.size());

	return mean;

}

float Covariance(char var1, char var2) {

	float var1Mean = mean(var1);
	float var2Mean = mean(var2);

	float Covariance = 0.0f;
	//Calulates the Covariance of var1, var 2
	for (unsigned int i = 0; i < vertices.size(); i++) {
		float sumTerm = (vertices[i](variable(var1))- var1Mean) * (vertices[i](variable(var2)) - var2Mean);
		Covariance += sumTerm;
	}

	Covariance = Covariance / vertices.size();
	
	return Covariance;

}

Eigen::Matrix3d computeCovarianceMatrix(void) {

	double CovXX = Covariance('x', 'x');
	double CovXY = Covariance('x', 'y');
	double CovXZ = Covariance('x', 'z');
	double CovYX = Covariance('y', 'x');
	double CovYY = Covariance('y', 'y');
	double CovYZ = Covariance('y', 'z');
	double CovZX = Covariance('z', 'x');
	double CovZY = Covariance('z', 'y');
	double CovZZ = Covariance('z', 'z');

	Eigen::Matrix3d m;
	m << CovXX, CovXY, CovXZ,
		 CovYX, CovYY, CovYZ,
		 CovZX, CovZY, CovZZ;
	
	return m;
	
}

void MinMaxObb() {

	//Mean Vector
	Eigen::Vector3d meanVector(mean('x'),mean('y'),mean('z'));
	obbMin(0) = obbMax(0) = mean('x');
	obbMin(1) = obbMax(1) = mean('y');
	obbMin(2) = obbMax(2) = mean('z');
	for (unsigned int i = 0; i < vertices.size(); i++) {
		//put them into the Eigen Vector reference frame
		Eigen::Vector3d newPos = vertices[i] - meanVector;
		Eigen::Vector3d newVert = eigV * newPos;
		//check min of max
		if (newVert(0) <= obbMin(0)) obbMin(0) = newVert(0); //Min X
		if (newVert(1) <= obbMin(1)) obbMin(1) = newVert(1); //Min y
		if (newVert(2) <= obbMin(2)) obbMin(2) = newVert(2); //Min z 
		if (newVert(0) >= obbMax(0)) obbMax(0) = newVert(0); //Max x
		if (newVert(1) >= obbMax(1)) obbMax(1) = newVert(1); //Max y
		if (newVert(2) >= obbMax(2)) obbMax(2) = newVert(2); //Max Z
	}
	//Set Vertices to draw
	obbVertices[0](0) = obbVertices[4](0) = obbVertices[7](0) = obbVertices[3](0) = obbMax(0);
	obbVertices[0](1) = obbVertices[4](1) = obbVertices[5](1) = obbVertices[1](1) = obbMax(1);
	obbVertices[0](2) = obbVertices[1](2) = obbVertices[2](2) = obbVertices[3](2) = obbMax(2);
	obbVertices[2](0) = obbVertices[6](0) = obbVertices[5](0) = obbVertices[1](0) = obbMin(0);
	obbVertices[2](1) = obbVertices[3](1) = obbVertices[7](1) = obbVertices[6](1) = obbMin(1);
	obbVertices[5](2) = obbVertices[4](2) = obbVertices[7](2) = obbVertices[6](2) = obbMin(2);
	
	//Put them in the global reference frame
	for (unsigned int i = 0; i < 8; i++) {
		obbVertices[i] = (eigV.inverse() * obbVertices[i]) + meanVector;
	}

} 

void drawObb(void) {

	alpha = 0.2f;
	
	glBegin(GL_QUADS);
	glColor4f(1.0f, 1.0f, 0.0f, alpha);
	//Back
	glVertex3f(obbVertices[4](0), obbVertices[4](1), obbVertices[4](2));
	glVertex3f(obbVertices[5](0), obbVertices[5](1), obbVertices[5](2));
	glVertex3f(obbVertices[6](0), obbVertices[6](1), obbVertices[6](2));
	glVertex3f(obbVertices[7](0), obbVertices[7](1), obbVertices[7](2));
	//Bottom
	glVertex3f(obbVertices[3](0), obbVertices[3](1), obbVertices[3](2));
	glVertex3f(obbVertices[2](0), obbVertices[2](1), obbVertices[2](2));
	glVertex3f(obbVertices[6](0), obbVertices[6](1), obbVertices[6](2));
	glVertex3f(obbVertices[7](0), obbVertices[7](1), obbVertices[7](2));
	//Left
	glVertex3f(obbVertices[1](0), obbVertices[1](1), obbVertices[1](2));
	glVertex3f(obbVertices[5](0), obbVertices[5](1), obbVertices[5](2));
	glVertex3f(obbVertices[6](0), obbVertices[6](1), obbVertices[6](2));
	glVertex3f(obbVertices[2](0), obbVertices[2](1), obbVertices[2](2));
	//Right 
	glVertex3f(obbVertices[4](0), obbVertices[4](1), obbVertices[4](2));
	glVertex3f(obbVertices[0](0), obbVertices[0](1), obbVertices[0](2));
	glVertex3f(obbVertices[3](0), obbVertices[3](1), obbVertices[3](2));
	glVertex3f(obbVertices[7](0), obbVertices[7](1), obbVertices[7](2));
	//Top
	glVertex3f(obbVertices[4](0), obbVertices[4](1), obbVertices[4](2));
	glVertex3f(obbVertices[5](0), obbVertices[5](1), obbVertices[5](2));
	glVertex3f(obbVertices[1](0), obbVertices[1](1), obbVertices[1](2));
	glVertex3f(obbVertices[0](0), obbVertices[0](1), obbVertices[0](2));
	//Front
	glVertex3f(obbVertices[0](0), obbVertices[0](1), obbVertices[0](2));
	glVertex3f(obbVertices[1](0), obbVertices[1](1), obbVertices[1](2));
	glVertex3f(obbVertices[2](0), obbVertices[2](1), obbVertices[2](2));
	glVertex3f(obbVertices[3](0), obbVertices[3](1), obbVertices[3](2));


	glEnd();
}

void generateObb(void) {
	
	//Calculate Covariance Matrix
	Eigen::Matrix3d covarianceMatrix = computeCovarianceMatrix();
	//Finds the Unit EigenVectors and Corresponding EigenValues
	Eigen::EigenSolver<Eigen::MatrixXd> covEigen(covarianceMatrix);
	//std::cout << "eigenvectors:" << std::endl << covEigen.eigenvectors() << std::endl;
	eigV = covEigen.eigenvectors().real();
	MinMaxObb();

}
Eigen::Vector3d linearInterp(Eigen::Vector3d v1, Eigen::Vector3d v2) {
	
	//Linerar Interpolation
	double x = v1(0) + ((v1(0) - v2(0)) * ((planeZ - v1(2)) / (v1(2) - v2(2))));
	double y = v1(1) + ((v1(1) - v2(1)) * ((planeZ - v1(2)) / (v1(2) - v2(2))));

	Eigen::Vector3d p(x,y,planeZ);
	return p;
	
}

std::vector<Eigen::Vector3d> intersect(Eigen::Vector3d V) {
	
	//new Vector to store the old points and Intersecting points
	std::vector<Eigen::Vector3d> points;
	//Vectors 1 & 2 infront
	if (vertices[V(0) - 1](2) >= planeZ && vertices[V(1) - 1](2) >= planeZ) {
		points.push_back(linearInterp(vertices[V(1) - 1], vertices[V(2) - 1]));
		points.push_back(linearInterp(vertices[V(0) - 1], vertices[V(2) - 1]));
		points.push_back(vertices[V(0) - 1]);
		points.push_back(vertices[V(1) - 1]);
	}
	//Vectors 2 & 3 infront
	else if (vertices[V(1) - 1](2) >= planeZ && vertices[V(2) - 1](2) >= planeZ) {
		points.push_back(linearInterp(vertices[V(1) - 1], vertices[V(0) - 1]));
		points.push_back(linearInterp(vertices[V(2) - 1], vertices[V(0) - 1]));
		points.push_back(vertices[V(1) - 1]);
		points.push_back(vertices[V(2) - 1]);
	}
	//Vectors 1 & 3 infront
	else if (vertices[V(0) - 1](2) >= planeZ && vertices[V(2) - 1](2) >= planeZ) {
		points.push_back(linearInterp(vertices[V(0) - 1], vertices[V(1) - 1]));
		points.push_back(linearInterp(vertices[V(2) - 1], vertices[V(1) - 1]));
		points.push_back(vertices[V(0) - 1]);
		points.push_back(vertices[V(2) - 1]);
	}
	//Only Vector 1 infront
	else if (vertices[V(0) - 1](2) >= planeZ) {
		points.push_back(linearInterp(vertices[V(0) - 1], vertices[V(1) - 1]));
		points.push_back(linearInterp(vertices[V(0) - 1], vertices[V(2) - 1]));
		points.push_back(vertices[V(0) - 1]);
		
	}
	//Only Vector2 infront
	else if (vertices[V(1) - 1](2) >= planeZ) {
		points.push_back(linearInterp(vertices[V(1) - 1], vertices[V(0) - 1]));
		points.push_back(linearInterp(vertices[V(1) - 1], vertices[V(2) - 1]));
		points.push_back(vertices[V(1) - 1]);
	}
	//Only Vector 3 infront
	else if (vertices[V(2) - 1](2) >= planeZ) {
		points.push_back(linearInterp(vertices[V(2) - 1], vertices[V(0) - 1]));
		points.push_back(linearInterp(vertices[V(2) - 1], vertices[V(1) - 1]));
		points.push_back(vertices[V(2) - 1]);
	
	}

	return points;
}	

void drawPartialMesh() {
	
	switch (meshmode) {
		case 'f':
			glBegin(GL_TRIANGLES);

			glColor4f(1.0f, 0.0f, 0.0f, alpha);
			for (unsigned int i = 0; i < faceIndices.size(); i++) {
				Eigen::Vector3d vertexIndex = faceIndices[i];
				//All points of triangle infront of Plane
				if (vertices[vertexIndex(0) - 1](2) >= planeZ && vertices[vertexIndex(1) - 1](2) >= planeZ && vertices[vertexIndex(2) - 1](2) >= planeZ) {
					glVertex3f(vertices[vertexIndex(0) - 1](0), vertices[vertexIndex(0) - 1](1), vertices[vertexIndex(0) - 1](2));
					glVertex3f(vertices[vertexIndex(1) - 1](0), vertices[vertexIndex(1) - 1](1), vertices[vertexIndex(1) - 1](2));
					glVertex3f(vertices[vertexIndex(2) - 1](0), vertices[vertexIndex(2) - 1](1), vertices[vertexIndex(2) - 1](2));
				}
				//Some Points of the Triangle in front of the plane
				else if (vertices[vertexIndex(0) - 1](2) >= planeZ || vertices[vertexIndex(1) - 1](2) >= planeZ || vertices[vertexIndex(2) - 1](2) >= planeZ) {
					std::vector<Eigen::Vector3d> newPoints =  intersect(vertexIndex);
					//2 point behind
					if (newPoints.size() == 3) {
						glVertex3f(newPoints[0](0), newPoints[0](1), newPoints[0](2));
						glVertex3f(newPoints[1](0), newPoints[1](1), newPoints[1](2));
						glVertex3f(newPoints[2](0), newPoints[2](1), newPoints[2](2));
					}
					//1 point behind
					else if(newPoints.size() == 4) {
						glVertex3f(newPoints[0](0), newPoints[0](1), newPoints[0](2));
						glVertex3f(newPoints[1](0), newPoints[1](1), newPoints[1](2));
						glVertex3f(newPoints[2](0), newPoints[2](1), newPoints[2](2));

						glVertex3f(newPoints[2](0), newPoints[2](1), newPoints[2](2));
						glVertex3f(newPoints[0](0), newPoints[0](1), newPoints[0](2));
						glVertex3f(newPoints[3](0), newPoints[3](1), newPoints[3](2));
					}
				}


			}
			glEnd();
			break;
		case 'l':
			glBegin(GL_LINES);
			glColor4f(0.0f, 0.75f, 1.0f, alpha);
			for (unsigned int i = 0; i < faceIndices.size(); i++) {
				Eigen::Vector3d vertexIndex = faceIndices[i];
				//Draw the Lines with all points and infront of the Intersecting Plane
				if (vertices[vertexIndex(0) - 1](2) >= planeZ && vertices[vertexIndex(1) - 1](2) >= planeZ && vertices[vertexIndex(2) - 1](2) >= planeZ) {
					glVertex3f(vertices[vertexIndex(0) - 1](0), vertices[vertexIndex(0) - 1](1), vertices[vertexIndex(0) - 1](2));
					glVertex3f(vertices[vertexIndex(1) - 1](0), vertices[vertexIndex(1) - 1](1), vertices[vertexIndex(1) - 1](2));

					glVertex3f(vertices[vertexIndex(1) - 1](0), vertices[vertexIndex(1) - 1](1), vertices[vertexIndex(1) - 1](2));
					glVertex3f(vertices[vertexIndex(2) - 1](0), vertices[vertexIndex(2) - 1](1), vertices[vertexIndex(2) - 1](2));

					glVertex3f(vertices[vertexIndex(2) - 1](0), vertices[vertexIndex(2) - 1](1), vertices[vertexIndex(2) - 1](2));
					glVertex3f(vertices[vertexIndex(0) - 1](0), vertices[vertexIndex(0) - 1](1), vertices[vertexIndex(0) - 1](2));
				}
				//Draw the Lines with new Intersecting Points
				else if(vertices[vertexIndex(0) - 1](2) >= planeZ || vertices[vertexIndex(1) - 1](2) >= planeZ || vertices[vertexIndex(2) - 1](2) >= planeZ) {
					std::vector<Eigen::Vector3d> newPoints = intersect(vertexIndex);
					//2 Points Behind
					if (newPoints.size() == 3) {
						glVertex3f(newPoints[0](0), newPoints[0](1), newPoints[0](2));
						glVertex3f(newPoints[1](0), newPoints[1](1), newPoints[1](2));

						glVertex3f(newPoints[1](0), newPoints[1](1), newPoints[1](2));
						glVertex3f(newPoints[2](0), newPoints[2](1), newPoints[2](2));

						glVertex3f(newPoints[2](0), newPoints[2](1), newPoints[2](2));
						glVertex3f(newPoints[0](0), newPoints[0](1), newPoints[0](2));
					}
					//1 Point Behind
					else if (newPoints.size() == 4) {
						glVertex3f(newPoints[0](0), newPoints[0](1), newPoints[0](2));
						glVertex3f(newPoints[1](0), newPoints[1](1), newPoints[1](2));

						glVertex3f(newPoints[1](0), newPoints[1](1), newPoints[1](2));
						glVertex3f(newPoints[2](0), newPoints[2](1), newPoints[2](2));

						glVertex3f(newPoints[2](0), newPoints[2](1), newPoints[2](2));
						glVertex3f(newPoints[3](0), newPoints[3](1), newPoints[3](2));

						glVertex3f(newPoints[3](0), newPoints[3](1), newPoints[3](2));
						glVertex3f(newPoints[0](0), newPoints[0](1), newPoints[0](2));
					}
				}
			}
			glEnd();
		case 'v':
			
			glBegin(GL_POINTS);
			glColor4f(0.0f, 1.0f, 0.0f, alpha);
			//Draws all the points in the mesh that have greater Z value
			for (unsigned int i = 0; i < vertices.size(); i++) {
				if(vertices[i](2) >= planeZ) glVertex3f(vertices[i](0), vertices[i](1), vertices[i](2));
			}
			glEnd();
			glPointSize(1);
			break;
		default:
			break;
	}
}

void drawFullMesh() {

	switch (meshmode) {
		case 'f':
			glBegin(GL_TRIANGLES);

			glColor4f(1.0f, 0.0f, 0.0f, alpha);
			//Draw a triangle for each the vertices in FaceIndices
			for (unsigned int i = 0; i < faceIndices.size(); i++) {
				Eigen::Vector3d vertexIndex = faceIndices[i];
				glVertex3f(vertices[vertexIndex(0) - 1](0), vertices[vertexIndex(0) - 1](1), vertices[vertexIndex(0) - 1](2));
				glVertex3f(vertices[vertexIndex(1) - 1](0), vertices[vertexIndex(1) - 1](1), vertices[vertexIndex(1) - 1](2));
				glVertex3f(vertices[vertexIndex(2) - 1](0), vertices[vertexIndex(2) - 1](1), vertices[vertexIndex(2) - 1](2));
			}
			glEnd();
			break;
		case 'l':
			glBegin(GL_LINES);
			//Draw line Connecting each Vertex in FaceIndices
			glColor4f(0.0f, 0.75f, 1.0f, alpha);
			for (unsigned int i = 0; i < faceIndices.size(); i++) {
				Eigen::Vector3d vertexIndex = faceIndices[i];
				glVertex3f(vertices[vertexIndex(0) - 1](0), vertices[vertexIndex(0) - 1](1), vertices[vertexIndex(0) - 1](2));
				glVertex3f(vertices[vertexIndex(1) - 1](0), vertices[vertexIndex(1) - 1](1), vertices[vertexIndex(1) - 1](2));

				glVertex3f(vertices[vertexIndex(1) - 1](0), vertices[vertexIndex(1) - 1](1), vertices[vertexIndex(1) - 1](2));
				glVertex3f(vertices[vertexIndex(2) - 1](0), vertices[vertexIndex(2) - 1](1), vertices[vertexIndex(2) - 1](2));

				glVertex3f(vertices[vertexIndex(2) - 1](0), vertices[vertexIndex(2) - 1](1), vertices[vertexIndex(2) - 1](2));
				glVertex3f(vertices[vertexIndex(0) - 1](0), vertices[vertexIndex(0) - 1](1), vertices[vertexIndex(0) - 1](2));
			}
			glEnd();
		case 'v':
			glBegin(GL_POINTS);
			glPointSize(1);
			glColor4f(0.0f, 1.0f, 0.0f, alpha);
			//Draws all the points in the mesh
			for (unsigned int i = 0; i < vertices.size(); i++) {
				glVertex3f(vertices[i](0), vertices[i](1), vertices[i](2));
			}
			glEnd();
			break;
		default:
			break;
	}
}

void scaleAndTranslate() {

	//Matrix to scale by 0.45 and translate by (-.3,-0.2,0.1)
	GLfloat sf[4][4] = { 0.45, 0, 0, -0.3,
		         0, 0.45, 0, -0.2,
		         0, 0, 0.45, 0.1,
		         0, 0, 0, 1 };
	
	//Matrix Multiplication for each point
	for (unsigned int i = 0; i < vertices.size(); i++) {
		float tempx = vertices[i](0);
		float tempy = vertices[i](1);
		float tempz = vertices[i](2);

		vertices[i](0) = tempx*sf[0][0] + tempy*sf[0][1] + tempz*sf[0][2] + sf[0][3];
		vertices[i](1) = tempx*sf[1][0] + tempy*sf[1][1] + tempz*sf[1][2] + sf[1][3];
		vertices[i](2) = tempx*sf[2][0] + tempy*sf[2][1] + tempz*sf[2][2] + sf[2][3];
		float h = tempx*sf[3][0] + tempy*sf[3][1] + tempz*sf[3][2] + sf[3][3];

		vertices[i](0) = vertices[i](0) / h;
		vertices[i](1) = vertices[i](1) / h;
		vertices[i](2) = vertices[i](2) / h;
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
	mouseZoom();
	// set the camera
	gluLookAt(eye[0], eye[1], eye[2],
            center[0], center[1], center[2],
			up[0], up[1], up[2]);
	
	//Sets Some itintial Properties to the light
	glEnable(GL_LIGHT1);
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
			//Draw Cube with Associated Texture Coordinates
		    //Front
			glBegin(GL_QUADS);
			glColor4f(1.0f,0.0f,0.0f,alpha);
			glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f, 1.0f, 1.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 1.0f);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,-1.0f, 1.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,-1.0f, 1.0f);
			
			//Top - Green
			
			glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, 1.0f, 1.0f);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1.0f, 1.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
			glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);
			

			//Right 
			
			glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, 1.0f, 1.0f);
			glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, -1.0f);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f);
			
			//Bottom
			
			glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, -1.0f, 1.0f);
			glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, -1.0f);
			
			//Back
			
			glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, -1.0f, -1.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, 1.0f, -1.0f);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
			
			//Left
			
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
			glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 1.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
			glEnd();
            break;
            
        case 'v': // to display points
            glBegin(GL_POINTS);
			
            glColor4f(0.0f,1.0f,0.0f,alpha);

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
            glColor4f(0.0f,0.0f,1.0f,alpha);
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
		case 'p': {//Faces with phong Illumination*/
			glPushMatrix();// Push Matrix so Light isnt rotated with the cube
			glLoadIdentity();
			glEnable(GL_LIGHTING);

			//Setting Properties of the Light and Cube Material
			GLfloat light_amb[] = { 0.2f,0.5f ,0.2f, 1.0f };
			GLfloat light_diff[] = { 0.0f,1.0f ,0.0f, 1.0f };
			GLfloat light_spec[] = { 1.0f,1.0f ,1.0f, 1.0f };

			GLfloat refl_amb[] = { 0.0f,0.5f,0.0f,1.0f };
			GLfloat refl_diff[] = { 0.0f,1.0f,0.0f,1.0f };
			GLfloat refl_spec[] = { 1.0f,1.0f,1.0f,1.0f };

			glLightfv(GL_LIGHT1, GL_AMBIENT, light_amb);
			glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diff);
			glLightfv(GL_LIGHT1, GL_SPECULAR, light_spec);

			glMaterialfv(GL_FRONT, GL_AMBIENT, refl_amb);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, refl_diff);
			glMaterialfv(GL_FRONT, GL_SPECULAR, refl_spec);

			glPopMatrix();
			//Draw cube with Vertex Normals
			//Front
			glBegin(GL_QUADS);
			glNormal3f(1.0f, 0.0f, 1.0f);	glVertex3f(1.0f, 1.0f, 1.0f);
			glNormal3f(-1.0f, 0.0f, 1.0f);	glVertex3f(-1.0f, 1.0f, 1.0f);
			glNormal3f(-1.0f, 0.0f, 1.0f);  glVertex3f(-1.0f, -1.0f, 1.0f);
			glNormal3f(1.0f, 0.0f, 1.0f);   glVertex3f(1.0f, -1.0f, 1.0f);


			//Top 
			glNormal3f(1.0f, 0.0f, 1.0f); glVertex3f(1.0f, 1.0f, 1.0f);
			glNormal3f(-1.0f, 0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 1.0f);
			glNormal3f(-1.0f, 0.0f, -1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
			glNormal3f(1.0f, 0.0f, -1.0f); glVertex3f(1.0f, 1.0f, -1.0f);

			//Right 

			glNormal3f(1.0f, 0.0f, 1.0f); glVertex3f(1.0f, 1.0f, 1.0f);
			glNormal3f(1.0f, 0.0f, -1.0f); glVertex3f(1.0f, 1.0f, -1.0f);
			glNormal3f(1.0f, 0.0f, -1.0f); glVertex3f(1.0f, -1.0f, -1.0f);
			glNormal3f(1.0f, 0.0f, 1.0f); glVertex3f(1.0f, -1.0f, 1.0f);

			//Bottom-

			glNormal3f(1.0f, 0.0f, 1.0f); glVertex3f(1.0f, -1.0f, 1.0f);
			glNormal3f(-1.0f, 0.0f, 1.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
			glNormal3f(-1.0f, 0.0f, -1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
			glNormal3f(1.0f, 0.0f, -1.0f); glVertex3f(1.0f, -1.0f, -1.0f);

			//Back

			glNormal3f(1.0f, 0.0f, -1.0f); glVertex3f(1.0f, -1.0f, -1.0f);
			glNormal3f(1.0f, 0.0f, -1.0f); glVertex3f(1.0f, 1.0f, -1.0f);
			glNormal3f(-1.0f, 0.0f, -1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
			glNormal3f(-1.0f, 0.0f, -1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);

			//Left-

			glNormal3f(-1.0f, 0.0f, -1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
			glNormal3f(-1.0f, 0.0f, -1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
			glNormal3f(-1.0f, 0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 1.0f);
			glNormal3f(-1.0f, 0.0f, 1.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
			glEnd();
			//Disable Lighting
			glDisable(GL_LIGHTING);
			glDisable(GL_LIGHT1);
			break;
		}
    }
	//Disable Texture to remove the texture from axis and Mesh
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	glPushMatrix();
	
	if (plane) {
		drawPartialMesh();	
		//Draw Intersecting Plane
		glBegin(GL_QUADS);
		alpha = 0.3f;
		glColor4f(1.0f, 0.0f, 0.5f, alpha);
		glVertex3f(2.0f, 2.0f, planeZ);
		glVertex3f(2.0f, -2.0f, planeZ);
		glVertex3f(-2.0f, -2.0f, planeZ);
		glVertex3f(-2.0f, 2.0f, planeZ);
		glEnd();
		
	}
	else {
		drawFullMesh();
	}
	
	switch (boundmode) {
		case'a':
			drawAbb();
			break;
		case'o':
			drawObb();
			break;
		default:
			break;
	}
	
	glPopMatrix();
    //Draw Dotted Cartesian coordinate system as lines
	glBegin(GL_LINES);
	alpha = 1.0f;
	glColor4f(1.0f, 1.0f, 1.0f,alpha);
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

void rotateMesh(float angle, char dir) {
	
	//Rotational Matrix Around X
	Eigen::Matrix3d Rx;
	Rx <<	1.0f, 0.0f, 0.0f,
			0.0f, cosf(angle), -sinf(angle),
			0.0f, sinf(angle), cosf(angle);

	//Rotational Matrix Around Y
	Eigen::Matrix3d Ry;
	Ry <<	cosf(angle), 0.0f, sinf(angle),
			0.0f, 1.0f, 0.0f,
			-sinf(angle), 0.0f, cosf(angle);
	

	//Rotational Matrix Around Z
	Eigen::Matrix3d Rz;
	Rz  <<	cosf(angle), -sinf(angle), 0.0f,
			sinf(angle), cosf(angle), 0.0f,
			0.0f, 0.0f, 1.0f;
	 ;
	
	//Temp variable to store Matrix product
	Eigen::MatrixXd temp;
		if (dir == 'x') {
			for (unsigned int i = 0; i < vertices.size(); i++) {
				temp = Rx * vertices[i];
				vertices[i] = temp;
			}
		}
		else if (dir == 'y') {

			for (unsigned int i = 0; i < vertices.size(); i++) {
			
				temp = Ry * vertices[i];
				vertices[i] = temp;
			}
		}

		else if (dir == 'z') {

			for (unsigned int i = 0; i < vertices.size(); i++) {
				temp = Rz* vertices[i];
				vertices[i] = temp;
			}
		}

		//recalculate the Obb
		generateObb();

}
void rotateCamera(float angle, char dir) {


	float h = 1.0f; //homogeneous Variable
	//Variables to Store the Current Eye/up Values
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

		//Multiply by the roational Matrix
		tempEx = eye[0] * Rx[0][0] + eye[1] * Rx[0][1] + eye[2] * Rx[0][2] + h*Rx[0][3];
		tempEy = eye[0] * Rx[1][0] + eye[1] * Rx[1][1] + eye[2] * Rx[1][2] + h*Rx[1][3];
		tempEz = eye[0] * Rx[2][0] + eye[1] * Rx[2][1] + eye[2] * Rx[2][2] + h*Rx[2][3];
		    h = eye[0] * Rx[3][0] + eye[1] * Rx[3][1] + eye[2] * Rx[3][2] + h*Rx[3][3];

		tempx = up[0] * Rx[0][0] + up[1] * Rx[0][1] + up[2] * Rx[0][2] + up[3] * Rx[0][3];
		tempy = up[0] * Rx[1][0] + up[1] * Rx[1][1] + up[2] * Rx[1][2] + up[3] * Rx[1][3];
		tempz = up[0] * Rx[2][0] + up[1] * Rx[2][1] + up[2] * Rx[2][2] + up[3] * Rx[2][3];
		tempw = up[0] * Rx[3][0] + up[1] * Rx[3][1] + up[2] * Rx[3][2] + up[3] * Rx[3][3];

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

		tempx = up[0] * Ry[0][0] + up[1] * Ry[0][1] + up[2] * Ry[0][2] + up[3] * Ry[0][3];
		tempy = up[0] * Ry[1][0] + up[1] * Ry[1][1] + up[2] * Ry[1][2] + up[3] * Ry[1][3];
		tempz = up[0] * Ry[2][0] + up[1] * Ry[2][1] + up[2] * Ry[2][2] + up[3] * Ry[2][3];
		tempw = up[0] * Ry[3][0] + up[1] * Ry[3][1] + up[2] * Ry[3][2] + up[3] * Ry[3][3];
	}

	else if (dir == 'z') {

		//Rotationall Matrix Around Z
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
	
	//Set the new Camera Settings 
	eye[0] = tempEx / h;
	eye[1] = tempEy / h;
	eye[2] = tempEz / h;

	up[0] = tempx / tempw;
	up[1] = tempy / tempw;
	up[2] = tempz / tempw;
	up[3] = tempw / tempw;
	
}
void resetCamera() {
	
	//Resets Camera Posistions
	eye[0] = 5.0f; 
	eye[1] = 5.0f; 
	eye[2] = 10.0f;
	center[0] = 0.0f;
	center[1] = 0.0f;
	center[2] = 0.0f;
	up[0] = 0.0f;
	up[1] = 1.0f;
	up[2] = 0.0f;
	up[3] = 1.0f;

	RotateX = 0.0f;
	RotateY = 0.0f;
	RotateZ = 0.0f;
		
}

/*
 * The reshape function sets up the
 viewport and projection
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
			rotateMesh(0.05f, 'y');
			break;
		case 'a':
			RotateY--;
			rotateMesh(-0.05f, 'y');
			break;
		case 'w':
			RotateX--;
			rotateMesh(-0.05f, 'x');
			break;
		case 's':
			RotateX++;
			rotateMesh(0.05f, 'x');
			break;
		case 'q':
			RotateZ++;
			rotateMesh(0.05f, 'z');
			break;
		case 'e':
			RotateZ--;
			rotateMesh(-0.05f, 'x');
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
		case'r':
			resetCamera();
			break;
		case'm':
			meshmode = 'f';
			break;
		case'n':
			meshmode = 'v';
			break;
		case'b':
			meshmode = 'l';
			break;
		case'k':
			rendermode = 'c'; //dont draw cube
			break;
		case'i':
			if (plane == 0) {
				plane = 1;
			}
			else plane = 0;
			break;
		case'z':
			boundmode = 'a';
			break;
		case'x':
			boundmode = 'o';
			break;
		case'c':
			boundmode = 'c'; //dont draw bound
			break;
		case'j':
			meshmode = 'c'; //dont draw mesh
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
		//Pan the Camera
		case GLUT_KEY_RIGHT:
			eye[0] = eye[0] + 0.1f;
			center[0] = center[0] + 0.1f;
			break;
		case GLUT_KEY_LEFT:
			eye[0] = eye[0] - 0.1f;
			center[0] = center[0] - 0.1f;
			break;
		//Move the Intersecting plane along Z
		case GLUT_KEY_UP:
			planeZ = planeZ + 0.1f;
			break;
		case GLUT_KEY_DOWN:
			planeZ = planeZ - 0.11f;
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

/*
 * Entry point to the application
 */
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitWindowSize(500, 500);
    glutCreateWindow("Graphics OpenGL Coursework");
    InitGL();
    rendermode='f';
	meshmode = ' ';
	loadOBJ("bunny.obj", vertices, faceIndices);
	scaleAndTranslate();
	generateObb();
	GLuint Texture = loadBMPToTexture("bricks.bmp");
	
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