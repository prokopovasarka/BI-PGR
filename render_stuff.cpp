//-----------------------------------------------------------------------------------------
/**
 * \file       render_stuff.cpp
 * \author     Šárka Prokopová
 * \date       2022/4/28
 * \brief      Functions for rendering, initialization and drawing
 *
*/
//-----------------------------------------------------------------------------------------
#include "render_stuff.h"

GLint SQUARES_AMOUNT = 500;  // amount of squares
GLfloat  waterVertices[SQUARE_SIZE * 10000]; // array for terrain
GLfloat WAVE_SPEED = 0.03f;

// set object geometry to NULL
MeshGeometry* towerGeometry = NULL;
MeshGeometry* skyboxGeometry = NULL;
MeshGeometry* waterGeometry = NULL;
MeshGeometry* explosionGeometry = NULL;
MeshGeometry* barGeometry = NULL;
MeshGeometry* houseGeometry = NULL;
MeshGeometry* cubeGeometry = NULL;
MeshGeometry* sphereGeometry = NULL;

// more meshes
std::vector<MeshGeometry*> maxwellGeometry;
std::vector<MeshGeometry*> duckGeometry;
std::vector<MeshGeometry*> balloonGeometry;
std::vector<MeshGeometry*> boatGeometry;

// paths to textures
const char* SKYBOX_CUBE_TEXTURE_FILE_PREFIX = "data/skybox/skybox";
const char* EXPLOSION_TEXTURE_PATH = "data/textures/explosion/explosion.png";
const char* DUDV_MAP = "data/textures/dudv.jpg";
const char* GRASS_TEXTURE_PATH = "data/textures/plant.png";
const char* LOADING_BAR_PATH = "data/textures/loadingBar.jpg";
// paths to models
const char* TOWER_MODEL_PATH = "data/models/tower/tower.obj";
const char* HOUSE_MODEL_PATH = "data/models/house/house.obj";
const char* CUBE_MODEL_PATH = "data/models/cube/cube.obj";
const char* MAXWELL_MODEL_PATH = "data/models/maxwell/maxwell.obj";
const char* SPHERE_MODEL_PATH = "data/models/icosphere.obj";
const char* DUCK_MODEL_PATH = "data/models/duck/duck.obj";
const char* BALLOON_MODEL_PATH = "data/models/balloons/balloon.obj";
const char* BOAT_MODEL_PATH = "data/models/boat/v_boat.obj";

//textures
GLuint grassTexture;
GLuint loadingBarTexture;


// positions of models
glm::vec3 towerPosition = glm::vec3(0.0f, 0.02f, 1.4f);
glm::vec3 cubePosition = glm::vec3(3.0f, -1.02f, 1.2f);
glm::vec3 cube2Position = glm::vec3(3.0f, -0.42f, 1.2f);
glm::vec3 cube3Position = glm::vec3(2.4f, -0.82f, 1.2f);
glm::vec3 spherePosition = glm::vec3(2.15, -2.72, 1.42);
glm::vec3 housePosition = glm::vec3(2.0, -2.8, 1.5);

// shaders
skyboxFarPlaneShaderProgram  skyboxShader;
SCommonShaderProgram shaderProgram;
SCommonShaderProgram waterShader;
ExplosionShaderProgram explosionShader;
BannerShaderProgram bannerShaderProgram;
BarShaderProgram barShaderProgram;

// uniform variables
GameUniformVariables gameUniVars;
// game state
GameState gameState;
//booleans
bool fog = false;

setUniforms renderObjects::uniSetter;

