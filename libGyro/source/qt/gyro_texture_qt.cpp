/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <libGyro/gyro_file_api.h>
#include <libGyro/gyro_texture_api.h>
#include <libGyro/gyro_system_api.h>
#include <gl/gyro_gl.h>
#include <QOpenGLWidget>
#include <libGyro/gyro_input_api.h>
#include <QImage>

extern GYInputInitParams    _initParams;
//glalDefaultFbo() lives in gyro_input_qt.cpp: it needs <QOpenGLContext>, which is incompatible with GLEW.

static bool _isPowerOfTwo(const int dimension) {
    return dimension > 0 && (dimension & (dimension - 1)) == 0;
}

int _gyVidTexAlloc(GYTexture *const tex) {
    QString fileName = tex->filename;
    if(fileName.startsWith("/assets/")) fileName.remove(0, (int)strlen("/assets/"));

    QImage qimg(fileName);
    if(qimg.isNull()) {
        tex->_data = nullptr;
        return 0;
    }

    tex->bpp    = qimg.depth();
    //Uploaded top row first: the engine's texture coordinates expect the image unflipped.
    qimg        = qimg.convertToFormat(QImage::Format_RGBA8888);
    tex->width  = qimg.width();
    tex->height = qimg.height();

    if(!_isPowerOfTwo(tex->width) || !_isPowerOfTwo(tex->height)) {
        _gyLog(GY_DEBUG_WARNING, "%s - <%d, %d> dimensions are not powers of two (suboptimal performance on some platforms).", tex->filename, tex->width, tex->height);
    }

    static_cast<QOpenGLWidget*>(_initParams.glWidget)->makeCurrent();

    GLuint texId = 0;
    glGenTextures(1, &texId);
    tex->_data = (void*)(uintptr_t)texId;
    glBindTexture(GL_TEXTURE_2D, texId);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, qimg.width(), qimg.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, qimg.constBits());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    return (int)texId;
}

int _gyVidTexFree(GYTexture *const tex) {
    if(!tex || !tex->_data) return 0;
    static_cast<QOpenGLWidget*>(_initParams.glWidget)->makeCurrent();
    const GLuint texId = (GLuint)(uintptr_t)tex->_data;
    glDeleteTextures(1, &texId);
    tex->_data = nullptr;
    return 1;
}

