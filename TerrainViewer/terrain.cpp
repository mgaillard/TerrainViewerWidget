#include "terrain.h"

#include <cassert>

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

	for (unsigned int i = 0; i < m_resolutionHeight; i++)
	{
		for (unsigned int j = 0; j < m_resolutionWidth; j++)
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

	m_resolutionHeight = static_cast<unsigned int >(image.rows);
	m_resolutionWidth = static_cast<unsigned int>(image.cols);

	m_data.resize(m_resolutionHeight * m_resolutionWidth, 0.0f);

	for (unsigned int i = 0; i < m_resolutionHeight; i++)
	{
		for (unsigned int j = 0; j < m_resolutionWidth; j++)
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

unsigned int Terrain::resolutionWidth() const
{
	return m_resolutionWidth;
}

unsigned int Terrain::resolutionHeight() const
{
	return m_resolutionHeight;
}

const float& Terrain::operator()(unsigned int i, unsigned int j) const
{
	assert(i < m_resolutionHeight);
	assert(j < m_resolutionWidth);

	return m_data[i * m_resolutionWidth + j];
}

float& Terrain::operator()(unsigned int i, unsigned int j)
{
	assert(i < m_resolutionHeight);
	assert(j < m_resolutionWidth);

	return m_data[i * m_resolutionWidth + j];
}

QVector3D Terrain::vertex(unsigned int i, unsigned int j) const
{
	assert(m_resolutionHeight > 0);
	assert(m_resolutionWidth > 0);

	const auto x = m_width * j / (m_resolutionWidth - 1);
	const auto y = m_height * i / (m_resolutionHeight - 1);
	const auto z = operator()(i, j);
	
	return { x, y, z };
}

QVector3D Terrain::normal(unsigned int i, unsigned int j) const
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

	return { 0.0f, 0.0f, 0.0f };
}
