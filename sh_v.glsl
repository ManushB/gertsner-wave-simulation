#version 430

layout(location = 0) in vec3 pos;		// Model-space position
layout(location = 1) in vec2 tex_uv;		// Model-space normal
layout(location = 2) in vec2 cols;
layout(location = 4) uniform float amp1;
layout(location = 5) uniform float amp2;
layout(location = 6) uniform float amp3;
layout(location = 7) uniform float amp4;
layout(location = 3) uniform float time;
layout(location = 8) uniform float phase;

//smooth out vec3 fragNorm;	// Model-space interpolated normal

// Passes the uv coordinates to the fragment shader.
// The uv coordinates will be automatically interpolated.
smooth out vec2 vsout_tex_uv;
uniform mat4 xform;

vec3 dir = normalize(vec3(1.0, 1.0, 1.0));
float l = 0.9;
float pi = 3.142;
float w = sqrt(9.8 * 2 * pi / l);

void main() {
	// Transform vertex position
	float Amp = amp1;
	float Amp2 = amp2;
	float Amp3 = amp3;
	float Amp4 = amp4;
	float Q1 = 1 / (w * Amp);
	float Q2 = 1 / (w * Amp2);
	float Q3 = 1 / (w * Amp3);
	float Q4 = 1 / (w * Amp4);
	vec3 wave1;
	vec3 wave2;
	vec3 wave3;
	vec3 wave4;
	vec3 wave;

	// Gertsner Waves
	wave1.x = (Q1 * Amp * dir.x * cos(dot(w * dir, pos) + phase * time));
	wave1.y = -(Q1 * Amp * dir.y * cos(dot(w * dir, pos) + phase * time));
	wave1.z = Amp * sin(dot(w * dir, pos) + phase * time);

	// Adding x and y of position to have effects resembling ocean waves deflection
	wave2.x = pos.x + (Q2 * Amp2 * dir.x * cos(dot(w * dir, pos) + phase * time));
	wave2.y = pos.y + -(Q2 * Amp2 * dir.y * cos(dot(w * dir, pos) + phase * time));
	wave2.z = Amp2 * sin(dot(w * dir, pos) + phase * time);

	wave3.x = (Q3 * Amp3 * dir.x * cos(dot(w * dir, pos) + phase * time));
	wave3.y = -(Q3 * Amp3 * dir.y * cos(dot(w * dir, pos) + phase * time));
	wave3.z = Amp3 * sin(dot(w * dir, pos) + phase * time);

	wave4.x = pos.x + (Q4 * Amp4 * dir.x * cos(dot(w * dir, pos) + phase * time));
	wave4.y = pos.y + -(Q4 * Amp4 * dir.y * cos(dot(w * dir, pos) + phase * time));
	wave4.z = Amp4 * sin(dot(w * dir, pos) + phase * time);
	
	wave.x = wave1.x + wave2.x + wave3.x + wave4.x;
	wave.y = wave1.y + wave2.y + wave3.y + wave4.y;
	wave.z = wave1.z + wave2.z + wave3.z + wave4.z;

	// Transform vertex position
	gl_Position = xform * vec4(pos + wave, 1.0);
	//vsout_tex_uv = cols;
	vsout_tex_uv = tex_uv;

	// Interpolate normals
	//fragNorm = tex_uv;
}