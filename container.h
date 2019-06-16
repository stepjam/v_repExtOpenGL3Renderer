#ifndef CONTAINER_H
#define CONTAINER_H

#include <vector>
#include <iostream>

template <class T>
class COcContainer
{
public:
     COcContainer();
    ~COcContainer();

     T* getFromId(int id);
     void removeAll();
     void add(T* object);
     void decrementAllUsedCount();
     void removeAllUnused();
     std::vector<T*> objects;

     int getIndex(int id);
     void removeAllFromIndex(int index);
};

template <class T>
COcContainer<T>::COcContainer()
{
}

template <class T>
COcContainer<T>::~COcContainer()
{
    removeAll();
}

template <class T>
void COcContainer<T>::removeAll()
{
    for (int i=0;i<int(objects.size());i++){
        delete objects[i];
    }
    objects.clear();
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
            delete objects[i];
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

template <class T>
int COcContainer<T>::getIndex(int id)
{
    for (int i=0;i<int(objects.size());i++)
    {
        if (objects[i]->getId()==id)
            return(i);
    }
    return(0);
}

template <class T>
void COcContainer<T>::removeAllFromIndex(int id)
{
    for (int i=id;i<int(objects.size());i++)
    {
        objects.erase(objects.begin()+i);
        i--; // reprocess this position
    }
}

#endif // CONTAINER_H
