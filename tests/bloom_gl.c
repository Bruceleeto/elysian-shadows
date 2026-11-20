#include <EGL/egl.h>
#include <gl/gyro_bloom_gl.h>
#include <gl/gyro_video_gl.h>
#include <common/gyro_video_common.h>
#include <libGyro/gyro_file_api.h>
#include <libGyro/gyro_system_api.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(expr) do { if(!(expr)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); abort(); } } while(0)

extern GLuint _glalMainFbo;
extern GLuint glalDirectionalShadowMapTextureArray;
extern GLuint glalPointShadowCubeMapTextureArray;
extern GLuint loadBloomFixtureTexture(const char* path);
static GLuint outputFbo;
static unsigned errors;

/* Replace only the editor-owned context/log hooks; run the real GL backend. */
GYGLProc __wrap_glalGetProcAddress(const char* name) { return eglGetProcAddress(name); }
void glalVidBindDefaultFbo(void) { glBindFramebuffer(GL_FRAMEBUFFER, outputFbo); }
void _gyPush(void) {}
void _gyPop(const unsigned count) { (void)count; }
void _gyLog(GY_DEBUG_SEVERITY level, const char* const format, ...) {
    if(level != GY_DEBUG_ERROR && level != GY_DEBUG_WARNING) return;
    ++errors;
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputc('\n', stderr);
}

static void readPixel(int x, int y, unsigned char pixel[4]) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, outputFbo);
    glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    CHECK(glGetError() == GL_NO_ERROR);
}

