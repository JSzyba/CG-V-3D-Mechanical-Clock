/*
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define GLM_FORCE_RADIANS
#define MODEL_PATH "Models/Clock.obj"
#define START_POS glm::vec3(0, 11, -25)
#define GEAR_POS glm::vec3(0, 15.5, -8)

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include "cube.h"
#include "constants.h"
#include "lodepng.h"
#include "shaderprogram.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

typedef unsigned int  GLuint;   /* 4-byte unsigned */

// struct containing all attributes of vertices
struct Vertex {
	std::vector<glm::vec4> position;
	std::vector<glm::vec4> normal;
	std::vector<glm::vec2> texCoords;
};

// struct containg all important information about texture
struct Texture {
	aiString type;
	GLuint tex;
};

// struct containing all important information about mesh
struct Mesh {
	Vertex vertex;
	std::vector<unsigned int> indices;

	Texture* tex;
};


glm::vec3 lightCubePos[4] = { glm::vec3(0.0f, 10.0f, -6.0f),
glm::vec3(0.0f, 10.0f, 16.0f),
glm::vec3(6.0f, 30.0f, 0.0f),
glm::vec3(-6.0f, 30.0f, 0.0f) }; //locations of light sources in world space

float aspectRatio;	// width to height ratio of window

float speed_x;		// angular speed in radians of model
float speed_y;		// angular speed in radians of model
float movement_x;	// angular speed in radians of camera
float movement_y;	// angular speed in radians of camera
float ws;			// speed of camera

double gearAngle;		// rotation angle of gears
double time;			// time elapsed from start of program to current frame
double oldTime;			// time elapsed from start of program to last frame
int fullCycle;			// full rotations of drum holding weight
double timeAmplifier;	// multiplier of elapsing time

int meshNumber;		// number of meshes
bool ifCaseVisible;	// flag describing if case should be visible

ShaderProgram* sp;	// pointer to the shader program
ShaderProgram* splc;

Texture planks; // textures for cubical models drawn alongside the clock
Texture lamp;

std::vector<Mesh> meshVec;					// vector of all meshes of the clock
std::vector<Texture> texVec;				// vector of all textures of the clock
std::vector<glm::vec3> rotationCenters;		// vector of rotation centers of objects
std::vector<double> gearRatio(29, 0.0);		// vector of gear ratios

// vectors used to control camera movement
glm::vec3 pos;
glm::vec3 dir;

GLuint readTexture(const char* filename) {
	GLuint tex;
	glActiveTexture(GL_TEXTURE0);

	//Load into computer's memory
	std::vector<unsigned char> image;   //Allocate a vector for image storage
	unsigned width, height;   //Variables for image size
	//Read image
	unsigned error = lodepng::decode(image, width, height, filename);

	//Import into graphics card's memory
	glGenTextures(1, &tex); //Initialize one handle
	glBindTexture(GL_TEXTURE_2D, tex); //Activate the handle
	//Import image into graphics card's memory associated with the handle
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)image.data());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return tex;
}

// procedures and functions used to read model from file using assimp library
void processMesh(aiMesh* mesh, const aiScene* scene, Mesh *givenMesh) {
	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		givenMesh->vertex.position.push_back(glm::vec4(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1));
		givenMesh->vertex.normal.push_back(glm::vec4(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z, 0));
		if (mesh->mTextureCoords[0]) {
			givenMesh->vertex.texCoords.push_back(glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y));
		}
		else {
			givenMesh->vertex.texCoords.push_back(glm::vec2(0.0f, 0.0f));
		}
		
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			givenMesh->indices.push_back(face.mIndices[j]);
		}
	}

	aiString textureName;	// name of the texture
	scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), textureName);	// getting first diffuse texture name
	// searching for texture in texVec
	for (int i = 0; i < texVec.size(); i++) {
		if (texVec[i].type == textureName) {
			givenMesh->tex = &texVec[i];
			break;
		}
	}
}
void processNode(aiNode* node, const aiScene* scene, std::vector<Mesh> &meshes) {
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		Mesh givenMesh;
		processMesh(mesh, scene, &givenMesh);
		if(givenMesh.indices.size() > 0)	meshes.push_back(givenMesh);
	}
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scene, meshes);
	}
}
bool loadModel(const std::string& path, std::vector<Mesh> &meshes) {
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
	
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		return false;
	}

	if (scene->HasMaterials())	// true when number of materials is greater than 0
	{
		for (int i = 0; i < scene->mNumMaterials; i++)
		{
			aiReturn ret;	// success flag

			aiMaterial* material = scene->mMaterials[i];	// get the current material
			
			aiString materialName;	// name of the material
			ret = material->Get(AI_MATKEY_NAME, materialName);	// get the material name
			if (ret != AI_SUCCESS) materialName = "";			// check if material was found

			int numTextures = material->GetTextureCount(aiTextureType_DIFFUSE); // amount of diffuse textures
			
			aiString textureName;	// filename of the texture
			if (numTextures > 0) {	// check if there are any diffuse textures
				material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), textureName);	// getting first diffuse texture

				// Creating new Texture 
				Texture newTex;
				newTex.type = textureName;
				newTex.tex = readTexture(textureName.C_Str());
				texVec.push_back(newTex);
			}
		}
	}

	processNode(scene->mRootNode, scene, meshes);
	return true;
}

