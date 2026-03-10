#pragma once
#include <unordered_map>
#include <typeindex>
#include <vector>
#include <cstdint>
#include <queue>

using EntityID = uint32_t;

class IComponentArray {
public: 
	virtual ~IComponentArray() = default;
	virtual void OnEntityDestroyed(EntityID entity) = 0;
};

template<typename T>
class ComponentArray : public IComponentArray {
public: 
	void Insert(EntityID entity, T component) {
		m_data[entity] = component;
	}

	void Remove(EntityID entity) {
		m_data.erase(entity);
	}

	T& Get(EntityID entity) {
		return m_data[entity];
	}

	bool Has(EntityID entity) {
		return m_data.find(entity) != m_data.end();
	}

	void OnEntityDestroyed(EntityID entity) override {
		if (Has(entity)) Remove(entity);
	}

	std::unordered_map<EntityID, T>& GetData() {
		return m_data;
	}

private:
	std::unordered_map<EntityID, T> m_data;
};

class EntityManager {
public: 
	EntityID CreateEntity() {
		return m_nextID++;
	}

	void DestroyEntity(EntityID entity) {
		for (auto& [type, array] : m_componentArrays) {
			array->OnEntityDestroyed(entity);
		}
	}

	template<typename T>
	void AddComponent(EntityID entity, T component) {
		GetOrCreateArray<T>()->Insert(entity, component);
	}

	template<typename T>
	void RemoveComponent(EntityID entity) {
		GetOrCreateArray<T>()->Remote(entity);
	}

	template<typename T>
	T& GetComponent(EntityID entity) {
		return GetOrCreateArray<T>()->Get(entity);
	}

	template<typename T>
	bool HasComponent(EntityID entity) {
		return GetOrCreateArray<T>()->Has(entity);
	}

	template<typename First, typename... Rest>
	std::vector<EntityID> GetEntitiesWith() {
		std::vector<EntityID> result;

		auto& firstData = GetOrCreateArray<First>()->GetData();

		for (auto& [entity, _] : firstData) {
			bool hasAll = (HasComponent<Rest>(entity) && ...);
			if (HasComponent<First>(entity) && hasAll) {
				result.push_back(entity);
			}
		}

		return result;
	}

private:
	uint32_t m_nextID = 0;
	std::queue<EntityID> m_freeIDs; // TODO: implement ID recycling
	std::unordered_map<std::type_index, IComponentArray*> m_componentArrays;

	template<typename T>
	ComponentArray<T>* GetOrCreateArray() {
		auto key = std::type_index(typeid(T));
		if (m_componentArrays.find(key) == m_componentArrays.end()) {
			m_componentArrays[key] = new ComponentArray<T>();
		}
		return static_cast<ComponentArray<T>*>(m_componentArrays[key]);
	}
};