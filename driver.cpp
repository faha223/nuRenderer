#include "renderer.h"

const unsigned short width = 0;		// Set the screen width (if 0, it uses the current screen resolution)
const unsigned short height = 0;	// Set the screen height (if 0, it uses the current screen height)
const unsigned short framerate = 60;	// Set the framerate
const unsigned short fov = 90;		// Set the field of view
const unsigned short demo = 10;		// Set the demo run time
const float Afilter = 16.0f;		// Set the level of anisotropic filtering
const float Aalias = 4.0f;		// Not yet implemented
const bool fullscreen = true;		// Set whether this should run in fullscreen or not

int main(int argc, char **argv)
{
	Renderer renderer(width, height, framerate, fov, Afilter, Aalias, fullscreen);
	// If the renderer was not created properly, OH SHIT
	if(!renderer.isOkay())
	{
		printf("Renderer did not initialize\n");	// Let the user know
		return 0;					// SEPPUKU
	}

	// otherwise...
        printf("GL Version: %s, The Way it's Meant to be Played\n", glGetString(GL_VERSION));

	#include "geometry.h"	// This crap is in another file to save space in this one

	float vertexA[arraySize];

	matrixStack transforms;

	VBObject box;				// Make a new VBO
	box.handle = 0;				// Default value for its handle
	box.numVerts = arraySize/8;		// Calculate the number of vertices it shall have to render
	glGenBuffersARB(1, &box.handle);	// and give it a VBO handle

	if(box.handle == 0)			// if the handle failed
	{
		printf("COLOSSAL FAILURE\n");	// Let the user know
		return 0;			// SEPPUKU
	}

	box.texture = renderer.loadTexture("box.png", GL_LINEAR_MIPMAP_LINEAR);	// Load the texture for the box and store its handle in the VBO
//	box.texture = renderer.loadTexture("16.png", GL_NEAREST);		// Load the texture for the box and store its handle in the VBO

	glBindBuffer(GL_ARRAY_BUFFER, box.handle);				// Bind the VBO
	glBufferData(GL_ARRAY_BUFFER, 288*sizeof(float), NULL, GL_DYNAMIC_DRAW);// and allocate its space

//	renderer.switchToToon();		// Switch to the toon shader (to test it)
//	renderer.switchToPhong();		// Switch to the phong shader
	renderer.switchToGouraud();
//	renderer.switchToGouraudToon();

	// Add a light to the scene
	renderer.add_light(vector3(0.0f, 20.0f, 0.0f), vector3(0.0f, -1.0f, 0.0f), vector3(1.0f, 1.0f, 1.0f), 1.0f, POINT_LIGHT);
//	renderer.add_light(vector3(0.0f, -20.0f, 0.0f), vector3(0.0f, 1.0f, 0.0f), vector3(1.0f, 1.0f, 1.0f), 1.0f, POINT_LIGHT);

	// make some rotation storing variables
	float x, y;
	// Draw some frames in realtime
	for(int i = 0; i < framerate*demo; ++i)				// n frames = framerate * demo_time
	{
		transforms.clear();
		x = 0.9f*float(i)/float(framerate);
		y = x;
		if(x >= 6.28f)
			x -= 6.28f;
		if(y >= 6.28f)
			y -= 6.28f;

		transforms.translatef(0.0f, 0.0f, -10.0f);
		transforms.pushMatrix();
		transforms.rotatef(0.0f, 1.0f, 0.0f, x);
		transforms.rotatef(1.0f, 0.0f, 0.0f, y);
		mat4 totalTransform = transforms.totalTransform();
		mat3 totalRotation = totalTransform.rotmat();
		for(unsigned int t = 0; t < 12; ++t)
		{
			// Transfer texture coordinates directly
			vertexA[24*t] = tri[t].verts[0].texCoords.x;
			vertexA[24*t+1] = tri[t].verts[0].texCoords.y;

			// rotate but do not translate normals
			vertexA[24*t+2] = (totalRotation*tri[t].verts[0].normal).x;
			vertexA[24*t+3] = (totalRotation*tri[t].verts[0].normal).y;
			vertexA[24*t+4] = (totalRotation*tri[t].verts[0].normal).z;

			// completely transform positions
			vertexA[24*t+5] = (totalTransform*vector4(tri[t].verts[0].position.x, tri[t].verts[0].position.y, tri[t].verts[0].position.z, 1.0f)).x;
			vertexA[24*t+6] = (totalTransform*vector4(tri[t].verts[0].position.x, tri[t].verts[0].position.y, tri[t].verts[0].position.z, 1.0f)).y;
			vertexA[24*t+7] = (totalTransform*vector4(tri[t].verts[0].position.x, tri[t].verts[0].position.y, tri[t].verts[0].position.z, 1.0f)).z;

			// Transfer texture coordinates directly
			vertexA[24*t+8] = tri[t].verts[1].texCoords.x;
			vertexA[24*t+9] = tri[t].verts[1].texCoords.y;

			// rotate but do not translate normals
			vertexA[24*t+10] = (totalRotation*tri[t].verts[1].normal).x;
			vertexA[24*t+11] = (totalRotation*tri[t].verts[1].normal).y;
			vertexA[24*t+12] = (totalRotation*tri[t].verts[1].normal).z;

			// completely transform positions
			vertexA[24*t+13] = (totalTransform*vector4(tri[t].verts[1].position.x, tri[t].verts[1].position.y, tri[t].verts[1].position.z, 1.0f)).x;
			vertexA[24*t+14] = (totalTransform*vector4(tri[t].verts[1].position.x, tri[t].verts[1].position.y, tri[t].verts[1].position.z, 1.0f)).y;
			vertexA[24*t+15] = (totalTransform*vector4(tri[t].verts[1].position.x, tri[t].verts[1].position.y, tri[t].verts[1].position.z, 1.0f)).z;

			// Transfer texture coordinates directly
			vertexA[24*t+16] = tri[t].verts[2].texCoords.x;
			vertexA[24*t+17] = tri[t].verts[2].texCoords.y;

			// rotate but do not translate normals
			vertexA[24*t+18] = (totalRotation*tri[t].verts[2].normal).x;
			vertexA[24*t+19] = (totalRotation*tri[t].verts[2].normal).y;
			vertexA[24*t+20] = (totalRotation*tri[t].verts[2].normal).z;

			// completely transform positions
			vertexA[24*t+21] = (totalTransform*vector4(tri[t].verts[2].position.x, tri[t].verts[2].position.y, tri[t].verts[2].position.z, 1.0f)).x;
			vertexA[24*t+22] = (totalTransform*vector4(tri[t].verts[2].position.x, tri[t].verts[2].position.y, tri[t].verts[2].position.z, 1.0f)).y;
			vertexA[24*t+23] = (totalTransform*vector4(tri[t].verts[2].position.x, tri[t].verts[2].position.y, tri[t].verts[2].position.z, 1.0f)).z;
		}
		glBufferData(GL_ARRAY_BUFFER, 288*sizeof(float), vertexA, GL_DYNAMIC_DRAW);
		renderer.push_VBO(box);
		transforms.popMatrix();
		renderer.DrawScene();
	}

	glDeleteBuffers(1, &box.handle);	// We don't need this VBO anymore

	// And... we're done here
	return 0;
}
