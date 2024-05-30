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
#define MODEL_PATH "Models/OteksturowanyZegar2.obj"
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
	//aiString TexType;
	//GLuint* tex;
};


float aspectRatio;	// width to height ratio of window

float speed_x;		// angular speed in radians of model
float speed_y;		// angular speed in radians of model
float movement_x;	// angular speed in radians of camera
float movement_y;	// angular speed in radians of camera
float ws;			// speed of camera

float gearAngle;	// rotation angle of gears
double time;		// time elapsed from start of program to current frame
double oldTime;		// time elapsed from start of program to last frame

int meshNumber;		// number of meshes
bool ifCaseVisible;	// flag describing if case should be visible

ShaderProgram* sp;	// pointer to the shader program

std::vector<Mesh> meshVec;					// vector of all meshes
std::vector<Texture> texVec;				// vector of all textures
std::vector<glm::vec3> rotationCenters;		// vector of rotation centers of objects

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

float calculateGearAngle(double oldTime, double time) {
	time = time * 2.0 * PI;
	oldTime = oldTime * 2.0 * PI;
	float oldTimeSin = sin(oldTime);
	float timeSin = sin(time);
	float angle = 0;
	if (oldTimeSin > 0.7 && timeSin > 0.7) {
		float temp1 = -1 * sin(2.346 - time);
		float temp2 = -1 * sin(2.346 - oldTime);
		angle = temp1 - temp2;
	}
	else if (oldTimeSin > 0.7 && timeSin <= 0.7) {
		float temp1 = -1 * sin(2.346 - 2.366);
		float temp2 = -1 * sin(2.346 - oldTime);
		angle = temp1 - temp2;
	}
	else if (oldTimeSin <= 0.7 && timeSin > 0.7) {
		float temp1 = -1 * sin(2.346 - time);
		float temp2 = -1 * sin(2.346 - 0.7754);
		angle = temp1 - temp2;
	}
	else if (oldTimeSin < -0.9 && timeSin < -0.9) {
		float temp1 = 0.5 * sin(2 * time + 2.47437);
		float temp2 = 0.5 * sin(2 * oldTime + 2.47437);
		angle = temp1 - temp2;
	}
	else if (oldTimeSin < -0.9 && timeSin >= -0.9) {
		float temp1 = 0.5 * sin(2 * 5.162 + 2.47437);
		float temp2 = 0.5 * sin(2 * oldTime + 2.47437);
		angle = temp1 - temp2;
	}
	else if (oldTimeSin >= -0.9 && timeSin < -0.9) {
		float temp1 = 0.5 * sin(2 * time + 2.47437);
		float temp2 = 0.5 * sin(2 * 4.261 + 2.47437);
		angle = temp1 - temp2;
	}
	else {
		angle = 0;
	}
	return angle / 1.63653278517;
}
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
glm::mat4 rotateObject(glm::mat4 M, float val, int meshIndex)
{
	M = glm::translate(M, rotationCenters[meshIndex]);
	M = glm::rotate(M, val, glm::vec3(0.0f, 0.0f, 1.0f));
	M = glm::translate(M, rotationCenters[meshIndex]*(-1.0f));
	return M;
}

