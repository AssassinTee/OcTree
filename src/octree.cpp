#include "octree.h"
#include <iostream>

#include <cstring>//memcopy for copy constructor!
#include <fstream>//serialize-stuff
#include <sstream>//node2string
#include <functional>

#include <boost/filesystem.hpp>//serialize and build a filesystem under ../serial_data!

#ifdef OCDEBUGLOADTEST
    #define OCDEBUG
    #define OCLOADTEST //Test the octree with a main
#endif

#ifdef OCDEBUGALL
    #define OCDEBUG
    #define OCSERIALICEALL //Activate the output for every single serialized object
    #define OCDESERIALIZEALL //Activate the output for every single deserialized object
    #define OCDRAW //Activate the output when any draw-function, but I would not recommend drawing all nodes for debug
    #define OCINSERT //Activate the output for the insert
    #define OCDRAWALL //Activate the output for all drawn leafs
    #define OCDELETEALL //Activates the output for all deleted nodes
#endif

#ifdef OCDEBUG//Activates ALL options except BIGTEST

    #define OCDELETE //Activate the output for deleting the OCTree
    #define OCCLEAR //Activate the output for clear
    #define OCSERIALIZE //Activate the output for serialize
    #define OCDESERIALIZE //Activate the output for deserialize
#endif



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

/**Node-Base*/
template <class CElement>
bool CNode<CElement>::isInRange(double fromx, double fromy, double fromz, double tox, double toy, double toz)
{
    return (fromx <= this->ma_max[0] && tox >= ma_min[0]//This could cause multiple rendering later
        && fromy <= this->ma_max[1] && toy >= ma_min[1]
        && fromz <= this->ma_max[2] && toz >= ma_min[2]);
}

template <class CElement>
bool CNode<CElement>::isInNode(double x, double y, double z)
{
    return (ma_min[0] <= x && ma_max[0] >= x
            && ma_min[1] <= y && ma_max[1] >= y
            && ma_min[2] <= z && ma_max[2] >= z);
}

/**GLOBAL-FUNCTIONS*/
template <class CElement>
std::string node2str(CNode<CElement>* node)
{
    static std::hash<std::string> hash_fn;//this is so ugly :C
    std::ostringstream os;
    os << *node;
    std::string b = os.str();
    os.str("");//reset the string stream
    os << hash_fn(b);
    return os.str();
}

template <class CElement>
void serializeBranch(CNodeBranch<CElement>* node, std::string path)
{

    std::ofstream ofs(path+node2str(node)+(node->isBranch() ? mimeTypeBranch : mimeTypeLeaf), std::ios::binary);

    /*ofs.write((char*)node, sizeof(*node));
    #ifdef OCSERIALIZEALL
    std::cout << "wrote node under <" << path+node2str(node)+(node->isBranch() ? mimeTypeBranch : mimeTypeLeaf)+">" << std::endl;
    #endif
    */
    cereal::BinaryOutputArchive oarchive (ofs);
    std::shared_ptr<CNodeBranch<CElement> > ptr1(node);
    oarchive( ptr1 );
    #ifdef OCSERIALIZEALL
    std::cout << "finished serialization of a Branch!" << std::endl;
    #endif
}

template <class CElement>
void serializeLeaf(CNodeLeaf<CElement>* node, std::string path)
{

    std::ofstream ofs(path+node2str(node)+(node->isBranch() ? mimeTypeBranch : mimeTypeLeaf), std::ios::binary);

    /*ofs.write((char*)node, sizeof(*node));
    #ifdef OCSERIALIZEALL
    std::cout << "wrote node under <" << path+node2str(node)+(node->isBranch() ? mimeTypeBranch : mimeTypeLeaf)+">" << std::endl;
    #endif
    */
    cereal::BinaryOutputArchive oarchive (ofs);
    std::shared_ptr<CNodeLeaf<CElement> > ptr1(node);
    oarchive( ptr1 );
    #ifdef OCSERIALIZEALL
    std::cout << "finished serialization of a Leaf!" << std::endl;
    #endif
}

template <class CElement>
CNodeBranch<CElement>* deserializeBranch(const std::string& b)
{
    //std::ifstream ifs(b, std::ios::binary | std::ios::ate);
    std::ifstream ifs(b, std::ios::binary);
    if(ifs.is_open()) {

        /*std::cout << "DEBUG1 " << sizeof(*node) << std::endl;
        ifs.read((char*)node, sizeof(*node));//this try was not working
        std::streampos size;

        size = ifs.tellg();

        ifs.seekg (0, std::ios::beg);
        ifs.read ((char*)node, size);
        ifs.close();

        //node = (CNode<CElement>*)memblock;//i don't like this solution, but it works and I think its most efficient
        */

        cereal::BinaryInputArchive iarchive(ifs);

        std::shared_ptr<CNodeBranch<CElement> >ptr1(new CNodeBranch<CElement>());
        iarchive( ptr1 );
        CNodeBranch<CElement>* myNode = new CNodeBranch<CElement>(ptr1.get(), true);

        #ifdef OCDESERIALIZEALL
        std::cout << sizeof(*myNode) << myNode->isBranch() << std::endl;
        std::cout << "finished deserialization of a Branch!" << std::endl;
        #endif
        return myNode;

    }
    else
    {
        std::cerr << "Unable to open file "<< b << std::endl;
        return nullptr;
    }
}

