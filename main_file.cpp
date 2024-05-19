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
float aspectRatio=1;
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
	glm::vec4 position;
	glm::vec4 normal;
	glm::vec2 texCoords;
};

std::vector<Vertex> vertices;
std::vector<unsigned int> indices;

std::vector<glm::vec4>verts;
std::vector<glm::vec4>norms;
std::vector<glm::vec2>teks;


struct Texture {
	unsigned int id;
	std::string type;
};

void processMesh(aiMesh* mesh, const aiScene* scene) {
	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex;
		vertex.position = glm::vec4(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1);
		vertex.normal = glm::vec4(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z, 0);
		if (mesh->mTextureCoords[0]) {
			vertex.texCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		}
		else {
			vertex.texCoords = glm::vec2(0.0f, 0.0f);
		}
		vertices.push_back(vertex);
        verts.push_back(vertex.position);
        norms.push_back(vertex.normal);
        teks.push_back(vertex.texCoords);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}
}

void processNode(aiNode* node, const aiScene* scene) {
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		processMesh(mesh, scene);
	}
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scene);
	}
}

bool loadModel(const std::string& path) {
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		return false;
	}
	processNode(scene->mRootNode, scene);
	return true;
}


unsigned int VAO, VBO, EBO;

void setupMesh() {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	// Pozycje wierzcho³ków
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	// Normalne wierzcho³ków
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

	// Tekstury wierzcho³ków
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

	glBindVertexArray(0);
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
    }
    if (action==GLFW_RELEASE) {
        if (key==GLFW_KEY_LEFT) speed_x=0;
        if (key==GLFW_KEY_RIGHT) speed_x=0;
        if (key==GLFW_KEY_UP) speed_y=0;
        if (key==GLFW_KEY_DOWN) speed_y=0;
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
	glClearColor(0,0,0,1);
	glEnable(GL_DEPTH_TEST);
	glfwSetWindowSizeCallback(window,windowResizeCallback);
	glfwSetKeyCallback(window,keyCallback);
	sp=new ShaderProgram("v_simplest.glsl",NULL,"f_simplest.glsl");
	tex0 = readTexture("metal.png");
	tex1 = readTexture("sky.png");
    std::string modelPath = "Pendulum.obj";
    if (!loadModel(modelPath)) {
        fprintf(stderr, "Failed to load model!\n");
        exit(EXIT_FAILURE);
    }
    printf("Vertices: %d\n", vertices.size());
    printf("Indices: %d\n", indices.size());
}

//Release resources allocated by the program
void freeOpenGLProgram(GLFWwindow* window) {
	//************Place any code here that needs to be executed once, after the main loop ends************
	delete sp;
}

