#ifndef CUBE_H_INCLUDED
#define CUBE_H_INCLUDED

float cubeTexCoords[] = {
		1.0f, 0.0f,	  0.0f, 1.0f,    0.0f, 0.0f,
		1.0f, 0.0f,   1.0f, 1.0f,    0.0f, 1.0f,

		1.0f, 0.0f,	  0.0f, 1.0f,    0.0f, 0.0f,
		1.0f, 0.0f,   1.0f, 1.0f,    0.0f, 1.0f,

		1.0f, 0.0f,	  0.0f, 1.0f,    0.0f, 0.0f,
		1.0f, 0.0f,   1.0f, 1.0f,    0.0f, 1.0f,

		1.0f, 0.0f,	  0.0f, 1.0f,    0.0f, 0.0f,
		1.0f, 0.0f,   1.0f, 1.0f,    0.0f, 1.0f,

		1.0f, 0.0f,	  0.0f, 1.0f,    0.0f, 0.0f,
		1.0f, 0.0f,   1.0f, 1.0f,    0.0f, 1.0f,

		1.0f, 0.0f,	  0.0f, 1.0f,    0.0f, 0.0f,
		1.0f, 0.0f,   1.0f, 1.0f,    0.0f, 1.0f,
};

float cubeNormals[] = {
	// Wall 1 (Back face)
	0.0f, 0.0f, -1.0f,  // Normal for all vertices of Wall 1
	0.0f, 0.0f, -1.0f,
	0.0f, 0.0f, -1.0f,
	0.0f, 0.0f, -1.0f,
	0.0f, 0.0f, -1.0f,
	0.0f, 0.0f, -1.0f,

	// Wall 2 (Front face)
	0.0f, 0.0f, 1.0f,  // Normal for all vertices of Wall 2
	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f,

	// Wall 3 (Bottom face)
	0.0f, -1.0f, 0.0f,  // Normal for all vertices of Wall 3
	0.0f, -1.0f, 0.0f,
	0.0f, -1.0f, 0.0f,
	0.0f, -1.0f, 0.0f,
	0.0f, -1.0f, 0.0f,
	0.0f, -1.0f, 0.0f,

	// Wall 4 (Top face)
	0.0f, 1.0f, 0.0f,  // Normal for all vertices of Wall 4
	0.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 0.0f,

	// Wall 5 (Left face)
	-1.0f, 0.0f, 0.0f,  // Normal for all vertices of Wall 5
	-1.0f, 0.0f, 0.0f,
	-1.0f, 0.0f, 0.0f,
	-1.0f, 0.0f, 0.0f,
	-1.0f, 0.0f, 0.0f,
	-1.0f, 0.0f, 0.0f,

	// Wall 6 (Right face)
	1.0f, 0.0f, 0.0f,  // Normal for all vertices of Wall 6
	1.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f,
};

float cubeVertices[] = {
	//Wall 1
	1.0f,-1.0f,-1.0f,1.0f,
	-1.0f, 1.0f,-1.0f,1.0f,
	-1.0f,-1.0f,-1.0f,1.0f,

	1.0f,-1.0f,-1.0f,1.0f,
	1.0f, 1.0f,-1.0f,1.0f,
	-1.0f, 1.0f,-1.0f,1.0f,

	//Wall 2
	-1.0f,-1.0f, 1.0f,1.0f,
	1.0f, 1.0f, 1.0f,1.0f,
	1.0f,-1.0f, 1.0f,1.0f,

	-1.0f,-1.0f, 1.0f,1.0f,
	-1.0f, 1.0f, 1.0f,1.0f,
	1.0f, 1.0f, 1.0f,1.0f,


	//Wall 3
	-1.0f,-1.0f,-1.0f,1.0f,
	1.0f,-1.0f, 1.0f,1.0f,
	1.0f,-1.0f,-1.0f,1.0f,

	-1.0f,-1.0f,-1.0f,1.0f,
	-1.0f,-1.0f, 1.0f,1.0f,
	1.0f,-1.0f, 1.0f,1.0f,

	//Wall 4
	-1.0f, 1.0f, 1.0f,1.0f,
	1.0f, 1.0f,-1.0f,1.0f,
	1.0f, 1.0f, 1.0f,1.0f,

	-1.0f, 1.0f, 1.0f,1.0f,
	-1.0f, 1.0f,-1.0f,1.0f,
	1.0f, 1.0f,-1.0f,1.0f,

	//Wall 5
	-1.0f,-1.0f,-1.0f,1.0f,
	-1.0f, 1.0f, 1.0f,1.0f,
	-1.0f,-1.0f, 1.0f,1.0f,

	-1.0f,-1.0f,-1.0f,1.0f,
	-1.0f, 1.0f,-1.0f,1.0f,
	-1.0f, 1.0f, 1.0f,1.0f,

	//Wall 6
	1.0f,-1.0f, 1.0f,1.0f,
	1.0f, 1.0f,-1.0f,1.0f,
	1.0f,-1.0f,-1.0f,1.0f,

	1.0f,-1.0f, 1.0f,1.0f,
	1.0f, 1.0f, 1.0f,1.0f,
	1.0f, 1.0f,-1.0f,1.0f,




};

unsigned int cubeIndices[] = {
	// Wall 1 (Back face)
	0, 1, 2, 3, 4, 5,
	// Wall 2 (Front face)
	6, 7, 8, 9, 10, 11,
	// Wall 3 (Bottom face)
	12, 13, 14, 15, 16, 17,
	// Wall 4 (Top face)
	18, 19, 20, 21, 22, 23,
	// Wall 5 (Left face)
	24, 25, 26, 27, 28, 29,
	// Wall 6 (Right face)
	30, 31, 32, 33, 34, 35
};

int cubeVertexCount = 36;

#endif