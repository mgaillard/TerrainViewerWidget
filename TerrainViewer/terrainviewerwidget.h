#ifndef TERRAINVIEWERWIDGET_H
#define TERRAINVIEWERWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_4_0_Core>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLDebugLogger>
#include <QOpenGLBuffer>
#include <QMatrix4x4>

#include "camera.h"

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)

class TerrainViewerWidget : public QOpenGLWidget, protected QOpenGLFunctions_4_0_Core
{
	Q_OBJECT

public:
	explicit TerrainViewerWidget(QWidget *parent = Q_NULLPTR);
	virtual ~TerrainViewerWidget();

	TerrainViewerWidget(const TerrainViewerWidget& widget) = delete;
	TerrainViewerWidget& operator=(TerrainViewerWidget other) = delete;
	TerrainViewerWidget(TerrainViewerWidget&&) = delete;
	TerrainViewerWidget& operator=(TerrainViewerWidget&&) = delete;

public slots:
	void cleanup();
	void printInfo();
	void loadTerrain(const QImage& image);

protected:
	void initializeGL() override;
	void resizeGL(int w, int h) override;
	void paintGL() override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;

private:
	QOpenGLDebugLogger* m_logger;
	QOpenGLShaderProgram* m_program;

	GLfloat m_terrainHeight;
	GLfloat m_terrainWidth;
	GLfloat m_terrainMaxAltitude;

	QOpenGLVertexArrayObject m_vao;
	QOpenGLBuffer m_vbo;
	GLsizei m_numberVertices;

	TrackballCamera m_camera;
};

#endif // TERRAINVIEWERWIDGET_H