#include "renderer.h"
#include <cstdio>
#include <cmath>
#include <fstream>

using namespace std;


// This is used to load a shader from file on the disk (you could write a shader and have it plaintext,
//	or better yet, include it to cheat and make it part of the source code, but I'm not gonna do
//	either of those things because having the shader source outside the app is more useful to the
//	user of the engine
char* loadTextFromFile(const char* = NULL);

Renderer::Renderer(const unsigned short &width, const unsigned short &height, const unsigned short &framerate, const unsigned short &fov, const float &afilter, const float &aalias, const bool &fullscreen)
{
	screen = NULL;					// Initialize the screen pointer, no dingleberries
	Init(width, height, framerate, fov, afilter, aalias, fullscreen);		// Init the renderer, and keep track of its state
}

bool Renderer::Init(const unsigned short &width, const unsigned short &height, const unsigned short &framerate, const unsigned short &fov, const float &afilter, const float &aalias, const bool &fullscreen)
{
	// Store the given values for width, height, framerate, field of view, anisotrophic filtering, antialiasing, and fullscreen
	lastFrame = 0;
	r_width = width;
	r_height = height;
	r_framerate = framerate;
	r_fov = fov;
	cv_af = afilter;
	cv_aa = aalias;
	cv_fullscreen = fullscreen;
	// We haven't yet renderered the screen
	sv_lastFrameUpdate = 0;
	// Init the SDL video subsystem
	int i_error = SDL_Init(SDL_INIT_VIDEO);
	// if the video subsystem failed to init, init has failed
	if(i_error == -1)
	{
		printf("SDL_Init Failed: %s\n", SDL_GetError());
		SDL_Quit();
		OKAY = false;
		return false;
	}

	// Ask the video card what's going on
	const SDL_VideoInfo* vidcard = SDL_GetVideoInfo();

	// If the given width was 0, ask the card what width it recommends
	if(r_width == 0)
		r_width = vidcard->current_w;
	// If the given height was 0, ask the card what height it recommends
	if(r_height == 0)
		r_height = vidcard->current_h;

	// create a video screen, fullscreen only if it has been decided
	if(cv_fullscreen)
		screen = SDL_SetVideoMode(r_width, r_height, 32, SDL_HWSURFACE | SDL_FULLSCREEN | SDL_OPENGL | SDL_DOUBLEBUF);
	else
		screen = SDL_SetVideoMode(r_width, r_height, 32, SDL_HWSURFACE | SDL_OPENGL | SDL_DOUBLEBUF);

	// If the screen could not be created, then init has failed
	if(screen == NULL)
	{
		OKAY = false;
		return false;
	}
	// Don't draw the cursor in this window
	SDL_ShowCursor(SDL_DISABLE);
	// Everything has init just fine
	glShadeModel(GL_SMOOTH);	// Use a smooth shading model when not using shaders to render (not often)

//	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);				// Black
	glClearColor(0.390625f, 0.58203125f, 0.92578125f, 1.0f);	// Cornflower Blue

	glEnable(GL_DEPTH_TEST);					// Enable the depth test
	glEnable(GL_TEXTURE_2D);					// Enable texturing
	glDepthFunc(GL_LEQUAL);						// Use linear depth testing
	glEnableClientState(GL_VERTEX_ARRAY);				// Enable support for vertex arrays
	glEnableClientState(GL_NORMAL_ARRAY);				// Enable support for normal arrays
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);			// Enable support for texture coordinate arrays

	OKAY = prepareScreen();						// prepare the screen, store the output into the OKAY variable

	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);			// Ensure that we have control over the buffers for buffer swaps (double buffering)
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_anisotropy);// Check the max supported level of anisotrophic filtering
	if(max_anisotropy < cv_af)					// if the current level is higher than the max
		cv_af = max_anisotropy;					// cut it back

	// Load Shaders
	loadShader(PHONG_SHADER, "phong");				// Loads the phong shader from phong.vert and phong.frag
	loadShader(TOON_SHADER, "toon");				// Loads the toon shader from toon.vert and toon.frag
	loadShader(GOURAUD_SHADER, "gouraud");				// Loads the gouraud shader from gouraud.vert and gouraud.frag
	loadShader(GOURAUD_TOON_SHADER, "gouraud-toon");		// Loads the gouraud toon shader from gouraudToon.vert and gouraudToon.frag

	return OKAY;							// We're good
}

