#pragma once

#include "data/Entity.h"

// A viewport of a scene.
class IViewport
{
public:
	Entity CreateEntity(Entity::Type type)
	{
		return Entity{ type, _registry.create(), &_registry };
	};

	void RemoveEntity(Entity entity)
	{
		_registry.destroy(entity);
	};

	const entt::registry& GetRegistry() const { return _registry; }

protected:
	entt::registry _registry;
};
