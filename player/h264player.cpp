#define GLES_VERSION 2 
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <string>

#include "Timer.h"
#include "Text.h"
#include "Texture.h"
#include "Shader.h"
#include "Matrix.h"
#include "Platform.h"
#include "Mathematics.h"
#include "EGLRuntime.h"

#include "RotoZoom.h"
#include "FrameBufferObject.h"

#include <stdio.h>
#include "mindroid/os/Thread.h"
#include "mindroid/os/Lock.h"
using namespace mindroid;

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <android/libon2/vpu_api.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <libavutil/mem.h>
}

#include "fbtools.h"

int stop = 0;

typedef enum Output
{
    OUTPUT_DISPLAY,
    OUTPUT_OPENGL_ROTO,
    OUTPUT_OPENGL_FBO
} Output;

Output output = OUTPUT_DISPLAY;
int half = 0;
float fps = 0;

struct VpuCodecContext* ctx = NULL;
RK_S32 fileSize =0, pkt_size =0;
RK_S32 ret = 0;
DecoderOut_t    decOut;
VideoPacket_t demoPkt;
VideoPacket_t* pkt =NULL;
DecoderOut_t *pOut = NULL;
VPU_FRAME *frame = NULL;
RK_S64 fakeTimeUs =0;
RK_U8* pExtra = NULL;
RK_U32 extraSize = 0;

Lock mutex;
enum {
    DECODING_NULL = 0,
    DECODING_FINISHED
};
int decoderState = DECODING_NULL;

char* movieFileName = NULL;

#include "hwc_copybit.h"
CopyBit copyBit;
void yuv2rgb(uint8_t* rgb, uint8_t *yuv420sp, int width, int height);

#define WINDOW_W 1920
#define WINDOW_H 1080
#define FBO_WIDTH    1920
#define FBO_HEIGHT   1080

using std::string;
using namespace MaliSDK;

string resourceDirectory = "assets/";
string imageFilename = "sintel.nv21";
string rotoVertexShaderFilename = "RotoZoom_cube.vert";
string rotoFragmentShaderFilename = "RotoZoom_cube.frag";
string FBOVertexShaderFilename = "FrameBufferObject_cube.vert";
string FBOFragmentShaderFilename = "FrameBufferObject_cube.frag";

GLuint programID = 0;
GLint iLocTextureMatrix = -1;
GLint iLocPosition = -1;
GLint iLocTextureMix = -1;
GLint iLocTexture = -1;
GLint iLocTexCoord = -1;

GLuint vertexShaderID = 0;
GLuint fragmentShaderID = 0;
//GLuint programID = 0;
//GLint iLocPosition = -1;
//GLint iLocTextureMix = -1;
//GLint iLocTexture = -1;
GLint iLocFillColor = -1;
//GLint iLocTexCoord = -1;
GLint iLocProjection = -1;
GLint iLocModelview = -1;

Matrix translation;
Matrix scale;
Matrix negativeTranslation;

static float angleX = 0;
static float angleY = 0;
static float angleZ = 0;
Matrix rotationX;
Matrix rotationY;
Matrix rotationZ;
//Matrix translation;
Matrix modelView;
Matrix projection;
Matrix projectionFBO;

/* Framebuffer variables. */
GLuint iFBO = 0;

/* Application textures. */
GLuint iFBOTex = 0;
GLuint iFrameTex = 0;

int windowWidth = -1;
int windowHeight = -1;

int textureWidth = 1920;
int textureHeight = 1080;
    
int imageWidth = 1920;
int imageHeight = 1088;
uint8_t *image = NULL;

uint8_t *rgbaImage = NULL;

uint8_t* pARGB_front = NULL;
int ARGBcapability_front =0;
int ARGBSize_front = 0;
uint8_t* pARGB_back = NULL;
int ARGBcapability_back =0;
int ARGBSize_back = 0;

Text* text;

#define CLAMP GL_REPEAT

int read_yuv(const char* filename, uint8_t* pdata, int width, int height)
{
    FILE *fp = fopen(filename, "rb");
    if ((pdata == NULL) || (fp == NULL)) return 0;
    LOGI("read yuv-frame(%dx%d) data from %s", width, height, filename );
    fread(pdata, width*height*3/2, 1, fp);
    fclose(fp);
    return 1;
}

