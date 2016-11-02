#include "octree.h"
#include <iostream>

#include <opencv/cv.h>

#include <awesomeIO/iReader.h>
#include <awesomeIO/ReaderFactory.h>
#include <awesomeIO/iWriter.h>
#include <awesomeIO/WriterFactory.h>


/**OcTree*/
template <class CElement>
COcTree<CElement>::COcTree(std::string name, double divLimit, double x[OCDIM], double y[OCDIM])
{
    setData(name, divLimit);
    mp_Origin = new CNodeBranch<CElement>(0, x, y, isLoadable());//TODO fix this
    load();
}

template <class CElement>
COcTree<CElement>::COcTree(std::string name, double divLimit, double x1, double x2, double x3, double y1, double y2, double y3)
{
    setData(name, divLimit);
    mp_Origin = new CNodeBranch<CElement>(0, x1, x2, x3, y1, y2, y3, isLoadable());//TODO fix this
    load();
}

template <class CElement>
COcTree<CElement>::COcTree(std::string name, double divLimit, double y1, double y2, double y3)
{
    setData(name, divLimit);
    mp_Origin = new CNodeBranch<CElement>(0, 0, 0, 0, y1, y2, y3, isLoadable());//TODO fix this
    load();
}

template <class CElement>
void COcTree<CElement>::setData(std::string name, double divLimit)
{
    m_Name = name;
    m_divLimit = divLimit;
    m_numNodes=1;
    m_numElements=0;
}

template <class CElement>
COcTree<CElement>::~COcTree()
{
    delete mp_Origin;
    #ifdef OCDELETE
    std::cout << "Deleted all nodes (with destructor)" << std::endl;
    #endif
    if(m_dataChanged) {
        save();
    }
    #ifdef OCSERIALIZE
    std::cout << (m_dataChanged ?  "saved all data into the save-directory" : "did not remove anything seralized, do this with clear!") << std::endl;
    #endif

}

template <class CElement>
bool COcTree<CElement>::insert(double p[OCDIM], CElement* dataset)
{
    return insert(p[0], p[1], p[2], dataset);
}

template <class CElement>
bool COcTree<CElement>::insert(double x, double y, double z, CElement* dataset)
{
    bool sucess = mp_Origin->insert(x, y, z, dataset, &m_divLimit, &m_numNodes, &m_numElements);
    if(!m_dataChanged && sucess) {
        m_dataChanged = true;
    }
    return sucess;
}

template <class CElement>
void COcTree<CElement>::clear()
{
    CNode<CElement>* start = new CNodeBranch<CElement>((CNodeBranch<CElement>*)mp_Origin);
    delete mp_Origin;
    boost::filesystem::remove_all(serializationPath+m_Name+'/');
    #if defined(OCDELETE) || defined(OCCLEAR) || defined(OCSERIALIZE)
    std::cout << "Deleted all nodes (with clear() )" << std::endl
        << "removed every serialized object under <"+serializationPath+">!" << std::endl
        << "created new origin-node" << std::endl;
    #endif
    mp_Origin = start;
    m_numNodes = 1;
    m_numElements = 0;
}

template <class CElement>
unsigned long COcTree<CElement>::getLevels()
{
    return mp_Origin->getTreeLevel();
}

//Serialization:
template <class CElement>
void COcTree<CElement>::save()
{
    if(!isLoadable()) {
        ::createDirectories(serializationPath+m_Name+'/');
    }
    serializeTreeData();
    mp_Origin->serialize(serializationPath+m_Name+'/');
}

template <class CElement>
void COcTree<CElement>::load()
{
    if(isLoadable())
    {
        deserializeTreeData();
        mp_Origin->deserialize(serializationPath+m_Name+'/');
        m_dataChanged = false;//just load the Tree into the ram, no data changed
    }
    else
    {
        m_dataChanged = true;//triggers a save when deleting the tree!
        std::cout << "INFO: Unable to load octree named " << m_Name << std::endl
            << "Files not found under " << serializationPath << m_Name << '/' << std::endl
            << "Maybe this octree was never saved before" << std::endl;
    }
}


template <class CElement>
bool COcTree<CElement>::isLoadable()
{
    using namespace boost::filesystem;//I think this is better readable
    std::string treepath = serializationPath+m_Name+'/';
    path p(treepath);
    return is_directory(p);
}

template <class CElement>
void COcTree<CElement>::serializeTreeData()
{
    std::ofstream ofs(serializationPath+m_Name+mimeTypeTreeData, std::ios::binary);
    cereal::BinaryOutputArchive oarchive(ofs);
    oarchive(m_divLimit, m_numNodes, m_numElements);
}

