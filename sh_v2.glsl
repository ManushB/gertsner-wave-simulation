#version 430

layout(location = 0) in vec3 pos;		// Model-space position

// Passes the uv coordinates to the fragment shader.
// The uv coordinates will be automatically interpolated.
smooth out vec2 vsout_tex_uv;
uniform mat4 xform;

vec3 dir = normalize(vec3(1.0, 1.0, 0.25));
float l = 0.2;
float pi = 3.142;
float w = sqrt(9.8 * 2 * pi / l);

void main() {
	// Transform vertex position
	float Amp = 0.14;
	float Qa = 1 / (w * Amp);
	float phase = 10.0f;
	
	vec3 wave1;
	
	vec3 wave;
	
	// Gertsner Waves
	wave1.x = (Qa * Amp * dir.x * cos(dot(w * dir, pos) + phase));
	wave1.y = -(Qa * Amp * dir.y * cos(dot(w * dir, pos) + phase));
	wave1.z = Amp * sin(dot(w * dir, pos) + phase);
	
	wave.x = pos.x + wave1.x;
	wave.y = pos.y + wave1.y;
	wave.z = wave1.z;

	// Transform vertex position
	gl_Position = xform * vec4(pos + wave, 1.0);

}