static void checkSize(int width, int height) {
    CHECK(glalVidOglInit(width, height, width, height));
    GLuint outputTexture;
    glGenTextures(1, &outputTexture);
    glBindTexture(GL_TEXTURE_2D, outputTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glGenFramebuffers(1, &outputFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, outputFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputTexture, 0);
    CHECK(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    const GLfloat scene[] = {0.2f, 0.3f, 0.4f, 1.0f};
    const GLfloat dark[] = {0.0f, 0.0f, 0.0f, 0.0f};
    const GLfloat glow[] = {0.1f, 0.0f, 0.0f, 1.0f};
    const GLfloat masked[] = {1.0f, 1.0f, 1.0f, 0.0f};
    unsigned char pixel[4];

    //A nonblack scene must survive bloom with no glow, including repeated frames.
    for(int frame = 0; frame < 3; ++frame) {
        glDisable(GL_SCISSOR_TEST);
        glBindFramebuffer(GL_FRAMEBUFFER, _glalMainFbo);
        glClearBufferfv(GL_COLOR, 0, scene);
        glClearBufferfv(GL_COLOR, 1, frame == 1 ? glow : dark);
        //Postprocessing must not inherit clipping/depth/blending from scene/UI draws.
        glEnable(GL_SCISSOR_TEST);
        glScissor(0, 0, 1, 1);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ZERO, GL_ZERO);
        gyVidApplyBloom();
        CHECK(glIsEnabled(GL_SCISSOR_TEST));
        readPixel(width / 2, height / 2, pixel);
        CHECK(pixel[1] >= 75 && pixel[1] <= 78);
        CHECK(pixel[2] >= 101 && pixel[2] <= 103);
        CHECK(pixel[3] == 255);
        if(frame == 1) CHECK(pixel[0] > 55);
        else CHECK(pixel[0] >= 50 && pixel[0] <= 52);
    }

    //RGB with a zero glow mask must not leak into the bloom result.
    glDisable(GL_SCISSOR_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, _glalMainFbo);
    glClearBufferfv(GL_COLOR, 1, masked);
    gyVidApplyBloom();
    readPixel(width / 2, height / 2, pixel);
    CHECK(pixel[0] >= 50 && pixel[0] <= 52);
    CHECK(pixel[1] >= 75 && pixel[1] <= 78);
    CHECK(pixel[2] >= 101 && pixel[2] <= 103);

    if(width >= 64 && height >= 64) {
        //An isolated source must spread into surrounding pixels, not simply be copied.
        glBindFramebuffer(GL_FRAMEBUFFER, _glalMainFbo);
        glClearBufferfv(GL_COLOR, 0, dark);
        glClearBufferfv(GL_COLOR, 1, dark);
        glEnable(GL_SCISSOR_TEST);
        glScissor(width / 2 - 4, height / 2 - 4, 8, 8);
        const GLfloat bright[] = {1.0f, 0.0f, 0.0f, 1.0f};
        glClearBufferfv(GL_COLOR, 1, bright);
        glDisable(GL_SCISSOR_TEST);
        gyVidApplyBloom();
        readPixel(width / 2 + 7, height / 2, pixel);
        CHECK(pixel[0] > 0 && pixel[1] == 0 && pixel[2] == 0);
    }
    glDeleteFramebuffers(1, &outputFbo);
    glDeleteTextures(1, &outputTexture);
    outputFbo = 0;
    glalVidBindDefaultFbo();
    CHECK(errors == 0);
}

/* Perspective terrain seams: neighbouring faces can be less than a world unit
 * apart near their shared edge. Test the game's actual scene FBO and shader,
 * including reallocation, and require the nearer face to win in either order. */
static void checkPerspectiveDepth(void) {
    for(int resize = 0; resize < 2; ++resize) {
        const int size = resize ? 79 : 64;
        CHECK(glalVidOglInit(size, size, size, size));
        gyVidSetDefaultShaderProgram();
        const GLuint program = glalLightingShader.program->program;
        GYMatrix4 projection;
        gyVidPerspectiveMat(projection, 45.0f, 1.0f, 0.5f, 9000.0f);
        glUniformMatrix4fv(glGetUniformLocation(program, "uProjViewMat"), 1, GL_FALSE, &projection[0][0]);
        glUniform1i(glGetUniformLocation(program, "uTextureEnabled"), 0);
        glUniform1i(glGetUniformLocation(program, "uNormalMapEnabled"), 0);
        glUniform1i(glGetUniformLocation(program, "uSpecularMapEnabled"), 0);
        glUniform4f(glGetUniformLocation(program, "uMaterial.ambientColor"), 1, 1, 1, 1);
        glUniform4f(glGetUniformLocation(program, "uMaterial.emissiveColor"), 0, 0, 0, 0);
        glUniform4f(glGetUniformLocation(program, "uAmbience"), 1, 1, 1, 1);
        GLuint vao, vbo;
        glGenVertexArrays(1, &vao);
        gyVidVertexArrayBind(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        const GLuint position = glalLightingShader.attribLoc[GLAL_SHADER_POS];
        glEnableVertexAttribArray(position);
        glVertexAttribPointer(position, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), NULL);
        glBindFramebuffer(GL_FRAMEBUFFER, _glalMainFbo);
        glViewport(0, 0, size, size);
        glDisable(GL_BLEND);
        glDisable(GL_SCISSOR_TEST);
        glDisable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LEQUAL);
        glClearDepth(1);
        for(int distance = 400; distance <= 800; distance += 100) {
            for(int order = 0; order < 2; ++order) {
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                for(int face = 0; face < 2; ++face) {
                    const int behind = face ^ order;
                    const float z = -(distance + (behind ? 0.25f : 0.0f));
                    const GLfloat vertices[][4] = {
                        {-100, -100, z, 1}, {100, -100, z, 1},
                        {-100, 100, z, 1}, {100, 100, z, 1}
                    };
                    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
                    glVertexAttrib4f(glalLightingShader.attribLoc[GLAL_SHADER_COLOR], behind, !behind, 0, 1);
                    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                }
                unsigned char pixel[4];
                glReadBuffer(GL_COLOR_ATTACHMENT0);
                glReadPixels(size / 2, size / 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
                CHECK(pixel[0] == 0 && pixel[1] > 0);
            }
        }
        gyVidVertexArrayBind(0);
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        gyVidPipelineInvalidate();
        CHECK(errors == 0 && glGetError() == GL_NO_ERROR);
    }
    puts("Perspective depth: nearer surfaces win in both draw orders before and after resize");
}

static void checkLightingOutput(void) {
    CHECK(glalVidOglInit(16, 16, 16, 16));
    gyVidSetDefaultShaderProgram();
    const GLuint program = glalLightingShader.program->program;
    const GLfloat identity[] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    glUniformMatrix4fv(glGetUniformLocation(program, "uProjViewMat"), 1, GL_FALSE, identity);
    glUniform1i(glGetUniformLocation(program, "uTextureEnabled"), 0);
    glUniform1i(glGetUniformLocation(program, "uNormalMapEnabled"), 0);
    glUniform1i(glGetUniformLocation(program, "uSpecularMapEnabled"), 0);
    glUniform4f(glGetUniformLocation(program, "uMaterial.ambientColor"), 1, 1, 1, 1);
    glUniform4f(glGetUniformLocation(program, "uMaterial.emissiveColor"), 0, 0, 0, 0);
    const GLfloat vertices[][4] = {{-1, -1, 0, 1}, {1, -1, 0, 1}, {-1, 1, 0, 1}, {1, 1, 0, 1}};
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    gyVidVertexArrayBind(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    const GLuint position = glalLightingShader.attribLoc[GLAL_SHADER_POS];
    glEnableVertexAttribArray(position);
    glVertexAttribPointer(position, 4, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), NULL);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, _glalMainFbo);

    //The corrected curve maps black to black; 0.5 at unit exposure to ~0.565.
    //Ambient alpha and the public exposure control each apply exactly once.
    const struct {float color, ambientExposure, exposure; int expected;} cases[] = {
        {0.0f, 1.0f, 1.0f, 0}, {0.5f, 1.0f, 1.0f, 144},
        {0.5f, 0.5f, 1.0f, 78}, {0.5f, 1.0f, 0.5f, 78}
    };
    for(unsigned i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        gyVidHighDynamicRangeExposure(cases[i].exposure);
        glUniform4f(glGetUniformLocation(program, "uAmbience"), 1, 1, 1, cases[i].ambientExposure);
        glVertexAttrib4f(glalLightingShader.attribLoc[GLAL_SHADER_COLOR],
                        cases[i].color, cases[i].color, cases[i].color, 1);
        const GLfloat staleGlow[] = {1, 0, 0, 1};
        glClearBufferfv(GL_COLOR, 1, staleGlow);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        unsigned char pixel[4];
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glReadPixels(8, 8, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
        CHECK(abs((int)pixel[0] - cases[i].expected) <= 1);
        CHECK(pixel[0] == pixel[1] && pixel[1] == pixel[2]);
        glReadBuffer(GL_COLOR_ATTACHMENT1);
        glReadPixels(8, 8, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
        CHECK(pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 0 && pixel[3] == 0);
    }
    gyVidHighDynamicRangeExposure(1.0f);

    //Use the real sphere textures. A horizontal sun produces dark centers;
    //a downward white key light must illuminate them even without shadow passes.
    GLuint textures[3];
    const char* paths[] = {
        "level/bloom_speec_spheres_test/tile/tilesheet.png",
        "level/bloom_speec_spheres_test/tile/tilesheet_n.png",
        "level/bloom_speec_spheres_test/tile/tilesheet_e.png"
    };
    for(unsigned i = 0; i < 3; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        textures[i] = loadBloomFixtureTexture(paths[i]);
        CHECK(textures[i] != 0);
    }
    glUniform1i(glGetUniformLocation(program, "uTextureEnabled"), 1);
    glUniform1i(glGetUniformLocation(program, "uNormalMapEnabled"), 1);
    glUniform1i(glGetUniformLocation(program, "uSpecularMapEnabled"), 1);
    glUniform4f(glGetUniformLocation(program, "uAmbience"), 0, 0, 0, 1);
    glVertexAttrib4f(glalLightingShader.attribLoc[GLAL_SHADER_COLOR], 1, 1, 1, 1);
    glVertexAttrib2f(glalLightingShader.attribLoc[GLAL_SHADER_TEX_COORD], 64.5f / 512, 32.5f / 1024);
    glVertexAttrib3f(glalLightingShader.attribLoc[GLAL_SHADER_NORMAL], 0, 0, 1);
    glVertexAttrib3f(glalLightingShader.attribLoc[GLAL_SHADER_TANGENT], 1, 0, 0);
    glVertexAttrib1f(glalLightingShader.attribLoc[GLAL_SHADER_NORMAL_Z], 1);
    GYCamera camera = {0};
    GYLightProperties properties = {.diffuseColor = {0.8f, 0.8f, 0.8f, 0}, .attenuationFactors = {1, 0, 0}};
    GYLightSource light = {.direction = {-1, 0, 0}, .spotCutoffAngle = 360,
                          .properties = &properties, .camera = &camera};
    gyVidLightSourceInit(&light, GY_LIGHT_DIRECTIONAL);

    //Force the unused shadow layer dark: non-shadow-casting lights must ignore it.
    GLuint depthFbo;
    glGenFramebuffers(1, &depthFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, depthFbo);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, glalDirectionalShadowMapTextureArray, 0, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    CHECK(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    glClearDepth(0);
    glDepthMask(GL_TRUE);
    glClear(GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, _glalMainFbo);
    glDeleteFramebuffers(1, &depthFbo);
    //Light initialization uses the current texture unit; rebind the material map.
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textures[2]);
    for(int direction = 0; direction < 2; ++direction) {
        if(direction) light.direction = (GYVector3){0, 0, -1};
        gyVidLightSourceClear();
        gyVidLightSourceAdd(&light);
        gyVidLightSourceAdd(NULL);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        unsigned char pixel[4];
        glReadPixels(8, 8, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
        if(direction) CHECK(pixel[0] > 100 && pixel[1] > 100 && pixel[2] > 100);
        else CHECK(pixel[0] < 8 && pixel[1] < 8 && pixel[2] < 8);
        glReadBuffer(GL_COLOR_ATTACHMENT1);
        glReadPixels(8, 8, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
        //Preserve the original ambient-relative threshold: at zero ambience,
        //lit samples are selected by brightness rather than the material mask.
        CHECK(pixel[3] == 255);
        CHECK(glalLightingUBOData.sources[0].shadowMapIdx == -1);
    }

    //Below the authored brightness threshold, retain the texture's red-channel mask.
    glUniform4f(glGetUniformLocation(program, "uAmbience"), 1, 1, 1, 1);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glReadBuffer(GL_COLOR_ATTACHMENT1);
    unsigned char materialGlow[4];
    glReadPixels(8, 8, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, materialGlow);
    CHECK(materialGlow[3] >= 6 && materialGlow[3] <= 8);

    //Changing the shadow camera after registration must update the uploaded light.
    for(int frame = 0; frame < 2; ++frame) {
        gyVidLightSourceClear();
        light.direction = frame ? (GYVector3){0, 0, -1} : (GYVector3){-1, 0, -1};
        gyVec3Normalize(&light.direction);
        gyVidLightSourceAdd(&light);
        gyVidLightSourceAdd(NULL);
        gyVidLightSourceBeginShadowPasses();
        gyVidLightSourceBeginShadowPass(&light, GY_CUBE_MAP_FACE_MAX);
        gyVidLightSourceEndShadowPasses();
        LightingUBOData uploaded;
        glBindBuffer(GL_UNIFORM_BUFFER, glalLightingUBO);
        glGetBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(uploaded), &uploaded);
        CHECK(uploaded.sources[0].shadowMapIdx == 0);
        CHECK(memcmp(uploaded.sources[0].projViewMat, camera.projViewMat, sizeof(GYMatrix4)) == 0);
    }
    gyVidLightSourceClear();
    gyVidLightSourceUninit(&light);

    //Each point light owns six consecutive cube-array faces, not one layer.
    GYLightSource points[2] = {
        {.position = {0, 0, 32, 1}, .properties = &properties, .camera = &camera},
        {.position = {0, 0, 64, 1}, .properties = &properties, .camera = &camera}
    };
    for(unsigned i = 0; i < 2; ++i) {
        gyVidLightSourceInit(&points[i], GY_LIGHT_POINT);
        gyVidLightSourceAdd(&points[i]);
    }
    gyVidLightSourceAdd(NULL);
    CHECK(((_LightInternals*)points[1]._data)->shadowMapIdx == 1);
    //Use small depth layers so we can verify the actual writes, not just attachment state.
    GLfloat depths[4 * 4 * 12] = {0};
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, glalPointShadowCubeMapTextureArray);
    glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_DEPTH_COMPONENT24, 4, 4, 12,
                 0, GL_DEPTH_COMPONENT, GL_FLOAT, depths);
    gyVidLightSourceBeginShadowPasses();
    for(unsigned face = 0; face < GY_CUBE_MAP_FACE_MAX; ++face) {
        gyVidLightSourceBeginShadowPass(&points[1], face);
        glClearDepth(0.5);
        glClear(GL_DEPTH_BUFFER_BIT);
    }
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, glalPointShadowCubeMapTextureArray);
    glGetTexImage(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_DEPTH_COMPONENT, GL_FLOAT, depths);
    for(unsigned i = 0; i < 4 * 4 * 12; ++i) {
        if(i < 4 * 4 * 6) CHECK(depths[i] == 0);
        else CHECK(depths[i] > 0.49f && depths[i] < 0.51f);
    }
    gyVidLightSourceEndShadowPasses();
    gyVidLightSourceClear();
    for(unsigned i = 0; i < 2; ++i) gyVidLightSourceUninit(&points[i]);
    glDeleteTextures(3, textures);
    gyVidVertexArrayBind(0);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    gyVidPipelineInvalidate();
    CHECK(glGetError() == GL_NO_ERROR && errors == 0);
}