bool rotoSetupGraphics(int width, int height)
{
    windowWidth = width;
    windowHeight = height;

    string imagePath = resourceDirectory + imageFilename;   
    image = (uint8_t*)malloc( imageWidth*imageHeight*3/2 );
    if( image == NULL ) {
        LOGE("malloc() error at %s:%i\n", __FILE__, __LINE__); return false;
    }
    if( !read_yuv( imagePath.c_str(), image,  imageWidth, imageHeight ) ) {
        LOGE("read_yuv(%s) error at %s:%i\n", imagePath.c_str(), __FILE__, __LINE__); return false;
    }
    rgbaImage = (uint8_t*)malloc(sizeof(int)*imageWidth*imageHeight);
    yuv2rgb(rgbaImage, image, imageWidth, imageHeight);

    /* Full paths to the shader and texture files */
    string vertexShaderPath = resourceDirectory + rotoVertexShaderFilename;
    string fragmentShaderPath = resourceDirectory + rotoFragmentShaderFilename;

    /* Initialize matrices. */
    /* Make scale matrix to centre texture on screen. */
    translation = Matrix::createTranslation(0.5f, 0.5f, 0.0f);
    scale = Matrix::createScaling(width / (float)textureWidth, height / (float)textureHeight, 1.0f); /* 2.0 makes it smaller, 0.5 makes it bigger. */
    negativeTranslation = Matrix::createTranslation(-0.5f, -0.5f, 0.0f);

    /* Initialize OpenGL ES. */
    GL_CHECK(glEnable(GL_CULL_FACE));
    GL_CHECK(glCullFace(GL_BACK));
    GL_CHECK(glEnable(GL_DEPTH_TEST));
    GL_CHECK(glEnable(GL_BLEND));
    /* Should do src * (src alpha) + dest * (1-src alpha). */
    GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    /* Initialize the Text object and add some text. */
    text = new Text(resourceDirectory.c_str(), windowWidth, windowHeight);
    text->addString(0, 0, "  MACs Simple RotoZoom Player", 255, 255, 255, 255);

    /* Load just base level texture data. */
    GL_CHECK(glActiveTexture(GL_TEXTURE0));
    GL_CHECK(glGenTextures(1, &iFrameTex));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, iFrameTex));
    GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaImage));

    /* Set texture mode. */
    GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D));
    //GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)); /* Default anyway. */
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, CLAMP));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, CLAMP));

    free(rgbaImage);
    rgbaImage = NULL;

    /* Process shaders. */
    GLuint vertexShaderID = 0;
    GLuint fragmentShaderID = 0;
    Shader::processShader(&vertexShaderID, vertexShaderPath.c_str(), GL_VERTEX_SHADER);
    Shader::processShader(&fragmentShaderID, fragmentShaderPath.c_str(), GL_FRAGMENT_SHADER);

    /* Set up shaders. */
    programID = GL_CHECK(glCreateProgram());
    GL_CHECK(glAttachShader(programID, vertexShaderID));
    GL_CHECK(glAttachShader(programID, fragmentShaderID));
    GL_CHECK(glLinkProgram(programID));
    GL_CHECK(glUseProgram(programID));

    /* Vertex positions. */
    iLocPosition = GL_CHECK(glGetAttribLocation(programID, "a_v4Position"));
    if(iLocPosition == -1)
    {
        LOGE("Attribute not found at %s:%i\n", __FILE__, __LINE__);
        return false;
    }
    GL_CHECK(glEnableVertexAttribArray(iLocPosition));

    /* Texture. */
    iLocTexture = GL_CHECK(glGetUniformLocation(programID, "u_s2dTexture"));
    if(iLocTexture == -1)
    {
        LOGD("Warning: Uniform not found at %s:%i\n", __FILE__, __LINE__);
    }
    else
    {
        GL_CHECK(glUniform1i(iLocTexture, 0));
    }

    /* Texture coordinates. */
    iLocTexCoord = GL_CHECK(glGetAttribLocation(programID, "a_v2TexCoord"));
    if(iLocTexCoord == -1)
    {
        LOGD("Warning: Attribute not found at %s:%i\n", __FILE__, __LINE__);
    }
    else
    {
        GL_CHECK(glEnableVertexAttribArray(iLocTexCoord));
    }

    /* Texture matrix. */
    iLocTextureMatrix = GL_CHECK(glGetUniformLocation(programID, "u_m4Texture"));
    if(iLocTextureMatrix == -1)
    {
        LOGD("Warning: Uniform not found at %s:%i\n", __FILE__, __LINE__);
    }
    else
    {
        GL_CHECK(glUniformMatrix4fv(iLocTextureMatrix, 1, GL_FALSE, scale.getAsArray()));
    }

    return true;
}

void rotoRenderFrame(void) 
{
    static float angleZTexture = 0.0f;
    static float angleZOffset = 0.0f;
    static float angleZoom = 0.0f;
    static Vec4f radius = {0.0f, 1.0f, 0.0f, 1.0f};

    /* Select our shader program. */
    GL_CHECK(glUseProgram(programID));

    /* Set up vertex positions. */
    GL_CHECK(glEnableVertexAttribArray(iLocPosition));
    GL_CHECK(glVertexAttribPointer(iLocPosition, 3, GL_FLOAT, GL_FALSE, 0, quadVertices));

    /* And texture coordinate data. */
    if(iLocTexCoord != -1)
    {
        GL_CHECK(glEnableVertexAttribArray(iLocTexCoord));
        GL_CHECK(glVertexAttribPointer(iLocTexCoord, 2, GL_FLOAT, GL_FALSE, 0, quadTextureCoordinates));
    }

    /* Reset viewport to the EGL window surface's dimensions. */
    if( !half ) { GL_CHECK(glViewport(0, 0, windowWidth, windowHeight)); }
    else if( half == 1 ) { GL_CHECK(glViewport(0,0, windowWidth/2, windowHeight/2)); }
    else if( half == 2 ) { GL_CHECK(glViewport(0,0, windowWidth/2, windowHeight/2)); }

    /* Clear the screen on the EGL surface. */
    //GL_CHECK(glClearColor(0.5f, 0.5f, 0.0f, 0.5f));
    GL_CHECK(glClearColor(0.0f, 0.0f, 0.5f, 1.0));
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    /* Construct a rotation matrix for rotating the texture about its centre. */
    Matrix rotateTextureZ = Matrix::createRotationZ(angleZTexture);

    Matrix rotateOffsetZ = Matrix::createRotationZ(angleZOffset);

    Vec4f offset = Matrix::vertexTransform(&radius, &rotateOffsetZ);

    /* Construct offset translation. */
    Matrix translateTexture = Matrix::createTranslation(offset.x, offset.y, offset.z);

    /* Construct zoom matrix. */
    Matrix zoom = Matrix::createScaling(sinf(degreesToRadians(angleZoom)) * 0.75f + 1.25f, sinf(degreesToRadians(angleZoom)) * 0.75f + 1.25f, 1.0f);

    /* Create texture matrix. Operations happen bottom-up order. */
    Matrix textureMovement = Matrix::identityMatrix * translation; /* Translate texture back to original position. */
    textureMovement = textureMovement * rotateTextureZ;            /* Rotate texture about origin. */
    textureMovement = textureMovement * translateTexture;          /* Translate texture away from origin. */
    textureMovement = textureMovement * zoom;                      /* Zoom the texture. */
    textureMovement = textureMovement * scale;                     /* Scale texture down in size from fullscreen to 1:1. */
    textureMovement = textureMovement * negativeTranslation;       /* Translate texture to be centred on origin. */

    GL_CHECK(glUniformMatrix4fv(iLocTextureMatrix, 1, GL_FALSE, textureMovement.getAsArray()));

    {
        AutoLock lock( mutex );
        /* Ensure the correct texture is bound to texture unit 0. */
        GL_CHECK(glActiveTexture(GL_TEXTURE0));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, iFrameTex));
        if( pARGB_front != NULL )
        {
		GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, FBO_WIDTH,FBO_HEIGHT,GL_RGBA, GL_UNSIGNED_BYTE, pARGB_back ));
		//GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FBO_WIDTH,FBO_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, pARGB_back));
        }

        /* And draw. */
        GL_CHECK(glDrawElements(GL_TRIANGLE_STRIP, sizeof(quadIndices) / sizeof(GLubyte), GL_UNSIGNED_BYTE, quadIndices));
    }

    /* Draw any text. */
    text->draw();

    /* Update rotation angles for animating. */
    angleZTexture += 1;
    angleZOffset += 1;
    angleZoom += 1;

    if(angleZTexture >= 360) angleZTexture -= 360;
    if(angleZTexture < 0) angleZTexture += 360;

    if(angleZOffset >= 360) angleZOffset -= 360;
    if(angleZOffset < 0) angleZOffset += 360;

    if(angleZoom >= 360) angleZoom -= 360;
    if(angleZoom < 0) angleZoom += 360;
}

