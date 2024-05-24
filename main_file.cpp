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

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include "constants.h"
#include "lodepng.h"
#include "shaderprogram.h"
#include "myCube.h"
#include "myTeapot.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

float speed_x=0; //angular speed in radians
float speed_y=0; //angular speed in radians
float movement_x = 0; //angular speed in radians
float movement_y = 0; //angular speed in radians
float ws = 0;
float aspectRatio=1;
glm::vec3 pos = glm::vec3(0, 16, -18);
ShaderProgram *sp; //Pointer to the shader program


//Uncomment to draw a cube
/*float* vertices = myCubeVertices;
float* texCoords= myCubeTexCoords;
float* colors = myCubeColors;
float* normals = myCubeNormals;
int vertexCount = myCubeVertexCount;
*/
//Uncomment to draw a teapot
float* verticess = myTeapotVertices;
float* texCoordes = myTeapotTexCoords;
float* colors = myTeapotColors;
float* normals = myTeapotVertexNormals;
int vertexCount = myTeapotVertexCount;

GLuint tex0;
GLuint tex1;

struct Vertex {
	std::vector<glm::vec4> position;
	std::vector<glm::vec4> normal;
	std::vector<glm::vec2> texCoords;
};

struct Texture {
	unsigned int id;
	std::string type;
};

struct Mesh {
	Vertex vertex;
	std::vector<unsigned int> indices;

	std::vector<glm::vec4>verts;
	std::vector<glm::vec4>norms;
	std::vector<glm::vec2>teks;
	Texture tex;
};

std::vector<Mesh> meshVec;

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
}

void processNode(aiNode* node, const aiScene* scene, std::vector<Mesh> &meshes) {
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		Mesh givenMesh;
		processMesh(mesh, scene, &givenMesh);
		meshes.push_back(givenMesh);
	}
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scene, meshes);
	}
}

bool loadModel(const std::string& path, std::vector<Mesh> &meshes) {
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		return false;
	}
	processNode(scene->mRootNode, scene, meshes);
	return true;
}


//Error processing callback procedure

void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

