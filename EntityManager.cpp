#include "EntityManager.h"
#include <iostream>

EntityManager::EntityManager()
    : m_totalEntities(0)
{
}

void EntityManager::update()
{
    // add entities from m_entitiesToAdd to the proper locations
    // add them to the vector of all entiteis
    // add them to the vector inside the map with the tag as the key

    for (auto e : m_entitiesToAdd)
    {
        m_entities.push_back(e);
        m_entityMap[e->tag()].push_back(e);
    }

    // remove dead entities from the vector of all entities
    removeDeadEntities(m_entities);

    // remove dead entities from each vector in the entity map
    // C++17 way of iterating through [key,value] pairs in a map
    for (auto &[tag, entityVec] : m_entityMap)
    {
        removeDeadEntities(entityVec);
    }
    m_entitiesToAdd.clear();
}

void EntityManager::removeDeadEntities(EntityVec &vec)
{

    // use removeif to remove the entity from the vector

    vec.erase(std::remove_if(vec.begin(), vec.end(), [](const std::shared_ptr<Entity> &e)
                             { return !e->isActive(); }),
              vec.end());
}

std::shared_ptr<Entity> EntityManager::addEntity(const std::string &tag)
{
    auto entity = std::shared_ptr<Entity>(new Entity(m_totalEntities++, tag));
    m_entitiesToAdd.push_back(entity);

    return entity;
}

const EntityVec &EntityManager::getEntities()
{
    return m_entities;
}

const EntityVec &EntityManager::getEntities(const std::string &tag)
{
    return m_entityMap[tag];
}