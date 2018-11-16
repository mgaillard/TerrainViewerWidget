#include "camera.h"

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
	QMatrix4x4 rotationMatrix;
	rotationMatrix.rotate(angle, m_up);
	
	const QVector3D atToEye(m_eye - m_at);
	const QVector3D rotatedAtToEye = rotationMatrix * atToEye;

	m_eye = m_at + rotatedAtToEye;
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

void Camera::moveForth(float distance)
{
	QVector3D eyeToAt(m_at - m_eye);
	eyeToAt.normalize();
	m_eye += distance * eyeToAt;
}

TrackballCamera::TrackballCamera(const QVector3D& eye,
								 const QVector3D& at,
								 const QVector3D& up,
								 float fovy,
								 float aspectRatio,
								 float nearPlane,
								 float farPlane) :
	Camera(eye, at, up, fovy, aspectRatio, nearPlane, farPlane),
	m_motionSensitivity(0.5f),
	m_mouseHold(false),
	m_x0(0),
	m_y0(0)
{

}

void TrackballCamera::mousePressed(int x, int y)
{
	m_mouseHold = true;
	m_x0 = x;
	m_y0 = y;
}

void TrackballCamera::mouseMoved(int x, int y)
{
	if (m_mouseHold)
	{
		roundLeftRight((m_x0 - x) * m_motionSensitivity);
		roundUpDown((m_y0 - y) * m_motionSensitivity);

		m_x0 = x;
		m_y0 = y;
	}
}

void TrackballCamera::mouseReleased()
{
	m_mouseHold = false;
}

void TrackballCamera::zoom(float distance)
{
	moveForth(distance);
}


