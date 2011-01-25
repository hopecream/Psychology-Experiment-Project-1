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

        StereoViewingFrame stereoFrame;

        // the function is only for test purpose 
        static void testRenderScene();

        inline const LARGE_INTEGER& getCounterFrequency() {return this->CounterFrequency;}

        PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
        PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT;

    private:
        BOOL WGLExtensionSupported(const char *extension_name);
    


        BOOL onSampleFPS;
        UINT displayMode;
        
        LARGE_INTEGER FPSCount;
        LARGE_INTEGER CounterFrequency;
        int iFrames;
        float fps;
};

