#version 430 core

layout (location=0) in vec3 vertexPosition;
out vec2 textureCoordinate;

void main() {

	gl_Position = vec4(vertexPosition, 1.0);
	textureCoordinate =  vertexPosition.xy * vec2(0.5) + vec2(0.5);

}