#include "octree.h"

#include <opencv/cv.h>

#include <awesomeIO/iReader.h>
#include <awesomeIO/ReaderFactory.h>
#include <awesomeIO/iWriter.h>
#include <awesomeIO/WriterFactory.h>

//#ifdef MAIN
int main(int argc, char* argv[])
{
    //TODO: Create input
    COcTree<float>* skalarTree = new COcTree<float>("octest-skalar-small", 0.1, 20, 20, 20);
    if(!skalarTree->isLoadable())//Data not there? Do the insert
    {
        float f = 1.0f;
        skalarTree->insert(1, 2, 3, &f);
    }
    skalarTree->draw();//draws EVERYTHING!!!
    delete skalarTree;

    return 0;
}
//#endif