// functions used to animate model
float calculateGearAngle(double oldTime, double time) {
	if (time < oldTime) return 0;

	float angle = floor(time - oldTime) * 6;

	oldTime = oldTime - floor(oldTime);
	time = time - floor(time);
	//check the value of oldTime fraction
	int oldCase;
	if (oldTime < 0.16) oldCase = 0;
	else if (oldTime < 0.3) oldCase = 1;
	else if (oldTime < 0.73) oldCase = 2;
	else oldCase = 3;
	//check the value of time fraction
	int newCase;
	if (time < 0.16) newCase = 0;
	else if (time < 0.3) newCase = 4;
	else if (time < 0.73) newCase = 8;
	else newCase = 12;
	//depending on the intervals the fraction parts of oldTime and time are,
	//set angle to the appropriate multiplication of time and oldTime
	switch (oldCase + newCase) {
		case 0: 
			angle += (time - oldTime) * 2.5;
			break;
		case 5:
			angle += (time - oldTime) * 7.857143;
			break;
		case 10:
			angle += (time - oldTime) * 0;
			break;
		case 15:
			angle += (time - oldTime) * 16.666666;
			break;

		case 4:
			angle += (0.16 - oldTime) * 2.5 +		(time - 0.16) * 7.857143;
			break;
		case 9:
			angle += (0.3 - oldTime) * 7.857143 +	(time - 0.3) * 0;
			break;
		case 14:
			angle += (0.73 - oldTime) * 0 +			(time - 0.73) * 16.666666;
			break;
		case 3:
			angle += (1.0 - oldTime) * 16.666666 +	(time - 0.0) * 2.5;
			break;

		case 8:
			angle += (0.16 - oldTime) * 2.5 +		(0.3 - 0.16) * 7.857143 +	(time - 0.3) * 0;
			break;
		case 13:
			angle += (0.3 - oldTime) * 7.857143 +	(0.73 - 0.3) * 0 +			(time - 0.73) * 16.66666;
			break;
		case 2:
			angle += (0.73 - oldTime) * 0 +			(1.0 - 0.73) * 16.66666 +	(time - 0.0) * 2.5;
			break;
		case 7:
			angle += (1.0 - oldTime) * 16.66666 +	(0.16 - 0.0) * 2.5 +		(time - 0.16) * 7.857143;
			break;

		case 12:
			angle += (0.16 - oldTime) * 2.5 +		(0.3 - 0.16) * 7.857143 +	(0.73 - 0.3) * 0 +				(time - 0.73) * 16.666666;
			break;
		case 1:
			angle += (0.3 - oldTime) * 7.857143 +	(0.73 - 0.3) * 0 +			(1.0 - 0.73) * 16.666666 +		(time - 0.0) * 2.5;
			break;
		case 6:
			angle += (0.73 - oldTime) * 0 +			(1.0 - 0.73) * 16.666666 +	(0.16 - 0.0) * 2.5 +			(time - 0.16) * 7.857143;
			break;
		case 11:
			angle += (1.0 - oldTime) * 16.666666 +	(0.16 - 0.0) * 2.5 +		(0.3 - 0.16) * 7.857143 +		(time - 0.3) * 0;
			break;
		default: angle = 0;
	}

	return angle;
}
//check for the lowest and highest positions in x-axis, y-axis and z-axis and return a point in the middle
glm::vec3 calculateCenter(int meshIndex) {
	
	if (rotationCenters[meshIndex] == glm::vec3(0, 0, 0)) {
		float minX, maxX, minY, maxY, minZ, maxZ;
		minX = meshVec[meshIndex].vertex.position[0].x;
		maxX = meshVec[meshIndex].vertex.position[0].x;
		minY = meshVec[meshIndex].vertex.position[0].y;
		maxY = meshVec[meshIndex].vertex.position[0].y;
		minZ = meshVec[meshIndex].vertex.position[0].z;
		maxZ = meshVec[meshIndex].vertex.position[0].z;
		for (int i = 1; i < meshVec[meshIndex].vertex.position.size(); i++) {
			if (meshVec[meshIndex].vertex.position[i].x > maxX) maxX = meshVec[meshIndex].vertex.position[i].x;
			if (meshVec[meshIndex].vertex.position[i].x < minX) minX = meshVec[meshIndex].vertex.position[i].x;
			if (meshVec[meshIndex].vertex.position[i].y > maxY) maxY = meshVec[meshIndex].vertex.position[i].y;
			if (meshVec[meshIndex].vertex.position[i].y < minY) minY = meshVec[meshIndex].vertex.position[i].y;
			if (meshVec[meshIndex].vertex.position[i].z > maxZ) maxZ = meshVec[meshIndex].vertex.position[i].z;
			if (meshVec[meshIndex].vertex.position[i].z < minZ) minZ = meshVec[meshIndex].vertex.position[i].z;
		}
		rotationCenters[meshIndex].x = (maxX + minX) / 2;
		rotationCenters[meshIndex].y = (maxY + minY) / 2;
		rotationCenters[meshIndex].z = (maxZ + minZ) / 2;
	}
	return rotationCenters[meshIndex];
}
//translate center of rotation to the origin, rotate and then translate back
glm::mat4 rotateObject(glm::mat4 M, float val, int meshIndex)
{
	M = glm::translate(M, rotationCenters[meshIndex]);
	M = glm::rotate(M, val, glm::vec3(0.0f, 0.0f, 1.0f));
	M = glm::translate(M, rotationCenters[meshIndex]*(-1.0f));
	return M;
}
//translated center of rotation to the origin, scale and then translate back
glm::mat4 scaleObject(glm::mat4 M, glm::vec3 val, int meshIndex)
{
	M = glm::translate(M, rotationCenters[meshIndex]);
	M = glm::scale(M, val);
	M = glm::translate(M, rotationCenters[meshIndex] * -1.0f);
	return M;
}

