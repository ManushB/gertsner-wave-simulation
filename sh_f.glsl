#version 330

out vec4 outCol;	            // Final pixel color
uniform sampler2D tex_unit;	    // Texture unit number.
smooth in vec2 vsout_tex_uv;	// Interpolated model-space normal

void main() {
	// Visualize normals as colors
	vec4 texture_color = texture(tex_unit, vsout_tex_uv);
	outCol = texture_color;
	//outCol.w = 0.01;
	//outCol = vec4(0.53, 0.8, 0.98, 1.0);
	//gl_FragColor = vec4(0.53, 0.8, 0.98, 1.0);
}