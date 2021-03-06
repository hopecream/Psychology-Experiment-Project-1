#include "stdafx.h"
#include "Screen.h"
#include "Scene.h"
#include "experiment.h"
#include "texture.h"
#include <sstream>

using namespace std;


Screen::Screen(DEVMODE& devMode):
    rDevMode(devMode)
{
    this->stopped = TRUE;
    this->onSampleFPS = FALSE;
    QueryPerformanceFrequency(&this->CounterFrequency);

}

Screen::~Screen(void)
{
    glDeleteTextures(this->texNo, this->texIDs);
    delete [] this->texIDs;
    
    glDeleteTextures(1, this->noiseTex);
    delete this->noiseTex;
}

// This part can be seen as SetupRC with other initializations
BOOL Screen::initGlut(UINT displayMode, string title)
{
    // Change the display settings to the selected resolution and refresh rate
    if(ChangeDisplaySettings(&rDevMode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
    {
        MessageBox(NULL, (LPCSTR)"Cannot change to selected desktop resolution.", NULL, MB_OK | MB_ICONSTOP);
        return FALSE;
    }

    //ostringstream ossGameModeString;
    //ossGameModeString << rDevMode.dmPelsWidth << "x" << rDevMode.dmPelsHeight
    //    << ":" << rDevMode.dmBitsPerPel << "@" << rDevMode.dmDisplayFrequency;
    //string gameModeString(ossGameModeString.str());

    // __argc and __argv are global variables
    // storing the arguments for the program
    glutInit(&__argc, __argv);
    glewInit();
    this->displayMode = displayMode;

    // Init the glut 
    glutInitDisplayMode(displayMode);

    // Multisample Setting
    // Try using the max possible multisample level
    int *sampleNumbers = NULL;
    int sampleNumbersSize;

    sampleNumbers = glutGetModeValues(GLUT_MULTISAMPLE, 
                                     &sampleNumbersSize); 

    if(sampleNumbers != NULL)
    {
        glutSetOption(GLUT_MULTISAMPLE, sampleNumbers[sampleNumbersSize - 1]);
        free(sampleNumbers);
    }
    
    glutInitWindowSize(rDevMode.dmPelsWidth, rDevMode.dmPelsHeight);
    glutCreateWindow(title.c_str());
        
    // FIX: Message Box will cannot be seen in game mode
    //glutGameModeString(gameModeString.c_str());
    //if(glutGameModeGet(GLUT_GAME_MODE_POSSIBLE))
    //{
    //    glutGameModeString(gameModeString.c_str());
    //    glutEnterGameMode();
    //}
    //else
    //    glutFullScreen();

    glutFullScreen();

    this->_hWnd = FindWindow(NULL, title.c_str());

    // Set the background color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearStencil(0x0);

    // Hide the cursor
    glutSetCursor(GLUT_CURSOR_NONE);

    glEnable(GL_DEPTH_TEST);

    // Setting up textures environment
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    this->stereoFrame.LoadFrame(string("./config/calib.txt"));

    // Enable VSync
    if(this->WGLExtensionSupported("WGL_EXT_swap_control"))
    {
        this->wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
        this->wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress("wglGetSwapIntervalEXT");
    }
    else
    {
        this->wglSwapIntervalEXT = NULL;
        this->wglGetSwapIntervalEXT = NULL;
    }

    if(this->wglSwapIntervalEXT != NULL)
        this->wglSwapIntervalEXT(1);

    // Enable Anisotropic sampling
    if(this->WGLExtensionSupported("GL_EXT_texture_filter_anisotropic"))
    {
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &this->_textureFilterAnisotropicLargest);
    }
    else
        this->_textureFilterAnisotropicLargest = -1.0f;

    if(this->initNoise(string("textures/noise.bmp")) == FALSE)
    {
        return FALSE;
    }

    return TRUE;
}

BOOL Screen::initTextures(vector<rTexture_t *>& textures)
{
    this->texNo = textures.size();
    this->colorIDs.resize(textures.size(), vector<GLuint>(3));
    this->texIDs = new GLuint[this->texNo];
    memset(this->texIDs, 0, sizeof(GLuint) * this->texNo);
    glGenTextures(this->texNo, this->texIDs);

    for(unsigned int i = 0; i < textures.size(); i ++)
    {
        switch(textures[i]->type)
        { 
            case 'T':
                {
                    glBindTexture(GL_TEXTURE_2D, this->texIDs[i]);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

                    if(this->_textureFilterAnisotropicLargest > 0)
                        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                                this->_textureFilterAnisotropicLargest);

                    BITMAP bm;
                    HBITMAP hBitmap;

                    hBitmap = textures[i]->hBitmap;
                    GetObject(hBitmap, sizeof(bm), &bm);

                    BYTE *imageBuffer = new BYTE[bm.bmWidthBytes * bm.bmHeight];
                    GetBitmapBits(hBitmap, bm.bmWidthBytes * bm.bmHeight, imageBuffer);

                    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, bm.bmWidth, bm.bmHeight,
                            GL_RGBA, GL_UNSIGNED_BYTE, imageBuffer);
                    //gluBuild2DMipmaps(GL_TEXTURE_2D, GL_LUMINANCE_ALPHA, bm.bmWidth, bm.bmHeight,
                    //        GL_RGBA, GL_UNSIGNED_BYTE, imageBuffer);
                    delete [] imageBuffer;                    
                    break;
                }
            case 'C':
                colorIDs[i][0] = textures[i]->color[0];
                colorIDs[i][1] = textures[i]->color[1];
                colorIDs[i][2] = textures[i]->color[2];
                break;
            default:
                break;
        }
    }

    return TRUE;
}

