#pragma once

#include "stdafx.h"
#include "texture.h"
#include "StereoViewingFrameV2.h"
#include <string>
#include <vector>
using namespace std;

class Screen
{
    public:
        Screen(DEVMODE&);
        ~Screen();

        BOOL initGlut(UINT displayMode, string title);

        BOOL cancelKMBinds();
        BOOL clear();
        BOOL displayString(string str, float x, float y);
        BOOL resetAllFunc(void);
        BOOL setDisplayFunc(void (*displayFunc)(void));
        BOOL setReshapeFunc(void (*func)(int w, int h));
        BOOL setKeyboardFunc(void (*func)(unsigned char key, int x, int y));
        BOOL setKeyboardSpecialFunc(void (*func)(int key, int x, int y));
        BOOL setMouseFunc(void (*func)(int button, int state, int x, int y));
        BOOL setMouseMotionFunc(void (*func)(int x, int y));
        BOOL setMousePassiveMotionFunc(void (*func)(int x, int y));
        BOOL setTimerFunc(unsigned int msecs, void (*func)(int timerID), int timerID);
        BOOL setIdleFunc(void (*func)(void));

        BOOL initTextures(vector<rTexture_t *>& textures);
        BOOL initNoise(string& noiseFilename);

        BOOL run();
        void render();

        // routines for caculating FPS
        BOOL startSampleFPS();
        float getFPS();
        BOOL stopSampleFPS();

        static void nullTimerFunc(int timerID);

        BOOL stopped;
        DEVMODE &rDevMode;

        int texNo;
        GLuint *texIDs;
        vector< vector<GLuint> > colorIDs;

        GLuint *noiseTex;

        StereoViewingFrame stereoFrame;


        inline const LARGE_INTEGER& getCounterFrequency() {return this->CounterFrequency;}

        inline const HWND hWnd() {return this->_hWnd;}
        inline const GLfloat textureFilterAnisotropicLargest() {return this->_textureFilterAnisotropicLargest;}

        PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
        PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT;

        // the function is only for test purpose 
        static void testRenderScene();


    private:
        BOOL WGLExtensionSupported(const char *extension_name);
    
        BOOL onSampleFPS;
        UINT displayMode;
        
        LARGE_INTEGER FPSCount;
        LARGE_INTEGER CounterFrequency;
        int iFrames;
        float fps;

        HWND _hWnd;
        GLfloat _textureFilterAnisotropicLargest;
};