template <class CElement>
CNodeLeaf<CElement>* deserializeLeaf(const std::string& b)
{
    //std::ifstream ifs(b, std::ios::binary | std::ios::ate);
    std::ifstream ifs(b, std::ios::binary);
    if(ifs.is_open()) {

        /*std::cout << "DEBUG1 " << sizeof(*node) << std::endl;
        ifs.read((char*)node, sizeof(*node));//this try was not working
        std::streampos size;

        size = ifs.tellg();

        ifs.seekg (0, std::ios::beg);
        ifs.read ((char*)node, size);
        ifs.close();

        //node = (CNode<CElement>*)memblock;//i don't like this solution, but it works and I think its most efficient
        */

        cereal::BinaryInputArchive iarchive(ifs);

        std::shared_ptr<CNodeLeaf<CElement> >ptr1(new CNodeLeaf<CElement>());
        iarchive( ptr1 );
        CNodeLeaf<CElement>* myNode = new CNodeLeaf<CElement>(ptr1.get(), true);

        #ifdef OCDESERIALIZEALL
        std::cout << sizeof(*myNode) << myNode->isBranch() << std::endl;
        std::cout << myNode->getElements().size() << std::endl;
        std::cout << "finished deserialization of a Leaf!" << std::endl;
        #endif
        return myNode;
    }
    else
    {
        std::cerr << "Unable to open file "<< b << std::endl;
        return nullptr;
    }
}

void createDirectories(std::string path)
{
    boost::filesystem::create_directories(path);
}



template <class CElement>
std::ostream& operator<<(std::ostream& os, CNode<CElement>& node)
{
    os << node.getLevel() << '-'
        << std::setprecision (std::numeric_limits<double>::digits10 + 1)
        << node.ma_min[0] << '-' << node.ma_min[1] << '-' << node.ma_min[2]
        << '-' << node.ma_max[0] << '-' << node.ma_max[1] << '-' << node.ma_max[2];
    return os;
}

/**Node-Branch*/
template <class CElement>
CNodeBranch<CElement>::~CNodeBranch()
{
    if(getNode(0) && !this->m_serialized) {
        for(auto e : mpa_Nodes) {
            if(!e) {
                std::cerr << "Your tree is not complete, this should not happen" << std::endl;
                continue;
            }
            delete e;
            #ifdef OCDELETEALL
            std::cout << "Deleted a node: " << *this << std::endl;
            #endif
        }
    }
}

template <class CElement>
CNodeBranch<CElement>::CNodeBranch(CNodeBranch<CElement>* other)
{
    std::memcpy(this->ma_min, other->ma_min, sizeof(this->ma_min));
    std::memcpy(this->ma_max, other->ma_max, sizeof(this->ma_max));
    this->m_level = other->m_level;
    this->m_serialized = false;
    //this->ma_min = other->ma_min;
    //this->ma_max = other->ma_max;
}

template <class CElement>
CNodeBranch<CElement>::CNodeBranch(CNodeBranch<CElement>* other, bool isSerialized)
{
    std::memcpy(this->ma_min, other->ma_min, sizeof(this->ma_min));
    std::memcpy(this->ma_max, other->ma_max, sizeof(this->ma_max));
    this->m_level = other->m_level;
    this->m_serialized = isSerialized;
    //this->ma_min = other->ma_min;
    //this->ma_max = other->ma_max;
}

template <class CElement>
CNodeBranch<CElement>::CNodeBranch(unsigned int level, double x[OCDIM], double y[OCDIM], bool isSerialized)
{
    this->ma_min = x;
    this->ma_max = y;
    this->m_level = level;
    this->m_serialized = isSerialized;
}

template <class CElement>
CNodeBranch<CElement>::CNodeBranch(unsigned int level, double x1, double x2, double x3, double y1, double y2, double y3, bool isSerialized)
{
    this->ma_min[0] = x1;
    this->ma_min[1] = x2;
    this->ma_min[2] = x3;
    this->ma_max[0] = y1;
    this->ma_max[1] = y2;
    this->ma_max[2] = y3;

    this->m_level = level;
    this->m_serialized = isSerialized;
}

