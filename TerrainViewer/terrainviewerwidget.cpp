#include "terrainviewerwidget.h"

#include <vector>
#include <array>
#include <cassert>

#include <QMouseEvent>
#include <QOpenGLShaderProgram>

TerrainViewerWidget::TerrainViewerWidget(QWidget *parent) :
	QOpenGLWidget(parent),
	m_logger(new QOpenGLDebugLogger(this)),
	m_program(nullptr),
	m_terrainHeight(1.0),
	m_terrainWidth(1.0),
	m_terrainMaxAltitude(1.0),
	m_numberVertices(0),
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
		m_vbo.destroy();
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

void TerrainViewerWidget::loadTerrain(const QImage& image)
{
	const auto height = image.height();
	const auto width = image.width();

	m_terrainHeight = GLfloat(height) / 10;
	m_terrainWidth = GLfloat(width) / 10;
	m_terrainMaxAltitude = 255.0;

	// Compute the geometry of the terrain
	std::vector<GLfloat> vertices(18 * (height - 1) * (width - 1));

	for (int i = 0; i < height - 1; i++)
	{
		for (int j = 0; j < width - 1; j++)
		{
			const auto index = 18 * (i * (width - 1) + j);

			const QVector3D topLeft(
				m_terrainHeight * i / (height - 1),
				qGray(image.pixel(i, j)) / m_terrainMaxAltitude,
				m_terrainWidth * j / (width - 1)
			);

			const QVector3D topRight(
				m_terrainHeight * i / (height - 1),
				qGray(image.pixel(i, j + 1)) / m_terrainMaxAltitude,
				m_terrainWidth * (j + 1) / (width - 1)
			);

			const QVector3D bottomLeft(
				m_terrainHeight * (i + 1) / (height - 1),
				qGray(image.pixel(i + 1, j)) / m_terrainMaxAltitude,
				m_terrainWidth * j / (width - 1)
			);
			
			const QVector3D bottomRight(
				m_terrainHeight * (i + 1) / (height - 1),
				qGray(image.pixel(i + 1, j + 1)) / m_terrainMaxAltitude,
				m_terrainWidth * (j + 1) / (width - 1)
			);

			// Upper triangle
			vertices[index + 0] = topLeft.x();
			vertices[index + 1] = topLeft.y();
			vertices[index + 2] = topLeft.z();
			vertices[index + 3] = topRight.x();
			vertices[index + 4] = topRight.y();
			vertices[index + 5] = topRight.z();
			vertices[index + 6] = bottomRight.x();
			vertices[index + 7] = bottomRight.y();
			vertices[index + 8] = bottomRight.z();

			// Lower triangle
			vertices[index + 9] = topLeft.x();
			vertices[index + 10] = topLeft.y();
			vertices[index + 11] = topLeft.z();
			vertices[index + 12] = bottomRight.x();
			vertices[index + 13] = bottomRight.y();
			vertices[index + 14] = bottomRight.z();
			vertices[index + 15] = bottomLeft.x();
			vertices[index + 16] = bottomLeft.y();
			vertices[index + 17] = bottomLeft.z();
		}
	}

	// Update the vbo
	m_vbo.bind();
	m_vbo.allocate(vertices.data(), vertices.size() * sizeof(GLfloat));
	m_numberVertices = vertices.size() / 3;
	m_vbo.release();

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
	m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/MainWindow/Shaders/fragment_shader.glsl");
	m_program->bindAttributeLocation("pos_attrib", posLoc);
	// m_program->bindAttributeLocation("tex_coord_attrib", 1);
	// m_program->bindAttributeLocation("normal_attrib", 2);
	m_program->link();

	m_program->bind();

	// Create a vertex array object.
	m_vao.create();
	QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
	
	// Declare a vector to hold vertices.
	const std::vector<GLfloat> triangle = {
		0.0, 0.0, 0.0,
		0.0, 0.0, 1.0,
		1.0, 0.0, 0.0
	};

	// Setup our vertex buffer object.
	m_vbo.create();
	m_vbo.bind();
	// Upload from main memory to gpu memory.
	m_vbo.allocate(triangle.data(), triangle.size() * sizeof(GLfloat));
	m_numberVertices = triangle.size() / 3;

	// Enable the position attribute.
	glEnableVertexAttribArray(posLoc);

	// Tell OpenGL how to get the attribute values out of the vbo (stride and offset).
	glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	m_vbo.release();
	m_program->release();

	// Init camera
	m_camera.setEye({ 0.5, 10.0, 0.5 });
	m_camera.setAt({ 0.5, 0.0, 0.5 });
	m_camera.setUp({ 1.0, 0.0, 0.0 });
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

	// Setup PVM matrices
	QMatrix4x4 worldMatrix;
	worldMatrix.translate(-m_terrainHeight / 2, 0.0, -m_terrainWidth / 2);
	const auto viewMatrix = m_camera.viewMatrix();
	const auto projectionMatrix = m_camera.projectionMatrix();
	const auto pvmMatrix = projectionMatrix * viewMatrix * worldMatrix;

	QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
	m_program->bind();

	// Update PVM matrix
	const int pvmMatrixLoc = m_program->uniformLocation("PVM");
	m_program->setUniformValue(pvmMatrixLoc, pvmMatrix);
	
	// Retain the current Polygon Mode
	GLint previousPolygonMode[2];
	glGetIntegerv(GL_POLYGON_MODE, previousPolygonMode);
	// Change Polygon Mode and draw the triangles
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawArrays(GL_TRIANGLES, 0, m_numberVertices);
	// Restore the previous Polygon Mode
	glPolygonMode(GL_FRONT_AND_BACK, previousPolygonMode[0]);

	m_program->release();
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
