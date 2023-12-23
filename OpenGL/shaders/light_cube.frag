#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform vec3 lightColor;
uniform sampler2D lampTexture;

void main() {
    vec3 texColor = lightColor * vec3(texture(lampTexture, TexCoords));
    FragColor = vec4(texColor, 1.0f);
}