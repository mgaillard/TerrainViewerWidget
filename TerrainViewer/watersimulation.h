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
	 * \brief Set the initial water level
	 * \param initialWaterLevel The new value of the initial water level
	 */
	void setInitialWaterLevel(float initialWaterLevel);

	/**
	 * \brief Start the simulation by setting the number of passes per iteration to one
	 */
	void start();
	
	/**
	 * \brief Stop the simulation by setting the number of passes per iterations to zero
	 */
	void stop();

private:
	void initComputeShader();
	void initTextures();

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