static void checkMovingPointLight(void) {
    enum { SIZE = 256 };
    CHECK(glalVidOglInit(SIZE, SIZE, SIZE, SIZE));
    gyVidSetDefaultShaderProgram();
    const GLuint program = glalLightingShader.program->program;
    const GLfloat projection[] = {1.0f/64, 0, 0, 0, 0, 1.0f/64, 0, 0,
                                  0, 0, 1.0f/256, 0, 0, 0, 0, 1};
    glUniformMatrix4fv(glGetUniformLocation(program, "uProjViewMat"), 1, GL_FALSE, projection);
    glUniform4f(glGetUniformLocation(program, "uAmbience"), 0, 0, 0, 1);
    glUniform1i(glGetUniformLocation(program, "uTextureEnabled"), 1);
    glUniform1i(glGetUniformLocation(program, "uSpecularMapEnabled"), 1);
    glUniform3f(glGetUniformLocation(program, "uCamWorldPos"), 0, 0, 0);
    gyVidHighDynamicRangeExposure(1);

    //The first sphere occupies atlas pixels (0,32) through (128,160).
    const GLfloat vertices[][6] = {
        {-64, -64, 0, 1, 0, 0.03125f}, {64, -64, 0, 1, 0.25f, 0.03125f},
        {-64, 64, 0, 1, 0, 0.15625f}, {64, 64, 0, 1, 0.25f, 0.15625f}
    };
    GLuint vao, vbo, outputTexture, textures[3];
    glGenVertexArrays(1, &vao);
    gyVidVertexArrayBind(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    GLuint position = glalLightingShader.attribLoc[GLAL_SHADER_POS];
    GLuint uv = glalLightingShader.attribLoc[GLAL_SHADER_TEX_COORD];
    glEnableVertexAttribArray(position);
    glVertexAttribPointer(position, 4, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), NULL);
    glEnableVertexAttribArray(uv);
    glVertexAttribPointer(uv, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void*)(4 * sizeof(GLfloat)));
    glVertexAttrib4f(glalLightingShader.attribLoc[GLAL_SHADER_COLOR], 1, 1, 1, 1);
    glVertexAttrib3f(glalLightingShader.attribLoc[GLAL_SHADER_NORMAL], 0, 0, 1);
    glVertexAttrib3f(glalLightingShader.attribLoc[GLAL_SHADER_TANGENT], 1, 0, 0);
    glVertexAttrib1f(glalLightingShader.attribLoc[GLAL_SHADER_NORMAL_Z], 1);
    const char* paths[] = {
        "level/bloom_speec_spheres_test/tile/tilesheet.png",
        "level/bloom_speec_spheres_test/tile/tilesheet_n.png",
        "level/bloom_speec_spheres_test/tile/tilesheet_e.png"
    };
    for(unsigned i = 0; i < 3; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        textures[i] = loadBloomFixtureTexture(paths[i]);
        CHECK(textures[i]);
    }
    glGenTextures(1, &outputTexture);
    glBindTexture(GL_TEXTURE_2D, outputTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, SIZE, SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glGenFramebuffers(1, &outputFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, outputFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputTexture, 0);
    CHECK(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    GYCamera camera = {0};
    //No specular or ambient contribution: this isolates the curved normal-map response.
    GYLightProperties properties = {.diffuseColor = {3, 3, 3, 0}, .attenuationFactors = {1, 0, 0}};
    GYLightSource light = {.position = {-256, 0, 0, 1}, .spotCutoffAngle = 360,
                          .properties = &properties, .camera = &camera};
    gyVidLightSourceInit(&light, GY_LIGHT_POINT);
    static unsigned char scene[SIZE * SIZE * 4], composite[SIZE * SIZE * 4];
    double centers[2];
    for(int side = 0; side < 3; ++side) {
        light.position.x = side == 1 ? 256 : -256;
        gyVidSetDefaultShaderProgram();
        glUniform1i(glGetUniformLocation(program, "uNormalMapEnabled"), side < 2);
        for(unsigned i = 0; i < 3; ++i) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, textures[i]);
        }
        gyVidVertexArrayBind(vao);
        glDisable(GL_SCISSOR_TEST);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glBindFramebuffer(GL_FRAMEBUFFER, _glalMainFbo);
        glViewport(0, 0, SIZE, SIZE);
        const GLfloat black[] = {0, 0, 0, 0};
        glClearBufferfv(GL_COLOR, 0, black);
        glClearBufferfv(GL_COLOR, 1, black);
        gyVidLightSourceClear();
        gyVidLightSourceAdd(&light);
        gyVidLightSourceAdd(NULL);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glReadPixels(0, 0, SIZE, SIZE, GL_RGBA, GL_UNSIGNED_BYTE, scene);
        gyVidApplyBloom();
        glBindFramebuffer(GL_READ_FRAMEBUFFER, outputFbo);
        glReadPixels(0, 0, SIZE, SIZE, GL_RGBA, GL_UNSIGNED_BYTE, composite);
        unsigned dark = 0, halo = 0;
        double mass = 0, weightedX = 0;
        for(unsigned y = 0; y < SIZE; ++y) for(unsigned x = 0; x < SIZE; ++x) {
            unsigned i = (y * SIZE + x) * 4;
            mass += scene[i];
            weightedX += x * scene[i];
            if(scene[i] == 0) {
                ++dark;
                if(composite[i] > 2) ++halo;
            }
            CHECK(composite[i] >= scene[i]);
            CHECK(composite[i + 3] == 255);
        }
        if(side < 2) {
            CHECK(mass > 10000 && dark > SIZE * SIZE / 4 && halo > 100);
            centers[side] = weightedX / mass;
        } else {
            //A flat surface with a same-height light cannot make this curved highlight.
            CHECK(mass == 0 && halo == 0);
        }
    }
    CHECK(centers[0] < SIZE * 0.4 && centers[1] > SIZE * 0.6);
    printf("Point-light highlights move from x=%.1f to %.1f; dark regions and bloom halo verified\n",
           centers[0], centers[1]);
    gyVidLightSourceClear();
    gyVidLightSourceUninit(&light);
    glDeleteTextures(3, textures);
    glDeleteTextures(1, &outputTexture);
    glDeleteFramebuffers(1, &outputFbo);
    outputFbo = 0;
    glalVidBindDefaultFbo();
    gyVidVertexArrayBind(0);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    gyVidPipelineInvalidate();
    CHECK(errors == 0 && glGetError() == GL_NO_ERROR);
}


