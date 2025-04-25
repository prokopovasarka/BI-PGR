#version 140

uniform sampler2D texSampler;  // sampler for the texture access

in vec3 position;           // vertex position in world space
in vec3 normal;             // vertex normal
in vec2 texCoord;           // incoming texture coordinates

uniform mat4 PVMmatrix;     // Projection * View * Model  --> model to clip coordinates
uniform mat4 Vmatrix;       // View                       --> world to eye coordinates
uniform mat4 Mmatrix;       // Model                      --> model to world coordinates
uniform mat4 normalMatrix;  // inverse transposed Mmatrix

uniform vec3 reflectorPosition;   // reflector position (world coordinates)
uniform vec3 reflectorDirection;  // reflector direction (world coordinates)

out float mydistance;             // distance from the start of the fog, to compute fog
out vec4 clipSpace;
smooth out vec3 vertexPosition;		// vertex position in world space
smooth out vec3 vertexNormal;    	// vertex normal in world space
smooth out vec2 texCoord_v;       // outgoing texture coordinates

void main() {

  vertexPosition = (Vmatrix * Mmatrix * vec4(position, 1.0)).xyz;         
  vertexNormal   = normalize( (Vmatrix * normalMatrix * vec4(normal, 0.0) ).xyz);  
  clipSpace = PVMmatrix * vec4(position, 1);  
  gl_Position = PVMmatrix * vec4(position, 1);  

  texCoord_v = texCoord;
  mydistance =  distance(vec4(0.0, 0.0, 0.0, 1.0), vec4(vertexPosition, 1.0));
  gl_ClipDistance[0] = texCoord_v.y - 1.1;
  gl_ClipDistance[1] = 1.1 - texCoord_v.y;

}
