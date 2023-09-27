#pragma once

#include "GlmCommon.h"

class Camera {
public:
    glm::mat4 getProjection();
    glm::mat4 getView();
    void setAspectRatio(float value)
    {
        m_aspectRatio = value;
    }

    void setFieldOfView(float value)
    {
        m_fov = value;
    }

    void setFarPlane(float value)
    {
        m_far = value;
    }

    void setNearPlane(float value)
    {
        m_near = value;
    }

    float getAspectRatio()
    {
        return m_aspectRatio;
    }

    float getFieldOfView()
    {
        return m_fov;
    }

    float getFarPlane()
    {
        return m_far;
    }

    float getNearPlane()
    {
        return m_near;
    }

    void setPosition(glm::vec3 &pos) {
        m_position = pos;
    }

    void setRotation(glm::vec3& value) {
        m_rotation = value;
    }

    glm::vec3 getPosition() const {
        return m_position;
    }

    glm::vec4 getViewPos() {
        return glm::vec4(m_position, 0) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);
    }
private:
    glm::vec3 m_position;
    glm::vec3 m_rotation;
    glm::vec3 m_scale = glm::vec3(1.f);

    float m_aspectRatio = 1.0f;
    float m_fov = glm::radians(60.f);
    float m_far = 100;
    float m_near = 0.1;
};