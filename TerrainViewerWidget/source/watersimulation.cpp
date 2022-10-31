#include "watersimulation.h"

#include <QOpenGLVersionFunctionsFactory>
#include <QOpenGLFunctions_4_3_Core>

using namespace TerrainViewer;

WaterSimulation::WaterSimulation() :
	m_passesPerIterations(1),
	m_initialWaterLevel(0.0f),
	m_rainRate(0.0f),
	m_evaporationRate(1e-4),
	m_timeStep(0.001f),
	m_bounceBoundaries(false),
	m_terrain(0.0, 0.0, 0.0),
	m_computeFlowProgram(nullptr),
	m_computeWaterMapProgram(nullptr),
	m_heightTexture(QOpenGLTexture::Target2D),
	m_waterMapTexture(QOpenGLTexture::Target2D),
	m_outFlowTexture(QOpenGLTexture::Target2D)
{
	
}

void WaterSimulation::cleanup()
{
	if (m_computeFlowProgram && m_computeWaterMapProgram)
	{
		m_heightTexture.destroy();
		m_waterMapTexture.destroy();
		m_outFlowTexture.destroy();
		m_computeFlowProgram.reset(nullptr);
		m_computeWaterMapProgram.reset(nullptr);
	}
}

QOpenGLTexture& WaterSimulation::waterMapTexture()
{
	return m_waterMapTexture;
}

void WaterSimulation::initSimulation(QOpenGLContext* context, const Terrain& terrain)
{
	m_terrain = terrain;

	initComputeShader();
	initTextures();
}

void WaterSimulation::computeIteration(QOpenGLContext* context)
{
	if (m_running)
	{
		for (int i = 0; i < m_passesPerIterations; i++)
		{
			computeSingleIteration(context);
		}
	}
}

