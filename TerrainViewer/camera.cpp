#include "camera.h"

using namespace TerrainViewer;

Camera::Camera(const QVector3D& eye,
			   const QVector3D& at,
			   const QVector3D& up,
			   float fovy,
			   float aspectRatio,
			   float nearPlane,
			   float farPlane) :
	m_eye(eye),
	m_at(at),
	m_up(up),
	m_fovy(fovy),
	m_aspectRatio(aspectRatio),
	m_near(nearPlane),
	m_far(farPlane)
{

}

QMatrix4x4 Camera::viewMatrix() const
{
	QMatrix4x4 viewMatrix;
	viewMatrix.lookAt(m_eye, m_at, m_up);
	return viewMatrix;
}

QMatrix4x4 Camera::projectionMatrix() const
{
	QMatrix4x4 projectionMatrix;
	projectionMatrix.perspective(m_fovy, m_aspectRatio, m_near, m_far);
	return projectionMatrix;
}

void Camera::setEye(const QVector3D& eye)
{
	m_eye = eye;
}

void Camera::setAt(const QVector3D& at)
{
	m_at = at;
}

void Camera::setUp(const QVector3D& up)
{
	m_up = up;
}

void Camera::setFovy(float fovy)
{
	m_fovy = fovy;
}

void Camera::setAspectRatio(float aspectRatio)
{
	m_aspectRatio = aspectRatio;
}

void Camera::setNearPlane(float nearPlane)
{
	m_near = nearPlane;
}

void Camera::setFarPlane(float farPlane)
{
	m_far = farPlane;
}

void Camera::roundLeftRight(float angle)
{
	// Rotation around z
	QMatrix4x4 rotationMatrix;
	rotationMatrix.rotate(angle, 0.0, 0.0, 1.0);
	
	const QVector3D atToEye(m_eye - m_at);
	const QVector3D rotatedAtToEye = rotationMatrix * atToEye;
	m_eye = m_at + rotatedAtToEye;

	m_up = (rotationMatrix * m_up).normalized();
}

void Camera::roundUpDown(float angle)
{
	const QVector3D atToEye(m_eye - m_at);
	const QVector3D rotateAxis = QVector3D::crossProduct(m_up, atToEye);

	QMatrix4x4 rotationMatrix;
	rotationMatrix.rotate(angle, rotateAxis);

	const QVector3D rotatedAtToEye = rotationMatrix * atToEye;
	m_eye = m_at + rotatedAtToEye;
	m_up = QVector3D::crossProduct(m_eye, rotateAxis).normalized();
}

void Camera::moveLeftRight(float distance)
{
	const QVector3D eyeToAt(m_at - m_eye);
	const QVector3D right = QVector3D::crossProduct(eyeToAt, m_up).normalized();
	m_at += distance * right;
	m_eye += distance * right;
}

void Camera::moveUpDown(float distance)
{
	const QVector3D up = m_up.normalized();
	m_at += distance * up;
	m_eye += distance * up;
}

void Camera::moveForth(float distance)
{
	QVector3D eyeToAt(m_at - m_eye);
	eyeToAt.normalize();
	m_eye += distance * eyeToAt;
}

OrbitCamera::OrbitCamera(const QVector3D& eye,
						 const QVector3D& at,
						 const QVector3D& up,
						 float fovy,
						 float aspectRatio,
						 float nearPlane,
						 float farPlane) :
	Camera(eye, at, up, fovy, aspectRatio, nearPlane, farPlane),
	m_roundMotionSensitivity(0.5f),
	m_moveMotionSensitivity(0.02f),
	m_mouseLeftButtonHold(false),
	m_mouseRightButtonHold(false),
	m_x0(0),
	m_y0(0)
{

}

void OrbitCamera::mouseLeftButtonPressed(int x, int y)
{
	m_mouseLeftButtonHold = true;
	m_mouseRightButtonHold = false;
	m_x0 = x;
	m_y0 = y;
}

void OrbitCamera::mouseRightButtonPressed(int x, int y)
{
	m_mouseLeftButtonHold = false;
	m_mouseRightButtonHold = true;
	m_x0 = x;
	m_y0 = y;
}

void OrbitCamera::mouseMoved(int x, int y)
{
	if (m_mouseLeftButtonHold)
	{
		roundLeftRight((m_x0 - x) * m_roundMotionSensitivity);
		roundUpDown((m_y0 - y) * m_roundMotionSensitivity);
	}

	if (m_mouseRightButtonHold)
	{
		moveLeftRight((m_x0 - x) * m_moveMotionSensitivity);
		moveUpDown((y - m_y0) * m_moveMotionSensitivity);
	}

	m_x0 = x;
	m_y0 = y;
}

void OrbitCamera::mouseReleased()
{
	m_mouseLeftButtonHold = false;
	m_mouseRightButtonHold = false;
}

void OrbitCamera::zoom(float distance)
{
	moveForth(distance);
}