//---------------------------------------------------LOAD MESHES-----------------------------------------------------------------------------
bool renderObjects::initHandler::loadSingleMesh(const std::string& fileName, SCommonShaderProgram& shader, MeshGeometry** geometry) {
	std::cout << "loading model: " << fileName << "\n";
	Assimp::Importer importer;

	// Unitize object in size (scale the model to fit into (-1..1)^3)
	importer.SetPropertyInteger(AI_CONFIG_PP_PTV_NORMALIZE, 1);

	// Load asset from the file - you can play with various processing steps
	const aiScene* scn = importer.ReadFile(fileName.c_str(), 0
		| aiProcess_Triangulate             // Triangulate polygons (if any).
		| aiProcess_PreTransformVertices    // Transforms scene hierarchy into one root with geometry-leafs only. For more see Doc.
		| aiProcess_GenSmoothNormals        // Calculate normals per vertex.
		| aiProcess_JoinIdenticalVertices);

	// abort if the loader fails
	if (scn == NULL) {
		std::cerr << "assimp error: " << importer.GetErrorString() << std::endl;
		*geometry = NULL;
		return false;
	}

	// some formats store whole scene (multiple meshes and materials, lights, cameras, ...) in one file, we cannot handle that in our simplified example
	if (scn->mNumMeshes != 1) {
		std::cerr << "this simplified loader can only process files with only one mesh" << std::endl;
		*geometry = NULL;
		return false;
	}

	// in this phase we know we have one mesh in our loaded scene, we can directly copy its data to OpenGL ...
	const aiMesh* mesh = scn->mMeshes[0];

	*geometry = new MeshGeometry;

	// vertex buffer object, store all vertex positions and normals
	glGenBuffers(1, &((*geometry)->vertexBufferObject));
	glBindBuffer(GL_ARRAY_BUFFER, (*geometry)->vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float) * mesh->mNumVertices, 0, GL_STATIC_DRAW); // allocate memory for vertices, normals, and texture coordinates
	// first store all vertices
	glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * sizeof(float) * mesh->mNumVertices, mesh->mVertices);
	// then store all normals
	glBufferSubData(GL_ARRAY_BUFFER, 3 * sizeof(float) * mesh->mNumVertices, 3 * sizeof(float) * mesh->mNumVertices, mesh->mNormals);

	// just texture 0 for now
	float* textureCoords = new float[2 * mesh->mNumVertices];  // 2 floats per vertex
	float* currentTextureCoord = textureCoords;

	// copy texture coordinates
	aiVector3D vect;

	if (mesh->HasTextureCoords(0)) {
		// we use 2D textures with 2 coordinates and ignore the third coordinate
		for (unsigned int idx = 0; idx < mesh->mNumVertices; idx++) {
			vect = (mesh->mTextureCoords[0])[idx];
			*currentTextureCoord++ = vect.x;
			*currentTextureCoord++ = vect.y;
		}
	}

	// finally store all texture coordinates
	glBufferSubData(GL_ARRAY_BUFFER, 6 * sizeof(float) * mesh->mNumVertices, 2 * sizeof(float) * mesh->mNumVertices, textureCoords);

	// copy all mesh faces into one big array (assimp supports faces with ordinary number of vertices, we use only 3 -> triangles)
	unsigned int* indices = new unsigned int[mesh->mNumFaces * 3];
	for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
		indices[f * 3 + 0] = mesh->mFaces[f].mIndices[0];
		indices[f * 3 + 1] = mesh->mFaces[f].mIndices[1];
		indices[f * 3 + 2] = mesh->mFaces[f].mIndices[2];
	}

	// copy our temporary index array to OpenGL and free the array
	glGenBuffers(1, &((*geometry)->elementBufferObject));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (*geometry)->elementBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * sizeof(unsigned) * mesh->mNumFaces, indices, GL_STATIC_DRAW);

	delete[] indices;

	// copy the material info to MeshGeometry structure
	const aiMaterial* mat = scn->mMaterials[mesh->mMaterialIndex];
	aiColor4D color;
	aiString name;
	aiReturn retValue = AI_SUCCESS;

	// Get returns: aiReturn_SUCCESS 0 | aiReturn_FAILURE -1 | aiReturn_OUTOFMEMORY -3
	mat->Get(AI_MATKEY_NAME, name); // may be "" after the input mesh processing. Must be aiString type!

	if ((retValue = aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &color)) != AI_SUCCESS)
		color = aiColor4D(0.0f, 0.0f, 0.0f, 0.0f);

	(*geometry)->diffuse = glm::vec3(color.r, color.g, color.b);

	if ((retValue = aiGetMaterialColor(mat, AI_MATKEY_COLOR_AMBIENT, &color)) != AI_SUCCESS)
		color = aiColor4D(0.0f, 0.0f, 0.0f, 0.0f);
	(*geometry)->ambient = glm::vec3(color.r, color.g, color.b);

	if ((retValue = aiGetMaterialColor(mat, AI_MATKEY_COLOR_SPECULAR, &color)) != AI_SUCCESS)
		color = aiColor4D(0.0f, 0.0f, 0.0f, 0.0f);
	(*geometry)->specular = glm::vec3(color.r, color.g, color.b);

	ai_real shininess, strength;
	unsigned int max;	// changed: to unsigned

	max = 1;
	if ((retValue = aiGetMaterialFloatArray(mat, AI_MATKEY_SHININESS, &shininess, &max)) != AI_SUCCESS)
		shininess = 1.0f;
	max = 1;
	if ((retValue = aiGetMaterialFloatArray(mat, AI_MATKEY_SHININESS_STRENGTH, &strength, &max)) != AI_SUCCESS)
		strength = 1.0f;
	(*geometry)->shininess = shininess * strength;

	(*geometry)->texture = 0;

	// load texture image
	if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
		// get texture name 
		aiString path; // filename

		aiReturn texFound = mat->GetTexture(aiTextureType_DIFFUSE, 0, &path);
		std::string textureName = path.data;

		size_t found = fileName.find_last_of("/\\");
		// insert correct texture file path 
		if (found != std::string::npos) { // not found
		  //subMesh_p->textureName.insert(0, "/");
			textureName.insert(0, fileName.substr(0, found + 1));
		}

		std::cout << "Loading texture file: " << textureName << std::endl;
		(*geometry)->texture = pgr::createTexture(textureName);
	}
	CHECK_GL_ERROR();

	glGenVertexArrays(1, &((*geometry)->vertexArrayObject));
	glBindVertexArray((*geometry)->vertexArrayObject);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (*geometry)->elementBufferObject); // bind our element array buffer (indices) to vao
	glBindBuffer(GL_ARRAY_BUFFER, (*geometry)->vertexBufferObject);

	glEnableVertexAttribArray(shader.posLocation);
	glVertexAttribPointer(shader.posLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

	if (gameUniVars.useLighting == true) {
		glEnableVertexAttribArray(shader.normalLocation);
		glVertexAttribPointer(shader.normalLocation, 3, GL_FLOAT, GL_FALSE, 0, (void*)(3 * sizeof(float) * mesh->mNumVertices));
	}
	else {
		glDisableVertexAttribArray(shader.colorLocation);
		// following line is problematic on AMD/ATI graphic cards
		// -> if you see black screen (no objects at all) than try to set color manually in vertex shader to see at least something
		glVertexAttrib3f(shader.colorLocation, color.r, color.g, color.b);
	}

	glEnableVertexAttribArray(shader.texCoordLocation);
	glVertexAttribPointer(shader.texCoordLocation, 2, GL_FLOAT, GL_FALSE, 0, (void*)(6 * sizeof(float) * mesh->mNumVertices));
	CHECK_GL_ERROR();

	glBindVertexArray(0);

	(*geometry)->numTriangles = mesh->mNumFaces;

	return true;
}