int rotoRenderMain(void)
{
    /* Intialize the Platform object for platform specific functions. */
    Platform* platform = Platform::getInstance();

    /* Initialize windowing system. */
    if(!half) platform->createWindow(WINDOW_W, WINDOW_H);
    else if(half == 1) platform->createWindow(WINDOW_W/2, WINDOW_H/2);
    else if(half == 2) platform->createWindow(WINDOW_W/2, WINDOW_H);

    /* Initialize EGL. */
    EGLRuntime::initializeEGL(EGLRuntime::OPENGLES2);
    EGL_CHECK(eglMakeCurrent(EGLRuntime::display, EGLRuntime::surface, EGLRuntime::surface, EGLRuntime::context));
    EGL_CHECK(eglSwapInterval(EGLRuntime::display, 1));

    /* Initialize OpenGL ES graphics subsystem. */
    rotoSetupGraphics(WINDOW_W, WINDOW_H);

    /* Timer variable to calculate FPS. */
    Timer fpsTimer;
    fpsTimer.reset();

    bool end = false;
    /* The rendering loop to draw the scene. */
    while(!end)
    {
        {
            AutoLock lock( mutex );
            if(decoderState >= DECODING_FINISHED) {
                end = true;
            }
        }

        /* If something has happened to the window, end the sample. */
        if(platform->checkWindow() != Platform::WINDOW_IDLE)
        {
            end = true;
        }

        /* Calculate FPS. */
        float FPS = fpsTimer.getFPS();
        if(fpsTimer.isTimePassed(1.0f))
        {
            fps = FPS;
            //printf(" FPS: %.1f", FPS);
            //LOGI("FPS:\t%.1f\n", FPS);
        }

        /* Render a single frame */
        rotoRenderFrame();
        //fb_waitforsync();

        /* 
         * Push the EGL surface color buffer to the native window.
         * Causes the rendered graphics to be displayed on screen.
         */
        eglSwapBuffers(EGLRuntime::display, EGLRuntime::surface);
    }

    if (pARGB_front) {
        free(pARGB_front);
        pARGB_front = NULL;
    }

    /* Shut down OpenGL ES. */
    /* Delete the texture. */
    GL_CHECK(glDeleteTextures(1, &iFrameTex));

    /* Shut down Text. */
    delete text;

    /* Shut down EGL. */
    EGLRuntime::terminateEGL();

    /* Shut down windowing system. */
    platform->destroyWindow();

    /* Shut down the Platform object*/
    delete platform;

    return 0;
}

//===========================================================================


