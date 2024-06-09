#version 330 core

in vec2 iTexCoord0;
uniform sampler2D textureMap0;
out vec4 FragColor;

void main()
{
    //FragColor = vec4(1.0,0.753,0.796,1.0); set all 4 vector values to 1.0
    FragColor = texture(textureMap0, iTexCoord0);
}

