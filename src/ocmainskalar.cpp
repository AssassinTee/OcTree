#include "octree.h"
#include <iostream>

#include <opencv/cv.h>

#include <awesomeIO/iReader.h>
#include <awesomeIO/ReaderFactory.h>
#include <awesomeIO/iWriter.h>
#include <awesomeIO/WriterFactory.h>

#include <boost/filesystem.hpp>//look if your data is there!

#include <algorithm>
#include <vector>
using namespace std;

void showDirectory(string imgPath)
{
    using namespace boost::filesystem;
    path p(imgPath);
    typedef vector<path> vec;             // store paths,
    vec v;                                // so we can sort them later

    copy(directory_iterator(p), directory_iterator(), back_inserter(v));

    sort(v.begin(), v.end());             // sort, since directory iteration
                                              // is not ordered on some file systems

    for (vec::const_iterator it (v.begin()); it != v.end(); ++it)
    {
        cout << "   " << *it << '\n';
        cout << "   -" << canonical(*it) << endl;
    }
}

bool checkDirectory(boost::filesystem::path p)
{
    using namespace boost::filesystem;
    cout << "checking directory " << p << endl;
    /*if(!exists(p));
    {
        cout << p << " does not exist!" << endl;
        showDirectory("../../");
        boost::filesystem::path full_path( boost::filesystem::current_path() );
        std::cout << "Current path is : " << full_path << std::endl;
        return false;
    }*/

    /*if(!is_directory(p));
    {
        cout << p << " is no directory or does not exist!" << endl;
        showDirectory("../../");
        boost::filesystem::path full_path( boost::filesystem::current_path() );
        std::cout << "Current path is : " << full_path << std::endl;
        return false;
    }*/
    return true;
}

int main(int argc, char* argv[])
{
    //TODO: Create input

    /*if(argc <= 1)//keinen Dateipfad//TODO: spaeter
    {
        cout << "You must give a Path to the data to read in from args!" << endl;
    }*/
    string imgPath = "../../data";
    string imgName = "R2022_blockface_1_447_masked.nii";

    showDirectory(imgPath);

    iReader* imageReader;

    //This gives a segfault when not working

    imageReader = ReaderFactory::New(imgPath+'/'+imgName);
    cout << "This happens!2" << endl;
    if(!imageReader) {
        cout << "Image at '" << imgPath+'/'+imgName << "' was not loadable!" << endl;
        return 0;
    }
    cout << "This happens!" << endl;
    COcTree<float>* skalarTree  = new COcTree<float>("octest-skalar-small", 0.1, 20, 20, 20);
    if(!skalarTree->isLoadable())//Data not there? Do the insert
    {
        float f = 1.0f;
        skalarTree->insert(1, 2, 3, &f);
    }
    skalarTree->draw(1, 2, 3);//draws EVERYTHING!!!
    delete skalarTree;

    return 0;
}