// This prepares the screen for drawing, and must be called anytime the video settings change
bool Renderer::prepareScreen()
{
	glViewport(0, 0, r_width, r_height);					// Make sure the game knows how big to make the viewport
	glMatrixMode(GL_PROJECTION);						// And use the projection matrix
	glLoadIdentity();							// Load the identity matrix into the projection matrix
	gluPerspective(r_fov/2, float(r_width)/float(r_height), 1.0f, 1000.0f);	// Set up the perspective matrix (changes with FOV)
	glMatrixMode(GL_MODELVIEW);						// Go back to the modelview matrix
	glLoadIdentity();							// and load the identity again
	return true;								// I have no way of knowing if this failed or not
}

// This call uses all the geometry passed to the renderer since the last time it was called and draws it all to the screen (then gets rid of it)
bool Renderer::DrawScene()
{
	// Return false for now because it doesn't do anything yet
	glClear(GL_DEPTH_BUFFER_BIT);	// Clear the depth buffer
	glClear(GL_COLOR_BUFFER_BIT);	// And color buffer

	shaderLightCount = glGetUniformLocation(phongShader, "numLights");
	glUniform1i(shaderLightCount, lights.size());

	if(lights.size() > 0)
	{
		GLfloat lightDirs[lights.size() * 3];

		for(unsigned int i = 0; i < lights.size(); ++i)
		{
			lightDirs[3*i] = lights[i].position.x;
			lightDirs[3*i+1] = lights[i].position.y;
			lightDirs[3*i+2] = lights[i].position.z;
		}

		shaderLightPointer = glGetUniformLocation(phongShader, "lightDir");
		glUniform3fv(shaderLightPointer, lights.size(), lightDirs);
	}
	else
	{
		glUniform1i(shaderLightCount, 0);
	}

	TrisToVBOs();			// Draw the triangles (this doesn't work correctly yet)
	TS.clear();			// and get rid of them

	// Oh dearest me, I've forgotten my vertex arrays

	for(unsigned int i = 0; i < VBOS.size(); ++i)	// then draw the VBOS
	{
		glBindBuffer(GL_ARRAY_BUFFER, VBOS[i].handle);		// Bind the VBO
		glInterleavedArrays(GL_T2F_N3F_V3F, 0, NULL);		// Set the interleaved array mode (might change later to include colors)
		glActiveTextureARB(GL_TEXTURE0_ARB);
		glBindTexture(GL_TEXTURE_2D, VBOS[i].texture);
		glDrawArrays(GL_TRIANGLES, 0, VBOS[i].numVerts);	// And Draw!
	}

	VBOS.clear();		// forget the VBOs. They're not leaving graphics memory, we just don't know in advance if we need to draw them next frame

	// I was gonna add in a simple font based FPS ticker, but it got complicated

//	usleep((1000000 / r_framerate) - (SDL_GetTicks() - lastFrame));							// elapse the time between buffer swaps
//	framerate += 1000.0f/(SDL_GetTicks() - lastFrame);
//	framerate /= 2.0f;
	lastFrame = SDL_GetTicks();
	SDL_GL_SwapBuffers();								// swap buffers
	glFinish();									// and flush to the screen

	return true;
}

bool Renderer::isOkay()				// Quick access to renderer attribues
{	return OKAY;	}

unsigned short Renderer::getWidth()		//
{	return r_width;	}

unsigned short Renderer::getHeight()		//
{	return r_height;	}

unsigned short Renderer::getFramerate()		//
{	return r_framerate;	}

unsigned short Renderer::getFOV()		//
{	return r_fov;	}

bool Renderer::push_VBO(const VBObject& vbo)
{
	VBOS.push_back(vbo);			// It's ready to be drawn, this is not intended for use with scene objects as it doesn't depend on the camera or world space
	return true;				// At some point, there will be error checking to prevent using too much video ram
}

bool Renderer::push_VA(const vertexArray& va)
{
	return true;				// Doesn't work yet
}

