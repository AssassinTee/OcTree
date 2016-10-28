#ifndef OCTREE_H
#define OCTREE_H

#include <list>
#include <string>
#include <limits>//<< Operator
#include <iomanip>//<< Operator

//#define CEREAL_THREAD_SAFE 1//This could be saver and better, i don't know
#include <cereal/archives/binary.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/list.hpp>

#define OCDIM 3 //size of the dimension for the tree
#define OCSIZE 8 //size of an octal tree(!!!); its 2^3;

/**DRAWING INTERFACE*/
class CDrawTree
{
    public:
        virtual void draw() = 0;
        virtual void draw(double fromx, double fromy, double fromz, double tox, double toy, double toz) = 0;
        virtual void draw(double x, double y, double z) = 0;
    private:
};

/**NODES*/
template <class CElement>
class CNode : public CDrawTree
{
    public:
        virtual ~CNode() {};
        virtual bool insert(double x, double y, double z, CElement* elem, double* divLimit, unsigned long* numNodes, unsigned long* numElements) = 0;
        virtual std::list<CElement> getElements() = 0;
        virtual CNode<CElement>* getNode(int index) = 0;
        virtual void serialize(std::string path) = 0;
        virtual void deserialize(std::string path) = 0;
        virtual bool isBranch() = 0;

        virtual unsigned long getTreeLevel() = 0;
        unsigned int getLevel() {return m_level;};

        //TODO: virtual *unkown find(x, y, z) = 0; really? or is draw enough
        template <class AnotherCElement>//Eleminates shadowing?!
        friend std::ostream& operator<<(std::ostream& os, CNode<AnotherCElement>& node);

        template <class Archive>
        void serialize( Archive & ar )
        {
            ar( cereal::binary_data( ma_min, sizeof(double)*OCDIM),
               cereal::binary_data( ma_max, sizeof(double)*OCDIM),
               m_level);
            //std::cout << "Base got serialized" << std::endl;
        }
    private:



    protected:
        //virtual bool isLeaf() = 0; //Needed?
        bool isInRange(double fromx, double fromy, double fromz, double tox, double toy, double toz);
        bool isInNode(double x, double y, double z);

        //TODO: Limits!
        unsigned int m_level;
        bool m_serialized;//Needed!
        double ma_min[OCDIM];
        double ma_max[OCDIM];
};

template <class CElement>
class CNodeBranch : public CNode<CElement>
{
    public:
        CNodeBranch() {};
        CNodeBranch(CNodeBranch* other);
        CNodeBranch(CNodeBranch* other, bool isSerialized);
        CNodeBranch(unsigned int level, double x[OCDIM], double y[OCDIM], bool isSerialized=false);
        CNodeBranch(unsigned int level, double x1, double x2, double x3, double y1, double y2, double y3, bool isSerialized=false);
        virtual ~CNodeBranch();

        CNode<CElement>* getNode(int index){ return mpa_Nodes[index];};
        bool insert(double x, double y, double z, CElement* elem, double* divLimit, unsigned long* numNodes, unsigned long* numElements);
        std::list<CElement> getElements();
        unsigned long getTreeLevel();
        bool isBranch() {return true;};

        //Serialization
        void serialize(std::string path);
        void deserialize(std::string path);

        template<class Archive>
        void serialize(Archive & archive)
        {
            archive( cereal::base_class<CNode<CElement> >(this)); // serialize things by passing them to the archive
        }

        //Draw
        void draw();//should call the draw of all subnodes
        void draw(double fromx, double fromy, double fromz, double tox, double toy, double toz);
        void draw(double x, double y, double z);

        // Register DerivedClassOne
    protected:
    private:
        CNode<CElement>* mpa_Nodes[OCSIZE]{0};//i'd like it more to init this in the constructor, but ok
};

template <class CElement>
class CNodeLeaf : public CNode<CElement>
{
    public:
        CNodeLeaf() {};
        CNodeLeaf(CNodeLeaf* other, bool isSeralized);
        CNodeLeaf(unsigned int level, double x[OCDIM], double y[OCDIM]);
        CNodeLeaf(unsigned int level, double x1, double x2, double x3, double y1, double y2, double y3);
        virtual ~CNodeLeaf();

        CNode<CElement>* getNode(int index){ return nullptr;};
        bool insert(double x, double y, double z, CElement* elem, double* divLimit, unsigned long* numNodes, unsigned long* numElements);
        std::list<CElement> getElements() { return mpa_Elements;};
        unsigned long getTreeLevel() {return this->getLevel();};
        bool isBranch() {return false;};

