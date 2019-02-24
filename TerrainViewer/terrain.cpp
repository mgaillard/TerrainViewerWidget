#include "terrain.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <cassert>

using namespace TerrainViewer;

Terrain::Terrain(float width, float height, float maxAltitude) :
	m_width(width),
	m_height(height),
	m_maxAltitude(maxAltitude),
	m_resolutionWidth(0),
	m_resolutionHeight(0)
{
}

bool Terrain::loadFromImage(const QImage& image)
{
	if (image.isNull())
	{
		return false;
	}

	m_resolutionHeight = image.height();
	m_resolutionWidth = image.width();

	m_data.resize(m_resolutionHeight * m_resolutionWidth, 0.0f);

	for (int i = 0; i < m_resolutionHeight; i++)
	{
		for (int j = 0; j < m_resolutionWidth; j++)
		{
			operator()(i, j) = (static_cast<float>(qGray(image.pixel(i, j))) / 255) * m_maxAltitude;
		}
	}

	return true;
}

bool Terrain::loadFromImage(const cv::Mat& image)
{
	// Check if the image is valid
	if (image.data == nullptr)
	{
		return false;
	}
	
	// Check if the format is supported
	if (image.type() != CV_8U && image.type() != CV_16U)
	{
		return false;
	}

	// If the image file is composed of too few pixels
	if (image.rows < 2 || image.cols < 2)
	{
		return false;
	}

	m_resolutionHeight = image.rows;
	m_resolutionWidth = image.cols;

	m_data.resize(m_resolutionHeight * m_resolutionWidth, 0.0f);

	for (int i = 0; i < m_resolutionHeight; i++)
	{
		for (int j = 0; j < m_resolutionWidth; j++)
		{
			float normalizedValue = 0.0f;

			switch (image.type())
			{
			case CV_8U:
				normalizedValue = float(image.at<uint8_t>(i, j)) / std::numeric_limits<uint8_t>::max();
				break;
			case CV_16U:
				normalizedValue = float(image.at<uint16_t>(i, j)) / std::numeric_limits<uint16_t>::max();
				break;
			}

			operator()(i, j) = normalizedValue * m_maxAltitude;
		}
	}

	return true;
}

bool Terrain::empty() const
{
	return (m_width == 0 || m_height == 0);
}

float Terrain::width() const
{
	return m_width;
}

float Terrain::height() const
{
	return m_height;
}

float Terrain::maxAltitude() const
{
	return m_maxAltitude;
}

const float* Terrain::data() const
{
	return m_data.data();
}

int Terrain::resolutionWidth() const
{
	return m_resolutionWidth;
}

int Terrain::resolutionHeight() const
{
	return m_resolutionHeight;
}

const float& Terrain::operator()(int i, int j) const
{
	assert(i >= 0 && i < m_resolutionHeight);
	assert(j >= 0 && j < m_resolutionWidth);

	return m_data[i * m_resolutionWidth + j];
}

float& Terrain::operator()(int i, int j)
{
	assert(i >= 0 && i < m_resolutionHeight);
	assert(j >= 0 && j < m_resolutionWidth);

	return m_data[i * m_resolutionWidth + j];
}

QVector3D Terrain::vertex(int i, int j) const
{
	assert(m_resolutionHeight > 0);
	assert(m_resolutionWidth > 0);

	const auto x = m_width * j / (m_resolutionWidth - 1);
	const auto y = m_height * i / (m_resolutionHeight - 1);
	const auto z = operator()(i, j);
	
	return { x, y, z };
}

QVector3D Terrain::normal(int i, int j) const
{
	assert(m_resolutionHeight > 0);
	assert(m_resolutionWidth > 0);

	if (i > 0 && i < (m_resolutionHeight - 1) && j > 0 && j < (m_resolutionWidth - 1))
	{
		const auto stepWidth = m_width / (m_resolutionWidth - 1);
		const auto stepHeight = m_height / (m_resolutionHeight - 1);

		const auto xDiff = (operator()(i, j + 1) - operator()(i, j - 1)) / (2 * stepWidth);
		const auto yDiff = (operator()(i + 1, j) - operator()(i - 1, j)) / (2 * stepHeight);

		return { -xDiff, -yDiff, 1.0f };
	}

	return { 0.0f, 0.0f, 1.0f };
}

// TODO: Implement a more efficient algorithm
// Sean Barrett, 2011-12-25
// http://nothings.org/gamedev/horizon/
// Heman library on Github
// https://github.com/prideout/heman
// Timonen, V., &Westerholm, J. (2010, May).Scalable Height Field Self‐Shadowing.
// In Computer Graphics Forum(Vol. 29, No. 2, pp. 723 - 731).Oxford, UK: Blackwell Publishing Ltd.
// http://wili.cc/research/hfshadow/hfshadow.pdf
float horizonAngle(const TerrainViewer::Terrain& terrain, int i, int j, int di, int dj)
{
	const int height = terrain.resolutionHeight();
	const int width = terrain.resolutionWidth();

	// Horizon angle for this point
	float horizonTan = 0.0;

	// Compute the horizon in this point for a direction (dx, dy)
	for (int k = i + di, l = j + dj; k >= 0 && k < height && l>= 0 && l < width; k += di, l += dj)
	{
		// Altitude difference
		const float dh = terrain(k, l) - terrain(i, j);
		// Distance between the two points
		const float d = std::sqrt((k - i)*(k - i) + (l - j)*(l - j));
		// Horizon angle to this point
		const float angle = dh / d;

		horizonTan = std::max(angle, horizonTan);
	}

	return M_PI_2 - std::atan(horizonTan);
}

std::vector<float> ambientOcclusion(const Terrain& terrain)
{
	const int height = terrain.resolutionHeight();
	const int width = terrain.resolutionWidth();

	const std::array<std::pair<int, int>, 16> directions = {
	{
		{ 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 },   // 4-connected neighborhood
		{ 1, 1 }, { -1, 1 }, { -1, -1 }, { 1, -1 }, // Diagonals
		{ 2, 1 }, { 2, -1 }, { -2, 1 }, { -2, -1 }, // Knight in chess
		{ 1, 2 }, { 1, -2 }, { -1, 2 }, { -1, -2 }
	} };

	std::vector<float> occlusion(height * width, 0.0f);

#pragma omp parallel for
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			for (const auto& direction : directions)
			{
				// Angle between 0 and pi/2
				const float angle = horizonAngle(terrain, i, j, direction.first, direction.second);

				// Percentage of the surface of the hemisphere that is accessible by ambient light.
				occlusion[i * width + j] += angle / (directions.size() * M_PI_2);
			}
		}
	}

	// Remap between 0 and 1.
	// TODO: Let the user choose the mapping
	const auto itMinMax = std::minmax_element(occlusion.begin(), occlusion.end());
	const float minimum = *itMinMax.first;
	const float maximum = *itMinMax.second; // Should be about 1.0
	for (unsigned int i = 0; i < occlusion.size(); i++)
	{
		occlusion[i] = (occlusion[i] - minimum) / (maximum - minimum);
	}

	return occlusion;
}
