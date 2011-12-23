#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include "matrices.h"
#include <string>
#include <vector>
#include <map>
#include <set>

//#define error printf("glError: %s\n", gluErrorString(glGetError()))

enum lightType
{
	POINT_LIGHT,					// A light that illuminates in all directions: has a position and a color
	DIRECTIONAL_LIGHT,				// A light that illuminates in an infinite number of directions within a confined range of its direction: hs a position, color, direction, and range
	LASER_LIGHT					// A special directional light with a VERY small range
};

enum shaderType
{
	PHONG_SHADER,					// A per-pixel shader for normal realistic lighting
	TOON_SHADER,					// A per-pixel shader for cartoon style cel lighting
	GOURAUD_SHADER,					// A per vertex shader for faster lighting
	GOURAUD_TOON_SHADER 				// A per vertex shader for faster cartoon style cel lighting
};

enum textureLevel
{
	DIFFUSE_MAP,					// A diffuse map tells you the base color of the texture at that point (RGB)
	NORMAL_MAP,					// A normal map tells you the normal of the surface that light is reflected off of at that point, this is used for more advanced lighting (RGB)
	HEIGHT_MAP,					// A height map will not be used in this engine right now (Grayscale or RGB)
	SPECULAR_MAP,					// A specular map will tell you the shininess of the material at that point (Grayscale or RGB)
};

struct light
{
	vector3 color;					// Stores the color and luminosity of the light (color is the direction, luminosity is the magnitude
	vector3 position;				// Stores the position of the light;
	vector3 direction;				// Stores the direction the light points in: not used for point lights
	float range;					// Stores the range of the light (the maximum angle from the direction the light points in that light can travel from this source): not used in point lights
	lightType type;					// Stored an enumerator for the type of light
};

class matrixStack					// This defines the matrixStack class
{
private:
	std::vector<mat4> data;				// Transformations in a scene graph are stored in a tree, however, the set of transformations
							//	to be applied to a single model form a stack starting at the root node and extending
							//	all the way to the child node where the model is stored. This saves memory during rendering
							//	because the transformations that are irrelevant aren't stored here.
	mat4 localTransform;				// This is the transform matrix that gets manipulated when you call the rotate and translate functions, it is for all itents and purposes the top of the stack
	mat4 T;						// This matrix stores all the matrices in the stack combined
public:
	matrixStack();
	~matrixStack();
	void pushMatrix();			// You can push a new matrix onto the matrix stack
	void popMatrix();				// or pop off the matrix on the top of the stack
	mat4 totalTransform();				// returns the combined transformation matrix held within the stack
	void rotatef(const float& = 1.0f, const float& = 0.0f, const float& = 0.0f, const float& = 0.0f);
	void translatef(const float& = 0.0f, const float& = 0.0f, const float& = 0.0f);
	void rotatefv(const vector3& = vector3(1.0f, 0.0f, 0.0f), const float& = 0.0f);
	void translatefv(const vector3& = vector3(0.0f, 0.0f, 0.0f));
	void clear();					// clears the data vector and pushes the identity matrix onto the stack
};

struct VBObject						// A vertex buffer object
{
	GLuint handle;					// Has a handle
	GLuint texture;
	unsigned int numVerts;				// and a number of vertices
	shaderType shader;				// and the specifier for which shader to draw this with
};

struct vertex						// A single vertex can be used in a trangle or a line, or by itself as a single point
{
	vector3 position, normal;			// A vertex consists of a single position vector, a single normal vector,
	vector2 texCoords;				// and a single set of texture coordinates
};

struct Triangle						// A single generic triangle. Not part of a Vertex Buffer Object directly
{
	vertex verts[3];				// three vertices
	GLuint texid;					// one texture id
	shaderType shader;				// and one shader
	vector4 color;					// and one color modifier
};

struct vertexArray					// Stores a single vertex array. These can be rendered directly, but are faster rendered from gfx memory with VBOs
{
	GLuint texture;					// Stores the texture handle
	float *data;					// Stores the vertex data
	short numVerts, numTris;			// Stores the number of vertices and triangles in the vertex array
	short *indices;					// Stores the indices of the vertices in the order they are to be rendered
	shaderType shader;				// Stores the shader used to render this object
};

mat4 rotmat(const float& = 0.0f, const float& = 0.0f, const float& = 0.0f, const float& = 0.0f);

