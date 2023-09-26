#include "Camera.h"

glm::mat4 Camera::getProjection()
{
	return glm::perspective(m_fov,m_aspectRatio,m_far,m_near);
}

glm::mat4 Camera::getView() {
	return glm::translate(glm::mat4(1.0),m_position) * glm::mat4_cast(m_rotation) * glm::scale(glm::mat4(1.f), m_scale);
}

