#version 110

#define MAX_LIGHTS 16

varying vec3 normal, eyeVec;
varying vec3 lightDirs[MAX_LIGHTS];

uniform vec3 lightDir[MAX_LIGHTS];
uniform int numLights;
uniform sampler2D diffuseMap;

void main()
{
	gl_Position = ftransform();
	normal = gl_Normal;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	vec4 vVertex = gl_Vertex;
	eyeVec = -vVertex.xyz;
	int i;
	for(i=0; (i<numLights)&&(i<MAX_LIGHTS); ++i)
		lightDirs[i] = vec3(lightDir[i] - vVertex.xyz);
}
