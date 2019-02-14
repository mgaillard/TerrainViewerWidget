#include "terrainviewerwidget.h"

#include <vector>
#include <array>
#include <cassert>

#include <QMouseEvent>
#include <QOpenGLShaderProgram>

using namespace TerrainViewer;

TerrainViewerWidget::TerrainViewerWidget(QWidget *parent) :
	QOpenGLWidget(parent),
	m_numberPatchesHeight(0),
	m_numberPatchesWidth(0),
	m_numberPatches(0),
	m_wireFrame(false),
	m_pixelsPerTriangleEdge(16.0),
	m_logger(new QOpenGLDebugLogger(this)),
	m_program(nullptr),
	m_terrain(0.0f, 0.0f, 0.0f),
	m_heightTexture(QOpenGLTexture::Target2D),
	m_normalTexture(QOpenGLTexture::Target2D),
	m_camera({ 0.0, 0.0, 0.0 }, { 0.0, 0.0, -1.0 }, { 0.0, 1.0, 0.0 }, 45.0f, 1.0f, 0.01f, 100.0f)
{
}

TerrainViewerWidget::~TerrainViewerWidget()
{
	cleanup();
}

void TerrainViewerWidget::cleanup()
{
	if (m_program)
	{
		makeCurrent();
		m_vao.destroy();
		m_vbo.destroy();
		m_heightTexture.destroy();
		m_normalTexture.destroy();
		m_program.reset(nullptr);
		doneCurrent();
	}
}

void TerrainViewerWidget::printInfo()
{
	qDebug() << "Vendor: " << QString::fromLatin1(reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
	qDebug() << "Renderer: " << QString::fromLatin1(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
	qDebug() << "Version: " << QString::fromLatin1(reinterpret_cast<const char*>(glGetString(GL_VERSION)));
	qDebug() << "GLSL Version: " << QString::fromLatin1(reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));
}

void TerrainViewerWidget::loadTerrain(const Terrain& terrain)
{
	assert(!terrain.empty());

	m_terrain = terrain;

	// Generate patches to match the terrain
	m_numberPatchesHeight = m_terrain.resolutionHeight() / 32;
	m_numberPatchesWidth = m_terrain.resolutionWidth() / 32;
	m_numberPatches = m_numberPatchesWidth * m_numberPatchesHeight;
	auto patches = generatePatches(m_terrain.height(), m_terrain.width(), m_numberPatchesHeight, m_numberPatchesWidth);

	// Update the vbo
	m_vbo.bind();
	m_vbo.allocate(patches.data(), patches.size() * sizeof(Patch));
	m_vbo.release();
	
	// Init the texture storing the height of the terrain
	initTerrainTexture();

	update();
}

void TerrainViewerWidget::initializeGL()
{
	connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &TerrainViewerWidget::cleanup);

	initializeOpenGLFunctions();
	m_logger->initialize();
	glClearColor(0.5, 0.5, 0.5, 1.0);	

	const auto posLoc = 0;

	m_program = std::make_unique<QOpenGLShaderProgram>();
	m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/MainWindow/Shaders/vertex_shader.glsl");
	m_program->addShaderFromSourceFile(QOpenGLShader::TessellationControl, ":/MainWindow/Shaders/tessellation_control.glsl");
	m_program->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, ":/MainWindow/Shaders/tessellation_evaluation.glsl");
	m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/MainWindow/Shaders/fragment_shader.glsl");
	m_program->bindAttributeLocation("pos_attrib", posLoc);
	m_program->link();

	m_program->bind();

	// Create a vertex array object.
	m_vao.create();
	QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
	// Setup our vertex buffer object.
	m_vbo.create();
	m_vbo.bind();

	// Enable attributes
	m_program->enableAttributeArray(posLoc);
	// Tell OpenGL how to get the attribute values out of the vbo (stride and offset).
	m_program->setAttributeArray(posLoc, nullptr, 3, 0);

	m_vbo.release();
	m_program->release();

	// Init camera
	m_camera.setEye({ 0.0, 0.0, 10.0 });
	m_camera.setAt({ 0.0, 0.0, 0.0 });
	m_camera.setUp({ 0.0, 1.0, 0.0 });
	m_camera.setFovy(45.0f);
	m_camera.setAspectRatio(float(width()) / height());
	m_camera.setNearPlane(0.01f);
	m_camera.setFarPlane(100.0f);

	// Print OpenGL info and Debug messages
	printInfo();
}

void TerrainViewerWidget::resizeGL(int w, int h)
{
	m_camera.setAspectRatio(float(w) / h);
}

