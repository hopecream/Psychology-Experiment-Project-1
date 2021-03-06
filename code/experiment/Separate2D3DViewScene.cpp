#include "stdafx.h"
#include "Separate2D3DViewScene.h"
#include "Trial.h"
#include "Experiment.h"
#include "CylinderObject.h"


Separate2D3DViewScene::Separate2D3DViewScene(cond_t& cond):
condition(cond), currStatus(DISPLAY_OBJECT), SWITCH_TIMERID(-1),
ROTATION_TIMERID(-1)
{
    this->_lLastRenderTime.QuadPart = 0;
}

Separate2D3DViewScene::~Separate2D3DViewScene(void)
{
    // FIX: need to be uninterruptable
    //Scene::unregisterTimer(this->ROTATION_TIMERID);
    Scene::unregisterTimer(this->SWITCH_TIMERID);
}

BOOL Separate2D3DViewScene::startScene()
{
    // Cancel all keyboards and mouses events bindings
    // and reset all other functions, e.g. display
    this->rScreen.resetAllFunc();

    // Get the random object
    TestObject& rObject = *this->condition.pRealObject;

    // Clear the screen
    this->rScreen.clear();

    // set display function and reshape function
    this->rScreen.setDisplayFunc(Scene::dispatchSceneRender);
    this->rScreen.setReshapeFunc(Scene::dispatchReshape);

    // Bind new keyboards and mouses events
    this->rScreen.setKeyboardFunc(Scene::dispatchKeyboardEvent);
    this->rScreen.setKeyboardSpecialFunc(Scene::dispatchKeyboardSpecialEvent);
    this->rScreen.setMousePassiveMotionFunc(Scene::dispatchMousePassiveMotionEvent);
    this->rScreen.setMouseFunc(Scene::dispatchMouseEvent);
    //bind timer event

    // Set the rotSpeed to 0 can make
    // the object stay still.
    if(rObject.rotSpeed != 0 && this->condition.dispMode == CONTINUOUS_DISPLAY)
    {
       // this->ROTATION_TIMERID = Scene::registerTimer();
       // GLfloat msecs;
       // msecs = 1000.0f / rObject.rotSpeed;
       // this->rScreen.setTimerFunc((unsigned int)msecs,
       //        Scene::dispatchTimerEvent, this->ROTATION_TIMERID);
    }

    if(this->condition.dispMode == DISCRETE_DISPLAY)
    {
        this->SWITCH_TIMERID = Scene::registerTimer();
        this->rScreen.setTimerFunc((unsigned int)(this->condition.secDisplay * 1000.0f),
                Scene::dispatchTimerEvent, this->SWITCH_TIMERID);
    }


    // Start running the scene
    // FIX: This is actually a run design if there are multiple screens
    // e.g. the program will be blocked for each run()
    this->initDisplay();

    this->rScreen.startSampleFPS();

    QueryPerformanceCounter(&this->_startTime);
    this->rScreen.run();
    QueryPerformanceCounter(&this->_endTime);

    this->_duration = (float)(this->_endTime.QuadPart - this->_startTime.QuadPart) /
        (float)(this->rScreen.getCounterFrequency().QuadPart);

    this->rScreen.stopSampleFPS();

    this->_fps = this->rScreen.getFPS();

    return this->status;
}