bool renderObjects::initHandler::loadMesh(const std::string& fileName, SCommonShaderProgram& shader, std::vector<MeshGeometry*>* geometryFull) {
	std::cout << "loading model: " << fileName << "\n";
	Assimp::Importer importer;

	// Unitize object in size (scale the model to fit into (-1..1)^3)
	importer.SetPropertyInteger(AI_CONFIG_PP_PTV_NORMALIZE, 1);

	// Load asset from the file - you can play with various processing steps
	const aiScene* scn = importer.ReadFile(fileName.c_str(), 0
		| aiProcess_Triangulate             // Triangulate polygons (if any).
		| aiProcess_PreTransformVertices    // Transforms scene hierarchy into one root with geometry-leafs only. For more see Doc.
		| aiProcess_GenSmoothNormals        // Calculate normals per vertex.
		| aiProcess_JoinIdenticalVertices);

	// abort if the loader fails

	if (scn == NULL) {
		std::cerr << "assimp error: " << importer.GetErrorString() << std::endl;
		//null geometry
		return false;
	}
	MeshGeometry* geometry;

	// in this phase we know we have one mesh in our loaded scene, we can directly copy its data to OpenGL ...

	for (int i = 0; i < scn->mNumMeshes; i++) {
		const aiMesh* mesh = scn->mMeshes[0];

		geometry = new MeshGeometry;


		// vertex buffer object, store all vertex positions and normals
		glGenBuffers(1, &(geometry->vertexBufferObject));
		glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBufferObject);
		glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float) * mesh->mNumVertices, 0, GL_STATIC_DRAW); // allocate memory for vertices, normals, and texture coordinates
		// first store all vertices
		glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * sizeof(float) * mesh->mNumVertices, mesh->mVertices);
		// then store all normals
		glBufferSubData(GL_ARRAY_BUFFER, 3 * sizeof(float) * mesh->mNumVertices, 3 * sizeof(float) * mesh->mNumVertices, mesh->mNormals);

		// just texture 0 for now
		float* textureCoords = new float[2 * mesh->mNumVertices];  // 2 floats per vertex
		float* currentTextureCoord = textureCoords;

		// copy texture coordinates
		aiVector3D vect;

		if (mesh->HasTextureCoords(0)) {
			// we use 2D textures with 2 coordinates and ignore the third coordinate
			for (unsigned int idx = 0; idx < mesh->mNumVertices; idx++) {
				vect = (mesh->mTextureCoords[0])[idx];
				*currentTextureCoord++ = vect.x;
				*currentTextureCoord++ = vect.y;
			}
		}

		// finally store all texture coordinates
		glBufferSubData(GL_ARRAY_BUFFER, 6 * sizeof(float) * mesh->mNumVertices, 2 * sizeof(float) * mesh->mNumVertices, textureCoords);

		// copy all mesh faces into one big array (assimp supports faces with ordinary number of vertices, we use only 3 -> triangles)
		unsigned int* indices = new unsigned int[mesh->mNumFaces * 3];
		for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
			indices[f * 3 + 0] = mesh->mFaces[f].mIndices[0];
			indices[f * 3 + 1] = mesh->mFaces[f].mIndices[1];
			indices[f * 3 + 2] = mesh->mFaces[f].mIndices[2];
		}

		// copy our temporary index array to OpenGL and free the array
		glGenBuffers(1, &(geometry->elementBufferObject));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry->elementBufferObject);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * sizeof(unsigned) * mesh->mNumFaces, indices, GL_STATIC_DRAW);

		delete[] indices;

		// copy the material info to MeshGeometry structure
		const aiMaterial* mat = scn->mMaterials[mesh->mMaterialIndex];
		aiColor4D color;
		aiString name;
		aiReturn retValue = AI_SUCCESS;

		// Get returns: aiReturn_SUCCESS 0 | aiReturn_FAILURE -1 | aiReturn_OUTOFMEMORY -3
		mat->Get(AI_MATKEY_NAME, name); // may be "" after the input mesh processing. Must be aiString type!

		if ((retValue = aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &color)) != AI_SUCCESS)
			color = aiColor4D(0.0f, 0.0f, 0.0f, 0.0f);

		geometry->diffuse = glm::vec3(color.r, color.g, color.b);

		if ((retValue = aiGetMaterialColor(mat, AI_MATKEY_COLOR_AMBIENT, &color)) != AI_SUCCESS)
			color = aiColor4D(0.0f, 0.0f, 0.0f, 0.0f);
		geometry->ambient = glm::vec3(color.r, color.g, color.b);

		if ((retValue = aiGetMaterialColor(mat, AI_MATKEY_COLOR_SPECULAR, &color)) != AI_SUCCESS)
			color = aiColor4D(0.0f, 0.0f, 0.0f, 0.0f);
		geometry->specular = glm::vec3(color.r, color.g, color.b);

		ai_real shininess, strength;
		unsigned int max;	// changed: to unsigned

		max = 1;
		if ((retValue = aiGetMaterialFloatArray(mat, AI_MATKEY_SHININESS, &shininess, &max)) != AI_SUCCESS)
			shininess = 1.0f;
		max = 1;
		if ((retValue = aiGetMaterialFloatArray(mat, AI_MATKEY_SHININESS_STRENGTH, &strength, &max)) != AI_SUCCESS)
			strength = 1.0f;
		geometry->shininess = shininess * strength;

		geometry->texture = 0;

		// load texture image
		if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
			// get texture name 
			aiString path; // filename

			aiReturn texFound = mat->GetTexture(aiTextureType_DIFFUSE, 0, &path);
			std::string textureName = path.data;

			size_t found = fileName.find_last_of("/\\");
			// insert correct texture file path 
			if (found != std::string::npos) { // not found
				//subMesh_p->textureName.insert(0, "/");
				textureName.insert(0, fileName.substr(0, found + 1));
			}

			std::cout << "Loading texture file: " << textureName << std::endl;
			geometry->texture = pgr::createTexture(textureName);
		}

		glGenVertexArrays(1, &(geometry->vertexArrayObject));
		glBindVertexArray(geometry->vertexArrayObject);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry->elementBufferObject); // bind our element array buffer (indices) to vao
		glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBufferObject);

		glEnableVertexAttribArray(shader.posLocation);
		glVertexAttribPointer(shader.posLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

		if (gameUniVars.useLighting == true) {
			glEnableVertexAttribArray(shader.normalLocation);
			glVertexAttribPointer(shader.normalLocation, 3, GL_FLOAT, GL_FALSE, 0, (void*)(3 * sizeof(float) * mesh->mNumVertices));
		}
		else {
			glDisableVertexAttribArray(shader.colorLocation);
			glVertexAttrib3f(shader.colorLocation, color.r, color.g, color.b);
		}

		glEnableVertexAttribArray(shader.texCoordLocation);
		glVertexAttribPointer(shader.texCoordLocation, 2, GL_FLOAT, GL_FALSE, 0, (void*)(6 * sizeof(float) * mesh->mNumVertices));

		glBindVertexArray(0);

		geometry->numTriangles = mesh->mNumFaces;
		geometryFull->push_back(geometry);
	}

	return true;
}

//------------------------------------------------------------DRAW SKYBOX------------------------------------------------------------------------------
void renderObjects::drawHandler::drawSkybox(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, skyboxFarPlaneShaderProgram& skyboxShader, MeshGeometry** geometry, GameUniformVariables gameUni) {
	glm::vec4 fogColor = FOG_COLOR;
	fog = false;
	glUseProgram(skyboxShader.program);

	glm::mat4 matrix = projectionMatrix * viewMatrix;
	glm::mat4 viewRotation = viewMatrix;

	viewRotation[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	glm::mat4 inversePVmatrix = glm::inverse(projectionMatrix * viewRotation);
	glUniformMatrix4fv(skyboxShader.inversePVmatrixLocation, 1, GL_FALSE, glm::value_ptr(inversePVmatrix));
	glUniform1i(skyboxShader.skyboxSamplerLocation, 0);
	glUniform1f(skyboxShader.isFogLocation, gameUni.isFog);

	glBindVertexArray((*geometry)->vertexArrayObject);
	glBindTexture(GL_TEXTURE_CUBE_MAP, (*geometry)->texture);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, (*geometry)->numTriangles + 2);

	glBindVertexArray(0);
	glUseProgram(0);
}

