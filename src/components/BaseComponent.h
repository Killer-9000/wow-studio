#pragma once

struct Entity;

struct BaseComponent
{
	BaseComponent() = delete;

	BaseComponent(Entity* entity)
		: _entity(entity)
	{ }

	Entity* _entity;
};