// error processing callback procedure
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}
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
		if (key == GLFW_KEY_Z) ws = 2;
		if (key == GLFW_KEY_C) ws = -0.1f;
		if (key == GLFW_KEY_B) printf("x: %f\ny: %f\nz: %f\n", pos.x, pos.y, pos.z);
		if (key == GLFW_KEY_O) ifCaseVisible = !ifCaseVisible;
		if (key == GLFW_KEY_P) pos = GEAR_POS;
		if (key == GLFW_KEY_L) pos = START_POS;
		if (key == GLFW_KEY_U) speed_y -= PI / 2 /100;
		if (key == GLFW_KEY_J) speed_y += PI / 2 / 100;
		if (key == GLFW_KEY_Y) ws = 8;
		if (key == GLFW_KEY_T) { time += 0.1; printf("time: %f\n", time); }
		if (key == GLFW_KEY_G) { time -= 0.1; printf("time: %f\n", time); }
		if (key == GLFW_KEY_R) { time += 0.01; printf("time: %f\n", time); }
		if (key == GLFW_KEY_F) { time -= 0.01; printf("time: %f\n", time); }
		if (key == GLFW_KEY_N) { gearAngle += 0.1; printf("gearAngle: %f\n", gearAngle); }
		if (key == GLFW_KEY_M) { gearAngle -= 0.1; printf("gearAngle: %f\n", gearAngle); }
		if (key == GLFW_KEY_I) { gearAngle += 0.01; printf("gearAngle: %f\n", gearAngle); }
		if (key == GLFW_KEY_K) { gearAngle -= 0.01; printf("gearAngle: %f\n", gearAngle); }
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

	glClearColor(1, 0.4f, 0.8f, 1);
	glEnable(GL_DEPTH_TEST);
	glfwSetWindowSizeCallback(window, windowResizeCallback);
	glfwSetKeyCallback(window, keyCallback);
	sp = new ShaderProgram("v_simplest.glsl", NULL, "f_simplest.glsl");

	// Loading model from file using assimp library
	if (!loadModel(MODEL_PATH, meshVec)) {
		fprintf(stderr, "Failed to load model!\n");
		exit(EXIT_FAILURE);
	}
	meshNumber = meshVec.size();

	// Calculating centers of rotation for all meshes
	rotationCenters = std::vector<glm::vec3>(meshNumber, glm::vec3(0, 0, 0));
	//fill(rotationCenters.begin(), rotationCenters.end(), glm::vec3(0, 0, 0));
	for (int i = 0; i < meshNumber; i++)	calculateCenter(i);
	
	// Describing centers of rotation of specific meshes which rotate in a specific way
	rotationCenters[11] = glm::vec3(0.0f, 15.356f, 1.10f); // hour hand
	rotationCenters[18] = glm::vec3(0.0f, 15.356f, 1.10f); // minutes hand
	rotationCenters[22] = glm::vec3(0.0f, 15.356f, 1.10f); // seconds hand
	rotationCenters[20] = glm::vec3(0.00779f, 17.7845f, -0.755f); // pendulum

	printf("Total number of meshes: %d\n", meshNumber);

	//for (int i = 0; i < meshNumber; i++) {
	//	printf("%d:\n", i);
	//	printf("\tpos:%d\n", meshVec[i].vertex.position.size());
	//	printf("\tnor:%d\n", meshVec[i].vertex.normal.size());
	//	printf("\ttex:%d\n", meshVec[i].vertex.texCoords.size());
	//	printf("\tind:%d\n", meshVec[i].indices.size());
	//	printf("\ttex:%d - %s\n\n", meshVec[i].tex, meshVec[i].TexType);
	//}
}
//Release resources allocated by the program
void freeOpenGLProgram(GLFWwindow* window) {
	for (int i = 0; i < texVec.size(); i++) {
		glDeleteTextures(1, &texVec[i].tex);
	}
	delete sp;
}
//Drawing procedure
void drawScene(GLFWwindow* window, float angle_x, float angle_y) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 V = glm::lookAt(
		pos,
		pos + dir,
		glm::vec3(0.0f, 1.0f, 0.0f)); //compute view matrix
	glm::mat4 P = glm::perspective(50.0f * PI / 180.0f, aspectRatio, 0.1f, 500.0f); //compute projection matrix

	sp->use();	//activate shading program

	// send parameters to graphics card
	glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));
	glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));

	glUniform1i(sp->u("textureMap0"), 0);	// associate sampler textureMap0 with the 0-th texturing unit
	glActiveTexture(GL_TEXTURE0);			// activating the 0-th texturing unit

	for (int x = 0; x < meshNumber; x++)
	{
		// Check for meshes that correspond to case
		if (!ifCaseVisible && (x == 14 || x == 15 || x == 24 || x == 25 || x == 26)) continue;

		glm::mat4 M = glm::mat4(1.0f);

		//M = glm::rotate(M, angle_y, glm::vec3(1.0f, 0.0f, 0.0f)); //Compute model matrix
		M = glm::rotate(M, PI + angle_x, glm::vec3(0.0f, 1.0f, 0.0f)); //Compute model matrix
		
		if (x == 11)	M = rotateObject(M, angle_y, x);
		if (x == 18)	M = rotateObject(M, angle_y * 12, x);
		if (x == 22)	M = rotateObject(M, angle_y * 12 * 60, x);
		if (x == 20)	M = rotateObject(M, glm::radians((sin(time*2*PI-PI/2)+1)*7), x);
		if (x == 21)	M = rotateObject(M, glm::radians(gearAngle), x);
		
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

	window = glfwCreateWindow(500, 500, "OpenGL", NULL, NULL);  //Create a window 500pxx500px titled "OpenGL" and an OpenGL context associated with it.

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

	speed_x = 0; //angular speed in radians
	speed_y = 0; //angular speed in radians
	movement_x = 0;
	movement_y = 0;
	ws = 0;
	ifCaseVisible = true;
	time = 0;
	oldTime = 0;
	gearAngle = 0;
	pos = START_POS;

	float angle_x=0; //current rotation angle of the object, x axis
	float angle_y=0; //current rotation angle of the object, y axis
	float player_speed_x = 0;
	float player_speed_y = 0;

	glfwSetTime(0); //Zero the timer
	//Main application loop
	while (!glfwWindowShouldClose(window)) //As long as the window shouldnt be closed yet...
	{
		double deltaTime = glfwGetTime();
		glfwSetTime(0); //Zero the timer

		angle_x += speed_x * deltaTime; //Add angle by which the object was rotated in the previous iteration
		angle_y += speed_y * deltaTime; //Add angle by which the object was rotated in the previous iteration

		player_speed_x += movement_x * deltaTime;
		player_speed_y += movement_y * deltaTime;

		glm::mat4 Mc = glm::rotate(glm::mat4(1.0f), player_speed_y, glm::vec3(0, 1, 0));
		Mc = glm::rotate(Mc, player_speed_x, glm::vec3(1, 0, 0));
		glm::vec4 dir_ = Mc * glm::vec4(0, 0, 1, 0);
		dir = glm::vec3(dir_);

		glm::vec3 mdir = glm::normalize(glm::vec3(dir.x, dir.y, dir.z));
		pos += ws * (float)deltaTime * mdir;

//		time += deltaTime;
//		gearAngle += calculateGearAngle(oldTime, time);
		oldTime = time;

		drawScene(window,angle_x,angle_y); //Execute drawing procedure
		glfwPollEvents(); //Process callback procedures corresponding to the events that took place up to now
	}
	freeOpenGLProgram(window);

	glfwDestroyWindow(window); //Delete OpenGL context and the window.
	glfwTerminate(); //Free GLFW resources
	exit(EXIT_SUCCESS);
}