bool FBOSetupGraphics(int width, int height)
{
    windowWidth = width;
    windowHeight = height;

    string imagePath = resourceDirectory + imageFilename;
    image = (uint8_t*)malloc( imageWidth*imageHeight*3/2 );
    if( image == NULL ) {
        LOGE("malloc() error at %s:%i\n", __FILE__, __LINE__); return false;
    }
    if( !read_yuv( imagePath.c_str(), image,  imageWidth, imageHeight ) ) {
        LOGE("read_yuv() error at %s:%i\n", __FILE__, __LINE__); return false;
    }
    rgbaImage = (uint8_t*)malloc(sizeof(int)*imageWidth*imageHeight);
    yuv2rgb(rgbaImage, image, imageWidth, imageHeight);

    /* Full paths to the shader files */
    string vertexShaderPath = resourceDirectory + FBOVertexShaderFilename; 
    string fragmentShaderPath = resourceDirectory + FBOFragmentShaderFilename;

    /* Initialize matrices. */
    projection = Matrix::matrixPerspective(45.0f, windowWidth/(float)windowHeight, 0.01f, 100.0f);
    projectionFBO = Matrix::matrixPerspective(45.0f, (FBO_WIDTH / (float)FBO_HEIGHT), 0.01f, 100.0f);
    /* Move cube 2 further away from camera. */
    translation = Matrix::createTranslation(0.0f, 0.0f, -1.25f);

    /* Initialize OpenGL ES. */
    GL_CHECK(glEnable(GL_CULL_FACE));
    GL_CHECK(glCullFace(GL_BACK));
    GL_CHECK(glEnable(GL_DEPTH_TEST));
    GL_CHECK(glEnable(GL_BLEND));
    /* Should do src * (src alpha) + dest * (1-src alpha). */
    GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    /* Initialize the Text object and add some text. */
    text = new Text(resourceDirectory.c_str(), windowWidth, windowHeight);
    text->addString(0, 0, "  MACs Simple FrameBuffer Object (FBO) Player", 255, 255, 0, 255);

    /* Initialize Frame texture. */
    GL_CHECK(glActiveTexture(GL_TEXTURE0));
    GL_CHECK(glGenTextures(1, &iFrameTex));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, iFrameTex));
    /* Set filtering. */
    GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FBO_WIDTH,FBO_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaImage));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, CLAMP));
    GL_CHECK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, CLAMP));

    free(rgbaImage);
    rgbaImage = NULL;

    /* Initialize FBO texture. */
    GL_CHECK(glGenTextures(1, &iFBOTex));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, iFBOTex));
    /* Set filtering. */
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FBO_WIDTH, FBO_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));

    /* Initialize FBOs. */
    GL_CHECK(glGenFramebuffers(1, &iFBO));

    /* Render to framebuffer object. */
    /* Bind our framebuffer for rendering. */
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, iFBO));

    /* Attach texture to the framebuffer. */
    GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, iFBOTex, 0));

    /* Check FBO is OK. */
    GLenum iResult = GL_CHECK(glCheckFramebufferStatus(GL_FRAMEBUFFER));
    if(iResult != GL_FRAMEBUFFER_COMPLETE)
    {
        LOGE("Framebuffer incomplete at %s:%i\n", __FILE__, __LINE__);
        return false;
    }

    /* Unbind framebuffer. */
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));

    /* Process shaders. */
    Shader::processShader(&vertexShaderID, vertexShaderPath.c_str(), GL_VERTEX_SHADER);
    Shader::processShader(&fragmentShaderID, fragmentShaderPath.c_str(), GL_FRAGMENT_SHADER);

    /* Set up shaders. */
    programID = GL_CHECK(glCreateProgram());
    GL_CHECK(glAttachShader(programID, vertexShaderID));
    GL_CHECK(glAttachShader(programID, fragmentShaderID));
    GL_CHECK(glLinkProgram(programID));
    GL_CHECK(glUseProgram(programID));

    /* Vertex positions. */
    iLocPosition = GL_CHECK(glGetAttribLocation(programID, "a_v4Position"));
    if(iLocPosition == -1)
    {
        LOGE("Attribute not found at %s:%i\n", __FILE__, __LINE__);
        return false;
    }
    GL_CHECK(glEnableVertexAttribArray(iLocPosition));

    /* Texture mix. */
    iLocTextureMix = GL_CHECK(glGetUniformLocation(programID, "u_fTex"));
    if(iLocTextureMix == -1)
    {
        LOGD("Warning: Uniform not found at %s:%i\n", __FILE__, __LINE__);
    }
    else 
    {
        GL_CHECK(glUniform1f(iLocTextureMix, 0.0));
    }

    /* Texture. */
    iLocTexture = GL_CHECK(glGetUniformLocation(programID, "u_s2dTexture"));
    if(iLocTexture == -1)
    {
        LOGD("Warning: Uniform not found at %s:%i\n", __FILE__, __LINE__);
    }
    else 
    {
        GL_CHECK(glUniform1i(iLocTexture, 0));
    }

    /* Vertex colors. */
    iLocFillColor = GL_CHECK(glGetAttribLocation(programID, "a_v4FillColor"));
    if(iLocFillColor == -1)
    {
        LOGD("Warning: Attribute not found at %s:%i\n", __FILE__, __LINE__);
    }
    else 
    {
        GL_CHECK(glEnableVertexAttribArray(iLocFillColor));
    }

    /* Texture coords. */
    iLocTexCoord = GL_CHECK(glGetAttribLocation(programID, "a_v2TexCoord"));
    if(iLocTexCoord == -1)
    {
        LOGD("Warning: Attribute not found at %s:%i\n", __FILE__, __LINE__);
    }
    else 
    {
        GL_CHECK(glEnableVertexAttribArray(iLocTexCoord));
    }

    /* Projection matrix. */
    iLocProjection = GL_CHECK(glGetUniformLocation(programID, "u_m4Projection"));
    if(iLocProjection == -1)
    {
        LOGD("Warning: Uniform not found at %s:%i\n", __FILE__, __LINE__);
    }
    else 
    {
        GL_CHECK(glUniformMatrix4fv(iLocProjection, 1, GL_FALSE, projection.getAsArray()));
    }

    /* Modelview matrix. */
    iLocModelview = GL_CHECK(glGetUniformLocation(programID, "u_m4Modelview"));
    if(iLocModelview == -1)
    {
        LOGD("Warning: Uniform not found at %s:%i\n", __FILE__, __LINE__);
    }
    /* We pass this for each object, below. */

    return true;
}

