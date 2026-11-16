/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <libGyro/gyro_system_api.h>
#include <gl/gyro_gl.h>
#include <gl/gyro_gl_utils.h>
#include <gl/gyro_video_gl.h>
#include <gl/gyro_bloom_gl.h>

extern GLuint _glalColorTex;
extern GLuint _glalGlowTex;

static GYShaderProgram* _downscaleProgram;
static GYShaderProgram* _blurProgram;
static GYShaderProgram* _compositeProgram;
static GLint _blurDirection;
static GLuint _framebuffer;
static GLuint _vao;
static GLuint _vbo;
static GLuint _textures[3];
static GLsizei _blurWidth;
static GLsizei _blurHeight;
static int _initialized;

static GYShaderProgram* _loadProgram(const char* vertexPath, const char* fragmentPath) {
    GYShader* vertex = gyShaderCreate(vertexPath);
    GYShader* fragment = gyShaderCreate(fragmentPath);
    GYShaderProgram* program = vertex && fragment ? gyShaderProgramCreate(vertex, fragment) : NULL;
    if(!program) {
        if(vertex) gyShaderDestroy(vertex);
        if(fragment) gyShaderDestroy(fragment);
    }
    return program;
}

static void _destroyProgram(GYShaderProgram** program) {
    if(!*program) return;
    GYShader* vertex = (*program)->vertShader;
    GYShader* fragment = (*program)->fragShader;
    gyShaderProgramDestroy(*program);
    gyShaderDestroy(vertex);
    gyShaderDestroy(fragment);
    *program = NULL;
}

int glalVidUninitializeBloom(void) {
    _initialized = 0;
    _destroyProgram(&_downscaleProgram);
    _destroyProgram(&_blurProgram);
    _destroyProgram(&_compositeProgram);
    glDeleteTextures(3, _textures);
    glDeleteFramebuffers(1, &_framebuffer);
    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_vbo);
    _textures[0] = _textures[1] = _textures[2] = 0;
    _framebuffer = _vao = _vbo = 0;
    _blurWidth = _blurHeight = 0;
    return 1;
}

int glalVidInitializeBloom(void) {
    //All postprocess vertex shaders use position at location 0 and UV at 1.
    static const GLfloat vertices[][4] = {
        {-1.0f, -1.0f, 0.0f, 0.0f},
        { 1.0f, -1.0f, 1.0f, 0.0f},
        {-1.0f,  1.0f, 0.0f, 1.0f},
        { 1.0f,  1.0f, 1.0f, 1.0f}
    };
    if(_initialized) return 1;
    _downscaleProgram = _loadProgram(GYRO_GLAL_DOWNSCALE_VERTEX_SHADER, GYRO_GLAL_DOWNSCALE_FRAGMENT_SHADER);
    _blurProgram = _loadProgram(GYRO_GLAL_BLOOM_VERTEX_SHADER, GYRO_GLAL_BLOOM_FRAGMENT_SHADER);
    _compositeProgram = _loadProgram(GYRO_GLAL_DOWNSCALE_VERTEX_SHADER, "/assets/shaders/bloomComposite.fsh");
    if(!_downscaleProgram || !_blurProgram || !_compositeProgram) {
        glalVidUninitializeBloom();
        return 0;
    }

    const GLint source = _protectedUniformQuery(_downscaleProgram, "uTexSrc", 0);
    const GLint blurSource = _protectedUniformQuery(_blurProgram, "uTexture", 0);
    const GLint scene = _protectedUniformQuery(_compositeProgram, "uScene", 0);
    const GLint glow = _protectedUniformQuery(_compositeProgram, "uGlow", 0);
    _blurDirection = _protectedUniformQuery(_blurProgram, "uConvDir", 0);
    if(source < 0 || blurSource < 0 || scene < 0 || glow < 0 || _blurDirection < 0) {
        glalVidUninitializeBloom();
        return 0;
    }
    glProgramUniform1i(_downscaleProgram->program, source, 0);
    glProgramUniform1i(_blurProgram->program, blurSource, 0);
    glProgramUniform1i(_compositeProgram->program, scene, 0);
    glProgramUniform1i(_compositeProgram->program, glow, 1);

    glGenFramebuffers(1, &_framebuffer);
    glGenTextures(3, _textures);
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);
    gyVidVertexArrayBind(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), NULL);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void*)(2 * sizeof(GLfloat)));
    gyVidVertexArrayBind(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    _initialized = 1;
    GY_GL_CHECK_ERROR();
    return 1;
}

int gyVidBloomResizeFramebuffer(unsigned width, unsigned height) {
    if(!_initialized) return 0;
    //Small or non-multiple-of-four viewports still need valid render targets.
    _blurWidth = width / 4 ? width / 4 : 1;
    _blurHeight = height / 4 ? height / 4 : 1;
    glActiveTexture(GL_TEXTURE0);
    for(unsigned i = 0; i < 3; ++i) {
        glBindTexture(GL_TEXTURE_2D, _textures[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _blurWidth, _blurHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    gyVidPipelineInvalidate();
    GY_GL_CHECK_ERROR();
    return 1;
}

static int _drawPass(GYShaderProgram* program, GLuint source, GLuint destination) {
    gyVidSetCurrentShaderProgram(program);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, destination, 0);
    const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE) {
        _gyLog(GY_DEBUG_ERROR, "Bloom framebuffer incomplete: 0x%x", status);
        return 0;
    }
    //Bind actual texture names; a mutable GYTexture wrapper defeats the pipeline's pointer cache.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, source);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    return 1;
}

void gyVidApplyBloom(void) {
    if(!_initialized || !_blurWidth || !_blurHeight) {
        gyVidPresentScene();
        return;
    }
    unsigned width, height;
    gyVidResolution(&width, &height);
    gyVidVertexBatchFlush();
    gyVidVertexBatchEnd();
    GYShaderProgram* previousProgram = glalCurShaderProg;
    GLboolean depthMask;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
    const GLboolean scissor = glIsEnabled(GL_SCISSOR_TEST);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDepthMask(GL_FALSE);
    gyVidVertexArrayBind(_vao);
    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glViewport(0, 0, _blurWidth, _blurHeight);

    int success = _drawPass(_downscaleProgram, _glalGlowTex, _textures[0]);
    glProgramUniform1i(_blurProgram->program, _blurDirection, 1);
    if(success) success = _drawPass(_blurProgram, _textures[0], _textures[1]);
    glProgramUniform1i(_blurProgram->program, _blurDirection, 0);
    if(success) success = _drawPass(_blurProgram, _textures[1], _textures[2]);

    glalVidBindDefaultFbo();
    glViewport(0, 0, width, height);
    glDepthMask(GL_TRUE);
    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT);
    if(success) {
        //The scene is already lit and tone-mapped. Add glow without lighting it again.
        gyVidSetCurrentShaderProgram(_compositeProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _glalColorTex);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, _textures[2]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    } else {
        gyVidPresentScene();
    }

    glDepthMask(depthMask);
    if(scissor) glEnable(GL_SCISSOR_TEST);
    glActiveTexture(glalTextureMapTexUnits[GLAL_TEXTURE_MAP]);
    gyVidVertexArrayBind(0);
    gyVidSetCurrentShaderProgram(previousProgram);
    //Direct GL state/texture changes must not leave the overlay or next scene using cached bindings.
    gyVidPipelineInvalidate();
    GY_GL_CHECK_ERROR();
}
