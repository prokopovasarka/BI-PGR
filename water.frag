#version 140

struct Material {      		 // structure that describes currently used material
		vec3  ambient;       // ambient component
		vec3  diffuse;       // diffuse component
		vec3  specular;      // specular component
		float shininess;     // sharpness of specular reflection

		bool  useTexture;    // defines whether the texture is used or not
};

uniform sampler2D texSampler;  // sampler for the texture access

struct Light {         // structure describing light parameters
		vec3  ambient;       // intensity & color of the ambient component
		vec3  diffuse;       // intensity & color of the diffuse component
		vec3  specular;      // intensity & color of the specular component
		vec3  position;      // light position
		vec3  spotDirection; // spotlight direction
		float spotCosCutOff; // cosine of the spotlight's half angle
		float spotExponent;  // distribution of the light energy within the reflector's cone (center->cone's edge)
};

uniform float time;         // time used for simulation of moving lights (such as sun)
uniform Material material;  // current material

uniform mat4 PVMmatrix;     // Projection * View * Model  --> model to clip coordinates
uniform mat4 Vmatrix;       // View                       --> world to eye coordinates
uniform mat4 Mmatrix;       // Model                      --> model to world coordinates
uniform mat4 normalMatrix;  // inverse transposed Mmatrix

uniform vec3 reflectorPosition;   // reflector position (world coordinates)
uniform vec3 reflectorDirection;  // reflector direction (world coordinates)

//MAKING REFLECTION
uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture;
uniform sampler2D dudvMapTexture;

uniform float moveFactor;

const float waveStrength = 0.02;

//----

uniform bool isFog;                // to enable fog
uniform bool spotLight;            // to turn on spot light on camera
uniform float lightIntensity;      // intensity of the directional and gloabl light, to set daytime
uniform float pointLightIntensity; // intensity of the lamp

smooth in vec2 texCoord_v;             // fragment texture coordinates
smooth in vec3 vertexPosition;         // vertex position in world space
smooth in vec3 vertexNormal;           // vertex normal
in float mydistance;				   // distance from the start of the fog, to compute fog
in vec4 clipSpace;


out vec4       color_f;        // outgoing fragment color

vec4 gray = vec4(0.7, 0.7, 0.7, 1.0);  // color for mixing fog

// fog evaluating
float fog() {

		vec4 mypos = vec4(0.0, 0.0, 0.0, 1.0);
		float density = 0.30;
		float e = 2.718;
		float z = mydistance;
		return pow(e, -density * z);
}

// evaluating spot light
vec4 evalSpotLight(Light light, Material material, vec3 vertexPosition, vec3 vertexNormal) {

		vec3 direction = normalize(light.position - vertexPosition);
		float diffusCoef = max(0.0, dot(vertexNormal, direction));
		float specCoef = max(0.0, dot(reflect(vertexNormal, -direction), normalize(-vertexPosition)));
		float spotCoef = max(0.0, dot(-direction, light.spotDirection));

		vec3 ret = material.specular * light.specular * pow(specCoef, material.shininess);
		ret += material.diffuse * light.diffuse * diffusCoef;
		ret += material.ambient * light.ambient;;

		if (spotCoef < light.spotCosCutOff)
				ret *= 0.0;
		else
				ret *= pow(spotCoef, light.spotExponent);

		return vec4(ret, 1.0);
}

// evaluating directional light
vec4 directionalLight(Light light, Material material, vec3 vertexPosition, vec3 vertexNormal) {

		vec3 direction = normalize(light.position);
		float cosA = dot(direction, vertexNormal);
		float cosB = dot(normalize(-vertexPosition), normalize(reflect(-direction, vertexNormal)));
		
		vec3 ret = pow(max(cosB, 0), material.shininess) * light.specular * material.specular;
		ret += material.ambient * light.ambient;
		ret += max(cosA, 0) * light.diffuse * material.diffuse;

		return vec4(ret, 1.0);
}

