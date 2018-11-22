#ifndef TERRAINVIEWERWIDGET_H
#define TERRAINVIEWERWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_4_3_Core>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLDebugLogger>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QMatrix4x4>

#include "camera.h"
#include "terrain.h"

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)

class TerrainViewerWidget : public QOpenGLWidget, protected QOpenGLFunctions_4_3_Core
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
	void loadTerrain(const Terrain& terrain);

protected:
	void initializeGL() override;
	void resizeGL(int w, int h) override;
	void paintGL() override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;

private:

	/**
	 * \brief A private type to temporarily store a vertex sent to OpenGL.
	 */
	struct Vertex
	{
		GLfloat x, y, z;
	};

	/**
	 * \brief A private type to temporarily store a patch sent to OpenGL.
	 */
	struct Patch
	{
		Vertex v[4];

		Patch(const GLfloat x, const GLfloat y, const GLfloat sizeX, const GLfloat sizeY)
		{
			v[0] = { x        , y        , 0.0 };
			v[1] = { x + sizeX, y        , 0.0 };
			v[2] = { x + sizeX, y + sizeY, 0.0 };
			v[3] = { x        , y + sizeY, 0.0 };
		}
	};

	int m_numberPatchesHeight;
	int m_numberPatchesWidth;
	GLsizei m_numberPatches;

	QOpenGLDebugLogger* m_logger;
	QOpenGLShaderProgram* m_program;

	Terrain m_terrain;

	QOpenGLVertexArrayObject m_vao;
	QOpenGLBuffer m_vbo;
	QOpenGLTexture m_terrainTexture;

	TrackballCamera m_camera;
};

#endif // TERRAINVIEWERWIDGET_H