#ifndef OCCONTAINER_H
#define OCCONTAINER_H

#include <vector>
#include <iostream>

template <class T>
class COcContainer
{
public:
     COcContainer();
    ~COcContainer();

     T* getFromId(int id);
     void add(T* object);
     void decrementAllUsedCount();
     void removeAllUnused();
     std::vector<T*> objects;
};
#endif // OCCONTAINER_H

template <class T>
COcContainer<T>::COcContainer()
{
}

template <class T>
COcContainer<T>::~COcContainer()
{
    for (int i=0;i<int(objects.size());i++)
        delete objects[i];
}

template <class T>
void COcContainer<T>::decrementAllUsedCount()
{
    for (int i=0;i<int(objects.size());i++)
        objects[i]->decrementUsedCount();
}

template <class T>
void COcContainer<T>::removeAllUnused()
{
    for (int i=0;i<int(objects.size());i++)
    {
        if (objects[i]->getUsedCount()<=0)
        {
            objects.erase(objects.begin()+i);
            i--; // reprocess this position
        }
    }
}

template <class T>
T* COcContainer<T>::getFromId(int id)
{
    for (int i=0;i<int(objects.size());i++)
    {
        if (objects[i]->getId()==id)
            return(objects[i]);
    }
    return(0);
}

template <class T>
void COcContainer<T>::add(T* object)
{
    objects.push_back(object);
}