bool Renderer::push_Triangle(const Triangle& t)
{
	return true;
}

// This reaches the destination, but it takes the wrong(est) path to get there
void Renderer::TrisToVBOs()
{
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBegin(GL_TRIANGLES);
	for(size_t i = 0; i < TS.size(); ++i)
	{
		for(int j = 0; j < 3; ++j)
		{
			glNormal3f(TS[i].verts[j].normal.x, TS[i].verts[j].normal.y, TS[i].verts[j].normal.z);
			glVertex3f(TS[i].verts[j].position.x, TS[i].verts[j].position.y, TS[i].verts[j].position.z);
		}
	}
	glEnd();

	//TODO: all of it
}

GLuint Renderer::loadTexture(const char* filename, const GLenum& filter)
{
	// go ahead and declare/initialize the basics
	GLuint texture = 0;
	SDL_Surface *surface = NULL;

	// Load the image if a filename was provided
	if(filename != NULL)
		surface = IMG_Load(filename);
	// If loading the image failed, we're done here
	if(surface == NULL)
		return texture;

	// Create a texture buffer to store it in when we're done
	glGenTextures(1, &texture);

	// If creating a buffer failed, we're done here
	if(texture == 0)
		return texture;

	// Now we begin turning this surface into a texture
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);			// Not sure why I use this
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);			// Use the specified filtering technique
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);			// Always wrap
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);			// Always wrap
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, cv_af);		// use the specified level of anisotropic filtering

	switch((surface->format->BitsPerPixel)%4)
	{
		case 3:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, surface->w, surface->h, 0, GL_RGB, GL_UNSIGNED_BYTE, surface->pixels);
			break;
		}
		case 0:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
			break;
		}
	}

	glGenerateMipmap(GL_TEXTURE_2D);

	textures.push_back(texture);
	SDL_FreeSurface(surface);
	return texture;
}

void Renderer::deleteTexture(const GLuint* handle)
{
	glDeleteBuffers(1, handle);
}

void matrixStack::rotatef(const float& x, const float& y, const float& z, const float& w)
{
	//TODO: all of it
	// Combining rotations is done by multiplying them, however, you multiply the rotation matrices, not the transformation matrices, so we may need to keep translations separate
	localTransform = rotmat(x, y, z, w) * localTransform;
}

void matrixStack::translatef(const float& x, const float& y, const float& z)
{
	// To translate, you just add the new displacement vector to the existing transform matrix
	localTransform = mat4(	localTransform.data[0], localTransform.data[4],	localTransform.data[8],	localTransform.data[12] + x,
				localTransform.data[1], localTransform.data[5], localTransform.data[9], localTransform.data[13] + y,
				localTransform.data[2], localTransform.data[6], localTransform.data[10], localTransform.data[14] + z,
				localTransform.data[3], localTransform.data[7], localTransform.data[11], localTransform.data[15]	);
}

void matrixStack::rotatefv(const vector3& v, const float& w)
{
	//TODO: all of it
	// Combining rotations is done by multiplying them, however, you multiply the rotation matrices, not the transformation matrices, so we may need to keep translations separate
	localTransform = rotmat(v.x, v.y, v.z, w) * localTransform;
}

void matrixStack::translatefv(const vector3& v)
{
	// To translate, you just add the new displacement vector to the existing transform matrix
	localTransform = mat4(	localTransform.data[0], localTransform.data[4],	localTransform.data[8],	localTransform.data[12] + v.x,
				localTransform.data[1], localTransform.data[5], localTransform.data[9], localTransform.data[13] + v.y,
				localTransform.data[2], localTransform.data[6], localTransform.data[10], localTransform.data[14] + v.z,
				localTransform.data[3], localTransform.data[7], localTransform.data[11], localTransform.data[15]	);
}

int Renderer::add_light(const vector3& position, const vector3& direction, const vector3& color, const float& range, const lightType& type)
{
	light newLight;
	newLight.position = position;
	newLight.direction = direction;
	newLight.range = range;
	newLight.color = color;
	newLight.type = type;
	lights.push_back(newLight);
	return 0;
}