void FBORenderFrame(void)
{
    /* Both main window surface and FBO use the same shader program. */
    GL_CHECK(glUseProgram(programID));

    /* Both drawing surfaces also share vertex data. */
    GL_CHECK(glEnableVertexAttribArray(iLocPosition));
    GL_CHECK(glVertexAttribPointer(iLocPosition, 3, GL_FLOAT, GL_FALSE, 0, cubeVertices));

    /* Including color data. */
    if(iLocFillColor != -1)
    {
        GL_CHECK(glEnableVertexAttribArray(iLocFillColor));
        GL_CHECK(glVertexAttribPointer(iLocFillColor, 4, GL_FLOAT, GL_FALSE, 0, cubeColors));
    }

    /* And texture coordinate data. */
    if(iLocTexCoord != -1)
    {
        GL_CHECK(glEnableVertexAttribArray(iLocTexCoord));
        GL_CHECK(glVertexAttribPointer(iLocTexCoord, 2, GL_FLOAT, GL_FALSE, 0, cubeTextureCoordinates));
    }

    /* Bind the FrameBuffer Object. */
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, iFBO));

    /* Set the viewport according to the FBO's texture. */
    GL_CHECK(glViewport(0, 0, FBO_WIDTH, FBO_HEIGHT));

    /* Clear screen on FBO. */
    GL_CHECK(glClearColor(0.15f, 0.15f, 0.15f, 1.0));
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    /* Create rotation matrix specific to the FBO's cube. */
    //rotationX = Matrix::createRotationX(-angleZ);
    //rotationY = Matrix::createRotationY(-angleY);
    //rotationZ = Matrix::createRotationZ(-angleX);
    rotationX = Matrix::createRotationX(0);
    rotationY = Matrix::createRotationY(0);
    rotationZ = Matrix::createRotationZ(90);

    /* Rotate about origin, then translate away from camera. */
    modelView = translation * rotationX;
    modelView = modelView * rotationY;
    modelView = modelView * rotationZ;

    /* Load FBO-specific projection and modelview matrices. */
    GL_CHECK(glUniformMatrix4fv(iLocModelview, 1, GL_FALSE, modelView.getAsArray()));
    GL_CHECK(glUniformMatrix4fv(iLocProjection, 1, GL_FALSE, projectionFBO.getAsArray()));

    /* The FBO cube doesn't get textured so zero the texture mix factor. */
    if(iLocTextureMix != -1)
    {
        GL_CHECK(glUniform1f(iLocTextureMix, 1.0));
    }

    {
        AutoLock lock( mutex );
        GL_CHECK(glActiveTexture(GL_TEXTURE0));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, iFrameTex));

        if( pARGB_front != NULL )
        {
		GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, FBO_WIDTH,FBO_HEIGHT,GL_RGBA, GL_UNSIGNED_BYTE, pARGB_back ));
		//GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FBO_WIDTH,FBO_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, pARGB_back));
        }

        /* Now draw the colored cube to the FrameBuffer Object. */
        GL_CHECK(glDrawElements(GL_TRIANGLE_STRIP, sizeof(cubeIndices) / sizeof(GLubyte), GL_UNSIGNED_BYTE, cubeIndices));
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER,0));
    }

    /* And unbind the FrameBuffer Object so subsequent drawing calls are to the EGL window surface. */

    /* Reset viewport to the EGL window surface's dimensions. */
    if( !half ) { GL_CHECK(glViewport(0, 0, windowWidth, windowHeight)); }
    else if( half == 1 ) { GL_CHECK(glViewport(0, 0, windowWidth/2, windowHeight/2)); }
    else if( half == 2 ) { GL_CHECK(glViewport(windowWidth/2, 0, windowWidth/2, windowHeight/2)); }

    /* Clear the screen on the EGL surface. */
    GL_CHECK(glClearColor(0.0f, 0.0f, 0.5f, 1.0));
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    /* Construct different rotation for main cube. */
    rotationX = Matrix::createRotationX(angleX);
    rotationY = Matrix::createRotationY(angleY);
    rotationZ = Matrix::createRotationZ(angleZ);

    /* Rotate about origin, then translate away from camera. */
    modelView = translation * rotationX;
    modelView = modelView * rotationY;
    modelView = modelView * rotationZ;

    /* Load EGL window-specific projection and modelview matrices. */
    GL_CHECK(glUniformMatrix4fv(iLocModelview, 1, GL_FALSE, modelView.getAsArray()));
    GL_CHECK(glUniformMatrix4fv(iLocProjection, 1, GL_FALSE, projection.getAsArray()));

    /* For the main cube, we use texturing so set the texture mix factor to 1. */
    if(iLocTextureMix != -1)
    {
        GL_CHECK(glUniform1f(iLocTextureMix, 1.0));
    }

    /* Ensure the correct texture is bound to texture unit 0. */
    GL_CHECK(glActiveTexture(GL_TEXTURE0));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, iFBOTex));

    /* And draw the cube. */
    GL_CHECK(glDrawElements(GL_TRIANGLE_STRIP, sizeof(cubeIndices) / sizeof(GLubyte), GL_UNSIGNED_BYTE, cubeIndices));

    /* Draw any text. */
    text->draw();

    /* Update cube's rotation angles for animating. */
    angleX += 3;
    angleY += 2;
    angleZ += 1;

    if(angleX >= 360) angleX -= 360;
    if(angleY >= 360) angleY -= 360;
    if(angleZ >= 360) angleZ -= 360;
}

int FBORenderMain(void)
{
    /* Intialize the Platform object for platform specific functions. */
    Platform* platform = Platform::getInstance();
    /* Initialize windowing system. */
    if(!half) platform->createWindow(WINDOW_W, WINDOW_H);
    else if(half == 1) platform->createWindow(WINDOW_W/2, WINDOW_H/2);
    else if(half == 2) platform->createWindow(WINDOW_W, WINDOW_H/2);

    /* Initialize EGL. */
    EGLRuntime::initializeEGL(EGLRuntime::OPENGLES2);
    EGL_CHECK(eglMakeCurrent(EGLRuntime::display, EGLRuntime::surface, EGLRuntime::surface, EGLRuntime::context));
    EGL_CHECK(eglSwapInterval(EGLRuntime::display, 1));

    /* Initialize OpenGL ES graphics subsystem. */
    FBOSetupGraphics(WINDOW_W, WINDOW_H);

    /* Timer variable to calculate FPS. */
    Timer fpsTimer;
    fpsTimer.reset();

    bool end = false;
    /* The rendering loop to draw the scene. */
    while(!end)
    {
        {
            AutoLock lock( mutex );
            if(decoderState >= DECODING_FINISHED) {
                end = true;
            }
        }

        /* If something has happened to the window, end the sample. */
        if(platform->checkWindow() != Platform::WINDOW_IDLE)
        {
            end = true;
        }
        
        /* Calculate FPS. */
        float FPS = fpsTimer.getFPS();
        if(fpsTimer.isTimePassed(1.0f))
        {
            fps = FPS;
            //printf("FPS: %.1f", FPS);
        }

        /* Render a single frame */
        FBORenderFrame();
     
        //fb_waitforsync();
        /* 
         * Push the EGL surface color buffer to the native window.
         * Causes the rendered graphics to be displayed on screen.
         */
        eglSwapBuffers(EGLRuntime::display, EGLRuntime::surface);
    }

    if (pARGB_front) {
        free(pARGB_front);
        pARGB_front = NULL;
    }

    /* Shut down OpenGL ES. */
    /* Shut down Text. */
    delete text;

    /* Shut down EGL. */
    EGLRuntime::terminateEGL();

    /* Shut down windowing system. */
    platform->destroyWindow();

    /* Shut down the Platform object. */
    delete platform;

    return 0;
}

//===========================================================================

void printhex( uint8_t *buf, int size ) {
    int x, y;
    for( x=1; x<=size; x++ ) {
        if( x == 1 ) printf( "%04x  ", x-1 );
        printf( "%02x ", buf[x-1] );
        if( x % 8 == 0 ) printf( " " ); 
        if( x % 16 == 0 ) {
            printf( " " );
            for( y = x - 15; y <= x; y++ ) {
                if( isprint( buf[y-1] ) ) printf( "%c", buf[y-1] );
                else printf( "." );
                if( y % 8 == 0 ) printf( " " );
            }
            if( x < size ) printf( "\n%04x  ", x );
        }
    }
    x--;
    if( x % 16 != 0 ) {
        for( y = x+1; y <= x + (16-(x % 16)); y++ ) {
            printf( "   " );
            if( y % 8 == 0 ) printf( " " );
        };
        printf( " " );
        for( y = (x+1) - (x % 16); y <= x; y++ ) {
            if( isprint( buf[y-1] ) ) printf( "%c", buf[y-1] );
            else printf( "." );
            if( y % 8 == 0 ) printf( " " );
        }
    }
    printf( "\n" );
}

