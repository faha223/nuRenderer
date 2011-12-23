#version 110

#define MAX_LIGHTS 16

varying float lambertTerm, specular;

uniform vec3 lightDir[MAX_LIGHTS];
uniform int numLights;
uniform sampler2D diffuseMap;

void main()
{
	lambertTerm = 0.0;
	specular = 0.0;
	gl_Position = ftransform();
	gl_TexCoord[0] = gl_MultiTexCoord0;
	vec4 vVertex = gl_Vertex;
	int i;
	float l = 0.0, s = 0.0;
	vec3 L;
	for(i=0; (i<numLights)&&(i<MAX_LIGHTS); ++i)
	{
		L = lightDir[i] - vVertex.xyz;
		l = dot(gl_Normal, normalize(L)) * 20.0/distance(lightDir[i] - vVertex.xyz, vec3(0.0, 0.0, 0.0));
		s = pow(max(dot(reflect(-normalize(L), gl_Normal), normalize(-vVertex.xyz)), 0.0), 8.0);
		lambertTerm += clamp (l, 0.0, 1.0);
		specular += clamp(s, 0.0, 1.0)*0.5;
	}
}