class Renderer						// The main class of this library. This class is used to handle all rendering in the application
{
private:
	bool cv_fullscreen, cv_vsync;							// Stores the flags for fullscreen rendering and vertical sync
	unsigned short r_width, r_height, r_framerate, r_fov;				// Stores the resolution, framerate, and field of view
	float cv_af, cv_aa;								// Stores the current levels of anisotrophic filtering and antialiasing
	unsigned int sv_lastFrameUpdate;						// Stores the time of the last frame refresh
	SDL_Surface *screen;								// Stores a pointer to the screen surface
	std::vector<VBObject> VBOS;							// Stores all of the VBO handles to be rendered to the screen
	std::vector<vertexArray> VAS;							// Stores all of the Vertex Arrays before they get put into vertex buffer objects
	std::vector<Triangle> TS;							// Stores individual triangles passed to the renderer
	std::vector<GLuint> textures;							// Stores all the textures currently in use by the program, this is where they get deleted from
	bool OKAY;									// Maintains whether the renderer is in good working order
	void TrisToVBOs();								// Stores all individual triangles into one VBO per texture
	std::vector<light> lights;							// Stores all lights in the current scene
	GLhandleARB toonShader, phongShader, gouraudShader, gouraudToonShader;		// Stores the handles for the shaders
	void loadShader(const shaderType& type, const char* shadername);		// Loads the shader associated with it, and stores it in the space for said shader
	GLint shaderLightCount, shaderLightPointer, shaderDiffuseMap;			// Stores the pointer for the shader light array
	float max_anisotropy;								// Stores the maximum level of anistropic filtering supported by the hardware
	int lastFrame;									// Stores the time of the last frame
	float framerate;								// stores the framerate
public:
	// Constructor
	Renderer(	const unsigned short &width = 640,
			const unsigned short &height = 480,
			const unsigned short &framerate = 125,
			const unsigned short &fov = 90,
			const float &afilter = 1.0f,
			const float &aalias = 1.0f,
			const bool &fullscreen = true);
	// Destructor
	~Renderer();
	// Init() initializes the screen using the passed values of width, height, framerate, and field of view; Returns a bool to tell the program if it succeeded or not
	bool Init(	const unsigned short &width = 640,
			const unsigned short &height = 480,
			const unsigned short &framerate = 125,
			const unsigned short &fov = 90,
			const float &afilter = 1.0f,
			const float &aalias = 1.0f,
			const bool &fullscreen = true);
	// prepareScreen() prepares the screen for rendering. Use this whenever you change the field of view. This does not clear the screen
	bool prepareScreen();
	// DrawScene() draws the current scene to the screen and clears all pending vertex arrays, storing them in video memory
	bool DrawScene();
	// isOkay() returns true if the renderer is in good working order, false otherwise
	bool isOkay();
	// getWidth() returns the width of the screen
	unsigned short getWidth();
	// getHeight() returns the height of the screen
	unsigned short getHeight();
	// getFramerate() returns the current max framerate of the screen (any number between 30 and 125)
	unsigned short getFramerate();
	// getFOV() returns the current FOV (any number between 60 and 150)
	unsigned short getFOV();
	// push_VBO puts a vertex buffer object directly onto the render stack
	bool push_VBO(const VBObject&);
	// push_VA puts a vertex array directly onto the render stack
	bool push_VA(const vertexArray&);
	// push_Triangle puts a triangle directly onto the render stack
	bool push_Triangle(const Triangle&);
	// add Light, returns the index of the light in the vector
	int add_light(const vector3& = vector3(0.0f, 0.0f, 0.0f), const vector3& = vector3(0.0f, -1.0f, 0.0f), const vector3& = vector3(1.0f, 1.0f, 1.0f), const float& = 1.0f, const lightType& = POINT_LIGHT);
	// changeLight, changes the attributes of a light already in the renderer, cannot change the type
	int change_light(const vector3& = vector3(0.0f, 0.0f, 0.0f), const vector3& = vector3(0.0f, 0.0f, 0.0f), const float& = 0.0f);
	// loadTexture loads the texture specified and returns the handle
	GLuint loadTexture(const char* filename, const GLenum& = GL_LINEAR_MIPMAP_LINEAR);
	// deleteTexture deletes a texture when the user calls it. It deletes only the texture specified by the user
	void deleteTexture(const GLuint* handle);
	// switchToPhong switches the renderer to Phong (per pixel) shader
	void switchToPhong();
	// switchToToon switches the renderer to the Toon shader
	void switchToToon();
	// switchToGouraud switches the renderer to the Gouraud (per vertex) shader
	void switchToGouraud();
	// switchToGouraudToon switches the renderer to the Gouraud (per vertex) Toon shader
	void switchToGouraudToon();
};

Renderer &operator <<(Renderer&, const VBObject&);
Renderer &operator <<(Renderer&, const vertexArray&);
Renderer &operator <<(Renderer&, const Triangle&);