// eval point light
vec4 pointLight(Light light, Material material, vec3 position, vec3 normal) {

		vec3 direction  = normalize(light.position);
		float cosA = dot(direction, normal);
		float cosB = dot(normalize(-position), normalize(reflect(-direction, normal)));

		vec3 ret = pow(max(cosB, 0), material.shininess) * light.specular * material.specular;
		ret += material.ambient * light.ambient;
		ret += max(cosA, 0) * light.diffuse * material.diffuse;

		float kc = 0.2f;
		float kl = 0.2f;
		float kq = 1.2f;
		float d = distance(light.position, position);
		float attenuationF = 1.0 / (kc + kl * d + kq * d*d);
		ret = attenuationF * ret;

		return vec4(ret, 1.0);
}


//type
Light sun;
Light cameraReflector;
Light sphereLight;

void setUpRefl() {
	// sphere - point light
		sphereLight.ambient = vec3(1.0) * pointLightIntensity;
		sphereLight.diffuse = vec3(1.0, 1.0, 0.5f) * pointLightIntensity;
		sphereLight.specular = vec3(1.0)* pointLightIntensity;
		sphereLight.position = (Vmatrix * vec4(-0.3, 1.4, 0.2, 1.0)).xyz;
	// camera reflector - spot light
		cameraReflector.ambient       = vec3(0.2f);
		cameraReflector.diffuse       = vec3(1.0);
		cameraReflector.specular      = vec3(1.0);
		cameraReflector.spotCosCutOff = 0.95f;
		cameraReflector.spotExponent  = 0.0;
		
		cameraReflector.position = (Vmatrix * vec4(reflectorPosition, 1.0)).xyz;
		cameraReflector.spotDirection = normalize((Vmatrix * vec4(reflectorDirection, 0.0)).xyz);
}

void main() {
		setUpRefl();
		sun.ambient = vec3(0.5f);
		sun.diffuse = vec3(1.0, 0.5, 0.5f);
		sun.specular = vec3(1.0);
		sun.position = (Vmatrix * vec4(-100.0, -5.0, 100.0, 0.0)).xyz;

		vec3 globalAmbientLight = vec3(0.1f);  // global light

		vec2 ndc = (clipSpace.xy/clipSpace.w)/2.0 + 0.5;
		vec2 refractTexCoords = vec2(ndc.x, ndc.y);
		vec2 reflectTexCoords = vec2(ndc.x, -ndc.y);

		vec2 distortion1 = (texture2D(dudvMapTexture,  vec2((texCoord_v.x/2.0 + 5.0)*6.0 + moveFactor, (texCoord_v.y/2.0 + 5.0)*6.0)).rg * 2.0 - 1.0) * waveStrength;
		vec2 distortion2 = (texture2D(dudvMapTexture,  vec2((-(texCoord_v.x/2.0 + 5.0)*6.0) + moveFactor, (texCoord_v.y/2.0 + 5.0)*6.0)).rg * 2.0 - 1.0 + moveFactor) * waveStrength;
		vec2 totalDist = distortion1 + distortion2;

		refractTexCoords += totalDist;
		refractTexCoords = clamp(refractTexCoords, 0.001, 0.999);

		reflectTexCoords += totalDist;
		reflectTexCoords.x = clamp(reflectTexCoords.x, 0.001, 0.999);
		reflectTexCoords.y = clamp(reflectTexCoords.y, -0.999, -0.001);

		vec4 reflectColor = texture(reflectionTexture, reflectTexCoords);
		vec4 refractColor = texture(refractionTexture, refractTexCoords);

		vec3 viewDir = normalize(vertexPosition); 
		float fresnelFactor = clamp(dot(viewDir, vec3(0.0, 1.0, 0.0)), 0.0, 1.0); // Fresnel Factor 

		fresnelFactor = pow(1.0 - fresnelFactor, 10.0); // strength

		color_f = mix(reflectColor, refractColor, fresnelFactor);

		if(spotLight){
			color_f += evalSpotLight(cameraReflector, material, vertexPosition, vertexNormal);
		}

		
		if (isFog) 
			color_f = fog()*color_f + (1 - fog()) * gray;
		
}
