#pragma once
#include <vector>
#include <unordered_map>
#include <memory>
#include <typeindex>
#include <any>
#include <array>
#include <cstdint>

using Entity = std::uint32_t;
const Entity MAX_ENTITIES = 5000;

class ComponentArray {
public:
    virtual ~ComponentArray() = default;
    virtual void entityDestroyed(Entity entity) = 0;
};

template<typename T>
class ComponentArrayImpl : public ComponentArray {
private:
    std::array<T, MAX_ENTITIES> componentArray;
    std::unordered_map<Entity, size_t> entityToIndexMap;
    std::unordered_map<size_t, Entity> indexToEntityMap;
    size_t size = 0;

public:
    void insertData(Entity entity, T component) {
        if (entityToIndexMap.find(entity) != entityToIndexMap.end()) {
            return;
        }

        size_t newIndex = size;
        entityToIndexMap[entity] = newIndex;
        indexToEntityMap[newIndex] = entity;
        componentArray[newIndex] = component;
        ++size;
    }

    void removeData(Entity entity) {
        if (entityToIndexMap.find(entity) == entityToIndexMap.end()) {
            return;
        }

        size_t indexOfRemovedEntity = entityToIndexMap[entity];
        size_t indexOfLastElement = size - 1;
        componentArray[indexOfRemovedEntity] = componentArray[indexOfLastElement];

        Entity entityOfLastElement = indexToEntityMap[indexOfLastElement];
        entityToIndexMap[entityOfLastElement] = indexOfRemovedEntity;
        indexToEntityMap[indexOfRemovedEntity] = entityOfLastElement;

        entityToIndexMap.erase(entity);
        indexToEntityMap.erase(indexOfLastElement);

        --size;
    }

    T& getData(Entity entity) {
        return componentArray[entityToIndexMap[entity]];
    }

    bool hasData(Entity entity) {
        return entityToIndexMap.find(entity) != entityToIndexMap.end();
    }

    void entityDestroyed(Entity entity) override {
        if (entityToIndexMap.find(entity) != entityToIndexMap.end()) {
            removeData(entity);
        }
    }

    std::vector<Entity> getEntities() {
        std::vector<Entity> entities;
        for (const auto& pair : entityToIndexMap) {
            entities.push_back(pair.first);
        }
        return entities;
    }
};

class ComponentManager {
private:
    std::unordered_map<std::type_index, std::shared_ptr<ComponentArray>> componentArrays;

    template<typename T>
    std::shared_ptr<ComponentArrayImpl<T>> getComponentArray() {
        std::type_index typeIndex(typeid(T));

        if (componentArrays.find(typeIndex) == componentArrays.end()) {
            componentArrays[typeIndex] = std::make_shared<ComponentArrayImpl<T>>();
        }

        return std::static_pointer_cast<ComponentArrayImpl<T>>(componentArrays[typeIndex]);
    }

public:
    template<typename T>
    void addComponent(Entity entity, T component) {
        getComponentArray<T>()->insertData(entity, component);
    }

    template<typename T>
    void removeComponent(Entity entity) {
        getComponentArray<T>()->removeData(entity);
    }

    template<typename T>
    T& getComponent(Entity entity) {
        return getComponentArray<T>()->getData(entity);
    }

    template<typename T>
    bool hasComponent(Entity entity) {
        return getComponentArray<T>()->hasData(entity);
    }

    void entityDestroyed(Entity entity) {
        for (auto const& pair : componentArrays) {
            auto const& component = pair.second;
            component->entityDestroyed(entity);
        }
    }

    template<typename T>
    std::vector<Entity> getEntitiesWithComponent() {
        return getComponentArray<T>()->getEntities();
    }
};

class EntityManager {
private:
    std::vector<Entity> availableEntities;
    uint32_t livingEntityCount = 0;

public:
    EntityManager() {
        for (Entity entity = MAX_ENTITIES - 1; entity >= 0; --entity) {
            availableEntities.push_back(entity);
            if (entity == 0) break;
        }
    }

    Entity createEntity() {
        Entity id = availableEntities.back();
        availableEntities.pop_back();
        ++livingEntityCount;
        return id;
    }

    void destroyEntity(Entity entity) {
        availableEntities.push_back(entity);
        --livingEntityCount;
    }
};

class ECS {
private:
    std::unique_ptr<ComponentManager> componentManager;
    std::unique_ptr<EntityManager> entityManager;

public:
    ECS() {
        componentManager = std::make_unique<ComponentManager>();
        entityManager = std::make_unique<EntityManager>();
    }

    Entity createEntity() {
        return entityManager->createEntity();
    }

    void destroyEntity(Entity entity) {
        entityManager->destroyEntity(entity);
        componentManager->entityDestroyed(entity);
    }

    template<typename T>
    void addComponent(Entity entity, T component) {
        componentManager->addComponent<T>(entity, component);
    }

    template<typename T>
    void removeComponent(Entity entity) {
        componentManager->removeComponent<T>(entity);
    }

    template<typename T>
    T& getComponent(Entity entity) {
        return componentManager->getComponent<T>(entity);
    }

    template<typename T>
    bool hasComponent(Entity entity) {
        return componentManager->hasComponent<T>(entity);
    }

    template<typename T>
    std::vector<Entity> getEntitiesWithComponent() {
        return componentManager->getEntitiesWithComponent<T>();
    }
};