BOOL Screen::initNoise(string& noiseFilename)
{
    HBITMAP hBitmap = NULL;
    hBitmap = (HBITMAP)::LoadImage(NULL, (LPCSTR)(noiseFilename.c_str()), 
                               IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    if(hBitmap == NULL)
    {
        string errorMsg = "Fail to load the noise texture ";
        errorMsg += noiseFilename;
        MessageBox(NULL, (LPSTR)(errorMsg.c_str()), NULL, MB_OK | MB_ICONERROR);
        return FALSE;
    }

    this->noiseTex = new GLuint;
    glGenTextures(1, this->noiseTex);

    glBindTexture(GL_TEXTURE_2D, *this->noiseTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    if(this->_textureFilterAnisotropicLargest > 0)
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
            this->_textureFilterAnisotropicLargest);

    BITMAP bm;
    GetObject(hBitmap, sizeof(bm), &bm);
    BYTE *imageBuffer = new BYTE[bm.bmWidthBytes * bm.bmHeight];
    GetBitmapBits(hBitmap, bm.bmWidthBytes * bm.bmHeight, imageBuffer);

    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, bm.bmWidth, bm.bmHeight,
        GL_RGBA, GL_UNSIGNED_BYTE, imageBuffer);

    delete [] imageBuffer; 

    return TRUE;
}

// Cancel all the keyboard and mouse events binding
BOOL Screen::cancelKMBinds()
{
    glutKeyboardFunc(NULL);
    glutSpecialFunc(NULL);

    glutMouseFunc(NULL);
    glutMotionFunc(NULL);
    glutPassiveMotionFunc(NULL);

    glutKeyboardUpFunc(NULL);

    return TRUE;
}

BOOL Screen::setKeyboardFunc(void (*func)(unsigned char key, int x, int y))
{
    glutKeyboardFunc(func);
    return TRUE;
}

BOOL Screen::setKeyboardSpecialFunc(void (*func)(int key, int x, int y))
{
    glutSpecialFunc(func);
    return TRUE;
}

BOOL Screen::setMouseFunc(void (*func)(int button, int state, int x, int y))
{
    glutMouseFunc(func);
    return TRUE;
}

