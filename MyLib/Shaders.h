#pragma once


#include "ShaderProgram.h"

enum ProgClass {
    PCLS_BUILD = 1,
    PCLS_FLAT = 2,
    PCLS_NOISE = 3
};
#define DEF_CLASS(c) \
    virtual int getClass() const override { return c; } \
    static int tClass() { return c; } \
    


class BaseProgram : public ShaderProgram {
public:
	BaseProgram()
        :trans("trans", this), vtx("vtx", this), colorAu("colorA", this)
	{}

	Mat4Uniform trans;
	Vec3Attrib vtx;

    Vec3Uniform colorAu; // common to noise and build

};


class FlatProgram : public BaseProgram {
public:
	virtual void getCodes() override;
    DEF_CLASS(PCLS_FLAT);
    
};



