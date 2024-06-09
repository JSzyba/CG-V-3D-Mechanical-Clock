#version 330

//Uniform variables
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

//Attributes
in vec4 vertex; //Vertex coordinates in model space
in vec4 normal; //Vertex normal in model space
in vec2 texCoord0;

//Varying variables
out vec4 l[4];
out vec4 n;
out vec2 iTexCoord0;

void main(void) {
    vec4 lp[4];
    lp[0]= vec4(0, 10, -6, 1); //light position, world space
    lp[1]= vec4(0, 10, 16, 1);
    lp[2]= vec4(6, 30, 0, 1);
    lp[3]= vec4(-6, 30, 0, 1);
    for(int i=0; i<4; i++)
        l[i] = normalize( lp[i] - M * vertex); //vector towards the light in world space
    n = normalize(M * normal); //normal vector in world space
    
    iTexCoord0 = texCoord0;

    gl_Position = P * V * M * vertex;
}