void keyCallback(GLFWwindow* window,int key,int scancode,int action,int mods) {
    if (action==GLFW_PRESS) {
        if (key==GLFW_KEY_LEFT) speed_x=-PI/2;
        if (key==GLFW_KEY_RIGHT) speed_x=PI/2;
        if (key==GLFW_KEY_UP) speed_y=PI/2;
        if (key==GLFW_KEY_DOWN) speed_y=-PI/2;
        if (key == GLFW_KEY_S) movement_x = PI / 2;
        if (key == GLFW_KEY_W) movement_x = -PI / 2;
        if (key == GLFW_KEY_A) movement_y = PI / 2;
        if (key == GLFW_KEY_D) movement_y = -PI / 2;
        if (key == GLFW_KEY_Z) ws = 10;
        if (key == GLFW_KEY_C) ws = -10;
		if (key == GLFW_KEY_B) printf("x: %f\ny: %f\nz: %f\n", pos.x, pos.y, pos.z);
    }
    if (action==GLFW_RELEASE) {
        if (key==GLFW_KEY_LEFT) speed_x=0;
        if (key==GLFW_KEY_RIGHT) speed_x=0;
        if (key==GLFW_KEY_UP) speed_y=0;
        if (key==GLFW_KEY_DOWN) speed_y=0;
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

//Initialization code procedure
void initOpenGLProgram(GLFWwindow* window) {
	//************Place any code here that needs to be executed once, at the program start************
	glClearColor(1,0.4f,0.8f,1);
	glEnable(GL_DEPTH_TEST);
	glfwSetWindowSizeCallback(window,windowResizeCallback);
	glfwSetKeyCallback(window,keyCallback);
	sp=new ShaderProgram("v_simplest.glsl",NULL,"f_simplest.glsl");
	tex0 = readTexture("metal.png");
	tex1 = readTexture("sky.png");
    std::string modelPath = "GrandfatherClock.obj";
    if (!loadModel(modelPath, meshVec)) {
        fprintf(stderr, "Failed to load model!\n");
        exit(EXIT_FAILURE);
    }
    printf("Vertices: %d\n", meshVec[0].verts.size());
    printf("Indices: %d\n", meshVec[0].teks.size());
}

//Release resources allocated by the program
void freeOpenGLProgram(GLFWwindow* window) {
	//************Place any code here that needs to be executed once, after the main loop ends************
	delete sp;
}
glm::vec3 dir = glm::vec3(0, 0, 1);

void drawScene(GLFWwindow* window,float angle_x,float angle_y) {
	//************Place any code here that draws something inside the window******************l
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 V=glm::lookAt(
        pos,
        pos+dir,
        glm::vec3(0.0f,1.0f,0.0f)); //compute view matrix
    glm::mat4 P=glm::perspective(50.0f*PI/180.0f, aspectRatio, 1.0f, 500.0f); //compute projection matrix

    sp->use();//activate shading program
    //Send parameters to graphics card
    glUniformMatrix4fv(sp->u("P"),1,false,glm::value_ptr(P));
    glUniformMatrix4fv(sp->u("V"),1,false,glm::value_ptr(V));

    glm::mat4 M=glm::mat4(1.0f);
	M=glm::rotate(M,angle_y,glm::vec3(1.0f,0.0f,0.0f)); //Compute model matrix
	M=glm::rotate(M,angle_x,glm::vec3(0.0f,1.0f,0.0f)); //Compute model matrix
    glUniformMatrix4fv(sp->u("M"),1,false,glm::value_ptr(M));
	
	glUniform1i(sp->u("textureMap0"), 0); // Associate sampler textureMap0 with the 0-th texturing unit
	glActiveTexture(GL_TEXTURE0); //Assign texture tex0 to the 0-th texturing unit
	glBindTexture(GL_TEXTURE_2D, tex0);
	glUniform1i(sp->u("textureMap1"), 1); // Associate sampler textureMap1 with the 1-st texturing unit
		glActiveTexture(GL_TEXTURE1); //Assign texture tex1 to the 1-st texturing unit
		glBindTexture(GL_TEXTURE_2D, tex1);

    
	
	for (int x = 0; x < 29; x++)
	{
		glEnableVertexAttribArray(sp->a("vertex")); //Enable sending data to the attribute vertex
		glVertexAttribPointer(sp->a("vertex"),4,GL_FLOAT,false,0, meshVec[x].vertex.position.data()); //Specify source of the data for the attribute vertex
		glEnableVertexAttribArray(sp->a("normal")); //Enable sending data to the attribute color
		glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, meshVec[x].vertex.normal.data()); //Specify source of the data for the attribute normal
		glEnableVertexAttribArray(sp->a("texCoord0")); //Enable sending data to the attribute texCoord0
		glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0, meshVec[x].vertex.texCoords.data()); //Specify source of the data for the attribute texCoord0
		
		//glDrawArrays(GL_TRIANGLES,0,vertices.size()); //Draw the object
		glDrawElements(GL_TRIANGLES, meshVec[x].indices.size(), GL_UNSIGNED_INT, meshVec[x].indices.data());
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


	float angle_x=0; //current rotation angle of the object, x axis
	float angle_y=0; //current rotation angle of the object, y axis
    float player_speed_x = 0;
    float player_speed_y = 0;
	glfwSetTime(0); //Zero the timer
	//Main application loop

	
	while (!glfwWindowShouldClose(window)) //As long as the window shouldnt be closed yet...
	{
        angle_x+=speed_x*glfwGetTime(); //Add angle by which the object was rotated in the previous iteration
		angle_y+=speed_y*glfwGetTime(); //Add angle by which the object was rotated in the previous iteration
        player_speed_x += movement_x * glfwGetTime(); 
        player_speed_y += movement_y * glfwGetTime(); 
        glm::mat4 Mc = glm::rotate(glm::mat4(1.0f), player_speed_y, glm::vec3(0, 1, 0));
        Mc = glm::rotate(Mc, player_speed_x, glm::vec3(1, 0, 0));
        glm::vec4 dir_ = Mc * glm::vec4(0, 0, 1, 0);
        dir = glm::vec3(dir_);

        glm::vec3 mdir = glm::normalize(glm::vec3(dir.x, dir.y, dir.z));

        pos += ws * (float)glfwGetTime() * mdir;
        glfwSetTime(0); //Zero the timer
        drawScene(window, angle_x, angle_y); //Execute drawing procedure
		glfwPollEvents(); //Process callback procedures corresponding to the events that took place up to now

	}
	freeOpenGLProgram(window);

	glfwDestroyWindow(window); //Delete OpenGL context and the window.
	glfwTerminate(); //Free GLFW resources
	exit(EXIT_SUCCESS);
}
