#ifndef CAMERA_H
#define CAMERA_H

#include <QVector3D>
#include <QMatrix4x4>

class Camera
{
public:

	/**
	 * \brief Create a camera
	 * \param eye The position of the eye
	 * \param at The position of the at, where the camera is looking
	 * \param up The up vector
	 * \param fovy Fovy of the camera
	 * \param aspectRatio Aspect ratio of the camera
	 * \param nearPlane Distance to the near plane
	 * \param farPlane Distance to the far plane
	 */
	Camera(const QVector3D& eye,
		   const QVector3D& at,
		   const QVector3D& up,
		   float fovy,
		   float aspectRatio,
		   float nearPlane,
		   float farPlane);

	/**
	 * \brief Return the view matrix
	 * \return The view matrix
	 */
	QMatrix4x4 viewMatrix() const;

	/**
	 * \brief Return the projection matrix
	 * \return The projection matrix
	 */
	QMatrix4x4 projectionMatrix() const;

	/**
	 * \brief Set the position of the eye
	 * \param eye The position of the eye
	 */
	void setEye(const QVector3D& eye);

	/**
	 * \brief Set the position of the at, where the camera is looking
	 * \param at The position of the at, where the camera is looking
	 */
	void setAt(const QVector3D& at);

	/**
	 * \brief Set the up vector
	 * \param up The up vector
	 */
	void setUp(const QVector3D& up);

	/**
	 * \brief Change the fovy of the camera
	 * \param fovy The fovy in radians
	 */
	void setFovy(float fovy);

	/**
	 * \brief Set the aspect ratio of the camera
	 * \param aspectRatio The aspect ratio of the camera
	 */
	void setAspectRatio(float aspectRatio);

	/**
	 * \brief Set the distance to the near plane
	 * \param nearPlane The distance to the near plane
	 */
	void setNearPlane(float nearPlane);

	/**
	 * \brief Set the distance to the far plane
	 * \param farPlane The distance to the far plane
	 */
	void setFarPlane(float farPlane);

	/**
	 * \brief Rotate the camera in the left right direction, around the up vector
	 * \param angle The angle of the rotation in radians
	 */
	void roundLeftRight(float angle);

	/**
	 * \brief Rotate the camera in the up down direction
	 * \param angle The angle of the rotation in radians
	 */
	void roundUpDown(float angle);

	/**
	 * \brief Move the camera in the direction of the at point
	 * \param distance The distance the camera will move
	 */
	void moveForth(float distance);

private:
	QVector3D m_eye;
	QVector3D m_at;
	QVector3D m_up;

	float m_fovy;
	float m_aspectRatio;
	float m_near;
	float m_far;
};

class TrackballCamera : public Camera
{
public:
	/**
	 * \brief Create a Trackball Camera
	 * \param eye The position of the eye
	 * \param at The position of the at, where the camera is looking
	 * \param up The up vector
	 * \param fovy Fovy of the camera
	 * \param aspectRatio Aspect ratio of the camera
	 * \param nearPlane Distance to the near plane
	 * \param farPlane Distance to the far plane
	 */
	TrackballCamera(const QVector3D& eye,
					const QVector3D& at,
					const QVector3D& up,
					float fovy,
					float aspectRatio,
					float nearPlane,
					float farPlane);

	/**
	 * \brief Call this function when the user press the mouse button
	 * \param x X coordinates of the mouse
	 * \param y Y coordinates of the mouse
	 */
	void mousePressed(int x, int y);

	/**
	 * \brief Call this function each time the user move the mouse
	 * \param x X coordinates of the mouse
	 * \param y Y coordinates of the mouse
	 */
	void mouseMoved(int x, int y);

	/**
	 * \brief Call this function when the user release the mouse button
	 */
	void mouseReleased();

	/**
	 * \brief Move the camera in the direction of the at point
	 * \param distance The distance the camera will move
	 */
	void zoom(float distance);

private:
	const float m_motionSensitivity;

	bool m_mouseHold;
	int m_x0;
	int m_y0;
};

#endif // CAMERA_H