void yuv2rgb(uint8_t* rgb, uint8_t *yuv420sp, int width, int height) {
    struct _rga_img_info_t src, dst;
    memset(&src, 0, sizeof(struct _rga_img_info_t));
    memset(&dst, 0, sizeof(struct _rga_img_info_t));
   
    src.yrgb_addr = (int)yuv420sp;
    src.vir_w = (width + 15)&(~15);
    src.vir_h = (height + 15)&(~15);
    src.format = RK_FORMAT_YCbCr_420_SP;
    src.act_w = width;
    src.act_h = height;
    src.x_offset = 0;
    src.y_offset = 0;

    dst.yrgb_addr = (uint32_t)rgb;
    dst.vir_w = (width + 31)&(~31);
    dst.vir_h = (height + 15)&(~15);
    dst.format = RK_FORMAT_RGBA_8888;
    dst.act_w = width;
    dst.act_h = height;
    dst.x_offset = 0;
    dst.y_offset = 0;

    int ret = copyBit.draw(&src, &dst, RK_MMU_SRC_ENABLE | RK_MMU_DST_ENABLE | RK_BT_601_MPEG);
    return;
}

void yuv2rgb_sw(int *rgb, uint8_t *yuv420sp, int width, int height) {
    int frameSize = width * height;
    int j, yp, uvp, u, v, i, y, y1192, r, g, b;

    for (j = 0, yp = 0; j < height; j++) {
        uvp = frameSize + (j >> 1) * width, u = 0, v = 0;
        for (i = 0; i < width; i++, yp++) {
            y = (0xff & ((int) yuv420sp[yp])) - 16;
            if (y < 0) {
                y = 0;
            }
            if ((i & 1) == 0) {
                v = (0xff & yuv420sp[uvp++]) - 128;
                u = (0xff & yuv420sp[uvp++]) - 128;
            }

            y1192 = 1192 * y;
            r = (y1192 + 1634 * v);
            g = (y1192 - 833 * v - 400 * u);
            b = (y1192 + 2066 * u);

            if (r < 0) {
                r = 0;
            } else if (r > 262143) {
                r = 262143;
            }
            if (g < 0) {
                g = 0;
            } else if (g > 262143) {
                g = 262143;
            }
            if (b < 0) {
                b = 0;
            } else if (b > 262143) {
                b = 262143;
            }

            rgb[yp] = 0xff000000 | ((r << 6) & 0xff0000) | ((g >> 2) & 0xff00)
                    | ((b >> 10) & 0xff);
            //rgb[yp] = ((r << 14) & 0xff000000) | ((g << 6) & 0xff0000)
            //        | ((b >> 2) | 0xff00);
        }
    }
}

int write_truecolor_tga( int* data, int width,  int height, int frame_count ) {
    char filename[256];
    sprintf( filename, "sintel.%d.tga", frame_count );
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) return 0;

    printf("write %d frame(%dx%d) data to %s\n", frame_count, width, height, filename );
    char header[ 18 ] = { 0 }; // char = byte
    header[ 2 ] = 2; // truecolor
    header[ 12 ] = width & 0xFF;
    header[ 13 ] = (width >> 8) & 0xFF;
    header[ 14 ] = height & 0xFF;
    header[ 15 ] = (height >> 8) & 0xFF;
    header[ 16 ] = 24; // bits per pixel
    fwrite((const char*)&header, 1, sizeof(header), fp);

    int x,y;
    for (y = height -1; y >= 0; y--)
        for (x = 0; x < width; x++) {
            char b = (data[x+(y*width)] & 0x0000FF);
            char g = (data[x+(y*width)] & 0x00FF00) >> 8;
            char r = (data[x+(y*width)] & 0xFF0000) >> 16;
            putc((int)(r & 0xFF),fp);
            putc((int)(g & 0xFF),fp);
            putc((int)(b & 0xFF),fp);
        }

    static const char footer[ 26 ] =
        "\0\0\0\0" // no extension area
        "\0\0\0\0" // no developer directory
        "TRUEVISION-XFILE" // yep, this is a TGA file
        ".";
    fwrite((const char*)&footer, 1, sizeof(footer), fp);

    fclose(fp);
    return 1;
}