// error processing callback procedure
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}
//camera controls, debug and world manipulation options
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_LEFT) speed_x = -PI / 2;
		if (key == GLFW_KEY_RIGHT) speed_x = PI / 2;
		if (key == GLFW_KEY_UP) speed_y = PI / 2;
		if (key == GLFW_KEY_DOWN) speed_y = -PI / 2;
		if (key == GLFW_KEY_S) movement_x = PI / 2;
		if (key == GLFW_KEY_W) movement_x = -PI / 2;
		if (key == GLFW_KEY_A) movement_y = PI / 2;
		if (key == GLFW_KEY_D) movement_y = -PI / 2;
		if (key == GLFW_KEY_Z) ws = 10;
		if (key == GLFW_KEY_C) ws = -10;
		if (key == GLFW_KEY_B) printf("x: %f\ny: %f\nz: %f\ntime: %f\n", pos.x, pos.y, pos.z, time);
		if (key == GLFW_KEY_O) ifCaseVisible = !ifCaseVisible;
		if (key == GLFW_KEY_P) pos = GEAR_POS;
		if (key == GLFW_KEY_L) pos = START_POS;
		if (key == GLFW_KEY_I) timeAmplifier *= 2.0;
		if (key == GLFW_KEY_K) timeAmplifier /= 2.0;
	}
	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_LEFT) speed_x = 0;
		if (key == GLFW_KEY_RIGHT) speed_x = 0;
		if (key == GLFW_KEY_UP) speed_y = 0;
		if (key == GLFW_KEY_DOWN) speed_y = 0;
		if (key == GLFW_KEY_W) movement_x = 0;
		if (key == GLFW_KEY_S) movement_x = 0;
		if (key == GLFW_KEY_A) movement_y = 0;
		if (key == GLFW_KEY_D) movement_y = 0;
		if (key == GLFW_KEY_Z) ws = 0;
		if (key == GLFW_KEY_C) ws = 0;
	}
}
void windowResizeCallback(GLFWwindow* window,int width,int height) {
    if (height==0) return;
    aspectRatio=(float)width/(float)height;
    glViewport(0,0,width,height);
}
//Initialization code procedure
void initOpenGLProgram(GLFWwindow* window) {
	aspectRatio = 1;

	glClearColor(0.67f, 0.84f, 0.9f, 1);
	glEnable(GL_DEPTH_TEST);
	glfwSetWindowSizeCallback(window, windowResizeCallback);
	glfwSetKeyCallback(window, keyCallback);
	sp = new ShaderProgram("v_simplest.glsl", NULL, "f_simplest.glsl");
	splc = new ShaderProgram("v_light_cube.glsl", NULL, "f_light_cube.glsl");

	// Loading model from file using assimp library
	if (!loadModel(MODEL_PATH, meshVec)) {
		fprintf(stderr, "Failed to load model!\n");
		exit(EXIT_FAILURE);
	}
	meshNumber = meshVec.size();

	// Calculating centers of rotation for all meshes
	rotationCenters = std::vector<glm::vec3>(meshNumber, glm::vec3(0, 0, 0));
	for (int i = 0; i < meshNumber; i++)
		calculateCenter(i);
	
	// Describing centers of rotation of specific meshes which rotate in a specific way
	rotationCenters[11] = glm::vec3(0.0f, 15.356f, 1.10f); // hour hand
	rotationCenters[18] = glm::vec3(0.0f, 15.356f, 1.10f); // minutes hand
	rotationCenters[22] = glm::vec3(0.0f, 15.356f, 1.10f); // seconds hand
	rotationCenters[20] = glm::vec3(0.00779f, 17.7845f, -0.755f); // pendulum

	// Describing gearRatio of different gears for apprioprate scaling rotation angle
	gearRatio[21] = -1.0;
	gearRatio[22] = -1.0;
	gearRatio[23] = -1.0;
	gearRatio[3] = 1.0 / 10;
	gearRatio[4] = 1.0 / 10;
	gearRatio[16] = -1.0 / 60;
	gearRatio[17] = -1.0 / 60;
	gearRatio[0] = (1.0 / 60) * (6.0 / 35.0);
	gearRatio[18] = -1.0 / 60;
	gearRatio[19] = -1.0 / 60;
	gearRatio[5] = 1.0 / 360;
	gearRatio[7] = 1.0 / 360;
	gearRatio[6] = -1.0 / 720;
	gearRatio[11] = -1.0 / 720;
	gearRatio[12] = -1.0 / 720;
	gearRatio[13] = -1.0 / 720;
	gearRatio[1] = 1.0 / 2880;
	gearRatio[2] = 1.0 / 2880;
	gearRatio[8] = -1.0 / 11520;
	gearRatio[9] = 1.0 / 28800;
	gearRatio[10] = 1.0 / 28800;

	printf("Total number of meshes: %d\n", meshNumber);

	planks.tex = readTexture("planks.png");
	lamp.tex = readTexture("lamp.png");
}
//Release resources allocated by the program
void freeOpenGLProgram(GLFWwindow* window) {
	for (int i = 0; i < texVec.size(); i++) {
		glDeleteTextures(1, &texVec[i].tex);
	}
	glDeleteTextures(1, &planks.tex);
	glDeleteTextures(1, &lamp.tex);
	delete sp;
	delete splc;
}