// initialize skybox geometry
void renderObjects::initHandler::initSkyboxGeometry(skyboxFarPlaneShaderProgram  skyboxShader, MeshGeometry** geometry) {

	*geometry = new MeshGeometry;

	static const float skyboxTextureCoord[] = {
			1.0f, 1.0f,
			-1.0f, 1.0f,
			1.0f,  -1.0f,
			-1.0f,  -1.0f,
	};

	GLuint targets[] = {
		GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
	};

	glGenBuffers(1, &((*geometry)->vertexBufferObject));
	glBindBuffer(GL_ARRAY_BUFFER, (*geometry)->vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxTextureCoord), skyboxTextureCoord, GL_STATIC_DRAW);
	glGenVertexArrays(1, &((*geometry)->vertexArrayObject));
	glBindVertexArray((*geometry)->vertexArrayObject);
	glEnableVertexAttribArray(skyboxShader.screenCoordLocation);
	glVertexAttribPointer(skyboxShader.screenCoordLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);
	glUseProgram(0);

	(*geometry)->numTriangles = 2;

	glActiveTexture(GL_TEXTURE0);

	glGenTextures(1, &((*geometry)->texture));
	glBindTexture(GL_TEXTURE_CUBE_MAP, (*geometry)->texture);

	// load pictures for skybox
	for (int i = 1; i < 7 ; i++) {
		std::string texName = std::string(SKYBOX_CUBE_TEXTURE_FILE_PREFIX) + std::to_string(i) + ".jpg";
		if (!pgr::loadTexImage2D(texName, targets[i - 1])) {
			printf("Couldn't load skybox texture.\n");
		}
	}

	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

// initialize all shaders
void renderObjects::initHandler::initializeShaderPrograms() {

	std::vector<GLuint> shaderList;

	// MAIN SHADER
	shaderList.push_back(pgr::createShaderFromFile(GL_VERTEX_SHADER, "lighting.vert"));
	shaderList.push_back(pgr::createShaderFromFile(GL_FRAGMENT_SHADER, "lighting.frag"));
	// create the shader program with two shaders
	shaderProgram.program = pgr::createProgram(shaderList);

	// get vertex attributes locations
	shaderProgram.posLocation = glGetAttribLocation(shaderProgram.program, "position");
	shaderProgram.normalLocation = glGetAttribLocation(shaderProgram.program, "normal");
	shaderProgram.texCoordLocation = glGetAttribLocation(shaderProgram.program, "texCoord");
	shaderProgram.colorLocation = glGetAttribLocation(shaderProgram.program, "color");
	// -----------------
	//  get uniforms locations
	shaderProgram.PVMmatrixLocation = glGetUniformLocation(shaderProgram.program, "PVMmatrix");
	shaderProgram.VmatrixLocation = glGetUniformLocation(shaderProgram.program, "Vmatrix");
	shaderProgram.MmatrixLocation = glGetUniformLocation(shaderProgram.program, "Mmatrix");
	shaderProgram.normalMatrixLocation = glGetUniformLocation(shaderProgram.program, "normalMatrix");
	shaderProgram.timeLocation = glGetUniformLocation(shaderProgram.program, "time");
	// material
	shaderProgram.ambientLocation = glGetUniformLocation(shaderProgram.program, "material.ambient");
	shaderProgram.diffuseLocation = glGetUniformLocation(shaderProgram.program, "material.diffuse");
	shaderProgram.specularLocation = glGetUniformLocation(shaderProgram.program, "material.specular");
	shaderProgram.shininessLocation = glGetUniformLocation(shaderProgram.program, "material.shininess");
	// texture
	shaderProgram.secTextureLocation = glGetUniformLocation(shaderProgram.program, "secTexture");
	shaderProgram.texSamplerLocation = glGetUniformLocation(shaderProgram.program, "texSampler");
	shaderProgram.texSampler2Location = glGetUniformLocation(shaderProgram.program, "texSampler2");
	shaderProgram.useTextureLocation = glGetUniformLocation(shaderProgram.program, "material.useTexture");
	// reflector
	shaderProgram.reflectorPositionLocation = glGetUniformLocation(shaderProgram.program, "reflectorPosition");
	shaderProgram.reflectorDirectionLocation = glGetUniformLocation(shaderProgram.program, "reflectorDirection");
	// fog switch
	shaderProgram.isFogLocation = glGetUniformLocation(shaderProgram.program, "fog");
	// lights
	shaderProgram.spotLightLocation = glGetUniformLocation(shaderProgram.program, "spotLight");
	shaderProgram.pointLightIntensityLocation = glGetUniformLocation(shaderProgram.program, "pointLightIntensity");
	shaderProgram.lightIntensityLocation = glGetUniformLocation(shaderProgram.program, "lightIntensity");

	//SKYBOX SHADER

	shaderList.clear();

	// push vertex shader and fragment shader
	shaderList.push_back(pgr::createShaderFromFile(GL_VERTEX_SHADER, "skybox.vert"));
	shaderList.push_back(pgr::createShaderFromFile(GL_FRAGMENT_SHADER, "skybox.frag"));

	skyboxShader.program = pgr::createProgram(shaderList);

	// get vertex attributes locations
	skyboxShader.screenCoordLocation = glGetAttribLocation(skyboxShader.program, "screenCoord");
	// get uniforms locations
	skyboxShader.skyboxSamplerLocation = glGetUniformLocation(skyboxShader.program, "skyboxSampler");
	skyboxShader.inversePVmatrixLocation = glGetUniformLocation(skyboxShader.program, "inversePVmatrix");
	skyboxShader.isFogLocation = glGetUniformLocation(skyboxShader.program, "fog");

	shaderList.clear();
	// WATER SHADER
	// use same vertex shader
	shaderList.push_back(pgr::createShaderFromFile(GL_VERTEX_SHADER, "lighting.vert"));
	shaderList.push_back(pgr::createShaderFromFile(GL_FRAGMENT_SHADER, "water.frag"));

	waterShader.program = pgr::createProgram(shaderList);
	// get vertex attributes locations
	waterShader.posLocation = glGetAttribLocation(waterShader.program, "position");
	waterShader.normalLocation = glGetAttribLocation(waterShader.program, "normal");
	waterShader.texCoordLocation = glGetAttribLocation(waterShader.program, "texCoord");
	waterShader.colorLocation = glGetAttribLocation(waterShader.program, "color");
	// get uniforms locations
	waterShader.PVMmatrixLocation = glGetUniformLocation(waterShader.program, "PVMmatrix");
	waterShader.VmatrixLocation = glGetUniformLocation(waterShader.program, "Vmatrix");
	waterShader.MmatrixLocation = glGetUniformLocation(waterShader.program, "Mmatrix");
	waterShader.normalMatrixLocation = glGetUniformLocation(waterShader.program, "normalMatrix");
	waterShader.timeLocation = glGetUniformLocation(waterShader.program, "time");
	// material
	waterShader.ambientLocation = glGetUniformLocation(waterShader.program, "material.ambient");
	waterShader.diffuseLocation = glGetUniformLocation(waterShader.program, "material.diffuse");
	waterShader.specularLocation = glGetUniformLocation(waterShader.program, "material.specular");
	waterShader.shininessLocation = glGetUniformLocation(waterShader.program, "material.shininess");
	// texture
	waterShader.texSamplerLocation = glGetUniformLocation(waterShader.program, "texSampler");
	waterShader.useTextureLocation = glGetUniformLocation(waterShader.program, "material.useTexture");
	// reflector
	waterShader.reflectorPositionLocation = glGetUniformLocation(waterShader.program, "reflectorPosition");
	waterShader.reflectorDirectionLocation = glGetUniformLocation(waterShader.program, "reflectorDirection");
	//reflection, refraction
	waterShader.reflectionTextureLocation = glGetUniformLocation(waterShader.program, "reflectionTexture");
	waterShader.refractionTextureLocation = glGetUniformLocation(waterShader.program, "refractionTexture");
	waterShader.dudvMapLocation = glGetUniformLocation(waterShader.program, "dudvMapTexture");
	waterShader.moveFactorLocation = glGetUniformLocation(waterShader.program, "moveFactor");
	// fog switch
	waterShader.isFogLocation = glGetUniformLocation(waterShader.program, "isFog");
	// lights
	waterShader.spotLightLocation = glGetUniformLocation(waterShader.program, "spotLight");
	waterShader.lightIntensityLocation = glGetUniformLocation(waterShader.program, "lightIntensity");

	shaderList.clear();

	// EXPLOSION SHADER
	// push vertex shader and fragment shader
	shaderList.push_back(pgr::createShaderFromFile(GL_VERTEX_SHADER, "explosion.vert"));
	shaderList.push_back(pgr::createShaderFromFile(GL_FRAGMENT_SHADER, "explosion.frag"));

	explosionShader.program = pgr::createProgram(shaderList);

	// get vertex attributes locations
	explosionShader.posLocation = glGetAttribLocation(explosionShader.program, "position");
	// get uniforms locations
	explosionShader.texCoordLocation = glGetAttribLocation(explosionShader.program, "texCoord");
	explosionShader.PVMmatrixLocation = glGetUniformLocation(explosionShader.program, "PVMmatrix");
	explosionShader.VmatrixLocation = glGetUniformLocation(explosionShader.program, "Vmatrix");
	explosionShader.timeLocation = glGetUniformLocation(explosionShader.program, "time");
	explosionShader.texSamplerLocation = glGetUniformLocation(explosionShader.program, "texSampler");
	explosionShader.frameDurationLocation = glGetUniformLocation(explosionShader.program, "frameDuration");

	shaderList.clear();

	// BANNER SHADER
	
	// push vertex shader and fragment shader
	shaderList.push_back(pgr::createShaderFromFile(GL_VERTEX_SHADER, "banner.vert"));
	shaderList.push_back(pgr::createShaderFromFile(GL_FRAGMENT_SHADER, "banner.frag"));

	// Create the program with two shaders
	bannerShaderProgram.program = pgr::createProgram(shaderList);

	// get position and color attributes locations
	bannerShaderProgram.posLocation = glGetAttribLocation(bannerShaderProgram.program, "position");
	bannerShaderProgram.texCoordLocation = glGetAttribLocation(bannerShaderProgram.program, "texCoord");
	// get uniforms locations
	bannerShaderProgram.PVMmatrixLocation = glGetUniformLocation(bannerShaderProgram.program, "PVMmatrix");
	bannerShaderProgram.timeLocation = glGetUniformLocation(bannerShaderProgram.program, "time");
	bannerShaderProgram.texSamplerLocation = glGetUniformLocation(bannerShaderProgram.program, "texSampler");

	shaderList.clear();

	// LOADING BAR SHADER

	// push vertex shader and fragment shader
	shaderList.push_back(pgr::createShaderFromFile(GL_VERTEX_SHADER, "loadingBar.vert"));
	shaderList.push_back(pgr::createShaderFromFile(GL_FRAGMENT_SHADER, "loadingBar.frag"));

	barShaderProgram.program = pgr::createProgram(shaderList);

	// Získejte pozice atributù (vertex position a texture coordinates)
	barShaderProgram.posLocation = glGetAttribLocation(barShaderProgram.program, "position");
	barShaderProgram.texCoordLocation = glGetAttribLocation(barShaderProgram.program, "texCoord");

	// Získejte pozice uniformních promìnných
	barShaderProgram.PVMmatrixLocation = glGetUniformLocation(barShaderProgram.program, "PVMmatrix");
	barShaderProgram.timeLocation = glGetUniformLocation(barShaderProgram.program, "time");
	barShaderProgram.texSamplerLocation = glGetUniformLocation(barShaderProgram.program, "texSampler");
}