//Drawing procedure
void drawScene(GLFWwindow* window,float angle_x,float angle_y) {
	//************Place any code here that draws something inside the window******************l

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 V=glm::lookAt(
        glm::vec3(10.0f,0.0f,-3.0f),
        glm::vec3(0.0f,10.0f,0.0f),
        glm::vec3(0.0f,1.0f,0.0f)); //compute view matrix
    glm::mat4 P=glm::perspective(50.0f*PI/180.0f, aspectRatio, 1.0f, 50.0f); //compute projection matrix

    sp->use();//activate shading program
    //Send parameters to graphics card
    glUniformMatrix4fv(sp->u("P"),1,false,glm::value_ptr(P));
    glUniformMatrix4fv(sp->u("V"),1,false,glm::value_ptr(V));

    glm::mat4 M=glm::mat4(1.0f);
	M=glm::rotate(M,angle_y,glm::vec3(1.0f,0.0f,0.0f)); //Compute model matrix
	M=glm::rotate(M,angle_x,glm::vec3(0.0f,1.0f,0.0f)); //Compute model matrix
    glUniformMatrix4fv(sp->u("M"),1,false,glm::value_ptr(M));

    glEnableVertexAttribArray(sp->a("vertex")); //Enable sending data to the attribute vertex
    glVertexAttribPointer(sp->a("vertex"),4,GL_FLOAT,false,0, verts.data()); //Specify source of the data for the attribute vertex

	glEnableVertexAttribArray(sp->a("texCoord0")); //Enable sending data to the attribute texCoord0
	glVertexAttribPointer(sp->a("texCoord0"), 2, GL_FLOAT, false, 0,teks.data()); //Specify source of the data for the attribute texCoord0

	glEnableVertexAttribArray(sp->a("normal")); //Enable sending data to the attribute color
	glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, norms.data()); //Specify source of the data for the attribute normal

	glUniform1i(sp->u("textureMap0"), 0); // Associate sampler textureMap0 with the 0-th texturing unit
	glActiveTexture(GL_TEXTURE0); //Assign texture tex0 to the 0-th texturing unit
	glBindTexture(GL_TEXTURE_2D, tex0);

	glUniform1i(sp->u("textureMap1"), 1); // Associate sampler textureMap1 with the 1-st texturing unit
	glActiveTexture(GL_TEXTURE1); //Assign texture tex1 to the 1-st texturing unit
	glBindTexture(GL_TEXTURE_2D, tex1);

    
    //glDrawArrays(GL_TRIANGLES,0,vertices.size()); //Draw the object
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, indices.data());

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


	float angle_x=0; //current rotation angle of the object, x axis
	float angle_y=0; //current rotation angle of the object, y axis
	glfwSetTime(0); //Zero the timer
	//Main application loop

	
	while (!glfwWindowShouldClose(window)) //As long as the window shouldnt be closed yet...
	{
        angle_x+=speed_x*glfwGetTime(); //Add angle by which the object was rotated in the previous iteration
		angle_y+=speed_y*glfwGetTime(); //Add angle by which the object was rotated in the previous iteration
        glfwSetTime(0); //Zero the timer
		drawScene(window,angle_x,angle_y); //Execute drawing procedure
		glfwPollEvents(); //Process callback procedures corresponding to the events that took place up to now

	}
	freeOpenGLProgram(window);

	glfwDestroyWindow(window); //Delete OpenGL context and the window.
	glfwTerminate(); //Free GLFW resources
	exit(EXIT_SUCCESS);
}



/*

#include <iostream>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "lodepng.h"

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

std::vector<Vertex> vertices;
std::vector<unsigned int> indices;
unsigned int VAO, VBO, EBO;
GLFWwindow* window;

bool initOpenGL();
void processMesh(aiMesh* mesh, const aiScene* scene);
void processNode(aiNode* node, const aiScene* scene);
bool loadModel(const std::string& path);
void setupMesh();
void render();
void checkCompileErrors(unsigned int shader, std::string type);

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

GLuint tex0;

unsigned int vertexShader, fragmentShader, shaderProgram;

const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

void main() {
    FragColor = vec4(1.0, 0.5, 0.2, 1.0);
}
)";

int main() {
    if (!initOpenGL()) {
        return -1;
    }

    std::string modelPath = "Pendulum.obj";
    if (!loadModel(modelPath)) {
        std::cerr << "Failed to load model!" << std::endl;
        return -1;
    }

    setupMesh();

    
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    checkCompileErrors(vertexShader, "VERTEX");

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    checkCompileErrors(fragmentShader, "FRAGMENT");

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    checkCompileErrors(shaderProgram, "PROGRAM");

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    while (!glfwWindowShouldClose(window)) {
        render();
    }

    glfwTerminate();
    return 0;
}



bool initOpenGL() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(800, 600, "OpenGL Assimp Example", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW!" << std::endl;
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    tex0 = readTexture("metal.png");
    return true;
}

void processMesh(aiMesh* mesh, const aiScene* scene) {
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        if (mesh->mTextureCoords[0]) {
            vertex.texCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        }
        else {
            vertex.texCoords = glm::vec2(0.0f, 0.0f);
        }
        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }
}

void processNode(aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(mesh, scene);
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

bool loadModel(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return false;
    }

    processNode(scene->mRootNode, scene);
    return true;
}

void setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

    glBindVertexArray(0);

    std::cout << "Vertices count: " << vertices.size() << std::endl;
    std::cout << "Indices count: " << indices.size() << std::endl;
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgram);

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
    unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
    unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glfwSwapBuffers(window);
    glfwPollEvents();
}

void checkCompileErrors(unsigned int shader, std::string type) {
    int success;
    char infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}
*/