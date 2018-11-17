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

void Terrain::loadFromImage(const QImage& image)
{
	if (image.isNull())
	{
		return;
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
	const auto y = operator()(i, j);
	const auto z = m_height * i / (m_resolutionHeight - 1);
	
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

		const auto dw = (operator()(i, j + 1) - operator()(i, j - 1)) / (2 * stepWidth);
		const auto dh = (operator()(i + 1, j) - operator()(i - 1, j)) / (2 * stepHeight);

		return { -dw, 1.0f, -dh };
	}

	return { 0.0f, 0.0f, 0.0f };
}
