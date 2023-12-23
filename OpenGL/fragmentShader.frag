#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec3 vertexPosition;
in vec2 TexCoord;

uniform float TIME;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float fade;

void main() {
	FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), fade);
}