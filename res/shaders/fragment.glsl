#version 430 core

in vec2 textureCoordinate;
layout (binding = 0) uniform sampler2D compute_buffer;
uniform int downsample_factor;
out vec4 screenColor;

vec3 transform_color(vec3 col) {
    const float e = 2.71828;
    const float k = 1.0;
    return vec3(1 - pow(e,-k*col.x), 1 - pow(e,-k*col.y), 1 - pow(e,-k*col.z));
}

void main() {

	vec3 texture_color = texture(compute_buffer, textureCoordinate / downsample_factor).xyz;	
    screenColor = vec4(transform_color(texture_color), 1.0);

}