template <class CElement>
bool CNodeBranch<CElement>::insert(double x, double y, double z, CElement* dataset, double* divLimit, unsigned long* numNodes, unsigned long* numElements)
{
     if(!this->isInNode(x, y, z))
        return false;

    if(!getNode(0)) {
        for(int i = 0; i < OCSIZE; i++) {
            double minx = i < OCSIZE/2  ? this->ma_min[0] : (this->ma_max[0]+this->ma_min[0])/2;//i%(OCTSIZE/1) < OCTSIZE/2
            double miny = !(i%2)        ? this->ma_min[1] : (this->ma_max[1]+this->ma_min[1])/2;//i%(OCTSIZE/2) < OCTSIZE/4
            double minz = (i%4) < 2     ? this->ma_min[2] : (this->ma_max[2]+this->ma_min[2])/2;//i%(OCTSIZE/4) < OCTSIZE/8

            double maxx = i >= OCSIZE/2 ? this->ma_max[0] : (this->ma_max[0]+this->ma_min[0])/2;//-std::numeric_limits<double>::min();//i%(OCTSIZE/1) > OCTSIZE/2
            double maxy = (i%2)         ? this->ma_max[1] : (this->ma_max[1]+this->ma_min[1])/2;//-std::numeric_limits<double>::min();//i%(OCTSIZE/2) > OCTSIZE/4
            double maxz = (i%4) >= 2    ? this->ma_max[2] : (this->ma_max[2]+this->ma_min[2])/2;//-std::numeric_limits<double>::min();//i%(OCTSIZE/4) > OCTSIZE/8

            if((this->ma_max[0]-this->ma_min[0]) > *divLimit) {//mache neue zweige auf
                mpa_Nodes[i] = new CNodeBranch<CElement>(this->m_level+1, minx, miny, minz, maxx, maxy, maxz);
            }
            else {//mache Bleatter

                mpa_Nodes[i] = new CNodeLeaf<CElement>  (this->m_level+1, minx, miny, minz, maxx, maxy, maxz);
            }
            *numNodes+=8;
        }
    }
    for(auto e : mpa_Nodes) {
        if(e->insert(x, y, z, dataset, divLimit, numNodes, numElements))//if inserted
            return true;//once inserted, don't insert more!
    }
    return false;
}

template <class CElement>
std::list<CElement> CNodeBranch<CElement>::getElements()
{

    std::list<CElement> l;
    if(getNode(0)) {
        for(auto e : mpa_Nodes) {
            std::list<CElement> cur = e->getElements();
            l.insert(l.end(), cur.begin(), cur.end());
        }
    }
    return l;
}

template <class CElement>
unsigned long CNodeBranch<CElement>::getTreeLevel()
{
    if(getNode(0))
    {
        for(auto e : mpa_Nodes) {
            unsigned long tmp = e->getLevel();
            if(tmp)
                return tmp;
        }
    }
    return 0;
}

/** Serializes all SUB-Nodes, BUT NOT THIS ONE*/
template <class CElement>
void CNodeBranch<CElement>::serialize(std::string path)
{
    if(getNode(0))
    {
        std::string tmpPath = path+node2str(this)+'/';
        ::createDirectories(tmpPath);
        for(auto e : mpa_Nodes)
        {
            e->serialize(tmpPath);
            if(e->isBranch())
                ::serializeBranch(dynamic_cast<CNodeBranch<CElement>* >(e), tmpPath);//use the global function
            else
                ::serializeLeaf(dynamic_cast<CNodeLeaf<CElement>* >(e), tmpPath);
            //don't delete here, the serialize stuff does this with a smartpointer!
        }
    }
    this->m_serialized = true;
}

/** Derializes all SUB-Nodes (if possible), BUT NOT THIS ONE, because
this one is not serialized!*/
template <class CElement>
void CNodeBranch<CElement>::deserialize(std::string nodepath)
{
    using namespace boost::filesystem;//makes my life easier
    nodepath = nodepath+node2str(this)+'/';
    if(this->m_serialized)
    {
        path p(nodepath);   // p reads clearer than argv[1] in the following code

        try//I hate try catch, but this is necessary
        {
            if (exists(p))    // does p actually exist?
            {
                if (is_directory(p))      // is p a directory?
                {
                    std::vector<path> v;
                    copy(directory_iterator(p), directory_iterator(), back_inserter(v)); // directory_iterator::value_type

                    int i = 0;
                    for(std::vector<path>::const_iterator it(v.begin()); it != v.end(); ++it) {

                        std::string curpath = canonical(*it).string();
                        if(curpath.rfind(mimeTypeOc) != std::string::npos) {
                            if(i >= OCSIZE) {
                                #ifdef OCDESERIALIZE
                                std::cout << "Tried to read more then " << OCSIZE << " files" << std::endl
                                    << "Somehow the Files got changed under tmp!" << std::endl;
                                #endif
                                break;
                            }
                            if(curpath.rfind(mimeTypeBranch) != std::string::npos)//if is Branch
                                mpa_Nodes[i] = ::deserializeBranch<CElement>(curpath);
                            else
                                mpa_Nodes[i] = ::deserializeLeaf<CElement>(curpath);
                            //deduction failes, so i need to pass CElement to the function, i have never seen this before

                            #ifdef OCDESERIALIZEALL
                            std::cout << i << " " << curpath << std::endl;
                            #endif
                            ++i;
                        }
                    }
                }
                else
                {
                    std::cout << p << " exists, but is not a directory!" << std::endl;
                    return;
                }
            }
            else
            {
                #ifdef OCDESERIALIZEALL
                std::cout << p << " does not exist!" << std::endl;
                #endif
                return;
            }
        }
        catch (const filesystem_error& ex)
        {
            std::cerr << ex.what() << std::endl;
            //return;
        }
    }
    for(auto e : mpa_Nodes) {
        e->deserialize(nodepath);
    }
    this->m_serialized = false;
}

