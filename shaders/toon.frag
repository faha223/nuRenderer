#version 110

#define MAX_LIGHTS 16

varying vec3 normal, eyeVec;
varying vec3 lightDirs[MAX_LIGHTS];

uniform vec3 lightDir[MAX_LIGHTS];
uniform int numLights;
uniform sampler2D diffuseMap;

void main (void)
{
	vec4 final_color = texture2D(diffuseMap, gl_TexCoord[0].st);
	vec3 N = normalize(normal), L;
	int i;
	float lambertTerm = 0.0, specular = 0.0, s, l;
	for (i=0; (i<numLights)&&(i<MAX_LIGHTS); ++i)
	{
		L = normalize(lightDirs[i]);

		l = dot(N,L) * 20.0/distance(lightDirs[i], vec3(0.0, 0.0, 0.0));

		if (l > 0.0)
			lambertTerm += l;

		s = pow(max(dot(reflect(-L, N), normalize(eyeVec)), 0.0), 8.0);
		specular += s*0.5;
	}
        if(lambertTerm < 0.25)
                lambertTerm = 0.125;
	else if((lambertTerm > 0.25)&&(lambertTerm <= 0.5))
                lambertTerm = 0.375;
	else if((lambertTerm > 0.5)&&(lambertTerm <= 0.75))
		lambertTerm = 0.625;
        else if((lambertTerm > 0.75)&&(lambertTerm <= 1.0))
                lambertTerm = 0.875;
        else
                lambertTerm = 1.0;
	gl_FragColor = vec4(clamp(final_color.x * lambertTerm + specular, 0.0, 1.0), clamp(final_color.y * lambertTerm + specular, 0.0, 1.0), clamp(final_color.z * lambertTerm + specular, 0.0, 1.0), 1.0);
}
