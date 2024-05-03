#pragma once
#include "Object3D.h"
#include "Animation.h"

template<class T>
class TranslationAnimation : public Animation<T> {
private:
	glm::vec3 m_translation;

	void applyAnimation(float_t dt) override {
		this->object().move(m_translation * dt);
	}
public:
	TranslationAnimation(T& object, float_t duration, 
		const glm::vec3& totalMovement) :
		Animation<T>(object, duration), m_translation(totalMovement / duration) {}
};
