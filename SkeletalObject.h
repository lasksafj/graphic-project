#pragma once
#include <memory>
#include <vector>
#include "SkeletalMesh.h"
#include "ShaderProgram.h"
/**
 * @brief Represents an object placed in a 3D scene. The object is a node in an hierarchy of
 * objects representing a single 3D model. Each object in the hierarchy has its own position,
 * orientation, and scale, by which it uniformly transforms a list of meshes in the object.
*/
class SkeletalObject {
private:
	// The object's list of meshes and children.
	std::vector<SkeletalMesh> m_meshes;
	std::vector<SkeletalObject> m_children;

	// The object's position, orientation, and scale in world space.
	glm::vec3 m_position;
	glm::vec3 m_orientation;
	glm::vec3 m_scale;
	glm::vec3 m_center;

	// The object's cached local->world transformation matrix.
	glm::mat4 m_modelMatrix;
	glm::mat4 m_baseTransform;

	// Some objects from Assimp imports have a "name" field, useful for debugging.
	std::string m_name;


	// Velocity
	glm::vec3 velocity;
	glm::vec3 rotational_velocity;
	// Acceleration
	//glm::vec3 acceleration;
	glm::vec3 rotational_acceleration;
	std::vector<glm::vec3> forces_list;
	// Object mass
	float_t mass;

	// Recomputes the local->world transformation matrix.
	void rebuildModelMatrix();

public:
	// No default constructor; you must have a mesh to initialize an object.
	SkeletalObject() = default;

	SkeletalObject(std::vector<SkeletalMesh>&& meshes);
	SkeletalObject(std::vector<SkeletalMesh>&& meshes, const glm::mat4& baseTransform);

	// Simple accessors.
	const glm::vec3& getPosition() const;
	const glm::vec3& getOrientation() const;
	const glm::vec3& getScale() const;
	const glm::vec3& getCenter() const;
	const std::string& getName() const;

	// Child management.
	size_t numberOfChildren() const;
	const SkeletalObject& getChild(size_t index) const;
	SkeletalObject& getChild(size_t index);

	// Simple mutators.
	void setPosition(const glm::vec3& position);
	void setOrientation(const glm::vec3& orientation);
	void setScale(const glm::vec3& scale);
	void setCenter(const glm::vec3& center);
	void setName(const std::string& name);

	// Transformations.
	void move(const glm::vec3& offset);
	void rotate(const glm::vec3& rotation);
	void grow(const glm::vec3& growth);
	void addChild(SkeletalObject&& child);

	// Rendering.
	void render(sf::RenderWindow& window, ShaderProgram& shaderProgram) const;
	void renderRecursive(sf::RenderWindow& window, ShaderProgram& shaderProgram, const glm::mat4& parentMatrix) const;



	// tick
	void tick(float_t dt);

	// add force
	void addForce(const glm::vec3& force);

	// Mass
	void setMass(float_t nMass) {
		mass = nMass;
	}
	float_t getMass() {
		return mass;
	}

	// Velocity
	void setVelocity(const glm::vec3& nVelocity) {
		velocity = nVelocity;
	}
	glm::vec3 getVelocity() {
		return velocity;
	}

	// rotational_velocity
	void setRotationalVelocity(const glm::vec3& n_rotational_velocity) {
		rotational_velocity = n_rotational_velocity;
	}
	glm::vec3 getRotationalVelocity() {
		return rotational_velocity;
	}

	void setRotationalAcceleration(const glm::vec3& n_rotational_acceleration) {
		rotational_acceleration = n_rotational_acceleration;
	}
	glm::vec3 getRotationalAcceleration() {
		return rotational_acceleration;
	}


	void addTexture(Texture texture);
};

