#ifndef WATERSIMULATION_H
#define WATERSIMULATION_H

#include <vector>

#include <QOpenGLContext>
#include <QOpenGLTexture>
#include <QOpenGLShaderProgram>

#include "terrain.h"

namespace TerrainViewer
{

class WaterSimulation
{
public:
	WaterSimulation();

	void cleanup();

	/**
	 * \brief Return the current water map as a texture
	 * \return The current water map as a texture
	 */
	QOpenGLTexture& waterMapTexture();

	void initSimulation(QOpenGLContext* context, const Terrain& terrain);
	
	void computeIteration(QOpenGLContext* context);

	void computeSingleIteration(QOpenGLContext* context);

	/**
	 * \brief Set the number of water simulation passes per call to computeIteration()
	 * \param passesPerIterations The number of passes per iteration
	 */
	void setPassesPerIterations(int passesPerIterations);
	
	/**
	 * \brief Set the initial water level
	 * \param initialWaterLevel The new value of the initial water level
	 */
	void setInitialWaterLevel(float initialWaterLevel);

	/**
	 * \brief Set the rain rate
	 * \param rainRate The value of the rain rate
	 */
	void setRainRate(float rainRate);

	/**
	 * \brief Set the evaporation rate
	 * \param evaporationRate The new value of the evaporation rate
	 */
	void setEvaporationRate(float evaporationRate);

	/**
	 * \brief Set the time step
	 * \param timeStep The new value of the time step
	 */
	void setTimeStep(float timeStep);

	/**
	 * \brief Set whether the water bounces on boundaries or not
	 * \param bounceOnBoundaries True if water bounces on boundaries, false if water flows through boundaries
	 */
	void setBounceOnBoundaries(bool bounceOnBoundaries);

	/**
	 * \brief Start the simulation
	 */
	void start();
	
	/**
	 * \brief Stop the simulation
	 */
	void stop();

private:
	void initComputeShader();
	void initTextures();

	bool m_running;
	int m_passesPerIterations;
	float m_initialWaterLevel;
	float m_rainRate;
	float m_evaporationRate;
	float m_timeStep;
	bool m_bounceBoundaries;
	
	Terrain m_terrain;

	std::unique_ptr<QOpenGLShaderProgram> m_computeFlowProgram;
	std::unique_ptr<QOpenGLShaderProgram> m_computeWaterMapProgram;
	
	QOpenGLTexture m_heightTexture;
	QOpenGLTexture m_waterMapTexture;
	QOpenGLTexture m_outFlowTexture;
};

}

#endif // WATERSIMULATION_H