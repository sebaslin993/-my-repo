

#define LOG_TAG "Fan3D"
#include "Fan.h"
#include "ShaderHelper.h"
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <cmath>

namespace {
const GLfloat kPositions[8][3] = {
    {-.5f,-.5f, .5f}, {-.5f, .5f, .5f},
    { .5f, .5f, .5f}, { .5f,-.5f, .5f},
    {-.5f,-.5f,-.5f}, {-.5f, .5f,-.5f},
    { .5f, .5f,-.5f}, { .5f,-.5f,-.5f}
};
const GLfloat kShades[8] = {1,1,1,1, .6f,.6f,.6f,.6f};
const GLushort kIndices[36] = {
    0,3,1, 3,2,1, 7,4,6, 4,5,6, 4,0,5, 0,1,5,
    3,7,2, 7,6,2, 1,2,5, 2,6,5, 3,0,7, 0,4,7
};
constexpr GLuint kPositionAttribute = 0;
constexpr GLuint kShadeAttribute = 1;
}

#ifdef PLATFORM_ANDROID
Fan::Fan(AAssetManager* manager) : assetManager(manager) { modelType = FanType; }
#else
Fan::Fan() { modelType = FanType; }
#endif

Fan::~Fan() {
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (ibo) glDeleteBuffers(1, &ibo);
    if (program) glDeleteProgram(program);
}

void Fan::InitModel() {
#ifdef PLATFORM_ANDROID
    program = ShaderHelper::buildProgramFromAssets(
        assetManager, "shader/FanVertex.glsl", "shader/FanFragment.glsl");
#else
    program = ShaderHelper::buildProgramFromFile("FanVertex.glsl", "FanFragment.glsl");
#endif
    if (!program) { LOGE("Fan: failed to build shader program"); return; }
    mvpLocation = glGetUniformLocation(program, "MODELVIEWPROJECTIONMATRIX");
    partColorLocation = glGetUniformLocation(program, "PARTCOLOR");
    if (mvpLocation < 0 || partColorLocation < 0) {
        LOGE("Fan: required uniforms missing"); glDeleteProgram(program); program=0; return;
    }

    const GLsizeiptr positionSize=sizeof(kPositions), shadeSize=sizeof(kShades);
    glGenBuffers(1,&vbo); glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBufferData(GL_ARRAY_BUFFER,positionSize+shadeSize,nullptr,GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER,0,positionSize,kPositions);
    glBufferSubData(GL_ARRAY_BUFFER,positionSize,shadeSize,kShades);
    glGenBuffers(1,&ibo); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(kIndices),kIndices,GL_STATIC_DRAW);
    glGenVertexArrays(1,&vao); glBindVertexArray(vao); glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glEnableVertexAttribArray(kPositionAttribute);
    glVertexAttribPointer(kPositionAttribute,3,GL_FLOAT,GL_FALSE,0,reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(kShadeAttribute);
    glVertexAttribPointer(kShadeAttribute,1,GL_FLOAT,GL_FALSE,0,reinterpret_cast<void*>(positionSize));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ibo);
    glBindVertexArray(0); glBindBuffer(GL_ARRAY_BUFFER,0); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

    transform.TransformInit();
    transform.TransformSetMatrixMode(VIEW_MATRIX); transform.TransformLoadIdentity();
    glEnable(GL_DEPTH_TEST);
}

void Fan::DrawCube(const glm::vec3& colour) {
    glm::mat4* mvp=transform.TransformGetModelViewProjectionMatrix();
    glUniformMatrix4fv(mvpLocation,1,GL_FALSE,glm::value_ptr(*mvp));
    glUniform3fv(partColorLocation,1,glm::value_ptr(colour));
    glDrawElements(GL_TRIANGLES,36,GL_UNSIGNED_SHORT,nullptr);
}

void Fan::Render() {
    if (!program || !vao) return;
    if (fanOn) {
        spinAngle += kBaseSpeed + dragBoost;
        if (spinAngle >= 360.0f) spinAngle -= 360.0f;
    }
    glUseProgram(program); glBindVertexArray(vao);
    transform.TransformSetMatrixMode(MODEL_MATRIX); transform.TransformLoadIdentity();
    transform.TransformTranslate(0.0f,0.8f,-8.0f);
    transform.TransformRotate(glm::radians(20.0f),0.0f,1.0f,0.0f);

    transform.TransformPushMatrix();
    transform.TransformTranslate(0,-2.6f,0); transform.TransformScale(1.6f,.25f,.8f);
    DrawCube({.45f,.28f,.12f}); transform.TransformPopMatrix();

    transform.TransformPushMatrix();
    transform.TransformTranslate(0,-1.21f,0); transform.TransformScale(.15f,2.53f,.15f);
    DrawCube({.55f,.55f,.58f}); transform.TransformPopMatrix();

    transform.TransformPushMatrix();
    transform.TransformTranslate(0,.2f,0); transform.TransformScale(.3f,.3f,.3f);
    DrawCube({.20f,.20f,.22f}); transform.TransformPopMatrix();

    const glm::vec3 bladeColours[4] = {
        {1,0,0}, {0,0,1}, {1,.45f,.05f}, {0,.8f,.15f}
    };
    for (int i=0;i<4;++i) {
        transform.TransformPushMatrix();
        transform.TransformTranslate(0,.2f,.15f);
        transform.TransformRotate(glm::radians(spinAngle+i*90.0f),0,0,1);
        transform.TransformTranslate(0,.55f,0); transform.TransformScale(.22f,.8f,.05f);
        DrawCube(bladeColours[i]); transform.TransformPopMatrix();
    }
    glBindVertexArray(0); glUseProgram(0);
}

void Fan::Resize(int w,int h) {
    if (h<=0) h=1;
    transform.TransformSetMatrixMode(PROJECTION_MATRIX); transform.TransformLoadIdentity();
    transform.TransformSetPerspective(glm::radians(60.0f),float(w)/float(h),.01f,1000.0f,0);
}

void Fan::TouchEventDown(float x,float y) {
    lastX=x; lastY=y; movedDistance=0; dragBoost=0;
    lastMoveTime=std::chrono::steady_clock::now();
}

void Fan::TouchEventMove(float x,float y) {
    const auto now=std::chrono::steady_clock::now();
    const float dx=x-lastX, dy=y-lastY, distance=std::sqrt(dx*dx+dy*dy);
    float dtMs=std::chrono::duration<float,std::milli>(now-lastMoveTime).count();
    dtMs=std::max(dtMs,.1f);
    const float velocity=distance/dtMs;
    movedDistance += distance;
    dragBoost=std::min(velocity*kBoostScale,kMaxBoost);
    LOGI("Fan: drag velocity %.2f px/ms -> boost %.2f deg/frame",
         velocity, dragBoost);
    lastX=x; lastY=y; lastMoveTime=now;
}

void Fan::TouchEventRelease(float,float) {
    if (movedDistance < kTapThreshold) {
        fanOn=!fanOn; LOGI("Fan %s",fanOn ? "ON" : "OFF");
    }
    dragBoost=0;
}