void WaterSimulation::computeSingleIteration(QOpenGLContext* context)
{
	auto f = QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_4_3_Core>(context);

	if (!f)
	{
		qFatal("Could not obtain required OpenGL context version");
	}

	// Local size in the compute shader
	const int localSizeX = 4;
	const int localSizeY = 4;

	if (m_computeFlowProgram)
	{
		m_computeFlowProgram->bind();

		// Update uniform values
		m_computeFlowProgram->setUniformValue("terrain_height", m_terrain.height());
		m_computeFlowProgram->setUniformValue("terrain_width", m_terrain.width());
		m_computeFlowProgram->setUniformValue("time_step", m_timeStep);
		m_computeFlowProgram->setUniformValue("water_increment", m_rainRate);
		m_computeFlowProgram->setUniformValue("evaporation_rate", m_evaporationRate);
		m_computeFlowProgram->setUniformValue("bounce_boundaries", m_bounceBoundaries);

		// Bind the height texture as an image
		const auto heightImageUnit = 0;
		f->glBindImageTexture(heightImageUnit, m_heightTexture.textureId(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);

		const auto waterImageUnit = 1;
		f->glBindImageTexture(waterImageUnit, m_waterMapTexture.textureId(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

		// Bind the normal texture as an image
		const auto outFlowImageUnit = 2;
		f->glBindImageTexture(outFlowImageUnit, m_outFlowTexture.textureId(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

		// Compute the number of blocks in each dimensions
		const int blocksX = std::max(1, 1 + ((m_terrain.resolutionWidth() - 1) / localSizeX));
		const int blocksY = std::max(1, 1 + ((m_terrain.resolutionHeight() - 1) / localSizeY));
		// Launch the compute shader and wait for it to finish
		f->glDispatchCompute(blocksX, blocksY, 1);
		f->glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		// Unbind the images
		f->glBindImageTexture(outFlowImageUnit, 0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		f->glBindImageTexture(waterImageUnit, 0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
		f->glBindImageTexture(heightImageUnit, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);

		m_computeFlowProgram->release();
	}

	if (m_computeWaterMapProgram)
	{
		m_computeWaterMapProgram->bind();

		// Update uniform values
		m_computeWaterMapProgram->setUniformValue("terrain_height", m_terrain.height());
		m_computeWaterMapProgram->setUniformValue("terrain_width", m_terrain.width());
		m_computeWaterMapProgram->setUniformValue("time_step", m_timeStep);
		m_computeWaterMapProgram->setUniformValue("water_increment", m_rainRate);
		m_computeWaterMapProgram->setUniformValue("evaporation_rate", m_evaporationRate);
		m_computeWaterMapProgram->setUniformValue("bounce_boundaries", m_bounceBoundaries);

		// Bind the height texture as an image
		const auto heightImageUnit = 0;
		f->glBindImageTexture(heightImageUnit, m_heightTexture.textureId(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);

		const auto waterImageUnit = 1;
		f->glBindImageTexture(waterImageUnit, m_waterMapTexture.textureId(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

		// Bind the normal texture as an image
		const auto outFlowImageUnit = 2;
		f->glBindImageTexture(outFlowImageUnit, m_outFlowTexture.textureId(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

		// Compute the number of blocks in each dimensions
		const int blocksX = std::max(1, 1 + ((m_terrain.resolutionWidth() - 1) / localSizeX));
		const int blocksY = std::max(1, 1 + ((m_terrain.resolutionHeight() - 1) / localSizeY));
		// Launch the compute shader and wait for it to finish
		f->glDispatchCompute(blocksX, blocksY, 1);
		f->glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		// Unbind the images
		f->glBindImageTexture(outFlowImageUnit, 0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		f->glBindImageTexture(waterImageUnit, 0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
		f->glBindImageTexture(heightImageUnit, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);

		m_computeWaterMapProgram->release();
	}
}

void WaterSimulation::setPassesPerIterations(int passesPerIterations)
{
	m_passesPerIterations = passesPerIterations;
}

void WaterSimulation::setInitialWaterLevel(float initialWaterLevel)
{
	m_initialWaterLevel = initialWaterLevel;
}

void WaterSimulation::setRainRate(float rainRate)
{
	m_rainRate = rainRate;
}

void WaterSimulation::setEvaporationRate(float evaporationRate)
{
	m_evaporationRate = evaporationRate;
}

void WaterSimulation::setTimeStep(float timeStep)
{
	m_timeStep = timeStep;
}

void WaterSimulation::setBounceOnBoundaries(bool bounceOnBoundaries)
{
	m_bounceBoundaries = bounceOnBoundaries;
}

void WaterSimulation::start()
{
	m_running = true;
}

void WaterSimulation::stop()
{
	m_running = false;
}

void WaterSimulation::initComputeShader()
{
	const QString shader_dir = ":/TerrainViewerWidget/shaders/";

	m_computeFlowProgram = std::make_unique<QOpenGLShaderProgram>();
	m_computeFlowProgram->addShaderFromSourceFile(QOpenGLShader::Compute, shader_dir + "compute_water_flow.glsl");
	m_computeFlowProgram->link();

	m_computeWaterMapProgram = std::make_unique<QOpenGLShaderProgram>();
	m_computeWaterMapProgram->addShaderFromSourceFile(QOpenGLShader::Compute, shader_dir + "compute_water_height.glsl");
	m_computeWaterMapProgram->link();
}

void WaterSimulation::initTextures()
{
	// TODO: reuse the texture from the terrain widget
	m_heightTexture.destroy();
	m_heightTexture.create();
	m_heightTexture.setFormat(QOpenGLTexture::R32F);
	m_heightTexture.setMinificationFilter(QOpenGLTexture::Linear);
	m_heightTexture.setMagnificationFilter(QOpenGLTexture::Linear);
	m_heightTexture.setWrapMode(QOpenGLTexture::ClampToEdge);
	m_heightTexture.setSize(m_terrain.resolutionWidth(), m_terrain.resolutionHeight());
	m_heightTexture.allocateStorage();
	m_heightTexture.setData(QOpenGLTexture::Red, QOpenGLTexture::Float32, m_terrain.data());

	const std::vector<float> initialWaterMap(m_terrain.resolutionWidth() * m_terrain.resolutionHeight(), m_initialWaterLevel);
	m_waterMapTexture.destroy();
	m_waterMapTexture.create();
	m_waterMapTexture.setFormat(QOpenGLTexture::R32F);
	m_waterMapTexture.setMinificationFilter(QOpenGLTexture::Linear);
	m_waterMapTexture.setMagnificationFilter(QOpenGLTexture::Linear);
	m_waterMapTexture.setWrapMode(QOpenGLTexture::ClampToEdge);
	m_waterMapTexture.setSize(m_terrain.resolutionWidth(), m_terrain.resolutionHeight());
	m_waterMapTexture.allocateStorage();
	m_waterMapTexture.setData(QOpenGLTexture::Red, QOpenGLTexture::Float32, initialWaterMap.data());

	const std::vector<float> outFlowMap(4 * m_terrain.resolutionWidth() * m_terrain.resolutionHeight(), 0.0);
	m_outFlowTexture.destroy();
	m_outFlowTexture.create();
	m_outFlowTexture.setFormat(QOpenGLTexture::RGBA32F);
	m_outFlowTexture.setMinificationFilter(QOpenGLTexture::Linear);
	m_outFlowTexture.setMagnificationFilter(QOpenGLTexture::Linear);
	m_outFlowTexture.setWrapMode(QOpenGLTexture::ClampToEdge);
	m_outFlowTexture.setSize(m_terrain.resolutionWidth(), m_terrain.resolutionHeight());
	m_outFlowTexture.allocateStorage();
	m_outFlowTexture.setData(QOpenGLTexture::RGBA, QOpenGLTexture::Float32, outFlowMap.data());
}