//Drawing procedure
void drawScene(GLFWwindow* window, float angle_x, float angle_y) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 V = glm::lookAt(
		pos,
		pos + dir,
		glm::vec3(0.0f, 1.0f, 0.0f)); //compute view matrix
	glm::mat4 P = glm::perspective(50.0f * PI / 180.0f, aspectRatio, 1.0f, 500.0f); //compute projection matrix

	sp->use();	//activate shading program

	// send parameters to graphics card
	glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));
	glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
	

	glUniform1i(sp->u("textureMap0"), 0);	// associate sampler textureMap0 with the 0-th texturing unit
	glActiveTexture(GL_TEXTURE0);			// activating the 0-th texturing unit

	//draw floor
	glm::mat4 N = glm::mat4(1.0f);
	N = glm::scale(N, glm::vec3(20.0f, 1.0f, 20.0f));
	N = glm::translate(N, glm::vec3(0.0f, -0.68f, 0.0f));
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(N));
	glBindTexture(GL_TEXTURE_2D, planks.tex);

	glEnableVertexAttribArray(sp->a("vertex"));
	glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, cubeVertices);

	glEnableVertexAttribArray(sp->a("normal"));
	glVertexAttribPointer(sp->a("normal"), 3, GL_FLOAT, false, 0, cubeNormals);

	glEnableVertexAttribArray(sp->a("texCoord0"));
	glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, cubeTexCoords);

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, cubeIndices);

	for (int x = 0; x < meshNumber; x++)
	{

		glm::mat4 M = glm::mat4(1.0f);

		//M = glm::rotate(M, angle_y, glm::vec3(1.0f, 0.0f, 0.0f)); //Compute model matrix
		M = glm::rotate(M, PI + angle_x, glm::vec3(0.0f, 1.0f, 0.0f)); //Compute model matrix
		
		// Check for meshes that correspond to case
		if (x == 14 || x == 15 || x == 24 || x == 25 || x == 26) {
			if (!ifCaseVisible) continue;
		}
		else if (x == 27)	M = glm::translate(M, glm::vec3(0, -gearRatio[10] * gearAngle / 360 * 2 * PI * 0.248100 - (2 * PI * 0.248100) * fullCycle, 0));	// weight
		else if (x == 28)	// wire
		{
			double temp = gearRatio[10] * gearAngle / 360 * 2 * PI * 0.248100;
			if (temp > 2 * PI * 0.248100) {	// check if full rotation was made
				fullCycle++;
				gearAngle -= 360.0 / gearRatio[10];
				time -= 360.0 / gearRatio[10] / 6.0;
				oldTime = time;
			}
			else {
				temp += (2 * PI * 0.248100) * fullCycle;
			}
			M = glm::translate(M, glm::vec3(0, -temp / 2, 0));
			M = scaleObject(M, glm::vec3(1, (temp + 0.383693)/(0.383693), 1), x);
		}
		else if (x == 20)	M = rotateObject(M, glm::radians((sin(time * 2 * PI - PI / 2) + 1) * 7), x);	// pendulum
		else if (x == 11) { M = rotateObject(M, glm::radians(gearRatio[x] * gearAngle), x); M = scaleObject(M, glm::vec3(2, 1, 2), x); }
		else if (x == 18) { M = rotateObject(M, glm::radians(gearRatio[x] * gearAngle), x); M = scaleObject(M, glm::vec3(2, 1, 2), x); }
		else if (x == 22) { M = rotateObject(M, glm::radians(gearRatio[x] * gearAngle), x); M = scaleObject(M, glm::vec3(2, 1, 2), x); }
		else				M = rotateObject(M, glm::radians(gearRatio[x] * gearAngle), x);					// other rotatable objects (gears, hands, wheels)
		
		glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M));	// sending matrix M to graphics card
		glBindTexture(GL_TEXTURE_2D, meshVec[x].tex->tex);				// binding apprioprate texture handle with active texturing unit
		
		glEnableVertexAttribArray(sp->a("vertex"));		// enable sending data to the attribute vertex
		glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, meshVec[x].vertex.position.data());		// specify source of the data for the attribute vertex
		glEnableVertexAttribArray(sp->a("normal"));		// enable sending data to the attribute color
		glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, meshVec[x].vertex.normal.data());			// specify source of the data for the attribute normal
		glEnableVertexAttribArray(sp->a("texCoord0"));	// enable sending data to the attribute texCoord0
		glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, meshVec[x].vertex.texCoords.data());	// specify source of the data for the attribute texCoord0

		glDrawElements(GL_TRIANGLES, meshVec[x].indices.size(), GL_UNSIGNED_INT, meshVec[x].indices.data());	// draw mesh
	}

	glDisableVertexAttribArray(sp->a("vertex")); //Disable sending data to the attribute vertex
	glDisableVertexAttribArray(sp->a("texCoord0")); //Disable sending data to the attribute texCoord0
	glDisableVertexAttribArray(sp->a("normal")); //Disable sending data to the attribute normal

	//draw lamps using additional shader
	splc->use();
	glUniformMatrix4fv(splc->u("view"), 1, false, glm::value_ptr(V));
	glUniformMatrix4fv(splc->u("projection"), 1, false, glm::value_ptr(P));
	glUniform1i(splc->u("textureMap0"), 0);
	glBindTexture(GL_TEXTURE_2D, lamp.tex);


	glm::mat4 Mc = glm::mat4(1.0f);
	for (int i = 0; i < 4; i++)
	{
		Mc = glm::translate(Mc, lightCubePos[i]);
		glUniformMatrix4fv(splc->u("model"), 1, false, glm::value_ptr(Mc));
		glEnableVertexAttribArray(splc->a("aPos"));
		glVertexAttribPointer(splc->a("aPos"), 4, GL_FLOAT, false, 0, cubeVertices);
		glEnableVertexAttribArray(splc->a("texCoord0"));
		glVertexAttribPointer(splc->a("texCoord0"), 2, GL_FLOAT, false, 0, cubeTexCoords);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, cubeIndices);
		Mc = glm::translate(Mc, lightCubePos[i] * (-1.0f));
	}
	
	glDisableVertexAttribArray(splc->u("view"));
	glDisableVertexAttribArray(splc->u("projection"));
	glDisableVertexAttribArray(splc->u("model"));
	glDisableVertexAttribArray(splc->a("aPos"));

	glfwSwapBuffers(window); //Copy back buffer to front buffer
}

