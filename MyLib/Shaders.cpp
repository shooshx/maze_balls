#include "Shaders.h"
#include "Mat.h"


const char *code_flat_vtx_glsl = R"---(
precision highp float;

uniform mat4 trans;
uniform vec3 colorA;
attribute vec3 vtx;
attribute vec3 colorAatt;
uniform int force_uni_color;

varying vec3 color;

// used for piece selection, not build selection
void main()
{	
    if (force_uni_color == 1)
	    color = colorA;
    else
        color = colorAatt;
    gl_Position = trans * vec4(vtx, 1.0);
}

)---";

const char *code_flat_frag_glsl = R"---(
precision highp float;

varying vec3 color;

void main (void)
{					
	gl_FragColor = vec4(color, 1.0);
	//gl_FragColor = vec4(1.0, 0.5, 0.5, 1.0);
}

)---";




void FlatProgram::getCodes() {
	m_vtxprog.push_back(code_flat_vtx_glsl);
	m_fragprog.push_back(code_flat_frag_glsl);
}