Renderer::~Renderer()		// Close everything and clean up
{
	VBOS.clear();		// Clear
	VAS.clear();		// All
	TS.clear();		// Dynamically
	lights.clear();		// Allocated Memory
	glDeleteObjectARB(phongShader);	// delete shaders
	glDeleteObjectARB(toonShader);	//
	for(unsigned int i = 0; i < textures.size(); ++i)	// gotta delete the textures before we get rid of their references
		glDeleteBuffers(1, &textures[i]);
	textures.clear();
	SDL_FreeSurface(screen);// And return it to the system
	SDL_Quit();
}

Renderer& operator <<(Renderer& R, const VBObject& vbo)
{
	R.push_VBO(vbo);	// Call the native function
	return R;
}

Renderer& operator <<(Renderer& R, const vertexArray& va)
{
	R.push_VA(va);		// Call the native function
	return R;
}

Renderer& operator <<(Renderer& R, const Triangle& t)
{
	R.push_Triangle(t);	// Call the native function
	return R;
}

void matrixStack::pushMatrix()
{
	data.push_back(localTransform);	// push onto the matrix stack the last thing given
	T = T*localTransform;		// The local transform is being pushed onto the stack, so it's going into the totalTransform matrix too
	localTransform = mat4::identity;
}

// Returns the total transform matrix
mat4 matrixStack::totalTransform()
{
	return T * localTransform;	// I've given up on trying to keep the local transform matrix in the total transform matrix
}

// This takes matrices off the stack
void matrixStack::popMatrix()
{
	if(data.size() > 0)		// if the stack is not empty
	{
		localTransform = data.back();	// temporarily store the matrix we're removing
		data.pop_back();	// delete it from the stack
	}
	T = mat4::identity;		// Gotta fully rebuild the total transform matrix now
	for(unsigned int i = 0; i > data.size(); ++i)
		T = T*data[i];		// Gotta get 'em All
}

// this clears the entire matrix stack (the equivalent of deleting it, and buying a new one)
void matrixStack::clear()
{
	data.clear();				// Clear the stack
	localTransform = mat4::identity;	// set the local transform to identity
	T = mat4::identity;			// set the total transform to identity
}

// This function generates a rotation matrix from an axis angle rotation
mat4 rotmat(const float& X, const float& Y, const float& Z, const float& w)
{
	float d = sqrt(X*X + Y*Y + Z*Z);
	float x = X/d, y = Y/d, z = Z/d;
	float c = cos(w), s = sin(w), C = 1-c;

	return mat4(	x*x*C+c,	x*y*C-z*s,	x*z*C+y*s, 0.0f,
			x*y*C+z*s,	y*y*C+c,	y*z*C-x*s, 0.0f,
			z*x*C-y*s,	z*y*C+x*s,	z*z*C+c,   0.0f,
			0.0f,		0.0f,		0.0f,	   1.0f	);
}

// constructor for the matrix stack
matrixStack::matrixStack()
{
	localTransform = mat4::identity;	// We need to define the local transform matrix as identity so we can multiply things by it without ill-effects
	T = mat4::identity;			// We need to define the total transform matrix as identity so we can multiply things by it without ill-effects
}

// destructor for the matrix stack
matrixStack::~matrixStack()
{
	clear();				// We need to clear the matrix stack when we're done with it
}

// This loads text from a file, I plan to use this for loading shaders
char* loadTextFromFile(const char* filename)
{
	char *out = NULL;			// Be prepared to return null if no filename was provided or a bad filename was provided
	if(filename == NULL)			// if no filename
		return out;			// return
	ifstream file(filename, ios::in);	// open the file
	if(!file.is_open())			// if bad filename
		return out;			// return
	file.seekg(0, ios::end);		// go to the end
	unsigned int count = file.tellg();	// record the cursor position so we know how many characters are in this bitch
	file.seekg(0, ios::beg);		// go back to the beginning

	out = new char[count + 1];		// allocate space for all these characters

	for(unsigned int i = 0; i < count; ++i)	// step through the file
		out[i] = file.get();		// and get them all in the array

	out[count] = '\0';			// end with null
	file.close();				// close the file

	return out;				// and return the array pointer
}