void TerrainViewerWidget::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	if (m_program && m_numberPatches > 0)
	{
		// Setup matrices
		QMatrix4x4 worldMatrix;
		worldMatrix.translate(-m_terrain.height() / 2, -m_terrain.width() / 2, 0.0);
		const auto normalMatrix = worldMatrix.normalMatrix();
		const auto viewMatrix = m_camera.viewMatrix();
		const auto projectionMatrix = m_camera.projectionMatrix();
		const auto pvMatrix = projectionMatrix * viewMatrix;
		const auto pvmMatrix = pvMatrix * worldMatrix;

		m_program->bind();

		// Update matrices
		m_program->setUniformValue("P", projectionMatrix);
		m_program->setUniformValue("V", viewMatrix);
		m_program->setUniformValue("M", worldMatrix);
		m_program->setUniformValue("N", normalMatrix);
		m_program->setUniformValue("PV", pvMatrix);
		m_program->setUniformValue("PVM", pvmMatrix);

		// Update viewportSize
		const QVector2D viewportSize(height(), width());
		m_program->setUniformValue("viewportSize", viewportSize);

		// Update terrain dimensions in the shader
		m_program->setUniformValue("terrain.height", m_terrain.height());
		m_program->setUniformValue("terrain.width", m_terrain.width());
		m_program->setUniformValue("terrain.resolution_height", m_terrain.resolutionHeight());
		m_program->setUniformValue("terrain.resolution_width", m_terrain.resolutionWidth());
		m_program->setUniformValue("terrain.max_altitude", m_terrain.maxAltitude());

		// Update pixelsPerTriangleEdge
		m_program->setUniformValue("pixelsPerTriangleEdge", m_pixelsPerTriangleEdge);

		// Bind the height texture
		const auto heightTextureUnit = 0;
		m_program->setUniformValue("terrain.height_texture", heightTextureUnit);
		m_heightTexture.bind(heightTextureUnit);

		// Bind the normal texture
		const auto normalTextureUnit = 1;
		m_program->setUniformValue("terrain.normal_texture", normalTextureUnit);
		m_normalTexture.bind(normalTextureUnit);

		// Bind the VAO containing the patches
		QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

		const auto verticesPerPatch = 4;
		m_program->setPatchVertexCount(verticesPerPatch);

		if (m_wireFrame)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}

		glDrawArrays(GL_PATCHES, 0, verticesPerPatch * m_numberPatches);
		
		if (m_wireFrame)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		m_normalTexture.release();
		m_heightTexture.release();
		m_program->release();
	}
}

void TerrainViewerWidget::mousePressEvent(QMouseEvent* event)
{
	const auto x = event->globalX();
	const auto y = event->globalY();

	m_camera.mousePressed(x, y);

	update();
}

void TerrainViewerWidget::mouseReleaseEvent(QMouseEvent* event)
{
	m_camera.mouseReleased();

	update();
}

void TerrainViewerWidget::mouseMoveEvent(QMouseEvent* event)
{
	const auto x = event->globalX();
	const auto y = event->globalY();

	m_camera.mouseMoved(x, y);

	update();
}

void TerrainViewerWidget::wheelEvent(QWheelEvent* event)
{
	const auto numDegrees = event->angleDelta() / 8;
	const auto numSteps = numDegrees / 15;
	m_camera.zoom(numSteps.y());

	update();
}

std::vector<TerrainViewerWidget::Patch> TerrainViewerWidget::generatePatches(float height, float width, int numberPatchesHeight, int numberPatchesWidth)
{
	const auto numberPatches = numberPatchesHeight * numberPatchesWidth;
	const auto patchSizeHeight = height / numberPatchesHeight;
	const auto patchSizeWidth = width / numberPatchesWidth;

	std::vector<Patch> patches;
	patches.reserve(numberPatches);
	for (auto i = 0; i < numberPatchesHeight; i++)
	{
		for (auto j = 0; j < numberPatchesWidth; j++)
		{
			const auto x = patchSizeHeight * i;
			const auto y = patchSizeWidth * j;

			patches.emplace_back(x, y, patchSizeHeight, patchSizeWidth);
		}
	}

	return patches;
}

void TerrainViewerWidget::initTerrainTexture()
{
	m_heightTexture.destroy();
	m_heightTexture.create();
	m_heightTexture.setFormat(QOpenGLTexture::R32F);
	m_heightTexture.setMinificationFilter(QOpenGLTexture::Linear);
	m_heightTexture.setMagnificationFilter(QOpenGLTexture::Linear);
	m_heightTexture.setWrapMode(QOpenGLTexture::ClampToEdge);
	m_heightTexture.setSize(m_terrain.resolutionWidth(), m_terrain.resolutionHeight());
	m_heightTexture.allocateStorage();
	m_heightTexture.setData(QOpenGLTexture::Red, QOpenGLTexture::Float32, m_terrain.data());

	// Compute normals
	std::vector<QVector3D> normals(m_terrain.resolutionWidth() * m_terrain.resolutionHeight());

#pragma omp parallel for
	for (int i = 0; i < m_terrain.resolutionHeight(); i++)
	{
		for (int j = 0; j < m_terrain.resolutionWidth(); j++)
		{
			const auto index = i * m_terrain.resolutionWidth() + j;

			normals[index] = m_terrain.normal(i, j);
		}
	}

	m_normalTexture.destroy();
	m_normalTexture.destroy();
	m_normalTexture.create();
	m_normalTexture.setFormat(QOpenGLTexture::RGB32F);
	m_normalTexture.setMinificationFilter(QOpenGLTexture::Linear);
	m_normalTexture.setMagnificationFilter(QOpenGLTexture::Linear);
	m_normalTexture.setWrapMode(QOpenGLTexture::ClampToEdge);
	m_normalTexture.setSize(m_terrain.resolutionWidth(), m_terrain.resolutionHeight());
	m_normalTexture.allocateStorage();
	m_normalTexture.setData(QOpenGLTexture::RGB, QOpenGLTexture::Float32, normals.data());
}
