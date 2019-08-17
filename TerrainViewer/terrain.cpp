#include "terrain.h"

#include <cassert>

#include "utils.h"

using namespace TerrainViewer;

Terrain::Terrain(float width, float height, float maxAltitude) :
	m_width(width),
	m_height(height),
	m_maxAltitude(maxAltitude),
	m_resolutionWidth(0),
	m_resolutionHeight(0)
{
}

Terrain::Terrain(float width, float height, float maxAltitude, int resolutionWidth, int resolutionHeight, std::vector<float> data) :
	m_width(width),
	m_height(height),
	m_maxAltitude(maxAltitude),
	m_resolutionWidth(resolutionWidth),
	m_resolutionHeight(resolutionHeight),
	m_data(std::move(data))
{
	assert(m_data.size() == m_resolutionHeight * m_resolutionWidth);
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

bool Terrain::saveInGrayscale8(const std::string& filename)
{
	cv::Mat image(m_resolutionHeight, m_resolutionWidth, CV_8U);

	for (int i = 0; i < m_resolutionHeight; i++) {
		for (int j = 0; j < m_resolutionWidth; j++) {
			// Remap the value between 0 and 255
			const double value = remap(operator()(i, j), 0.f, m_maxAltitude,
									   static_cast<float>(std::numeric_limits<uint8_t>::lowest()),
									   static_cast<float>(std::numeric_limits<uint8_t>::max()));
			const auto color = static_cast<uint8_t>(value);
			image.at<unsigned char>(i, j) = color;
		}
	}

	return cv::imwrite(filename, image);
}

bool Terrain::saveInGrayscale16(const std::string& filename)
{
	cv::Mat image(m_resolutionHeight, m_resolutionWidth, CV_16U);

	for (int i = 0; i < m_resolutionHeight; i++) {
		for (int j = 0; j < m_resolutionWidth; j++) {
			// Remap the value between 0 and 65535
			const double value = remap(operator()(i, j), 0.f, m_maxAltitude,
									   static_cast<float>(std::numeric_limits<uint16_t>::lowest()),
									   static_cast<float>(std::numeric_limits<uint16_t>::max()));
			const auto color = static_cast<uint16_t>(value);
			image.at<unsigned short>(i, j) = color;
		}
	}

	return cv::imwrite(filename, image);
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

float Terrain::cellWidth() const
{
	return m_width / m_resolutionWidth;
}

float Terrain::cellHeight() const
{
	return m_height / m_resolutionHeight;
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

int Terrain::cellIndex(int i, int j) const
{
	assert(i >= 0 && i < m_resolutionHeight);
	assert(j >= 0 && j < m_resolutionWidth);

	return i * m_resolutionWidth + j;
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

const float& Terrain::atClamp(int i, int j) const
{
	const int k = clamp(i, 0, m_resolutionHeight - 1);
	const int l = clamp(j, 0, m_resolutionWidth - 1);

	return m_data[k * m_resolutionWidth + l];
}

float& Terrain::atClamp(int i, int j)
{
	const int k = clamp(i, 0, m_resolutionHeight - 1);
	const int l = clamp(j, 0, m_resolutionWidth - 1);

	return m_data[k * m_resolutionWidth + l];
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
