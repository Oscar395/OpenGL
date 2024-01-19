#version 330 core
out vec4 FragColor;

void main(){
    vec3 outColor = vec3(1.0f, 1.0f, 1.0f);
	outColor = pow(outColor.rgb, vec3(1.0 / 0.6));
	FragColor = vec4(outColor, 1.0);
}