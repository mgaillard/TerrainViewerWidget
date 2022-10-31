﻿#ifndef TERRAIN_H
#define TERRAIN_H

#include <vector>

#include <QImage>
#include <QVector3D>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace TerrainViewer
{

class Terrain
{
public:

	Terrain(float width, float height, float maxAltitude);

	Terrain(float width, float height, float maxAltitude, int resolutionWidth, int resolutionHeight, std::vector<float> data);

	/**
	 * \brief Load a terrain from an image
	 * \param image A QImage containing the height map
	 */
	bool loadFromImage(const QImage& image);


	/**
	 * \brief Load a terrain from an image
	 * \param image A cv::Mat image containing the height map
	 */
	bool loadFromImage(const cv::Mat& image);

	/**
	 * \brief Save the height-map in a grayscale 8 bits file
	 * \param filename Name of the file
	 * \return True if successfully saved, false otherwise
	 */
	bool saveInGrayscale8(const std::string& filename);

	/**
	 * \brief Save the height-map in a grayscale 16 bits file
	 * \param filename Name of the file
	 * \return True if successfully saved, false otherwise
	 */
	bool saveInGrayscale16(const std::string& filename);

	/**
	 * \brief Return true if the terrain contains no data, false otherwise
	 * \return True if the terrain contains no data, false otherwise
	 */
	bool empty() const;

	/**
	 * \brief Return the width of the terrain 
	 * \return The width of the terrain 
	 */
	float width() const;

	/**
	 * \brief Return the height of the terrain
	 * \return The height of the terrain
	 */
	float height() const;

	/**
	 * \brief Return the width of a cell in the terrain
	 * \return The width of a cell in the terrain
	 */
	float cellWidth() const;

	/**
	 * \brief Return the height of a cell in the terrain
	 * \return The height of a cell in the terrain
	 */
	float cellHeight() const;

	/**
	 * \brief Return the terrain max altitude
	 * \return The terrain max altitude
	 */
	float maxAltitude() const;

	/**
	 * \brief Returns pointer to the underlying array serving as element storage.
	 * \return A pointer to the underlying array serving as element storage.
	 */
	const float* data() const;

	/**
	 * \brief Return the terrain resolution on the width axis
	 * \return The terrain resolution on the width axis
	 */
	int resolutionWidth() const;

	/**
	 * \brief Return the terrain resolution on the height axis
	 * \return The terrain resolution on the height axis
	 */
	int resolutionHeight() const;

	/**
	 * \brief Return the index in a 1D row major array of a cell
	 * \param i Vertex Y coordinate (height axis)
	 * \param j Vertex X coordinate (width axis)
	 * \return The index of the cell
	 */
	int cellIndex(int i, int j) const;

	/**
	 * \brief Get access to the altitude of a vertex
	 * \param i Vertex Y coordinate (height axis)
	 * \param j Vertex X coordinate (width axis)
	 * \return The altitude of the vertex
	 */
	const float& operator()(int i, int j) const;
	
	/**
	 * \brief Get access to the altitude of a vertex
	 * \param i Vertex Y coordinate (height axis)
	 * \param j Vertex X coordinate (width axis)
	 * \return The altitude of the vertex
	 */
	float& operator()(int i, int j);

	/**
	 * \brief Get access to the altitude of a vertex. Clamp to edge
	 * \param i Vertex Y coordinate (height axis)
	 * \param j Vertex X coordinate (width axis)
	 * \return The altitude of the vertex
	 */
	const float& atClamp(int i, int j) const;

	/**
	 * \brief Get access to the altitude of a vertex. Clamp to edge
	 * \param i Vertex Y coordinate (height axis)
	 * \param j Vertex X coordinate (width axis)
	 * \return The altitude of the vertex
	 */
	float& atClamp(int i, int j);

	/**
	 * \brief Return the 3D point of a vertex
	 * \param i Vertex Y coordinate (height axis)
	 * \param j Vertex X coordinate (width axis)
	 * \return The 3D point of a vertex
	 */
	QVector3D vertex(int i, int j) const;

	/**
	 * \brief Return the normal vector of a vertex
	 * \param i Vertex Y coordinate (height axis)
	 * \param j Vertex X coordinate (width axis)
	 * \return The normal vector of a vertex
	 */
	QVector3D normal(int i, int j) const;

private:
	float m_width;
	float m_height;
	float m_maxAltitude;

	int m_resolutionWidth;
	int m_resolutionHeight;

	std::vector<float> m_data;
};

}

#endif // TERRAIN_H