// initialize materials
void renderObjects::initHandler::initMaterial(MeshGeometry** geometry, Material material) {
	(*geometry)->ambient = material.ambient;
	(*geometry)->diffuse = material.diffuse;
	(*geometry)->specular = material.specular;
	(*geometry)->shininess = material.shininess;
	(*geometry)->texture = pgr::createTexture(material.texture);
}

// initialize water
void renderObjects::initHandler::initWater(SCommonShaderProgram& shader, MeshGeometry** geometry, waterBufferMaker* waterFBOHandler) {
	generateWater(waterVertices);

	*geometry = new MeshGeometry;
	(*geometry)->numTriangles = SQUARES_AMOUNT * 2;
	glGenVertexArrays(1, &((*geometry)->vertexArrayObject));
	glBindVertexArray((*geometry)->vertexArrayObject);

	glGenBuffers(1, &((*geometry)->vertexBufferObject));
	glBindBuffer(GL_ARRAY_BUFFER, (*geometry)->vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(waterVertices), waterVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(shader.posLocation);
	glVertexAttribPointer(shader.posLocation, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(shader.normalLocation);
	glVertexAttribPointer(shader.normalLocation, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

	glEnableVertexAttribArray(shader.texCoordLocation);
	glVertexAttribPointer(shader.texCoordLocation, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, waterFBOHandler->getReflectionTexture());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, waterFBOHandler->getRefractionTexture());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, waterFBOHandler->getdudvMapTexID());
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindVertexArray(0);
	CHECK_GL_ERROR();
}

// initialize explosion
void renderObjects::initHandler::initExplosion(ExplosionShaderProgram& explosionShader, MeshGeometry** geometry) {
	*geometry = new MeshGeometry;

	glGenVertexArrays(1, &((*geometry)->vertexArrayObject));
	glBindVertexArray((*geometry)->vertexArrayObject);
	CHECK_GL_ERROR();

	glGenBuffers(1, &((*geometry)->vertexBufferObject));
	glBindBuffer(GL_ARRAY_BUFFER, (*geometry)->vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(explosionVertexData), explosionVertexData, GL_STATIC_DRAW);

	(*geometry)->texture = pgr::createTexture(EXPLOSION_TEXTURE_PATH);

	glEnableVertexAttribArray(explosionShader.posLocation);
	glVertexAttribPointer(explosionShader.posLocation, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	glEnableVertexAttribArray(explosionShader.texCoordLocation);
	glVertexAttribPointer(explosionShader.texCoordLocation, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glBindVertexArray(0);
	(*geometry)->numTriangles = explosionVertexCount;
}

// initialize all models
void renderObjects::initHandler::initializeModels(waterBufferMaker* waterFBOHandler) {
	initSkyboxGeometry(skyboxShader, &skyboxGeometry);
	std::cout << "Loading texture file: " << "dudv" << std::endl;
	waterFBOHandler->setDudvMapTex(pgr::createTexture(DUDV_MAP));
	initWater(waterShader, &waterGeometry, waterFBOHandler);
	initBarGeometry(barShaderProgram.program, &barGeometry);
	initExplosion(explosionShader, &explosionGeometry);
	if (loadSingleMesh(TOWER_MODEL_PATH, shaderProgram, &towerGeometry) != true) {
		std::cerr << "initializeModels(): tower model loading failed." << std::endl;
	}
	if (loadSingleMesh(CUBE_MODEL_PATH, shaderProgram, &cubeGeometry) != true) {
		std::cerr << "initializeModels(): Cube model loading failed." << std::endl;
	}
	grassTexture = pgr::createTexture(GRASS_TEXTURE_PATH);
	if(!grassTexture) {
		std::cerr << "loading failed." << std::endl;
	};
	if (loadSingleMesh(HOUSE_MODEL_PATH, shaderProgram, &houseGeometry) != true) {
		std::cerr << "initializeModels(): house model loading failed." << std::endl;
	}
	if (loadMesh(MAXWELL_MODEL_PATH, shaderProgram, &maxwellGeometry) != true) {
		std::cerr << "initializeModels(): maxwell model loading failed." << std::endl;
	}
	if (loadMesh(DUCK_MODEL_PATH, shaderProgram, &duckGeometry) != true) {
		std::cerr << "initializeModels(): flamingo model loading failed." << std::endl;
	}
	if (loadSingleMesh(SPHERE_MODEL_PATH, shaderProgram, &sphereGeometry) != true) {
		std::cerr << "initializeModels(): sphere model loading failed." << std::endl;
	}
	if (loadMesh(BALLOON_MODEL_PATH, shaderProgram, &balloonGeometry) != true) {
		std::cerr << "initializeModels(): balloon model loading failed." << std::endl;
	}
	if (loadMesh(BOAT_MODEL_PATH, shaderProgram, &boatGeometry) != true) {
		std::cerr << "initializeModels(): balloon model loading failed." << std::endl;
	}
}

// initialize banner geometry
void renderObjects::initHandler::initBarGeometry(GLuint shader, MeshGeometry** geometry) {
	*geometry = new MeshGeometry;

	(*geometry)->texture = pgr::createTexture(LOADING_BAR_PATH);
	loadingBarTexture = (*geometry)->texture;
	glBindTexture(GL_TEXTURE_2D, (*geometry)->texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);

	glGenVertexArrays(1, &((*geometry)->vertexArrayObject));
	glBindVertexArray((*geometry)->vertexArrayObject);

	glGenBuffers(1, &((*geometry)->vertexBufferObject));
	glBindBuffer(GL_ARRAY_BUFFER, (*geometry)->vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(barVertexData), barVertexData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(barShaderProgram.posLocation);
	glEnableVertexAttribArray(barShaderProgram.texCoordLocation);
	// vertices of triangles - start at the beginning of the interlaced array
	glVertexAttribPointer(barShaderProgram.posLocation, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	// texture coordinates of each vertices are stored just after its position
	glVertexAttribPointer(barShaderProgram.texCoordLocation, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glBindVertexArray(0);

	(*geometry)->numTriangles = barNumQuadVertices;
}

//--------------------------------------------------------------------------------TEXTURES--------------------------------------------------------

// draw water
void renderObjects::drawHandler::drawWater(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, SCommonShaderProgram& shaderProgram, MeshGeometry** geometry, GameUniformVariables gameUni) {

	GLfloat factor = WAVE_SPEED * gameState.elapsedTime;
	factor = std::fmod(factor, 1.0);
	glUseProgram(shaderProgram.program);
	uniSetter.setWaterUni(shaderProgram, factor);
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::scale(modelMatrix, glm::vec3(1, 1, 1));
	uniSetter.setTransformUniforms(modelMatrix, viewMatrix, projectionMatrix, shaderProgram);
	glUniform1f(shaderProgram.timeLocation, gameState.elapsedTime);

	uniSetter.setMaterialUniforms( *geometry, shaderProgram, gameUni );

	glBindVertexArray((*geometry)->vertexArrayObject);
	glDrawArrays(GL_TRIANGLES, 0, 3 * (*geometry)->numTriangles);
	glBindVertexArray(0);
	glUseProgram(0);
}

// draws banner with credits
void renderObjects::drawHandler::drawBar(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, float loadingBarWidth) {
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);                     // alpha 
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(barShaderProgram.program);

	glm::mat4 P = glm::ortho(-1.f, 1.f, -1.f, 1.f);    // 2-D projectionviewport
	glm::mat4 V = glm::mat4(1.f);                      //no view
	glm::mat4 M = glm::translate(glm::mat4(1.f), glm::vec3(0.0f, 0.95f, 0.f)) // right up
		* glm::scale(glm::mat4(1.f), glm::vec3(1.0f, 0.2f, 1.f));

	glm::mat4 PVM = P * V * M;
	glUniformMatrix4fv(barShaderProgram.PVMmatrixLocation, 1, GL_FALSE, glm::value_ptr(PVM));

	// Pass the speed to the shader
	glUniform1f(barShaderProgram.timeLocation, loadingBarWidth);  // Pass 'speed' directly to the shader to control texture cut-off

	// Bind texture
	glBindTexture(GL_TEXTURE_2D, loadingBarTexture);
	glUniform1i(barShaderProgram.texSamplerLocation, 0); // Assuming texture unit 0 is used

	// Bind the vertex array object and draw the geometry
	glBindVertexArray(barGeometry->vertexArrayObject);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, barGeometry->numTriangles);

	glBindVertexArray(0);

	glUseProgram(0);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

//--------------------------------------------------------------------------------MODELS----------------------------------------------------------

// draw single object
void renderObjects::drawHandler::drawObject(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, SCommonShaderProgram& shaderProgram, std::vector<MeshGeometry*>* geometry, GameUniformVariables gameUni, ObjectProp param) {
	glUseProgram(shaderProgram.program);
	glm::mat4 modelMatrix = glm::mat4(1.0f);

	if (param.align) {
		modelMatrix = splineHandler::alignObject(param.position, param.front, param.up);
	}
	else {
		modelMatrix = glm::translate(modelMatrix, param.position);
		modelMatrix = glm::rotate(modelMatrix, param.angle, param.front);
	}

	modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0, 1.0, 1.0) * param.size);
	uniSetter.setTransformUniforms(modelMatrix, viewMatrix, projectionMatrix, shaderProgram);

	for (int i = 0; i < (*geometry).size(); i++) {

		uniSetter.setMaterialUniforms((*geometry)[i],shaderProgram, gameUni);

		glBindVertexArray((*geometry)[i]->vertexArrayObject);
		glDrawElements(GL_TRIANGLES, (*geometry)[i]->numTriangles * 3, GL_UNSIGNED_INT, 0);
	}
	glBindVertexArray(0);
	glUseProgram(0);
	return;
}

// draw cube model
void renderObjects::drawHandler::drawCube(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, SCommonShaderProgram& shaderProgram, MeshGeometry** geometry, GameUniformVariables gameUni, glm::vec3 cubePosition, float angle) {
	glUseProgram(shaderProgram.program);
	uniSetter.setMaterialUniforms((*geometry), shaderProgram, gameUni);

	glm::mat4 modelMatrix;
	modelMatrix = splineHandler::alignObject(cubePosition, glm::vec3(0.4, 1.0, 0.0), glm::vec3(0.0f, 0.5f, 1.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2, 0.2, 0.2));
	modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(1.0, 0.0, 0.0));
	uniSetter.setTransformUniforms(modelMatrix, viewMatrix, projectionMatrix, shaderProgram);
	//set material uniforms with two textures
	glUniform3fv(shaderProgram.diffuseLocation, 1, glm::value_ptr(cubeGeometry->diffuse));
	glUniform3fv(shaderProgram.ambientLocation, 1, glm::value_ptr(cubeGeometry->ambient));
	glUniform3fv(shaderProgram.specularLocation, 1, glm::value_ptr(cubeGeometry->specular));
	glUniform1f(shaderProgram.shininessLocation, cubeGeometry->shininess);

	glUniform1i(shaderProgram.useTextureLocation, 1);
	glUniform1i(shaderProgram.texSamplerLocation, 0);

	//glUniform1i(shaderProgram.secTextureLocation, 1);
	glUniform1i(shaderProgram.texSampler2Location, 4);

	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, cubeGeometry->texture);
	glActiveTexture(GL_TEXTURE0 + 4);
	glBindTexture(GL_TEXTURE_2D, grassTexture);

	glBindVertexArray(cubeGeometry->vertexArrayObject);
	glDrawElements(GL_TRIANGLES, cubeGeometry->numTriangles * 3, GL_UNSIGNED_INT, 0);

	// to make sure we have texture only at cube
	glUniform1i(shaderProgram.secTextureLocation, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	return;
}

// draws tower model
void renderObjects::drawHandler::drawTower(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, SCommonShaderProgram& shaderProgram, MeshGeometry** geometry, GameUniformVariables gameUni, glm::vec3 towerPosition) {
	glUseProgram(shaderProgram.program);
	uniSetter.setMaterialUniforms(*geometry, shaderProgram, gameUni);

	glm::mat4 modelMatrix;
	modelMatrix = splineHandler::alignObject(towerPosition, glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0f, 0.0f, 1.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(1.5, 1.5, 1.5));
	uniSetter.setTransformUniforms(modelMatrix, viewMatrix, projectionMatrix, shaderProgram);
	glBindVertexArray((*geometry)->vertexArrayObject);
	glDrawElements(GL_TRIANGLES, (*geometry)->numTriangles * 3, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	return;
}

// draw sphere model
void renderObjects::drawHandler::drawSphere(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, SCommonShaderProgram& shaderProgram, MeshGeometry** geometry, GameUniformVariables gameUni, glm::vec3 spherePosition) {
	glUseProgram(shaderProgram.program);
	uniSetter.setMaterialUniforms( (*geometry), shaderProgram, gameUni);

	glm::mat4 modelMatrix;
	modelMatrix = splineHandler::alignObject(spherePosition, glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0f, 0.0f, 1.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(0.05, 0.05, 0.05));
	uniSetter.setTransformUniforms(modelMatrix, viewMatrix, projectionMatrix, shaderProgram);
	glBindVertexArray((*geometry)->vertexArrayObject);
	glDrawElements(GL_TRIANGLES, (*geometry)->numTriangles * 3, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	return;
}

// draw house model
void renderObjects::drawHandler::drawHouse(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, SCommonShaderProgram& shaderProgram, MeshGeometry** geometry, GameUniformVariables gameUni, glm::vec3 houœePosition) {
	glUseProgram(shaderProgram.program);
	uniSetter.setMaterialUniforms( (*geometry), shaderProgram, gameUni);

	glm::mat4 modelMatrix;
	modelMatrix = splineHandler::alignObject(housePosition, glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0f, 0.0f, 1.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(0.7, 0.7, 0.7));
	modelMatrix = glm::rotate(modelMatrix, 4.7f, glm::vec3(1.0, 0.0, 0.0));
	modelMatrix = glm::rotate(modelMatrix, 4.7f, glm::vec3(0.0, 0.0, 1.0));
	uniSetter.setTransformUniforms(modelMatrix, viewMatrix, projectionMatrix, shaderProgram);
	glBindVertexArray((*geometry)->vertexArrayObject);
	glDrawElements(GL_TRIANGLES, (*geometry)->numTriangles * 3, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	return;
}

// draw duck
void renderObjects::drawHandler::drawDuck(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, SCommonShaderProgram& shaderProgram, GameUniformVariables gameUni, ObjectProp param, glm::vec3 position, glm::vec3 dir) {
	glUseProgram(shaderProgram.program);

	glm::mat4 modelMatrix = splineHandler::alignObject(position,
		dir, glm::vec3(0.0f, 0.0f, 1.0f));

	modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0, 1.0, 1.0) * param.size);
	uniSetter.setTransformUniforms(modelMatrix, viewMatrix, projectionMatrix, shaderProgram);
	for (int i = 0; i < duckGeometry.size(); i++) {

		uniSetter.setMaterialUniforms( duckGeometry[i], shaderProgram, gameUni);
		glBindVertexArray(duckGeometry[i]->vertexArrayObject);
		glDrawElements(GL_TRIANGLES, duckGeometry[i]->numTriangles * 3, GL_UNSIGNED_INT, 0);
	}
	glBindVertexArray(0);
	glUseProgram(0);
}

void renderObjects::drawHandler::drawMaxwell(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, SCommonShaderProgram& shaderProgram, GameUniformVariables gameUni, ObjectProp param, glm::vec3 position, glm::vec3 dir) {
	glUseProgram(shaderProgram.program);

	glm::mat4 modelMatrix = splineHandler::alignObject(position, dir, glm::vec3(0.0f, 0.0f, 1.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0, 1.0, 1.0) * param.size);
	uniSetter.setTransformUniforms(modelMatrix, viewMatrix, projectionMatrix, shaderProgram);
	for (int i = 0; i < maxwellGeometry.size(); i++) {

		uniSetter.setMaterialUniforms(maxwellGeometry[i], shaderProgram, gameUni);
		glBindVertexArray(maxwellGeometry[i]->vertexArrayObject);
		glDrawElements(GL_TRIANGLES, maxwellGeometry[i]->numTriangles * 3, GL_UNSIGNED_INT, 0);
	}
	glBindVertexArray(0);
	glUseProgram(0);
}

// draw explosion method for animation
void renderObjects::drawHandler::drawExplosionMet(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, ExplosionShaderProgram& explosionShader, Explosion* explosion, MeshGeometry** geometry) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(explosionShader.program);

	glm::mat4 matrix = glm::translate(glm::mat4(1.0f), explosion->position);
	matrix = glm::scale(matrix, glm::vec3(1.0) * explosion->size);

	glm::mat4 rotationMatrix = glm::mat4(
		viewMatrix[0],
		viewMatrix[1],
		viewMatrix[2],
		glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
	);
	matrix = matrix * glm::transpose(rotationMatrix);

	glm::mat4 PVMmatrix = projectionMatrix * viewMatrix * matrix;

	glUniform1i(explosionShader.texSamplerLocation, 0);
	glUniform1f(explosionShader.frameDurationLocation, explosion->frameDuration);
	glUniformMatrix4fv(explosionShader.VmatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniformMatrix4fv(explosionShader.PVMmatrixLocation, 1, GL_FALSE, glm::value_ptr(PVMmatrix));
	glUniform1f(explosionShader.timeLocation, explosion->currentTime - explosion->startTime);

	glBindVertexArray((*geometry)->vertexArrayObject);
	glBindTexture(GL_TEXTURE_2D, (*geometry)->texture);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, (*geometry)->numTriangles);
	glBindVertexArray(0);

	glUseProgram(0);
	glDisable(GL_BLEND);
}

// draw explosion
void renderObjects::drawHandler::drawExplosion(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, Explosion* explosion) {
	drawExplosionMet(viewMatrix, projectionMatrix, explosionShader, explosion, &explosionGeometry);
}

// draw all models and animations
void renderObjects::drawHandler::drawEverything(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, bool drawWaterBool, std::map<std::string, ObjectProp> loadProps) {
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	//glStencilFunc(GL_ALWAYS, 0, -1);
	drawSkybox(viewMatrix, projectionMatrix, skyboxShader, &skyboxGeometry, gameUniVars);
	drawTower(viewMatrix, projectionMatrix, shaderProgram, &towerGeometry, gameUniVars, towerPosition);
	drawCube(viewMatrix, projectionMatrix, shaderProgram, &cubeGeometry, gameUniVars, cubePosition, 8.0f);
	drawCube(viewMatrix, projectionMatrix, shaderProgram, &cubeGeometry, gameUniVars, cube2Position, 3.0f);
	drawCube(viewMatrix, projectionMatrix, shaderProgram, &cubeGeometry, gameUniVars, cube3Position, 11.0f);
	//glStencilFunc(GL_ALWAYS, 3, -1);
	//drawObject(viewMatrix, projectionMatrix, shaderProgram, &maxwellGeometry, gameUniVars, loadProps["maxwell"]);
	//glDisable(GL_STENCIL_TEST);
	drawObject(viewMatrix, projectionMatrix, shaderProgram, &duckGeometry, gameUniVars, loadProps["duck2"]);
	drawObject(viewMatrix, projectionMatrix, shaderProgram, &duckGeometry, gameUniVars, loadProps["duck3"]);
	drawObject(viewMatrix, projectionMatrix, shaderProgram, &balloonGeometry, gameUniVars, loadProps["balloon"]);
	drawObject(viewMatrix, projectionMatrix, shaderProgram, &boatGeometry, gameUniVars, loadProps["boat"]);
	drawHouse(viewMatrix, projectionMatrix, shaderProgram, &houseGeometry, gameUniVars, housePosition);
	glStencilFunc(GL_ALWAYS, 1, -1);
	drawSphere(viewMatrix, projectionMatrix, shaderProgram, &sphereGeometry, gameUniVars, spherePosition);
	glDisable(GL_STENCIL_TEST);

	if (drawWaterBool) {
		drawWater(viewMatrix, projectionMatrix, waterShader, &waterGeometry, gameUniVars);
	}

	glDisable(GL_STENCIL_TEST);
}

//--------------------------------------------------------------------------------CLEANING--------------------------------------------------------
// clean shaders
void renderObjects::cleanupShaderPrograms(void) {

	pgr::deleteProgramAndShaders(shaderProgram.program);
	pgr::deleteProgramAndShaders(skyboxShader.program);
	pgr::deleteProgramAndShaders(waterShader.program);
	pgr::deleteProgramAndShaders(bannerShaderProgram.program);
	pgr::deleteProgramAndShaders(explosionShader.program);
	pgr::deleteProgramAndShaders(barShaderProgram.program);

}

// clean geometry
void cleanupGeometry(MeshGeometry* geometry) {

	glDeleteVertexArrays(1, &(geometry->vertexArrayObject));
	glDeleteBuffers(1, &(geometry->elementBufferObject));
	glDeleteBuffers(1, &(geometry->vertexBufferObject));


	if (geometry->texture != 0)
		glDeleteTextures(1, &(geometry->texture));
}

// clean model's geometry
void renderObjects::cleanupModels() {
	cleanupGeometry(towerGeometry);
	cleanupGeometry(skyboxGeometry);
	cleanupGeometry(waterGeometry);
	cleanupGeometry(houseGeometry);
	cleanupGeometry(cubeGeometry);
	cleanupGeometry(sphereGeometry);
	cleanupGeometry(barGeometry);
	std::vector<MeshGeometry*> v = maxwellGeometry;
	for (std::vector<MeshGeometry*>::iterator it = v.begin(); it != v.end(); ++it) {
		cleanupGeometry(*it);
	}
	v = duckGeometry;
	for (std::vector<MeshGeometry*>::iterator it = v.begin(); it != v.end(); ++it) {
		cleanupGeometry(*it);
	}
	v = balloonGeometry;
	for (std::vector<MeshGeometry*>::iterator it = v.begin(); it != v.end(); ++it) {
		cleanupGeometry(*it);
	}
	v = boatGeometry;
	for (std::vector<MeshGeometry*>::iterator it = v.begin(); it != v.end(); ++it) {
		cleanupGeometry(*it);
	}
}