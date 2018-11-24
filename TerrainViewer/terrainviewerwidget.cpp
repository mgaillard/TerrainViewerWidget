#include "terrainviewerwidget.h"

#include <vector>
#include <array>
#include <cassert>

#include <QMouseEvent>
#include <QOpenGLShaderProgram>

TerrainViewerWidget::TerrainViewerWidget(QWidget *parent) :
	QOpenGLWidget(parent),
	m_numberPatchesHeight(0),
	m_numberPatchesWidth(0),
	m_numberPatches(0),
	m_logger(new QOpenGLDebugLogger(this)),
	m_program(nullptr),
	m_terrain(0.0f, 0.0f, 0.0f),
	m_terrainTexture(QOpenGLTexture::Target2D),
	m_camera({ 0.0, 0.0, 0.0 }, { 0.0, 0.0, -1.0 }, { 0.0, 1.0, 0.0 }, 45.0f, 1.0f, 0.01f, 100.0f)
{
}

TerrainViewerWidget::~TerrainViewerWidget()
{
	cleanup();
}

void TerrainViewerWidget::cleanup()
{
	if (m_program != nullptr)
	{
		makeCurrent();
		m_vao.destroy();
		m_vbo.destroy();
		m_terrainTexture.destroy();
		delete m_program;
		m_program = nullptr;
		doneCurrent();
	}
}

void TerrainViewerWidget::printInfo()
{
	qDebug() << "Vendor: " << QString::fromLatin1(reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
	qDebug() << "Renderer: " << QString::fromLatin1(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
	qDebug() << "Version: " << QString::fromLatin1(reinterpret_cast<const char*>(glGetString(GL_VERSION)));
	qDebug() << "GLSL Version: " << QString::fromLatin1(reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));

	const auto messages = m_logger->loggedMessages();

	if (!messages.empty())
	{
		qDebug() << "OpenGL Debug messages:";
	}

	for (const auto& message : messages)
	{
		qDebug() << message;
	}
}

void TerrainViewerWidget::loadTerrain(const Terrain& terrain)
{
	m_terrain = terrain;

	const auto height = m_terrain.resolutionHeight();
	const auto width = m_terrain.resolutionWidth();

	assert(height > 0);
	assert(width > 0);

	// Generate patches to match the terrain
	m_numberPatchesHeight = height / 32;
	m_numberPatchesWidth = width / 32;
	m_numberPatches = m_numberPatchesWidth * m_numberPatchesHeight;

	const auto patchSizeWidth = m_terrain.width() / m_numberPatchesWidth;
	const auto patchSizeHeight = m_terrain.height() / m_numberPatchesHeight;

	std::vector<Patch> patches;
	patches.reserve(m_numberPatches);
	for (int i = 0; i < m_numberPatchesHeight; i++)
	{
		for (int j = 0; j < m_numberPatchesWidth; j++)
		{
			const auto x = patchSizeHeight * i;
			const auto y = patchSizeWidth * j;

			patches.emplace_back(x, y, patchSizeHeight, patchSizeWidth);
		}
	}

	// Update the vbo
	m_vbo.bind();
	m_vbo.allocate(patches.data(), patches.size() * sizeof(Patch));
	m_vbo.release();
	
	// Init the texture to store the height of the terrain
	m_terrainTexture.destroy();
	m_terrainTexture.create();
	m_terrainTexture.setFormat(QOpenGLTexture::R32F);
	m_terrainTexture.setMinificationFilter(QOpenGLTexture::Linear);
	m_terrainTexture.setMagnificationFilter(QOpenGLTexture::Linear);
	m_terrainTexture.setWrapMode(QOpenGLTexture::ClampToEdge);
	m_terrainTexture.setSize(width, height);
	m_terrainTexture.allocateStorage();
	m_terrainTexture.setData(QOpenGLTexture::Red, QOpenGLTexture::Float32, m_terrain.data());

	update();
}

void TerrainViewerWidget::initializeGL()
{
	connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &TerrainViewerWidget::cleanup);

	initializeOpenGLFunctions();
	m_logger->initialize();
	glClearColor(0.5, 0.5, 0.5, 1.0);	

	const auto posLoc = 0;

	m_program = new QOpenGLShaderProgram;
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
	glEnableVertexAttribArray(posLoc);
	// Tell OpenGL how to get the attribute values out of the vbo (stride and offset).
	glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const GLvoid*>(0));

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

	if (m_numberPatches > 0)
	{
		// Setup matrices
		QMatrix4x4 worldMatrix;
		worldMatrix.translate(-m_terrain.height() / 2, -m_terrain.width() / 2, 0.0);
		const auto normalMatrix = worldMatrix.normalMatrix();
		const auto viewMatrix = m_camera.viewMatrix();
		const auto projectionMatrix = m_camera.projectionMatrix();
		const auto pvMatrix = projectionMatrix * viewMatrix;
		const auto pvmMatrix = pvMatrix * worldMatrix;

		QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
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

		// Update terrain dimensions
		m_program->setUniformValue("terrain_height", m_terrain.height());
		m_program->setUniformValue("terrain_width", m_terrain.width());
		m_program->setUniformValue("terrain_resolution_height", m_terrain.resolutionHeight());
		m_program->setUniformValue("terrain_resolution_width", m_terrain.resolutionWidth());
		m_program->setUniformValue("terrain_max_altitude", m_terrain.maxAltitude());

		// Bind the terrain texture
		const auto textureUnit = 0;
		m_program->setUniformValue("terrain", textureUnit);
		m_terrainTexture.bind(textureUnit);

		const auto verticesPerPatch = 4;
		glPatchParameteri(GL_PATCH_VERTICES, verticesPerPatch);
		glDrawArrays(GL_PATCHES, 0, verticesPerPatch * m_numberPatches);

		m_program->release();

		m_terrainTexture.release();
	}
}

void TerrainViewerWidget::mousePressEvent(QMouseEvent* event)
{
	const int x = event->globalX();
	const int y = event->globalY();

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
