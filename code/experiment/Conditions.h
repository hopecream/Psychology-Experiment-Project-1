#pragma once

#include "stdafx.h"
#include "ConditionConstraints.h"
#include "TestObjectFactory.h"
#include "TestObject.h"
#include "Screen.h"
#include "texture.h"
#include "rangeType.h"
#include <hash_map>

using namespace stdext;

struct conditionStruct
{
    TestObject *pRealObject; // Store the real object's pointer here
    vector<texture_t *> textures; // Stores the textures for the object

    int repeatTime; // the times that the condition need to be repeated in one section

    displayMode_t dispMode;

    // Used only when the display mode is DISCRETE_DISPLAY
    float secDisplay; // seconds for displaying object
    float secBlackScreen; // seconds for displaying black screen
    int constraintID;
    int constraintGroupID;

    void reset();

    vector<GLfloat> xyz3D;
    vector<GLfloat> xyz2D;
};
typedef struct conditionStruct cond_t;

class Conditions
{
    public:
        // rFilename: filename of the configuration file
        // numConditins: number of conditions to be generated
        // rObjectFactories: reference to the vector of supported object factories
        // rScr: reference to the Screen class
        explicit Conditions(ifstream& rFin, int numConditions,
                vector<TestObjectFactory *>& rObjectFactories, Screen& rScr);
        explicit Conditions(ifstream& rFin,
                vector<TestObjectFactory *>& rObjectFactories, Screen& rScr);
        ~Conditions(void);

        // This must be called before using the conditions
        // return true if the configuration is read without 
        // problem
        virtual BOOL initConditions();

        // Add the constraint
        // Conditions need to be regenerated after adding 
        // the new constraint
        // return the index of the constraint being added
        int addConstraint(condCons_t* pConstraint);
        
        // Manually add constraints
        // return the index of the condition being added
        int addCondition(int constraintIndex);
        int addCondition(cond_t* pCondition);
        int addAllConditionsFromConstraint(int constraintIndex);

        // generate conditions according to the constraints
        // the old ones will be cleared
        // return true if all the conditions are generated 
        // successfully
        virtual BOOL generateConditions();
        virtual BOOL generateAllConditions();

        virtual BOOL clearConditions();

        // shuffle the generated conditions
        virtual void shuffleConditions(int times);
        
        // get all constraints
        const vector<condCons_t *>& getAllConstraints();

        // get all conditions
        virtual const vector<cond_t *>& getAllConditions();

        // get the specific condition using its index
        virtual cond_t& operator[](int &rhs);

        // number of sections
        int numSections;
    protected:
        vector<condCons_t *> constraints;
        vector<cond_t *> conditions;

        // We can use hash_map to find the object factory by its name
        // and then we can use the factory to generate the object
        vector<TestObjectFactory *>& objectFactories;
        hash_map<string, TestObjectFactory *> objectFactoryNameMap;

        // Store all the textures. By providing a texture name to
        // the texture map, we can get an index of the real texture
        // information in the textures vector
        vector<rTexture_t *> textures;
        hash_map<string, int> textureMap;

        // Store the screen class
        Screen& rScreen;

        // number of conditions need to be generated
        int numConditions;

        // number of times all the conditions need to be repeated
        int conditionRepeatTimesPerSec;

        // Configuration file
        ifstream& overallFin;

        // Print reading error
        void printReadRangeError(string name, int constraintID);
        
        // Reading various things from the configuration file
        template<typename T>
        BOOL readRange(ifstream& fin, rangeType<T>& vec);
        BOOL readConstraints(ifstream& fin, vector<condCons_t *>& rConstraints);
        BOOL readTextures(ifstream& fin);

        // Cylinder's own parameter reading function
        BOOL cylinderParameterReadingFunction(ifstream& fin, condCons_t& constraint, int constraintID);

        // TODO: other new objects' own parameter reading functions can be put here

        // Forbid the operations below
        Conditions& operator=(Conditions& rhs); 
};
