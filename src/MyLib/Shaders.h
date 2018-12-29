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
    FlatProgram() : colorAatt("colorAatt", this), force_uni_color("force_uni_color", this),
                    normal("normal", this), modelMat("modelMat", this), normalMat("normalMat", this),
                    lightPos("lightPos", this), globalT("globalT", this), projMat("projMat", this), 
                    b_clip_top("b_clip_top", this), do_shading("do_shading", this)
    {}
	virtual void getCodes() override;
    DEF_CLASS(PCLS_FLAT);
    
    AttribParam colorAatt;
    IntUniform force_uni_color;
    Vec3Attrib normal;

    Mat4Uniform projMat;
    Mat4Uniform modelMat;
    Mat3Uniform normalMat;
    Vec3Uniform lightPos;
    Mat4Uniform globalT;
    IntUniform b_clip_top;
    IntUniform do_shading;

    void setModelMat(const Mat4& m) {
        modelMat.set(m);
        normalMat.set(m.toNormalsTrans());
    }
};



