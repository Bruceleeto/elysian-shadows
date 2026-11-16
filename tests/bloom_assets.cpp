#include <gl/gyro_gl.h>
#include <QImage>

//Load the fixture's exact RGBA pixels without creating a Qt window or GL widget.
extern "C" GLuint loadBloomFixtureTexture(const char* path) {
    QImage image(path);
    if(image.isNull()) return 0;
    image = image.convertToFormat(QImage::Format_RGBA8888);
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image.width(), image.height(), 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, image.constBits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    return texture;
}
