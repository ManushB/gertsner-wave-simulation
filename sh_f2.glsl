#version 330

smooth in vec3 fragNorm;	// Interpolated model-space normal

//out vec3 outCol;	// Final pixel color

void main() {
	// Visualize normals as colors
	gl_FragColor = vec4(0.54, 0.27, 0.07, 1.0);
}