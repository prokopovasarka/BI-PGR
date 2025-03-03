#version 140
  
// uniforms
uniform samplerCube skyboxSampler;
uniform bool fog;           // swith fog

in vec3 texCoord_v;           // fragment texture coordinates
in float mydistance;          // distance for fog

out vec4 color_f;             // outgoing color

// color for mixing
vec4 gray = vec4(0.6, 0.6, 0.6, 1.0);


// applies fog to the incoming color
vec4 useFog() {
	vec4 mypos = vec4(0.0, 0.0, 0.0, 1.0);
	float density = 0.70;
	float e = 2.718;
	float z = mydistance;

	return color_f = pow(e, -density * z)*color_f + (1 - pow(e, -density * z)) * gray;
}

void main() {
	color_f =  texture(skyboxSampler, texCoord_v);
	if(fog) {
			useFog();
	}
}