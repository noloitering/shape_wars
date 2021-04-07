#pragma once

#include "Component.h"
#include <vector>
#include <memory>
#include <string>

class Entity {
	friend class EntityManager;
	private:
		const size_t m_id = 0;
		const std::string m_tag = "default";
		bool m_active = true;
		
		Entity(const size_t & id, const std::string & tag, bool active = true)
			: m_id(id), m_tag(tag), m_active(active) {}
		void destroy();
	public:
		// components
		std::shared_ptr<CTransform> cTransform;
		std::shared_ptr<CShape> cShape;
		std::shared_ptr<CCollision> cCollision;
		std::shared_ptr<CInput> cInput;
		std::shared_ptr<CScore> cScore;
		std::shared_ptr<CDuration> cDuration;
		std::shared_ptr<CDash> cDash;
		// getters
		bool isActive() const;
		const std::string & tag() const;
		const size_t & id() const;
};