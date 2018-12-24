#include "Shaders.h"
#include "Mat.h"


const char *code_flat_vtx_glsl = R"---(
precision highp float;

uniform mat4 trans;
uniform mat4 modelMat;
uniform mat3 normalMat;

uniform vec3 colorA;
uniform vec3 lightPos;
attribute vec3 vtx;
attribute vec3 colorAatt;
attribute vec3 normal;
uniform int force_uni_color;

varying vec3 color;
varying float lightIntensity;

// used for piece selection, not build selection
void main()
{	
    if (force_uni_color == 1)
	    color = colorA;
    else
        color = colorAatt;
    gl_Position = trans * vec4(vtx, 1.0);

    //vec3 lightPos = vec3(0.0, 10.0, 50.0);
	vec3 ECposition = vec3(modelMat * vec4(vtx, 1.0));
	vec3 tnorm      = normalize(normalMat * normal); 
	lightIntensity  = dot(normalize(lightPos - ECposition), tnorm);
}

)---";

const char *code_flat_frag_glsl = R"---(
precision highp float;

varying vec3 color;
varying float lightIntensity;

void main (void)
{					
	gl_FragColor = vec4(color, 1.0) * lightIntensity;
	//gl_FragColor = vec4(1.0, 0.5, 0.5, 1.0);
}

)---";




void FlatProgram::getCodes() {
	m_vtxprog.push_back(code_flat_vtx_glsl);
	m_fragprog.push_back(code_flat_frag_glsl);
}

