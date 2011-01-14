#include "stdafx.h"
#include "Trial.h"
#include "Scene.h"
#include "PreTrialScene.h"
#include "Separate2D3DViewScene.h"
#include "Overlapped2DViewScene.h"
#include "TestObject.h"
#include "Conditions.h"
#include "experiment.h"

#include <vector>
#include <sstream>
#include <string>

using namespace std;

Trial::Trial(int trialID, cond_t& cond):
    condition(cond)
{
    this->trialID = trialID;
    this->currState = IDLE;
    this->finished = FALSE;
}

Trial::~Trial(void)
{
}

BOOL Trial::startTrial()
{
    BOOL ret = TRUE;

    //Proceed to the next scene if
    //the trial is not finished and 
    //each scene is executed correctly
    while(this->finished == FALSE &&
            ret == TRUE)
    {
        ret = this->proceedNextScene();
    }

    return ret;
}

BOOL Trial::proceedNextScene()
{
    Scene *pScene;
    BOOL ret;

    switch(this->currState)
    {
        case IDLE:
            {
                pScene = new PreTrialScene();
                this->currState = PRE_TRIAL_SCENE;
                ret = pScene->startScene();

                delete pScene;

                break;
            }
        case PRE_TRIAL_SCENE:
            {
                pScene = new Separate2D3DViewScene(this->condition);
                this->currState = MAIN_SCENE;
                ret = pScene->startScene();

                delete pScene;

                break;
            }
        case MAIN_SCENE:
            {
                Experiment *pExperi = Experiment::getInstance(NULL);
                if(pExperi->experiMode == EXPERIMENT)
                {
                    // Write the trial result to the output file
                    this->recordTrialInfo();

                    this->currState = IDLE;
                    this->finished = TRUE;
                }
                else
                {
                    // In practice mode, so show the result comparison
                    pScene = new Overlapped2DViewScene(this->condition);
                    this->currState = POST_TRIAL_SCENE;
                    ret = pScene->startScene();

                    delete pScene;
                }

                break;
            }
        case POST_TRIAL_SCENE:
            {
                this->currState = IDLE;
                this->finished = TRUE;
                break;
            }
        default:
            return FALSE;
            break;
    }
    return TRUE;
}

BOOL Trial::recordTrialInfo()
{
    TestObject& rObject = *this->condition.pRealObject;
    ostringstream ossTI; // TI = Trial Information  

    ossTI << "Trial ID: " << this->trialID + 1 << endl;
    ossTI << rObject.genObjPara() ;

    Experiment *pExperi = Experiment::getInstance(NULL);
    pExperi->writeOutputs(ossTI.str());

    return TRUE;
}

