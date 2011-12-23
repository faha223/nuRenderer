#version 110

#define MAX_LIGHTS 16

varying float lambertTerm, specular;

uniform vec3 lightDir[MAX_LIGHTS];
uniform int numLights;
uniform sampler2D diffuseMap;

void main (void)
{
	float l = lambertTerm;
	vec4 final_color = texture2D(diffuseMap, gl_TexCoord[0].st);

	if(l < 0.1)
		l = 0.1;
	gl_FragColor = vec4(clamp(final_color.x * l + specular, 0.0, 1.0), clamp(final_color.y * l + specular, 0.0, 1.0), clamp(final_color.z * l + specular, 0.0, 1.0), 1.0);
}
