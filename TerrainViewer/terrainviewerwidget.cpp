#include "terrainviewerwidget.h"

#include <vector>
#include <cassert>

#include <QMouseEvent>
#include <QOpenGLShaderProgram>

#include "utils.h"
#include "tessellation_utils.h"
#include "terrainimages.h"
#include "occlusion.h"

using namespace TerrainViewer;

// Default parameters
const Parameters TerrainViewerWidget::default_parameters = {
	Palette::demScreen,
	Shading::uniformLight,
	false,    // wireframe
	1.f,      // pixelsPerTriangleEdge
};

TerrainViewerWidget::TerrainViewerWidget(QWidget *parent) :
	QOpenGLWidget(parent),
	m_numberPatchesHeight(0),
	m_numberPatchesWidth(0),
	m_numberPatches(0),
	m_parameters(default_parameters),
	m_logger(new QOpenGLDebugLogger(this)),
	m_program(nullptr),
	m_computeNormalsProgram(nullptr),
	m_terrain(0.0f, 0.0f, 0.0f),
	m_heightTexture(QOpenGLTexture::Target2D),
	m_normalTexture(QOpenGLTexture::Target2D),
	m_lightMapTexture(QOpenGLTexture::Target2D),
	m_camera({ 0.0, 0.0, 0.0 }, { 0.0, 0.0, -1.0 }, { 0.0, 1.0, 0.0 }, 45.0f, 1.0f, 0.01f, 100.0f)
{
	
}

TerrainViewerWidget::~TerrainViewerWidget()
{
	cleanup();
}

const Terrain& TerrainViewerWidget::terrain() const
{
	return m_terrain;
}

const Parameters& TerrainViewerWidget::parameters() const
{
	return m_parameters;
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
		m_lightMapTexture.destroy();
		m_program.reset(nullptr);
		m_computeNormalsProgram.reset(nullptr);
		doneCurrent();
	}
}

void TerrainViewerWidget::printInfo()
{
	qDebug() << "Vendor: " << QString::fromLatin1(reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
	qDebug() << "Renderer: " << QString::fromLatin1(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
	qDebug() << "Version: " << QString::fromLatin1(reinterpret_cast<const char*>(glGetString(GL_VERSION)));
	qDebug() << "GLSL Version: " << QString::fromLatin1(reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));

	// Display computer shader constants
	qDebug() << "Compute shader capabilities:";
	int workgroupCount[3];
	int workgroupSize[3];
	int workgroupInvocations;

	// The maximum number of work groups that may be dispatched to a compute shader
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &workgroupCount[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &workgroupCount[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &workgroupCount[2]);
	qDebug() << "Maximum number of work groups: "
	         << "\tx: " << workgroupCount[0]
	         << "\ty: " << workgroupCount[1]
	         << "\tz: " << workgroupCount[2];

	// The maximum size of a work groups that may be used during compilation of a compute shader
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &workgroupSize[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &workgroupSize[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &workgroupSize[2]);
	qDebug() << "Maximum size of work groups: "
	         << "\tx: " << workgroupSize[0]
	         << "\ty: " << workgroupSize[1]
	         << "\tz: " << workgroupSize[2];

	// The number of invocations in a single local work group (i.e., the product of the three dimensions)
	// that may be dispatched to a compute shader
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &workgroupInvocations);
	qDebug() << "Maximum number of invocations in a single local work group: " << workgroupInvocations;
}

void TerrainViewerWidget::loadTerrain(const Terrain& terrain)
{
	assert(!terrain.empty());

	m_terrain = terrain;

	// Generate patches to match the terrain. The minimum number of patch to generate is 1.
	m_numberPatchesHeight = std::max(1, m_terrain.resolutionHeight() / 32);
	m_numberPatchesWidth = std::max(1, m_terrain.resolutionWidth() / 32);
	m_numberPatches = m_numberPatchesWidth * m_numberPatchesHeight;
	auto patches = generateTessellationPatches(m_terrain.height(), m_terrain.width(),
											   m_numberPatchesHeight, m_numberPatchesWidth);

	// Update the vbo
	m_vbo.bind();
	m_vbo.allocate(patches.data(), patches.size() * sizeof(TessellationPatch));
	m_vbo.release();

	// Precompute the horizon angles
	m_horizonAngles = computeHorizonAngles(m_terrain);
	
	// Init the textures storing the information of the terrain
	initTerrainTexture();
	initNormalTexture();
	initLightMapTexture();

	update();
}

void TerrainViewerWidget::setParameters(const Parameters& parameters)
{
	const bool shadingChanged = (m_parameters.shading != parameters.shading);

	m_parameters = parameters;

	if (m_program)
	{
		m_program->bind();
		updateParameters();
		m_program->release();

		// Update the light map if the lighting model changed
		if (shadingChanged)
		{
			initLightMapTexture();
		}

		// Parameters changed, we update the view
		update();
	}
}

void TerrainViewerWidget::initializeGL()
{
	connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &TerrainViewerWidget::cleanup);

	initializeOpenGLFunctions();
	m_logger->initialize();
	glClearColor(0.5, 0.5, 0.5, 1.0);	

	const auto posLoc = 0;

	m_computeNormalsProgram = std::make_unique<QOpenGLShaderProgram>();
	m_computeNormalsProgram->addShaderFromSourceFile(QOpenGLShader::Compute, ":/MainWindow/Shaders/compute_normals.glsl");
	m_computeNormalsProgram->link();

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

	// Update the uniform variables in the shader that are given as parameters
	updateParameters();

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

		// Bind the height texture
		const auto heightTextureUnit = 0;
		m_program->setUniformValue("terrain.height_texture", heightTextureUnit);
		m_heightTexture.bind(heightTextureUnit);

		// Bind the normal texture
		const auto normalTextureUnit = 1;
		m_program->setUniformValue("terrain.normal_texture", normalTextureUnit);
		m_normalTexture.bind(normalTextureUnit);

		// Bind the light-map texture
		const auto lightMapTextureUnit = 2;
		m_program->setUniformValue("terrain.lightMap_texture", lightMapTextureUnit);
		m_lightMapTexture.bind(lightMapTextureUnit);

		// Bind the VAO containing the patches
		QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

		const auto verticesPerPatch = 4;
		m_program->setPatchVertexCount(verticesPerPatch);

		if (m_parameters.wireFrame)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}

		glDrawArrays(GL_PATCHES, 0, verticesPerPatch * m_numberPatches);
		
		if (m_parameters.wireFrame)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		m_lightMapTexture.release();
		m_normalTexture.release();
		m_heightTexture.release();
		m_program->release();
	}
}