BOOL Separate2D3DViewScene::renderScene()
{
    TestObject& rObject = *this->condition.pRealObject;

    // Rotate the Object
    if(this->condition.dispMode == CONTINUOUS_DISPLAY)
    {
        LARGE_INTEGER lCurrent;
        if(this->_lLastRenderTime.QuadPart == 0)
            QueryPerformanceCounter(&this->_lLastRenderTime);

        QueryPerformanceCounter(&lCurrent);
        float fTime = (float)(lCurrent.QuadPart - this->_lLastRenderTime.QuadPart) /
            (float)(this->rScreen.getCounterFrequency().QuadPart);

        GLfloat step = fTime * rObject.rotSpeed;

        rObject.rotate(step);
        this->_lLastRenderTime.QuadPart = lCurrent.QuadPart;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int scrWidth = this->rScreen.rDevMode.dmPelsWidth;
    int scrHeight = this->rScreen.rDevMode.dmPelsHeight;
    GLfloat fAspect = (GLfloat)scrWidth / (GLfloat)scrHeight;

    glViewport(0, 0, scrWidth, scrHeight);
        
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glLoadMatrixd(this->rScreen.stereoFrame.centerprojmatrix.data());

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if(this->condition.dispMode == CONTINUOUS_DISPLAY ||
            (this->condition.dispMode == DISCRETE_DISPLAY &&
             this->currStatus == DISPLAY_OBJECT))
    {
        //////////////////////////////////////////////////////
        // Draw the 3D View

        glColor3ub(255, 255, 255);

        // Draw the cylinder in 3D view
        rObject.draw(GLU_FILL, TRUE, TRUE, TRUE, 1.0f, 1.0f, rObject.initZAsptRatio,
                this->condition.xyz3D[0],
                this->condition.xyz3D[1],
                this->condition.xyz3D[2]);
        //rObject.draw(GLU_FILL, TRUE, TRUE, TRUE, 1.0f, 1.0f, rObject.adjZAsptRatio,
        //        this->condition.xyz3D[0],
        //        this->condition.xyz3D[1],
        //        this->condition.xyz3D[2]);

    }
    //////////////////////////////////////////////////////
    // Draw 2D View

    rObject.draw2D(GLU_FILL, 1.0f, 1.0f, rObject.adjZAsptRatio,
            this->condition.xyz2D[0],
            this->condition.xyz2D[1],
            this->condition.xyz2D[2]);

#ifdef _DEBUG
    ostringstream ossDebug;
    ossDebug.precision(2);
    ossDebug << "Debug Mode:" << endl;
    
    ossDebug << "VSync: ";
    if(this->rScreen.wglGetSwapIntervalEXT != NULL &&
            this->rScreen.wglGetSwapIntervalEXT() > 0)
        ossDebug << "On";
    else
        ossDebug << "Off";
    ossDebug << endl;

    ossDebug << "Anisotropic Sampling: ";
    if(this->rScreen.textureFilterAnisotropicLargest() > 0)
        ossDebug << this->rScreen.textureFilterAnisotropicLargest();
    else
        ossDebug << "Off";
    ossDebug << endl;
 
    int samples;
    ossDebug << "Samples per buffer: ";
    glGetIntegerv(GL_SAMPLES_ARB, &samples);
    ossDebug << samples << endl;

    LARGE_INTEGER lCurrent;
    QueryPerformanceCounter(&lCurrent);
    float fTime = (float)(lCurrent.QuadPart - this->_startTime.QuadPart) /
        (float)(this->rScreen.getCounterFrequency().QuadPart);
    ossDebug << fixed << "Elapsed time: " << fTime << endl;
   
    ossDebug << fixed << "Current Rotation Degree: " << rObject.currRotDeg << endl;
    ossDebug << fixed << "FPS: " << this->rScreen.getFPS() << endl;
    ossDebug << "Pitch, Yaw, Roll: " << rObject.pitch << ", " << rObject.yaw 
        << ", " << rObject.roll << endl;
    ossDebug << "Height: " << rObject.height << endl;
    ossDebug << "Radius: " << (dynamic_cast<CylinderObject&>(rObject)).radius << endl;
    ossDebug << "Initial Aspect Ratio: " << rObject.initZAsptRatio << endl;
    ossDebug << fixed << "Adjusted Aspect Ratio: " << rObject.adjZAsptRatio << endl;
    ossDebug << "Rotation Speed: " << rObject.rotSpeed << endl;
    ossDebug << "Max Rotation Degree: " << rObject.maxRotDeg << endl;
    this->rScreen.displayString(ossDebug.str(), 23, -8);
#endif

    this->rScreen.render();

    return TRUE;
}


BOOL Separate2D3DViewScene::reshape(int w, int h)
{
    glViewport(0, 0, w, h);

    GLfloat fAspect = (GLfloat)w / (GLfloat)h;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    //gluPerspective(35.0f, fAspect, 0.01f, 50.0f);
    //gluLookAt(0.0f, 30.0f, 30.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    return TRUE;
}

BOOL Separate2D3DViewScene::initDisplay()
{
    this->reshape(this->rScreen.rDevMode.dmPelsWidth,
            this->rScreen.rDevMode.dmPelsHeight); 

    glEnable(GL_MULTISAMPLE_ARB);

    return TRUE;
}


BOOL Separate2D3DViewScene::handleKeyboardEvent(unsigned char key, int x, int y)
{
    switch(key)
    {
        case VK_SPACE:
            {
                if(this->_isStopConditionSatisfied())
                    this->rScreen.stopped = TRUE;
                break;
            }
        case VK_ESCAPE:
            {
                int ret = MessageBox(this->rScreen.hWnd(), "Do you want to abort the experiment?",
                    "Abort", MB_YESNO | MB_ICONWARNING);
                if(ret == IDYES)
                {
                    this->rScreen.stopped = TRUE;
                    this->status = FALSE;
                }

                break;
            }
        default:
            break;
    }
    return TRUE;
}

BOOL Separate2D3DViewScene::handleKeyboardSpecialEvent(int key, int x, int y)
{
    TestObject& rObject = *this->condition.pRealObject;

    switch(key)
    {
        case GLUT_KEY_UP:
            rObject.adjustAsptRatio(0.01f);
            break;
        case GLUT_KEY_DOWN:
            rObject.adjustAsptRatio(-0.01f);
            break;
        default:
            break;
    }
    return TRUE;
}

BOOL Separate2D3DViewScene::handleMouseEvent(int button, int state, int x, int y)
{
    if(button == GLUT_LEFT_BUTTON && state == GLUT_UP)
    {
        if(this->_isStopConditionSatisfied())
            this->rScreen.stopped = TRUE;
    }

    return TRUE;
}

BOOL Separate2D3DViewScene::handleMouseMotionEvent(int x, int y)
{
    return TRUE;
}

BOOL Separate2D3DViewScene::handleMousePassiveMotionEvent(int x, int y)
{
    int lastY = this->rScreen.rDevMode.dmPelsHeight >> 1;
    TestObject& rObject = *this->condition.pRealObject;

    if(y > lastY)
    {
        rObject.adjustAsptRatio(0.01f);
        glutWarpPointer(0, lastY);
    }
    else if(y < lastY)
    {
        rObject.adjustAsptRatio(-0.01f);
        glutWarpPointer(0, lastY);
    }
    else
    {
    }
    return TRUE;
}

BOOL Separate2D3DViewScene::handleTimerEvent(int timerID)
{
    GLfloat step = 1.0f;
    TestObject& rObject = *this->condition.pRealObject;

    if(timerID == this->ROTATION_TIMERID && 
            this->condition.dispMode == CONTINUOUS_DISPLAY)
    {
        GLfloat msecs;
        step = 1.0f;
        if(rObject.rotSpeed != 0.0f)
            msecs = 1000.0f / rObject.rotSpeed;
        else 
            return TRUE;

        rObject.rotate(step);

        this->rScreen.setTimerFunc((unsigned int)msecs,
                Scene::dispatchTimerEvent, this->ROTATION_TIMERID);

    }
    else if(this->condition.dispMode == DISCRETE_DISPLAY &&
            timerID == this->SWITCH_TIMERID)
    {

        if(this->currStatus == DISPLAY_OBJECT)
        {
            if(this->condition.secBlackScreen != 0.0f)
            {
                this->currStatus = DISPLAY_BLACKSCREEN;
                this->rScreen.setTimerFunc((unsigned int)(this->condition.secBlackScreen * 1000.0f),
                        Scene::dispatchTimerEvent, this->SWITCH_TIMERID);
            }
            else
            {
                // According to the new requirement, the rotation degree is either 0, or maxRotDeg, or
                // -maxRotDeg
                //step = (this->condition.secDisplay + this->condition.secBlackScreen) * rObject.rotSpeed;
                //rObject.rotate(step);
                rObject.rotate();
                this->rScreen.setTimerFunc((unsigned int)(this->condition.secDisplay * 1000.0f),
                        Scene::dispatchTimerEvent, this->SWITCH_TIMERID);
            }
        }
        else
        {
            this->currStatus = DISPLAY_OBJECT;
            // According to the new requirement, the rotation degree is either 0, or maxRotDeg, or
            // -maxRotDeg
            //step = (this->condition.secDisplay + this->condition.secBlackScreen) * rObject.rotSpeed;
            //rObject.rotate(step);
            rObject.rotate();
            this->rScreen.setTimerFunc((unsigned int)(this->condition.secDisplay * 1000.0f),
                    Scene::dispatchTimerEvent, this->SWITCH_TIMERID);
        }
    }
    else
    {
    }

return TRUE;
}