int readsave_frames(int videoStreamIdx, AVFormatContext *pFormatCtx,
                    AVCodecContext *pCodecCtx, AVCodec *pCodec)
{
    int             i;
    AVPacket        packet;

    memset(&demoPkt, 0, sizeof(VideoPacket_t));
    pkt = &demoPkt;
    pkt->data = NULL;
    pkt->pts = VPU_API_NOPTS_VALUE;
    pkt->dts = VPU_API_NOPTS_VALUE;

    memset(&decOut, 0, sizeof(DecoderOut_t));
    pOut = &decOut;
    pOut->data = (RK_U8*)(malloc)(sizeof(VPU_FRAME));
    if (pOut->data ==NULL) {
        printf("DECODE_ERR_RET(ERROR_MEMORY);\n"); return -1;
    }
    memset(pOut->data, 0, sizeof(VPU_FRAME));

    ret = vpu_open_context(&ctx);
    if (ret || (ctx ==NULL)) {
        printf("DECODE_ERR_RET(ERROR_MEMORY);\n"); return -1;
    }

    ctx->codecType = CODEC_DECODER;
    ctx->videoCoding = OMX_ON2_VIDEO_CodingAVC;
    ctx->width = 1920;
    ctx->height = 1080;
    ctx->no_thread = 1;
    if ((ret = ctx->init(ctx, pExtra, extraSize)) !=0) {
       printf( "DECODE_ERR_RET(ERROR_INIT_VPU);\n"); return -1;
    }

    Timer fpsTimer;
    fpsTimer.reset();

    RK_U8 *data = NULL;
    for(i=0; !(ctx->decoder_err) && av_read_frame(pFormatCtx, &packet) >= 0;) {
        {
            AutoLock lock( mutex );
            if(decoderState >= DECODING_FINISHED) {
                break;
            }
        }
        // Is this a packet from the video stream?
        if(packet.stream_index==videoStreamIdx) {
            i++;

            data = packet.data;
            pkt_size = packet.size;

            if (pkt->data ==NULL) {
                pkt->data = (RK_U8*)(malloc)(pkt_size);
                if (pkt->data ==NULL) {
                    printf("DECODE_ERR_RET(ERROR_MEMORY);\n"); return -1;
                }
                pkt->capability = pkt_size;
            }
            if (pkt->capability <((RK_U32)pkt_size)) {
                pkt->data = (RK_U8*)(realloc)((void*)(pkt->data), pkt_size);
                if (pkt->data ==NULL) {
                    printf("DECODE_ERR_RET(ERROR_MEMORY);\n"); return -1;
                }
                pkt->capability = pkt_size;
            }
            memcpy(pkt->data,data,pkt_size);
            pkt->size = pkt_size;
            pkt->pts = fakeTimeUs;
            fakeTimeUs += 40000;

            do {
                /// Decode video frame
                pOut->size = 0;
                if ((ret = ctx->decode(ctx, pkt, pOut)) !=0) {
                    printf("DECODE_ERR_RET(ERROR_VPU_DECODE);\n"); return -1;
                } else {
                  ;
                }
                // Did we get a video frame?
                if ((pOut->size) && (pOut->data)) {
                    VPU_FRAME *frame = (VPU_FRAME *)(pOut->data);
                    printf("\rFrame [%d]: pOut->size=%d, pkt_pts=%d, ret=%d; FPS: %.1f", i,(int)pOut->size,(int) pkt->pts,(int) ret, fps ); fflush(stdout);

                    VPUMemLink(&frame->vpumem);

                    RK_U32 wAlign16 = ((frame->DisplayWidth+ 15) & (~15));
                    RK_U32 hAlign16 = ((frame->DisplayHeight + 15) & (~15));
                    RK_U8* frameImage = (RK_U8*)(frame->vpumem.vir_addr);
                    if( output == OUTPUT_DISPLAY ) {
                        //fb_waitforsync();
                        yuv2rgb(fb_mem, frameImage, windowWidth, windowHeight);
                        float FPS = fpsTimer.getFPS();
                        if(fpsTimer.isTimePassed(1.0f)) {
                            fps = FPS;
                            //printf("FPS: %.1f", FPS);
                        }
                    } else {
                        ARGBSize_back = sizeof(int)*wAlign16*hAlign16;
                        if (pARGB_back ==NULL) {
                            pARGB_back = (RK_U8*)(malloc)(ARGBSize_back);
                            if (pARGB_back ==NULL) {
                                printf("DECODE_ERR_RET(ERROR_MEMORY);\n"); 
                                return -1;
                            }
                            ARGBcapability_back = ARGBSize_back;
                        }
                        if (ARGBcapability_back <((RK_U32)ARGBSize_back)) {
                            pARGB_back = (RK_U8*)(realloc)((void*)(pARGB_back),ARGBSize_back);
                            if (pARGB_back ==NULL) {
                                printf("DECODE_ERR_RET(ERROR_MEMORY);\n"); 
                                return -1;
                            }
                            ARGBcapability_back = ARGBSize_back;
                        }
                        yuv2rgb(pARGB_back, frameImage, wAlign16, hAlign16);

			// uncomment to write every 100th frame to tga file
			//if((i%100) == 1) write_truecolor_tga((int*)pARGB_back,FBO_WIDTH,FBO_HEIGHT,i);

                        mutex.lock();
                        {
				// store front temporary
				uint8_t* pARGB_tmp = pARGB_front;
				int ARGBcapability_tmp = ARGBcapability_front;
				int ARGBSize_tmp = ARGBSize_front;

                                // front = back
				pARGB_front = pARGB_back;
				ARGBcapability_front = ARGBcapability_back;
				ARGBSize_front = ARGBSize_back;

				// back = stored front
				pARGB_back = pARGB_tmp;
				ARGBcapability_back = ARGBcapability_tmp;
				ARGBSize_back = ARGBSize_tmp;
                        }
                        mutex.unlock();
                    }
                    VPUFreeLinear(&frame->vpumem);
                    pOut->size = 0;
                }
            } while((pkt->size>0) && !(ctx->decoder_err));
        }
        // Free the packet that was allocated by av_read_frame
        av_free_packet(&packet);
    }
    printf("\n");
    if (pARGB_back) {
        free(pARGB_back);
        pARGB_back = NULL;
    }
    if (pkt && pkt->data) {
        free(pkt->data);
        pkt->data = NULL;
    }
    if (pOut && (pOut->data)) {
        free(pOut->data);
        pOut->data = NULL;
    }
    if (ctx) {
        //vpu_close_context(&ctx);
        ctx = NULL;
    }
    return 0;
}

int decodeMain(void) {
    AVFormatContext *pFormatCtx;
    int             videoStreamIdx;
    AVCodecContext  *pCodecCtx;
    AVCodec         *pCodec;

    // Register all formats and codecs
    av_register_all();

    pFormatCtx = avformat_alloc_context();

    /// Open video file
    if(avformat_open_input(&pFormatCtx, movieFileName, 0, NULL) != 0) {
        printf("avformat_open_input failed: Couldn't open file\n"); return -1;
    }

    /// Retrieve stream information
    if(avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        printf("avformat_find_stream_info failed: Couldn't find stream information\n"); return -1;
    }

    /// Dump information about file onto standard error
    //av_dump_format(pFormatCtx, 0, movieFileName, 0);

    /// Find the first video stream
    {
        int i = 0;
        videoStreamIdx=-1;
        for(i=0; pFormatCtx->nb_streams; i++)
        {
            if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) { //CODEC_TYPE_VIDEO
                videoStreamIdx=i;
                break;
            }
        }
    }
    /// Check if video stream is found
    if(videoStreamIdx==-1)
        return -1; // Didn't find a video stream

    /// Get a pointer to the codec context for the video stream
    pCodecCtx = pFormatCtx->streams[videoStreamIdx]->codec;

    /// Find the decoder for the video stream
    pCodec = avcodec_find_decoder( pCodecCtx->codec_id);
    if(pCodec==NULL) {
        fprintf(stderr, "Unsupported codec!\n");
        return -1; // Codec not found
    }

    /// Copy and pad extradata
    int extraDataSize = pCodecCtx->extradata_size;
    uint8_t* pExtraData = (uint8_t*)malloc(FF_INPUT_BUFFER_PADDING_SIZE+extraDataSize);
    memset(pExtraData,0,FF_INPUT_BUFFER_PADDING_SIZE+extraDataSize);
    memcpy(pExtraData, pCodecCtx->extradata, extraDataSize);
    //printhex(pExtraData, extraDataSize );
    pExtra = pExtraData;
    extraSize = extraDataSize;

    /// Open codec
    if( avcodec_open2(pCodecCtx, pCodec, NULL) < 0 )
        return -1; // Could not open codec

    // Read frames and save them to disk
    if ( readsave_frames(videoStreamIdx, pFormatCtx, pCodecCtx, pCodec) < 0)
        return -1;

    /// Close the codec
    avcodec_close(pCodecCtx);

    /// Close the video file
    avformat_close_input(&pFormatCtx);

    free(pExtraData);

    stop++;

    return 0;
}

