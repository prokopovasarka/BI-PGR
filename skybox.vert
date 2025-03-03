#version 140
  
 uniform mat4 inversePVmatrix;
 in vec2 screenCoord;
 out vec3 texCoord_v;
 out float mydistance;
 in vec3 position; 
	
  void main() {
    // compute texCoord
    vec4 farCoordinates = vec4(screenCoord, 0.9999, 1.0);
    vec4 worldViewCoord = inversePVmatrix * farCoordinates;

    texCoord_v = worldViewCoord.xyz / worldViewCoord.w;
    gl_Position = farCoordinates;
		mydistance =  distance(vec4(0.0, 0.0, 0.0, 1.0), farCoordinates );

  }