BOOL Screen::setMouseMotionFunc(void (*func)(int x, int y))
{
    glutMotionFunc(func);
    return TRUE;
}

BOOL Screen::setMousePassiveMotionFunc(void (*func)(int x, int y))
{
    glutPassiveMotionFunc(func);
    return TRUE;
}


// Call this to display the string
BOOL Screen::displayString(string str, float x, float y)
{
    //glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(x, y);
    //glWindowPos2f(x, y);

    glutBitmapString(GLUT_BITMAP_9_BY_15,
            reinterpret_cast<const unsigned char *>(str.c_str()));

    return TRUE;
}

// Call this before actual rendering
void Screen::render()
{
    // calculate the FPS

    if(this->onSampleFPS)
    {
        this->iFrames ++;
        if(this->iFrames == 100)
        {
            float fTime;
            LARGE_INTEGER lCurrent;

            QueryPerformanceCounter(&lCurrent);

            fTime = (float)(lCurrent.QuadPart - this->FPSCount.QuadPart) /
                (float)(this->CounterFrequency.QuadPart);

            this->fps = (float)iFrames / fTime;

            this->iFrames = 0;
            QueryPerformanceCounter(&this->FPSCount);
        }
    }


    // If we set double buffer, then swap the buffer
    // If we set single buffer, just flush the buffer
    if(((this->displayMode) & GLUT_DOUBLE) != 0)
    {
        glutSwapBuffers();
        glutPostRedisplay();
    }
    else
    {
        glFlush();
    }
}

BOOL Screen::setDisplayFunc(void (*displayFunc)(void))
{
    glutDisplayFunc(displayFunc);
    return TRUE;
}

BOOL Screen::setReshapeFunc(void (*func)(int w, int h))
{
    glutReshapeFunc(func);
    return TRUE;
}

BOOL Screen::setTimerFunc(unsigned int msecs, void (*func)(int timerID), int timerID)
{
    if(msecs == 0)
        msecs = 1;
    glutTimerFunc(msecs, func, timerID);
    return TRUE;
}

void Screen::nullTimerFunc(int timerID)
{
    return;
}

BOOL Screen::setIdleFunc(void (*func)(void))
{
    glutIdleFunc(func);
    return TRUE;
}

// Clear the screen
BOOL Screen::clear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    this->render();

    return TRUE;
}

// Use this to reset all the glut functions
BOOL Screen::resetAllFunc()
{
    this->cancelKMBinds();
    //this->setDisplayFunc(NULL); 
    //this->setReshapeFunc(NULL);
    this->setTimerFunc(0, Screen::nullTimerFunc, 0); 
    //this->setIdleFunc(NULL);

    return TRUE;
}

// Call this after everything ready
BOOL Screen::run()
{
    this->stopped = FALSE; 

    while(this->stopped == FALSE)
    {
        glutMainLoopEvent();
    }

    return TRUE;
}


BOOL Screen::startSampleFPS()
{
    this->onSampleFPS = TRUE;
    this->fps = 0;
    iFrames = 0;
    QueryPerformanceCounter(&this->FPSCount);
    return TRUE;
}

BOOL Screen::stopSampleFPS()
{
    this->onSampleFPS = FALSE;
    return TRUE;
}

float Screen::getFPS()
{
    return this->fps;
}

BOOL Screen::WGLExtensionSupported(const char *extension_name)
{
    char* _wglGetExternsionsStringEXT = NULL;

    _wglGetExternsionsStringEXT = (char*)glGetString(GL_EXTENSIONS);

    if(_wglGetExternsionsStringEXT != NULL &&
            strstr(_wglGetExternsionsStringEXT, extension_name) == NULL)
        return FALSE;

    else

        return TRUE;

}

// the function is only for test purpose
void Screen::testRenderScene()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_POLYGON);
    glVertex2f(-0.5, -0.5);
    glVertex2f(-0.5, 0.5);
    glVertex2f(0.5, 0.5);
    glVertex2f(0.5, -0.5);
    glEnd();
    glFlush();
}