template <class CElement>
void COcTree<CElement>::deserializeTreeData()
{
    std::ifstream ifs(serializationPath+m_Name+mimeTypeTreeData, std::ios::binary);
    cereal::BinaryInputArchive iarchive(ifs);
    iarchive(m_divLimit, m_numNodes, m_numElements);
}


/*Draw*/
template <class CElement>
void COcTree<CElement>::draw()
{
    mp_Origin->draw();
    #ifdef OCDRAW
    std::cout << "Drew all nodes" << std::endl;
    #endif
}

template <class CElement>
void COcTree<CElement>::draw(double fromx, double fromy, double fromz, double tox, double toy, double toz)
{
    mp_Origin->draw(fromx, fromy, fromz, tox, toy, toz);
}

template <class CElement>
void COcTree<CElement>::draw(double x, double y, double z)
{
    mp_Origin->draw(x, y, z);
}

#if defined(OCLOADTEST) //&& (defined(BIGTEST) || defined(SMALLTEST))
int main(void) {
    std::cout << "OcTree:" << std::endl;
    #ifdef BIGTEST
    COcTree<int>* v = new COcTree<int>("octest-big", 0.0000001, 1.0, 1.0, 1.0);//TODO: Konstruktor
    #elif defined(SMALLTEST)
    COcTree<int>* v = new COcTree<int>("octest-small", 0.1, 1.0, 1.0, 1.0);//TODO: Konstruktor
    #else
    COcTree<int>* v = new COcTree<int>("octest-medium", 0.00001, 1.0, 1.0, 1.0);//TODO: Konstruktor
    #endif
    int a[] = {0, 1, 2, 3, 4, 5};
    double p[] = {0.500000000001, 0.5000000001, 0.5000000001}, p2[] = {4.0, 4.0, 4.0}, p3[] = {6.0, 6.0, 6.0};

    int n = 2000000;
    std::cout << "Num Nodes after initialization: "<< v->numNodes() << std::endl;
    if(v->numNodes() == 1) {
        v->insert(p, &(a[0]));
        v->insert(p, &(a[1]));
        v->insert(p2, &(a[2]));
        v->insert(p3, &(a[2]));
        for(int i = 0; i <= n; i++) {
            double p4[] = {i*1.0/n, i*1.0/n, i*1.0/n};//just a diagonal
            v->insert(p4, &(a[3]));
            //if(i == n/2+1)
                //v->serialize();
        }
    }
    else
    {
        std::cout << "Found and loaded the OcTree sucessfully" << std::endl;
    }
    v->draw();//draw every fucking thing, nothing should happen to this moment
    for(int i = 0; i <= n; i++) {
        double p4[] = {i*1.0/n, i*1.0/n, i*1.0/n};//test everything
        v->draw(p4[0], p4[1], p4[2]);//draw at a position, nothing should happen to this momen
    }

    std::cout << "numNodes: " << v->numNodes() << std::endl
        << "numNodes%8= " << v->numNodes()%8 << std::endl
        << "numElements: " << v->numElements() << std::endl
        << "Level: " << v->getLevels() << std::endl
        ;//<< "Origin node: " << *(v->mp_Origin) << std::endl;

    /*v->serialize();
    std::cout << "finished serialization" << std::endl;
    std::cout << "Waiting to press Enter" << std::endl;
    getchar();
    std::cout << "continued..." << std::endl;
    v->deserialize();
    std::cout << "finished deserialization" << std::endl;
    v->draw();*/
    /*
    v->clear();

    std::cout << "cleared Tree, now rebuilding" << std::endl;

    for(int i = 0; i < n; i++)
    {
        double p4[] = {i*1.0/n, i*1.0/n, i*1.0/n};//diagonal durch den raum
        v->insert(p4, &(a[3]));
    }
    std::cout << "numNodes: " << v->numNodes() << std::endl
        << "numNodes%8= " << v->numNodes()%8 << std::endl
        << "numElements: " << v->numElements() << std::endl
        << "Level: " << v->getLevels() << std::endl;
    */
    std::cout << "started saving all data" << std::endl;
    v->save();
    std::cout << "finished saving all data" << std::endl;
    delete v;
    return 0;
}
#endif

/*
int main(void){
    COcTree<float>* skalarTree  = new COcTree<float>("octest-skalar-small", 0.1, 20, 20, 20);
    if(!skalarTree->isLoadable())//Data not there? Do the insert
    {
        float f = 1;
        skalarTree->insert(1, 2, 3, &f);
    }
    skalarTree->draw(1, 2, 3);//draws EVERYTHING!!!
    delete skalarTree;
    return 0;
}*/

template class COcTree<int>;
template class COcTree<float>;
