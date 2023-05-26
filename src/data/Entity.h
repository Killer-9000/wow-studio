#pragma once

#include "components/Doodad.h"
#include "components/MapTile.h"
#include "components/Model.h"
#include "components/WorldModelObject.h"

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/detail/type_quat.hpp>

struct Entity
{
public:
	struct Transform
	{
		glm::vec3 position;
		glm::quat rotation;
		glm::vec3 scale;
	};

	enum class Type
	{
		MapTile = 0,
		Model,
		WMO,
		Doodad,
	};

	Entity() { }

	Entity(Type type, entt::entity entity, entt::registry* registry)
		: _entity(entity), _registry(registry)
	{
		_typeComponent = &AddComponent<Type>(type);
		_transformComponent = &AddComponent<Transform>();
		switch (type)
		{
		case Type::MapTile: _mapTileComponent = &AddComponent<MapTile>(this); break;
		case Type::Model: _modelComponent = &AddComponent<Model>(this); break;
		case Type::WMO: _WMOComponent = &AddComponent<WorldModelObject>(this); break;
		case Type::Doodad: _doodadComponent = &AddComponent<Doodad>(this); break;
		default:
			assert(false && "Unknown Entity Type.");
		}
	}

	template <class T, class... Args>
	T& AddComponent(Args&&... args)
	{
		assert(!HasComponent<T>() && "Entity already has this component.");
		return _registry->emplace<T>(_entity, std::forward<Args>(args)...);
	}

	template <class T>
	T& GetComponent()
	{
		assert(HasComponent<T>() && "Entity doesn't have component.");
		return _registry->get<T>(_entity);
	}

	template <class T>
	bool HasComponent()
	{
		assert(_entity != entt::null && "Entity is null.");
		return _registry->any_of<T>(_entity);
	}

	template <class T>
	void RemoveComponent()
	{
		assert(HasComponent<T>() && "Entity doesn't have component.");
		_registry->remove<T>(_entity);
	}

	operator const entt::entity() { return _entity; }
	operator bool() { return _entity != entt::null; }

	// Cached components
	Type* GetType() { return _typeComponent; }
	Transform* GetTransform() { return _transformComponent; }

	MapTile* GetMapTile() { return _mapTileComponent; }
	Model* GetModel() { return _modelComponent; }
	WorldModelObject* GetWorldModelObject() { return _WMOComponent; }
	Doodad* GetDoodad() { return _doodadComponent; }

private:
	entt::entity _entity = entt::null;
	entt::registry* _registry = nullptr;

	// Cached components
	Type* _typeComponent = nullptr;
	Transform* _transformComponent = nullptr;

	MapTile* _mapTileComponent = nullptr;
	Model* _modelComponent = nullptr;
	WorldModelObject* _WMOComponent = nullptr;
	Doodad* _doodadComponent = nullptr;
};