#include "Entity.h"

void Entity::destroy()
{
	m_active = false;
}

bool Entity::isActive() const
{
	return m_active;
}

const size_t & Entity::id() const
{
	return m_id;
}

const std::string & Entity::tag() const
{
	return m_tag;
}
