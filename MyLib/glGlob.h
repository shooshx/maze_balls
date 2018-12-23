#pragma once

#ifdef QT_CORE_LIB
#define  GLEW_STATIC
#include <gl/glew.h>

#else
#include <GLES2/gl2.h>
#endif


#include <string>

void mglCheckErrors(const char* place = nullptr);
void mglCheckErrorsC(const char* place = nullptr);
void mglCheckErrors(const std::string& s);
void mglCheckErrorsC(const std::string& s);
