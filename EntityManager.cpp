#include "EntityManager.h"

EntityManager::EntityManager()
{
}

void EntityManager::update()
{

    for (auto e : m_entitiesToAdd)
    {
        m_entities.push_back(e);
        // m_entityMap[tag].push_back(entity);
    }

    m_entitiesToAdd.clear();

    // remove dead entities
    removeDeadEntities(m_entities);

    // add new entities
    for (auto &[tag, entityVec] : m_entityMap)
    {
        removeDeadEntities(entityVec);
    }
}

void EntityManager::removeDeadEntities(EntityVec &vec)
{
    /* vec.erase(std::remove_if(vec.begin(), vec.end(), [](const std::shared_ptr<Entity> &entity) {
                   return !entity->isActive();
               }),
               vec.end());*/

    // use std::remove_if to move all dead entities to the end of the vector
}

std::shared_ptr<Entity> EntityManager::addEntity(const std::string &tag)
{
    auto entity = std::make_shared<Entity>(m_totalEntities++, tag);
    m_entitiesToAdd.push_back(entity);
    m_entityMap[tag].push_back(entity); // wasnt in original code
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