void Renderer::loadShader(const shaderType &type, const char *shadername)
{
        if(shadername == NULL)                  // if no filename given
		return;
        ifstream file((string("shaders/") + string(shadername) + string(".vert")).c_str(), ios::in);    // try to open the vertex shader
        if(!file.is_open())                     // if the file doesn't exist
		return;
        file.close();
        file.open((string("shaders/") + string(shadername) + string(".frag")).c_str(), ios::in);        // try to open the fragment shader
        if(!file.is_open())                     // if the file doesn't exist
                return;                       // cant load the shader
        file.close();
        char *vs, *fs;
        GLhandleARB vertBMap, fragBMap, shader;

        shader = glCreateProgramObjectARB();
        vertBMap = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
        fragBMap = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

        vs = loadTextFromFile((string("shaders/") + string(shadername) + string(".vert")).c_str()); // load the file
        fs = loadTextFromFile((string("shaders/") + string(shadername) + string(".frag")).c_str()); // load the file
        const char* vBMap = vs;
        const char* fBMap = fs;

        glShaderSourceARB(vertBMap, 1, &vBMap, NULL);
        glShaderSourceARB(fragBMap, 1, &fBMap, NULL);

        free(vs); free(fs);

        char *infoLog = NULL;
        GLint length = 0;

        glCompileShaderARB(fragBMap);
        glCompileShaderARB(vertBMap);

        glGetShaderiv(fragBMap, GL_INFO_LOG_LENGTH, &length);
        infoLog = new char[length + 1];
        glGetShaderInfoLog(fragBMap, length, &length, infoLog);
        infoLog[length] = '\0';
	printf("%s, frag-compilelog: %s\n", shadername, infoLog);
        delete [] infoLog;

        length = 0;
        glGetShaderiv(vertBMap, GL_INFO_LOG_LENGTH, &length);
        infoLog = new char[length + 1];
        glGetShaderInfoLog(vertBMap, length, &length, infoLog);
        infoLog[length] = '\0';
	printf("%s, vert-compilelog: %s\n", shadername, infoLog);
        delete [] infoLog;

        glAttachObjectARB(shader, vertBMap);
        glAttachObjectARB(shader, fragBMap);

        glLinkProgramARB(shader);

	switch(type)
	{
		case TOON_SHADER:
		{
			toonShader = shader;
			break;
		}
		case PHONG_SHADER:
		{
			phongShader = shader;
			break;
		}
		case GOURAUD_SHADER:
		{
			gouraudShader = shader;
			break;
		}
		case GOURAUD_TOON_SHADER:
		{
			gouraudToonShader = shader;
			break;
		}
		default:
		{
			break;
		}
	}
}

void Renderer::switchToPhong()
{
	glUseProgramObjectARB(phongShader);		// Renderer with the Phong shader
	shaderLightCount = glGetUniformLocationARB(phongShader, "numLights");
	shaderLightPointer = glGetUniformLocationARB(phongShader, "lightDir");
	shaderDiffuseMap = glGetUniformLocationARB(phongShader, "diffuseMap");
}

void Renderer::switchToToon()
{
	glUseProgramObjectARB(toonShader);		// Renderer with the Toon shader
	shaderLightCount = glGetUniformLocationARB(toonShader, "numLights");
	shaderLightPointer = glGetUniformLocation(toonShader, "lightDir");
	shaderDiffuseMap = glGetUniformLocationARB(toonShader, "diffuseMap");
}

void Renderer::switchToGouraud()
{
	glUseProgramObjectARB(gouraudShader);		// Renderer with the Gouraud shader
	shaderLightCount = glGetUniformLocationARB(gouraudShader, "numLights");
	shaderLightPointer = glGetUniformLocation(gouraudShader, "lightDir");
	shaderDiffuseMap = glGetUniformLocationARB(gouraudShader, "diffuseMap");
}

void Renderer::switchToGouraudToon()
{
	glUseProgramObjectARB(gouraudToonShader);	// Renderer with the Gouraud Toon shader
	shaderLightCount = glGetUniformLocationARB(gouraudToonShader, "numLights");
	shaderLightPointer = glGetUniformLocation(gouraudToonShader, "lightDir");
	shaderDiffuseMap = glGetUniformLocationARB(gouraudToonShader, "diffuseMap");
}