        //Serialization
        void serialize(std::string path) {return;};//Nothing to do here
        void deserialize(std::string path) {return;};//Nothing to do here

        template<class Archive>
        void serialize(Archive & archive)//seralize the list of pointers!
        {
            archive( cereal::base_class<CNode<CElement> >(this));
            archive( mpa_Elements ); // serialize things by passing them to the archive
            //std::cout << "Leaf got serialized" << std::endl;
        }

        //Draw
        void draw();//should draw the Elements in the list
        void draw(double fromx, double fromy, double fromz, double tox, double toy, double toz);
        void draw(double x, double y, double z);

    protected:
    private:
        std::list<CElement> mpa_Elements;
};

/**NODE-GLOBAL*/
template <class CElement>
static void serializeBranch(CNodeBranch<CElement>* node);

template <class CElement>
static void serializeLeaf(CNodeLeaf<CElement>* node);

template <class CElement>
static CNodeBranch<CElement>* deserializeBranch(const std::string& b);

template <class CElement>
static CNodeLeaf<CElement>* deserializeLeaf(const std::string& b);

template <class CElement>
static std::string node2str(CNode<CElement>* node);

template <class CElement>
std::ostream& operator<<(std::ostream& os, CNode<CElement>& node);

void createDirectories(std::string path);

/**OCTREE-GLOBAL*/
static const std::string serializationPath = "../ocdata/";
static const std::string mimeTypeOc = ".oc";
static const std::string mimeTypeBranch = ".ocbranch";
static const std::string mimeTypeLeaf = ".ocleaf";
static const std::string mimeTypeTreeData = ".ocdata";

/**OCTREE*/
template <class CElement>//Element to insert in the octree//Why a template? Later to Vector class
class COcTree : CDrawTree
{
    public:
        COcTree(std::string name, double divLimit, double y1, double y2, double y3);//TODO: konstruktor bekommt daten
        COcTree(std::string name, double divLimit, double x[OCDIM], double y[OCDIM]);
        COcTree(std::string name, double divLimit, double x1, double x2, double x3, double y1, double y2, double y3);
        virtual ~COcTree();//TODO: Destruktor???
        bool insert(double p[OCDIM], CElement* dataset);//Haenge in den Baum ein
        bool insert(double x, double y, double z, CElement* dataset);//Haenge in den Baum ein
        void clear();

        //Getter
        unsigned long numNodes() {return m_numNodes;};
        unsigned long numElements() {return m_numElements;};
        unsigned long getLevels();

        //Draw
        void draw();
        void draw(double fromx, double fromy, double fromz, double tox, double toy, double toz);
        void draw(double x, double y, double z);

        //Serialize: TODO: Change/Remove this
        void serialize() {mp_Origin->serialize(serializationPath);};
        void deserialize() {mp_Origin->deserialize(serializationPath);};
        void save(); //{};
        void load(); //{mp_Origin->deserialize(serializationPath+m_Name+'/');}
        bool isLoadable();

    protected:
    private:
        void serializeTreeData();
        void deserializeTreeData();

        void setData(std::string name, double divLimit);//Helper-Funktion fuer den Konstruktor

        //Variabeln
        CNode<CElement>* mp_Origin;//Wurzelknoten
        double m_divLimit;
        bool m_dataChanged;//triggers save on delete
        unsigned long m_numNodes;//Needed??     Anzahl der Nodes
        unsigned long m_numElements;//Needed??  Anzahl der Elemente des Baumes
        std::string m_Name;//Name is needed, when you restart the programm, you can actually deserialize it if the path is there!!!
                            //The Name is required to find the right OCTree and don't run into abinguoity (multy purpose fails)
};


// Register EmbarassingDerivedClass with a less embarrasing name
CEREAL_REGISTER_TYPE(CNodeBranch<int>);
CEREAL_REGISTER_TYPE_WITH_NAME(CNodeLeaf<int>, "CNodeLeaf");
//CEREAL_REGISTER_TYPE_WITH_NAME(std::list<int>, "mpa_Elements");

// Note that there is no need to register the base class, only derived classes
//  However, since we did not use cereal::base_class, we need to clarify
//  the relationship (more on this later)
CEREAL_REGISTER_POLYMORPHIC_RELATION(CNode<int>, CNodeBranch<int>);
CEREAL_REGISTER_POLYMORPHIC_RELATION(CNode<int>, CNodeLeaf<int>);

#endif // OCTREE_H
