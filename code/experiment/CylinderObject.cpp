#include "stdafx.h"
#include "CylinderObject.h"
#include <string>
#include <sstream>
#include <string>
using namespace std;

const UINT CylinderObject::objectID = 1;

CylinderObject::CylinderObject(void) : TestObject()
{
    this->objName = string("Cylinder");

    // Initialize the ranges
    // FIX: The value should be read from
    // the configuration files later
    radiusRange.push_back(1.0f);
    radiusRange.push_back(2.0f);
    radiusRange.push_back(3.0f);
}

CylinderObject::CylinderObject(CylinderObject &rObj) : TestObject(rObj)
{
   this->objName = string("Cylinder");
   this->radius = rObj.radius;
}

CylinderObject::~CylinderObject(void)
{
}
        
TestObject *CylinderObject::newObj(void)
{
   return (new CylinderObject()); 
}

TestObject *CylinderObject::newObj(TestObject &rObject)
{
   return (new CylinderObject(static_cast< CylinderObject& >(rObject))); 
}

string CylinderObject::getObjName(void)
{
    return this->objName;
}

UINT CylinderObject::getObjID()
{
    return this->objectID;
}

void CylinderObject::setRandPara()
{
    TestObject::setRandPara();

    int randIndex;

    // FIX: rand() maybe is not good enough

    randIndex = rand() % this->radiusRange.size();
    this->radius = this->radiusRange[randIndex];
}

BOOL CylinderObject::adjustAsptRatio(GLfloat delta)
{
    if(adjZAsptRatio + delta <= 0)
    {
        return FALSE;
    }
    else
    {
        adjZAsptRatio += delta;
        return TRUE;
    }
}

string CylinderObject::genObjDesc()
{
    string strDesc;

    strDesc = TestObject::genObjDesc();

    ostringstream ossObj;

    ossObj << "Radius range: ";
    for(vector<GLfloat>::iterator it = this->radiusRange.begin(); 
        it != this->radiusRange.end(); it ++)
        {
            ossObj << (GLfloat)(*it) << " ";
        }
    ossObj << endl;

    strDesc += ossObj.str();

    return strDesc;
}

string CylinderObject::genObjPara()
{
    string strPara;

    strPara = TestObject::genObjPara();

    ostringstream ossObj;

    ossObj << "Radius: " << this->radius << endl;

    strPara += ossObj.str();

    return strPara;
}

void CylinderObject::draw()
{
    GLUquadricObj *pCylinder;

    pCylinder = gluNewQuadric();

    gluQuadricDrawStyle(pCylinder, GLU_FILL);
    gluQuadricNormals(pCylinder, GLU_SMOOTH);

    // Draw the cylinder
    gluCylinder(pCylinder, this->radius, this->radius, this->height, 32, 32);

    gluDeleteQuadric(pCylinder);

    // TODO: Draw the upper face

    // TODO: Draw the bottom face
}