void TerrainViewerWidget::mousePressEvent(QMouseEvent* event)
{
	const auto x = event->globalX();
	const auto y = event->globalY();

	if (event->button() == Qt::LeftButton)
	{
		m_camera.mouseLeftButtonPressed(x, y);
	}
	else if (event->button() == Qt::RightButton)
	{
		m_camera.mouseRightButtonPressed(x, y);
	}

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
	// Default speed
	float speed = 1.0;

	if (event->modifiers() & Qt::ShiftModifier)
	{
		// If shift is used, zooming is 4 times faster
		speed = 4.0;
	}
	else if (event->modifiers() & Qt::ControlModifier)
	{
		// If control is used, zooming is twice slower
		speed = 0.5;
	}

	const auto numDegrees = event->angleDelta() / 8;
	const auto numSteps = numDegrees / 15;
	m_camera.zoom(speed * numSteps.y());

	update();
}

void TerrainViewerWidget::computeNormalsOnShader()
{
	// Local size in the compute shader
	const int localSizeX = 4;
	const int localSizeY = 4;

	if (m_computeNormalsProgram)
	{
		m_computeNormalsProgram->bind();

		// Update uniform values
		m_computeNormalsProgram->setUniformValue("terrain_height", m_terrain.height());
		m_computeNormalsProgram->setUniformValue("terrain_width", m_terrain.width());

		// Bind the height texture as an image
		const auto heightImageUnit = 0;
		glBindImageTexture(heightImageUnit, m_heightTexture.textureId(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);

		// Bind the normal texture as an image
		const auto normalImageUnit = 1;
		glBindImageTexture(normalImageUnit, m_normalTexture.textureId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

		// Compute the number of blocks in each dimensions
		const int blocksX = std::max(1, 1 + ((m_terrain.resolutionWidth() - 1) / localSizeX));
		const int blocksY = std::max(1, 1 + ((m_terrain.resolutionHeight() - 1) / localSizeY));
		// Launch the compute shader and wait for it to finish
		glDispatchCompute(blocksX, blocksY, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		// Unbind the images
		glBindImageTexture(normalImageUnit, 0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		glBindImageTexture(heightImageUnit, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);

		m_computeNormalsProgram->release();
	}
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
}

void TerrainViewerWidget::initNormalTexture(bool computeOnShader)
{
	m_normalTexture.destroy();
	m_normalTexture.destroy();
	m_normalTexture.create();
	m_normalTexture.setFormat(QOpenGLTexture::RGBA32F);
	m_normalTexture.setMinificationFilter(QOpenGLTexture::Linear);
	m_normalTexture.setMagnificationFilter(QOpenGLTexture::Linear);
	m_normalTexture.setWrapMode(QOpenGLTexture::ClampToEdge);
	m_normalTexture.setSize(m_terrain.resolutionWidth(), m_terrain.resolutionHeight());
	m_normalTexture.allocateStorage();

	if (computeOnShader)
	{
		// Compute the normals on the GPU with the compute shader
		computeNormalsOnShader();
	}
	else
	{
		const std::vector<QVector4D> normals = computeNormals(m_terrain);

		m_normalTexture.setData(QOpenGLTexture::RGBA, QOpenGLTexture::Float32, normals.data());
	}	
}

void TerrainViewerWidget::initLightMapTexture()
{
	const std::vector<float> lightMap = computeLightMap(m_terrain, m_horizonAngles, m_parameters);

	m_lightMapTexture.destroy();
	m_lightMapTexture.destroy();
	m_lightMapTexture.create();
	m_lightMapTexture.setFormat(QOpenGLTexture::R32F);
	m_lightMapTexture.setMinificationFilter(QOpenGLTexture::Linear);
	m_lightMapTexture.setMagnificationFilter(QOpenGLTexture::Linear);
	m_lightMapTexture.setWrapMode(QOpenGLTexture::ClampToEdge);
	m_lightMapTexture.setSize(m_terrain.resolutionWidth(), m_terrain.resolutionHeight());
	m_lightMapTexture.allocateStorage();
	m_lightMapTexture.setData(QOpenGLTexture::Red, QOpenGLTexture::Float32, lightMap.data());
}

void TerrainViewerWidget::updateParameters()
{
	if (m_program)
	{
		// Update palette
		m_program->setUniformValue("palette", static_cast<int>(m_parameters.palette));

		// Update shading
		m_program->setUniformValue("shading", static_cast<int>(m_parameters.shading));

		// Update pixelsPerTriangleEdge
		m_program->setUniformValue("pixelsPerTriangleEdge", m_parameters.pixelsPerTriangleEdge);
	}
}
