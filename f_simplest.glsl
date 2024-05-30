#version 330

out vec4 pixelColor; //Output variable. Almost final pixel color.

uniform sampler2D textureMap0;

//Varying variables
in vec4 n;
in vec4 l[4];
in vec2 iTexCoord0;

void main(void) {
	//Normalized, interpolated vectors
	vec4 ml[4];
	for(int i=0; i<4; i++)
	ml[i] = normalize(l[i]);
	vec4 mn = normalize(n);

	vec4 kd = texture(textureMap0, iTexCoord0);

	float nl[4];
	//Lighting model computation
	for(int i=0; i<4; i++)
		nl[i] = clamp(dot(mn, ml[i]), 0, 1);
	pixelColor = vec4(kd.rgb * nl[0] + kd.rgb * nl[1] + kd.rgb * nl[2] + kd.rgb * nl[3], kd.a);
}