/* Optional diagnostic: render the authored map with an explicitly positioned light.
 * Run from assets/: bloom_gl --render-spheres output.ppm x y z diffuse specular quadratic
 * Keeping camera, textures and bloom fixed makes lighting comparisons reproducible. */
static void renderSpheres(const char* output, const float parameters[6]) {
    enum { SIZE = 800, MAP_WIDTH = 50, MAP_HEIGHT = 50 };
    CHECK(glalVidOglInit(SIZE, SIZE, SIZE, SIZE));
    gyVidSetDefaultShaderProgram();
    const GLuint program = glalLightingShader.program->program;
    const GLfloat projection[] = {1.0f/400, 0, 0, 0, 0, -1.0f/400, 0, 0,
                                  0, 0, 1.0f/256, 0, -448.0f/400, 400.0f/400, 0, 1};
    glUniformMatrix4fv(glGetUniformLocation(program, "uProjViewMat"), 1, GL_FALSE, projection);
    glUniform4f(glGetUniformLocation(program, "uAmbience"), 0, 0, 0, 1);
    glUniform1i(glGetUniformLocation(program, "uTextureEnabled"), 1);
    glUniform1i(glGetUniformLocation(program, "uNormalMapEnabled"), 1);
    glUniform1i(glGetUniformLocation(program, "uSpecularMapEnabled"), 1);
    glUniform3f(glGetUniformLocation(program, "uCamWorldPos"), 448, 400, 0);
    glUniform4f(glGetUniformLocation(program, "uMaterial.diffuseColor"), 1, 1, 1, 1);
    glUniform4f(glGetUniformLocation(program, "uMaterial.emissiveColor"), 0, 0, 0, 0);
    glUniform1f(glGetUniformLocation(program, "uMaterial.shininess"), 1);
    gyVidHighDynamicRangeExposure(1);

    FILE* map = fopen("level/bloom_speec_spheres_test/area/new_area1/map.bin", "rb");
    CHECK(map && fseek(map, 104, SEEK_SET) == 0);
    GLfloat vertices[MAP_WIDTH * MAP_HEIGHT * 6][6];
    unsigned count = 0;
    const unsigned corners[] = {0, 1, 2, 2, 1, 3};
    for(unsigned y = 0; y < MAP_HEIGHT; ++y) for(unsigned x = 0; x < MAP_WIDTH; ++x) {
        unsigned char bytes[4];
        CHECK(fread(bytes, 1, 4, map) == 4);
        const unsigned tile = bytes[0] | (unsigned)bytes[1] << 8 |
                              (unsigned)bytes[2] << 16 | (unsigned)bytes[3] << 24;
        const unsigned index = tile & 511;
        if(!index) continue;
        CHECK((tile >> 9) == 0); //This fixture contains only flat, unrotated ground tiles.
        for(unsigned j = 0; j < 6; ++j) {
            const unsigned cx = corners[j] & 1, cy = corners[j] >> 1;
            const GLfloat vertex[] = {x * 32.0f - 16 + cx * 32, y * 32.0f - 16 + cy * 32,
                                      0, 1, (index % 16 + cx) / 16.0f, (index / 16 + cy) / 32.0f};
            memcpy(vertices[count++], vertex, sizeof(vertex));
        }
    }
    fclose(map);
    GLuint vao, vbo, textures[3], outputTexture;
    glGenVertexArrays(1, &vao);
    gyVidVertexArrayBind(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(vertices[0]), vertices, GL_STATIC_DRAW);
    const GLuint position = glalLightingShader.attribLoc[GLAL_SHADER_POS];
    const GLuint uv = glalLightingShader.attribLoc[GLAL_SHADER_TEX_COORD];
    glEnableVertexAttribArray(position);
    glVertexAttribPointer(position, 4, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), NULL);
    glEnableVertexAttribArray(uv);
    glVertexAttribPointer(uv, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void*)(4 * sizeof(GLfloat)));
    glVertexAttrib4f(glalLightingShader.attribLoc[GLAL_SHADER_COLOR], 1, 1, 1, 1);
    glVertexAttrib3f(glalLightingShader.attribLoc[GLAL_SHADER_NORMAL], 0, 0, 1);
    glVertexAttrib3f(glalLightingShader.attribLoc[GLAL_SHADER_TANGENT], 1, 0, 0);
    glVertexAttrib1f(glalLightingShader.attribLoc[GLAL_SHADER_NORMAL_Z], 1);
    const char* paths[] = {"tilesheet.png", "tilesheet_n.png", "tilesheet_e.png"};
    for(unsigned i = 0; i < 3; ++i) {
        char path[256];
        snprintf(path, sizeof(path), "level/bloom_speec_spheres_test/tile/%s", paths[i]);
        glActiveTexture(GL_TEXTURE0 + i);
        textures[i] = loadBloomFixtureTexture(path);
        CHECK(textures[i]);
    }
    glActiveTexture(GL_TEXTURE0 + 3);
    glGenTextures(1, &outputTexture);
    glBindTexture(GL_TEXTURE_2D, outputTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, SIZE, SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glGenFramebuffers(1, &outputFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, outputFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputTexture, 0);
    CHECK(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    GYCamera camera = {0};
    GYLightProperties properties = {.diffuseColor = {parameters[3], parameters[3], parameters[3], 0},
        .specularColor = {parameters[4], parameters[4], parameters[4], 0},
        .attenuationFactors = {0.00005f, 0.00005f, parameters[5]}};
    GYLightSource light = {.position = {parameters[0], parameters[1], parameters[2], 1},
        .spotCutoffAngle = 360, .properties = &properties, .camera = &camera};
    gyVidLightSourceInit(&light, GY_LIGHT_POINT);
    gyVidLightSourceClear();
    gyVidLightSourceAdd(&light);
    gyVidLightSourceAdd(NULL);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glBindFramebuffer(GL_FRAMEBUFFER, _glalMainFbo);
    glViewport(0, 0, SIZE, SIZE);
    const GLfloat black[] = {0, 0, 0, 0};
    glClearBufferfv(GL_COLOR, 0, black);
    glClearBufferfv(GL_COLOR, 1, black);
    glDrawArrays(GL_TRIANGLES, 0, count);
    gyVidApplyBloom();
    static unsigned char pixels[SIZE * SIZE * 3];
    glBindFramebuffer(GL_READ_FRAMEBUFFER, outputFbo);
    glReadPixels(0, 0, SIZE, SIZE, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    FILE* file = fopen(output, "wb");
    CHECK(file);
    fprintf(file, "P6\n%d %d\n255\n", SIZE, SIZE);
    for(int y = SIZE - 1; y >= 0; --y) CHECK(fwrite(pixels + y * SIZE * 3, 3, SIZE, file) == SIZE);
    CHECK(fclose(file) == 0);
    gyVidLightSourceClear();
    gyVidLightSourceUninit(&light);
    glDeleteTextures(3, textures);
    glDeleteTextures(1, &outputTexture);
    glDeleteFramebuffers(1, &outputFbo);
    outputFbo = 0;
    glalVidBindDefaultFbo();
    gyVidVertexArrayBind(0);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    gyVidPipelineInvalidate();
    CHECK(errors == 0 && glGetError() == GL_NO_ERROR);
}

int main(int argc, char** argv) {
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if(display == EGL_NO_DISPLAY || !eglInitialize(display, NULL, NULL)) {
        fputs("No headless EGL display available\n", stderr);
        return 77;
    }
    CHECK(eglBindAPI(EGL_OPENGL_API));
    const EGLint configAttributes[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT, EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_NONE
    };
    EGLConfig config;
    EGLint count;
    CHECK(eglChooseConfig(display, configAttributes, &config, 1, &count) && count);
    const EGLint contextAttributes[] = {
        EGL_CONTEXT_MAJOR_VERSION, 4, EGL_CONTEXT_MINOR_VERSION, 1,
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT, EGL_NONE
    };
    EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttributes);
    CHECK(context != EGL_NO_CONTEXT);
    const EGLint surfaceAttributes[] = {EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE};
    EGLSurface surface = eglCreatePbufferSurface(display, config, surfaceAttributes);
    CHECK(surface != EGL_NO_SURFACE);
    CHECK(eglMakeCurrent(display, surface, surface, context));
    CHECK(gyFileInit(NULL));
    if(argc == 9 && strcmp(argv[1], "--render-spheres") == 0) {
        float parameters[6];
        for(unsigned i = 0; i < 6; ++i) parameters[i] = strtof(argv[i + 3], NULL);
        renderSpheres(argv[2], parameters);
    } else {
        CHECK(argc == 1);
        checkSize(64, 64);
        checkSize(79, 61);
        checkSize(3, 2);
        CHECK(glalVidUninitializeBloom());
        CHECK(glalVidInitializeBloom());
        checkSize(64, 64);
        checkPerspectiveDepth();
        checkLightingOutput();
        checkMovingPointLight();
    }
    CHECK(glalVidUninitializeBloom());
    CHECK(errors == 0 && glGetError() == GL_NO_ERROR);
    CHECK(gyFileQuit());
    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(display, surface);
    eglDestroyContext(display, context);
    eglTerminate(display);
    puts(argc == 1 ? "Bloom scene preservation, glow, mask, resize, reinitialization and lighting output passed"
                   : "Sphere fixture render saved");
    return 0;
}
