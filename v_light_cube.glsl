#version 330 core
in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

in vec2 texCoord0;
out vec2 iTexCoord0;

void main()
{
	iTexCoord0 = texCoord0;
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}

