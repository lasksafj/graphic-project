#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <glm/ext.hpp>
#include "SkeletalObject.h"
#include <iostream>


void SkeletalObject::rebuildModelMatrix() {
	auto m = glm::translate(glm::mat4(1), m_position);
	m = glm::translate(m, m_center * m_scale);
	m = glm::rotate(m, m_orientation[2], glm::vec3(0, 0, 1));
	m = glm::rotate(m, m_orientation[0], glm::vec3(1, 0, 0));
	m = glm::rotate(m, m_orientation[1], glm::vec3(0, 1, 0));
	m = glm::scale(m, m_scale);
	m = glm::translate(m, -m_center);
	m = m * m_baseTransform;
	m_modelMatrix = m;
}

SkeletalObject::SkeletalObject(std::vector<SkeletalMesh>&& meshes)
	: SkeletalObject(std::move(meshes), glm::mat4(1)) {
	rebuildModelMatrix();
}

SkeletalObject::SkeletalObject(std::vector<SkeletalMesh>&& meshes, const glm::mat4& baseTransform)
	: m_meshes(meshes), m_position(), m_orientation(), m_scale(1.0),
	m_center(), m_baseTransform(baseTransform)
{
	rebuildModelMatrix();
}

const glm::vec3& SkeletalObject::getPosition() const {
	return m_position;
}

const glm::vec3& SkeletalObject::getOrientation() const {
	return m_orientation;
}

const glm::vec3& SkeletalObject::getScale() const {
	return m_scale;
}

/**
 * @brief Gets the center of the object's rotation.
 */
const glm::vec3& SkeletalObject::getCenter() const {
	return m_center;
}

const std::string& SkeletalObject::getName() const {
	return m_name;
}

size_t SkeletalObject::numberOfChildren() const {
	return m_children.size();
}

const SkeletalObject& SkeletalObject::getChild(size_t index) const {
	return m_children[index];
}

SkeletalObject& SkeletalObject::getChild(size_t index) {
	return m_children[index];
}

void SkeletalObject::setPosition(const glm::vec3& position) {
	m_position = position;
	rebuildModelMatrix();
}

void SkeletalObject::setOrientation(const glm::vec3& orientation) {
	m_orientation = orientation;
	rebuildModelMatrix();
}

void SkeletalObject::setScale(const glm::vec3& scale) {
	m_scale = scale;
	rebuildModelMatrix();
}

/**
 * @brief Sets the center point of the object's rotation, which is otherwise a rotation around
   the origin in local space..
 */
void SkeletalObject::setCenter(const glm::vec3& center)
{
	m_center = center;
}

void SkeletalObject::setName(const std::string& name) {
	m_name = name;
}

void SkeletalObject::move(const glm::vec3& offset) {
	m_position = m_position + offset;
	rebuildModelMatrix();
}

void SkeletalObject::rotate(const glm::vec3& rotation) {
	m_orientation = m_orientation + rotation;
	rebuildModelMatrix();
}

void SkeletalObject::grow(const glm::vec3& growth) {
	m_scale = m_scale * growth;
	rebuildModelMatrix();
}

void SkeletalObject::addChild(SkeletalObject&& child)
{
	m_children.emplace_back(child);
}

void SkeletalObject::render(sf::RenderWindow& window, ShaderProgram& shaderProgram) const {
	renderRecursive(window, shaderProgram, glm::mat4(1));
}

/**
 * @brief Renders the object and its children, recursively.
 * @param parentMatrix the model matrix of this object's parent in the model hierarchy.
 */
void SkeletalObject::renderRecursive(sf::RenderWindow& window, ShaderProgram& shaderProgram, const glm::mat4& parentMatrix) const {
	// This object's true model matrix is the combination of its parent's matrix and the object's matrix.
	glm::mat4 trueModel = parentMatrix * m_modelMatrix;
	shaderProgram.setUniform("model", trueModel);
	// Render each mesh in the object.
	for (auto& mesh : m_meshes) {
		mesh.render(window, shaderProgram);
	}
	// Render the children of the object.
	for (auto& child : m_children) {
		child.renderRecursive(window, shaderProgram, trueModel);
	}
}


void SkeletalObject::tick(float_t dt) {
	glm::vec3 total_force(0, 0, 0);
	for (auto& force : forces_list) {
		total_force += force;
	}
	auto acceleration = total_force / mass;
	velocity += acceleration * dt;
	m_position += velocity * dt;

	rotational_velocity += rotational_acceleration * dt;
	m_orientation += rotational_velocity * dt;

	//std::cout << forces_list.size() << "\n";
	forces_list.clear();
	rebuildModelMatrix();
}

void SkeletalObject::addForce(const glm::vec3& force) {
	forces_list.push_back(force);
}

void SkeletalObject::addTexture(Texture texture)
{
	for (auto& mesh : m_meshes) {
		mesh.addTexture(texture);
	}
	// Render the children of the object.
	for (auto& child : m_children) {
		child.addTexture(texture);
	}
}