#include "Camera.h"

glm::mat4 Camera::getProjection()
{
	return glm::perspective(glm::radians(m_fov),m_aspectRatio,m_near, m_far);
}

glm::mat4 Camera::getView() {
	glm::mat4 tranlation = glm::translate(glm::mat4(1.0), m_position);
	glm::mat4 rotation = glm::rotate(glm::mat4(1.f), glm::radians(m_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	rotation = glm::rotate(rotation, glm::radians(m_rotation.y), glm::vec3(0.f, 1.0f, 0.0f));
	rotation = glm::rotate(rotation, glm::radians(m_rotation.z), glm::vec3(0.f, 0.0f, 1.0f));
	glm::mat4 scale = glm::scale(glm::mat4(1.f), m_scale);
	return  tranlation* rotation;
}

