#ifndef TERRAINVIEWERWIDGET_H
#define TERRAINVIEWERWIDGET_H

#include <memory>

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
#include "terrainviewerparameters.h"
#include "occlusion.h"

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)

namespace TerrainViewer
{

class TerrainViewerWidget : public QOpenGLWidget, protected QOpenGLFunctions_4_3_Core
{
	Q_OBJECT

public:
	static const Parameters default_parameters;

	explicit TerrainViewerWidget(QWidget *parent = Q_NULLPTR);
	virtual ~TerrainViewerWidget();

	TerrainViewerWidget(const TerrainViewerWidget& widget) = delete;
	TerrainViewerWidget& operator=(TerrainViewerWidget other) = delete;
	TerrainViewerWidget(TerrainViewerWidget&&) = delete;
	TerrainViewerWidget& operator=(TerrainViewerWidget&&) = delete;

public slots:
	void cleanup();
	void printInfo();

	/**
	 * \brief Load and display a terrain in the widget. The terrain cannot be empty.
	 * \param terrain A non empty terrain.
	 */
	void loadTerrain(const Terrain& terrain);

	/**
	 * \brief Change the parameters of the widget.
	 * \param parameters The new parameters.
	 */
	void setParameters(const Parameters& parameters);

	/**
	 * \brief Return an image of the normal texture.
	 * \return A 8 bits RGB image of the normal texture.
	 */
	QImage normalTexture() const;

	/**
	 * \brief Return an image of the light map texture.
	 * \return A 8 bits grayscale image of the light map texture.
	 */
	QImage lightMapTexture() const;

	/**
	 * \brief Return an image of the terrain texture with lighting.
	 * \return A 8 bits color image of the terrain texture.
	 */
	QImage demTexture() const;

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

	/**
	 * \brief Generate patches for the tessellation shader.
	 * \param height Height of the terrain.
	 * \param width Width of the terrain.
	 * \param numberPatchesHeight Number of patches in the height axis.
	 * \param numberPatchesWidth Number of patches in the width axis.
	 * \return A vector of patches for the tessellation shader.
	 */
	static std::vector<Patch> generatePatches(float height, float width, int numberPatchesHeight, int numberPatchesWidth);

	/**
	 * \brief Compute the normals of the terrain in a compute shader.
	 * Height map and normals textures must be initialized.
	 * Read from the height map texture and directly update the normal texture.
	 */
	void computeNormalsOnShader();

	/**
	 * \brief Initialize the texture storing the height of the terrain.
	 */
	void initTerrainTexture();

	/**
	 * \brief Initialize the texture storing the normals.
	 * Compute the normals on the shader based on the height map texture.
	 * Height map texture must be initialized.
	 * \param computeOnShader True to compute the normals on the GPU using a compute shader.
	 *						  False to compute on the CPU.
	 */
	void initNormalTexture(bool computeOnShader = true);

	/**
	 * \brief Compute the coefficients of the texture storing the light map.
	 * \return The coefficients of the light map.
	 */
	std::vector<float> computeLightMapTexture() const;

	/**
	 * \brief Initialize the texture storing the light map.
	 */
	void initLightMapTexture();

	/**
	 * \brief Update the uniform variables in the shader that are given as parameters.
	 * The m_program must be bound when this function is called.
	 */
	void updateParameters();

	int m_numberPatchesHeight;
	int m_numberPatchesWidth;
	GLsizei m_numberPatches;

	Parameters m_parameters;

	QOpenGLDebugLogger* m_logger;
	std::unique_ptr<QOpenGLShaderProgram> m_program;
	std::unique_ptr<QOpenGLShaderProgram> m_computeNormalsProgram;

	Terrain m_terrain;

	std::vector<HorizonAngles> m_horizonAngles;

	QOpenGLVertexArrayObject m_vao;
	QOpenGLBuffer m_vbo;
	QOpenGLTexture m_heightTexture;
	QOpenGLTexture m_normalTexture;
	QOpenGLTexture m_lightMapTexture;

	OrbitCamera m_camera;
};

}

#endif // TERRAINVIEWERWIDGET_H