#pragma once
#include "Object3D.h"
#include "Animation.h"
#include "SkeletalObject.h"

/**
 * @brief Rotates an object at a continuous rate over an interval.
 */
template<class T>
class RotationAnimation : public Animation<T> {
private:
	/**
	 * @brief How much to increment the orientation by each second.
	 */
	glm::vec3 m_perSecond;

	/**
	 * @brief Advance the animation by the given time interval.
	 */
	void applyAnimation(float_t dt) override {
		//std::cout << "dt " << m_perSecond << "\n";
		this->object().rotate(m_perSecond * dt);
	}

public:
	using Animation<T>::object;
	/**
	 * @brief Constructs a animation of a constant rotation by the given total rotation 
	 * angle, linearly interpolated across the given duration.
	 */
	RotationAnimation(T& object, float_t duration, const glm::vec3& totalRotation) : 
		Animation<T>(object, duration), m_perSecond(totalRotation / duration) {

	}
};