//===========================================================================

int fbMain(void) {
    windowWidth = fb_var.xres;
    windowHeight = fb_var.yres;
    //printf("display width=%d, height=%d\n",windowWidth,windowHeight);
    
    string imagePath = resourceDirectory + imageFilename;
    image = (uint8_t*)malloc( imageWidth*imageHeight*3/2 );
    if( image == NULL ) {
        LOGE("malloc() error at %s:%i\n", __FILE__, __LINE__); return -1;
    }
    if( !read_yuv( imagePath.c_str(), image,  imageWidth, imageHeight ) ) {
        LOGE("read_yuv() error at %s:%i\n", __FILE__, __LINE__); return -1;
    }
    yuv2rgb(fb_mem, image, windowWidth, windowHeight);

    Timer fpsTimer;
    fpsTimer.reset();

    bool end = false;
    while(!end)
    {
        mutex.lock();
        if(decoderState >= DECODING_FINISHED) {
            end = true;
            break;
        }
        mutex.unlock();
        float FPS = fpsTimer.getFPS();
        if(fpsTimer.isTimePassed(1.0f)) {
            //printf("FPS: %.1f", FPS);
        }
        sleep(1);
    }

    if(!image) free(image);
    image = NULL;

    return 0;
}

//===========================================================================

class RenderThread : public Thread {
    virtual void run() {
        if( output == OUTPUT_DISPLAY ) {
            fbMain();
        } else if( output == OUTPUT_OPENGL_ROTO ) {
            rotoRenderMain();
        } else if( output == OUTPUT_OPENGL_FBO ) {
            FBORenderMain();
        }
    }
};

class DecoderThread : public Thread {
	virtual void run() {
		decodeMain();
	}
};

#include <stdio.h>
#include <termios.h>
#include <unistd.h>

int mygetch ( void )
{
  int ch;
  struct termios oldt, newt;

  tcgetattr ( STDIN_FILENO, &oldt );
  newt = oldt;
  newt.c_lflag &= ~( ICANON | ECHO );
  tcsetattr ( STDIN_FILENO, TCSANOW, &newt );
  ch = getchar();
  tcsetattr ( STDIN_FILENO, TCSANOW, &oldt );

  return ch;
}

#include <stdio.h>
#include <fcntl.h>
#include <termios.h>

#define NB_ENABLE 0
#define NB_DISABLE 1

int kbhit()
{
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}

void nonblock(int state)
{
    struct termios ttystate;
 
    //get the terminal state
    tcgetattr(STDIN_FILENO, &ttystate);
 
    if (state==NB_ENABLE)
    {
        //turn off canonical mode
        ttystate.c_lflag &= ~ICANON;
        //minimum of number input read.
        ttystate.c_cc[VMIN] = 1;
    }
    else if (state==NB_DISABLE)
    {
        //turn on canonical mode
        ttystate.c_lflag |= ICANON;
    }
    //set the terminal attributes.
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
}

int main(int argc, char *argv[]) {
    machybris_init(argc,argv);

    output = OUTPUT_DISPLAY; // assume fd
    half = false;
    if(argc == 2) {
        movieFileName = argv[1]; 
    } else if( argc == 3 ) {
        movieFileName = argv[2]; 
        if(strcmp(argv[1],"fbo") == 0) output = OUTPUT_OPENGL_FBO; 
        else if(strcmp(argv[1],"roto") == 0) output = OUTPUT_OPENGL_ROTO; 
        else if(strcmp(argv[1],"fd") == 0) output = OUTPUT_DISPLAY; 
        else if(strcmp(argv[1],"fbo1") == 0) { output = OUTPUT_OPENGL_FBO; half = 1; }
        else if(strcmp(argv[1],"roto1") == 0) { output = OUTPUT_OPENGL_ROTO; half = 1; }
        else if(strcmp(argv[1],"fbo2") == 0) { output = OUTPUT_OPENGL_FBO; half = 2; }
        else if(strcmp(argv[1],"roto2") == 0) { output = OUTPUT_OPENGL_ROTO; half = 2; }
        else {
            printf("usage: %s [fd|fbo[1|2]|roto[1|2]] filename\n",argv[0]);return -1;
        }
    } else {
        printf("usage: %s [fd|fbo|roto] filename\n",argv[0]);return -1;
    }

    if( !fb_init((char*)"/dev/fb0", NULL )) {
        printf("fb_init() failed\n"); return -1;
    }

    sleep(1);

    sp<RenderThread> renderThread = new RenderThread();
    renderThread->start();

    sleep(1);

    sp<DecoderThread> decoderThread = new DecoderThread();
    decoderThread->start();
    
    nonblock(NB_ENABLE);
    while(!stop)
    {
        usleep(1);
        if(kbhit()!=0)
        {
            fgetc(stdin);
            stop++;
        }
    }
    nonblock(NB_DISABLE);

    {   
        AutoLock lock( mutex );
        decoderState = DECODING_FINISHED;
    }

    decoderThread->join();
    {   
        AutoLock lock( mutex );
        decoderState = DECODING_FINISHED;
    }

    renderThread->join();

    fb_cleanup();

    printf("done!\n");
    return 0;
}
