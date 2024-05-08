#pragma once
#include <memory>
#include "Animation.h"
#include "RotationAnimation.h"
#include "TranslationAnimation.h"
#include <functional>

template<class T>
class Animator {
private:
	/**
	 * @brief How much time has elapsed since the animation started.
	 */
	float_t m_currentTime;
	/**
	 * @brief The time at which we transition to the next animation.
	 */
	float_t m_nextTransition;
	/**
	 * @brief The sequence of animations to play.
	 */
	std::vector<std::function<std::unique_ptr<Animation<T>>(void)>> m_animations;
	/**
	 * @brief The current (active) animation.
	 */
	std::unique_ptr<Animation<T>> m_currentAnimation;
	/**
	 * @brief The index of the current animation.
	 */
	int32_t m_currentIndex;

	bool repeat;
	
	/**
	 * @brief Activate the next animation.
	 */
	void nextAnimation() {
		++m_currentIndex;
		if (m_currentIndex < m_animations.size()) {
			m_currentAnimation = m_animations[m_currentIndex]();
			m_currentAnimation->start();
			m_nextTransition = m_nextTransition + m_currentAnimation->duration();
		}
		else {
			m_currentIndex = -1;
			m_currentAnimation = nullptr;
			if (repeat) {
				start();
			}
		}
	};

public:
	/**
	 * @brief Constructs an Animator that acts on the given object.
	 */
	Animator() :
		m_currentTime(0),
		m_nextTransition(0),
		m_currentIndex(-1), 
		repeat(false),
		m_currentAnimation(nullptr) {
	}

	/**
	 * @brief Add an Animation to the end of the animation sequence.
	 */
	void addAnimation(std::function<std::unique_ptr<Animation<T>>(void)> animationFactory) {
		m_animations.emplace_back(std::move(animationFactory));
	}

	/**
	 * @brief Activate the Animator, causing its active animation to receive future tick() calls.
	 */
	void start() {
		m_currentTime = 0;
		m_nextTransition = 0;
		nextAnimation();
	};

	/**
	 * @brief Advance the animation sequence by the given time interval, in seconds.
	 */
	void tick(float_t dt) {
		if (m_currentIndex >= 0) {
			float_t lastTime = m_currentTime;
			m_currentTime += dt;
			// If our current time surpasses the next transition time, we need to tick
			// both the active animation (up to the transition time), and the subsequent animation
			// (by the amount we exceeded the transition time).
			if (m_currentTime >= m_nextTransition) {
				m_currentAnimation->tick(m_nextTransition - lastTime);
				float_t overTime = m_currentTime - m_nextTransition;
				nextAnimation();
				if (m_currentAnimation != nullptr) {
					m_currentAnimation->tick(overTime);
				}
			}
			else {
				m_currentAnimation->tick(dt);
			}
		}
	}

	bool finish() {
		return m_currentAnimation == nullptr;
	}

	void clearAnimation() {
		m_animations.clear();
	}

	void setRepeat(bool val) {
		repeat = val;
	}
};