int main(void)
{
	GLFWwindow* window; //Pointer to object that represents the application window

	glfwSetErrorCallback(error_callback);//Register error processing callback procedure

	if (!glfwInit()) { //Initialize GLFW library
		fprintf(stderr, "Can't initialize GLFW.\n");
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(500, 500, "Clock", NULL, NULL);  //Create a window 500pxx500px titled "OpenGL" and an OpenGL context associated with it.

	if (!window) //If no window is opened then close the program
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); //Since this moment OpenGL context corresponding to the window is active and all OpenGL calls will refer to this context.
	glfwSwapInterval(1); //During vsync wait for the first refresh

	GLenum err;
	if ((err=glewInit()) != GLEW_OK) { //Initialize GLEW library
		fprintf(stderr, "Can't initialize GLEW: %s\n", glewGetErrorString(err));
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window); //Call initialization procedure

	speed_x = 0;
	speed_y = 0;
	movement_x = 0;
	movement_y = 0;
	ws = 0;
	ifCaseVisible = true;
	time = 0;
	oldTime = 0;
	gearAngle = 0;
	pos = START_POS;
	timeAmplifier = 1;
	fullCycle = 0;

	float angle_x=0;
	float angle_y=0;
	float player_speed_x = 0;
	float player_speed_y = 0;

	glfwSetTime(0); //Zero the timer
	//Main application loop
	while (!glfwWindowShouldClose(window)) //As long as the window shouldnt be closed yet...
	{
		double deltaTime = glfwGetTime();
		glfwSetTime(0); //Zero the timer

		angle_x += speed_x * deltaTime;
		angle_y += speed_y * deltaTime;

		player_speed_x += movement_x * deltaTime;
		player_speed_y += movement_y * deltaTime;

		if (player_speed_x > 1.4f)
			player_speed_x = 1.4f;
		if (player_speed_x <= -1.4f)
			player_speed_x = -1.4f;

		glm::mat4 Mc = glm::rotate(glm::mat4(1.0f), player_speed_y, glm::vec3(0, 1, 0));
		Mc = glm::rotate(Mc, player_speed_x, glm::vec3(1, 0, 0));
		glm::vec4 dir_ = Mc * glm::vec4(0, 0, 1, 0);
		dir = glm::vec3(dir_);

		glm::vec3 mdir = glm::normalize(glm::vec3(dir.x, dir.y, dir.z));
		pos += ws * (float)deltaTime * mdir;

		time += timeAmplifier * deltaTime;
		gearAngle += calculateGearAngle(oldTime, time);	// calculating how much does gear rotate depending on pendulum state
		oldTime = time;

		drawScene(window,angle_x,angle_y); //Execute drawing procedure
		glfwPollEvents(); //Process callback procedures corresponding to the events that took place up to now
	}
	freeOpenGLProgram(window);

	glfwDestroyWindow(window); //Delete OpenGL context and the window.
	glfwTerminate(); //Free GLFW resources
	exit(EXIT_SUCCESS);
}

