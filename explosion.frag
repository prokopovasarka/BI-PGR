#version 140

uniform float time;           // used to select proper animation frame
uniform mat4 Vmatrix;         // view (camera) transform --> world to eye coordinates
uniform sampler2D texSampler; // sampler for texture access

smooth in vec3 position_v;    // fragment position
smooth in vec2 texCoord_v;    // fragment texture coordinates

out vec4 color_f;             // outgoing fragment color

uniform ivec2 pattern = ivec2(12, 7);  // patern for explosion, amount of frames
uniform float frameDuration = 0.01f;   // how much frame lasts


// to evaluate texture coordinates
vec4 sampleTexture(int frame) {

	vec2 offset = vec2(1.0) / vec2(pattern); 

	vec2 coordBase = texCoord_v / vec2(pattern);
	highp float xFrame = (frame % pattern.x)*offset.x;
	highp float yFrame = (-(frame / pattern.x) + pattern.y)*offset.y + 0.01;

	coordBase.x += xFrame;
	coordBase.y += yFrame;

	return texture(texSampler, coordBase);
}

void main() {

		 int frame = int(time / frameDuration);

		color_f = sampleTexture(frame);

}