/** Calls the draw() Method of each subnode*/
template <class CElement>
void CNodeBranch<CElement>::draw()
{
    if(getNode(0))
    {
        for(auto e : mpa_Nodes)
            e->draw();
    }
}

/** Calls the draw(args) Methof of each subnode if itself is in range**/
template <class CElement>
void CNodeBranch<CElement>::draw(double fromx, double fromy, double fromz, double tox, double toy, double toz)
{
    if(getNode(0) && this->isInRange(fromx, fromy, fromz, tox, toy, toz)) {
        for(auto e : mpa_Nodes)
            e->draw(fromx, fromy, fromz, tox, toy, toz);
    }
}

template <class CElement>
void CNodeBranch<CElement>::draw(double x, double y, double z)
{
    if(getNode(0) && this->isInNode(x, y, z)) {
        for(auto e : mpa_Nodes)
            e->draw(x, y, z);
    }
}

/**Node-Leaf*/
template <class CElement>//dtor
CNodeLeaf<CElement>::~CNodeLeaf()
{
    //maybe delete the elements here
}

//ctors
template <class CElement>
CNodeLeaf<CElement>::CNodeLeaf(CNodeLeaf<CElement>* other, bool isSeralized)
{
    std::memcpy(this->ma_min, other->ma_min, sizeof(this->ma_min));
    std::memcpy(this->ma_max, other->ma_max, sizeof(this->ma_max));
    this->m_level = other->m_level;
    this->m_serialized = isSeralized;
    mpa_Elements = other->mpa_Elements;
}

template <class CElement>
CNodeLeaf<CElement>::CNodeLeaf(unsigned int level, double x[OCDIM], double y[OCDIM]) {
    this->ma_min = x;
    this->ma_max = y;
    this->m_level = level;
}

template <class CElement>
CNodeLeaf<CElement>::CNodeLeaf(unsigned int level, double x1, double x2, double x3, double y1, double y2, double y3) {
    this->ma_min[0] = x1;
    this->ma_min[1] = x2;
    this->ma_min[2] = x3;
    this->ma_max[0] = y1;
    this->ma_max[1] = y2;
    this->ma_max[2] = y3;

    this->m_level = level;
}

template <class CElement>
bool CNodeLeaf<CElement>::insert(double x, double y, double z, CElement* dataset, double* divLimit, unsigned long* numNodes, unsigned long* numElements)
{
    if(!this->isInNode(x, y, z))
        return false;

    mpa_Elements.push_back(*dataset);
    #ifdef OCINSERT
    std::cout << "Inserted Element sucessfully" << std::endl;
    #endif
    (*numElements)++;
    return true;
}

/*Draw*/
template <class CElement>
void CNodeLeaf<CElement>::draw()
{
    #ifdef OCDRAWALL
    std::cout << "Draw an element sucessfully (" << mpa_Elements.size() << " Elements)" << std::endl;
    #endif
    //TODO draw all elements
}

template <class CElement>
void CNodeLeaf<CElement>::draw(double fromx, double fromy, double fromz, double tox, double toy, double toz)
{
    if(this->isInRange(fromx, fromy, fromz, tox, toy, toz)) {
        //TODO: draw all elements of a node or only the ones in range???
        #ifdef OCDRAW
        std::cout << "Draw an element sucessfully (" << mpa_Elements.size() << " Elements)" << std::endl;
        #endif
    }
}

template <class CElement>
void CNodeLeaf<CElement>::draw(double x, double y, double z)
{
    if(this->isInNode(x, y, z)) {
        //TODO: draw all elements of a node or only the ones in range???
        #ifdef OCDRAW
        std::cout << "Draw an element sucessfully (" << mpa_Elements.size() << " Elements)" << std::endl;
        #endif
    }
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
