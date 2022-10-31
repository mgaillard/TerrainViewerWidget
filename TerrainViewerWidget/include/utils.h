#ifndef UTILS_H
#define UTILS_H

template<typename T>
const T& clamp(const T& v, const T& lo, const T& hi)
{
	assert(lo <= hi);

	if (v < lo)
	{
		return lo;
	}
	else if (v > hi)
	{
		return hi;
	}
	else
	{
		return v;
	}
}

template<typename T>
T remap(const T& x, const T& in_start, const T& in_end, const T& out_start, const T& out_end)
{
	assert(in_start != in_end);

	return out_start + (out_end - out_start) * (x - in_start) / (in_end - in_start);
}

template<typename T>
T remap_clamp(const T& x, const T& in_start, const T& in_end, const T& out_start, const T& out_end)
{
	assert(in_start != in_end);

	if (x < in_start)
	{
		return out_start;
	}
	else if (x > in_end)
	{
		return out_end;
	}
	else
	{
		return out_start + (out_end - out_start) * (x - in_start) / (in_end - in_start);
	}
}

template<typename T>
T lerp(const T& a, const T& b, const T& x)
{
	// FMA friendly
	return x * b + (a - a * x);
}

template<typename T>
T lerp_clamp(const T& a, const T& b, const T& x)
{
	if (x < 0.0)
	{
		return a;
	}
	else if (x > 1.0)
	{
		return b;
	}
	else
	{
		return lerp(a, b, x);
	}
}

template <typename T>
int sgn(T val)
{
	return (T(0) < val) - (val < T(0));
}

/**
 * \brief Convert a 3D vector representing colors in RGB space to a QRgb type.
 * \param vector A 3D vector representing a RGB color in real space (between 0.0 and 1.0)
 * \return A QRgb object
 */
inline QRgb toQRgb(const QVector3D& vector)
{
	const auto r = static_cast<int>(vector.x() * 255.0);
	const auto g = static_cast<int>(vector.y() * 255.0);
	const auto b = static_cast<int>(vector.z() * 255.0);

	return qRgb(r, g, b);
}

/**
 * \brief Gradient of color for DEM displayed on a screen.
 * \tparam T float or double
 * \param t A real number between 0.0 and 1.0
 * \return A 3D vector representing a RGB color in real space (between 0.0 and 1.0)
 */
template <typename T>
QVector3D colorDemScreen(T t)
{
	const std::array<T, 6> offset = {
		0.0,
		0.125,
		0.25,
		0.5,
		0.75,
		1.0
	};

	// Gradient name: "DEM screen"
	// Source: http://soliton.vm.bytemark.co.uk/pub/cpt-city/views/totp-svg.html
	const std::array<QVector3D, 6> color = {
		QVector3D(0, 132, 53) / 255.0,
		QVector3D(51, 204, 0) / 255.0,
		QVector3D(244, 240, 113) / 255.0,
		QVector3D(244, 189, 69) / 255.0,
		QVector3D(153, 100, 43) / 255.0,
		QVector3D(255, 255, 255) / 255.0
	};

	for (int i = 0; i < 5; i++)
	{
		if (t >= offset[i] && t < offset[i + 1])
		{
			const T s = (t - offset[i]) / (offset[i + 1] - offset[i]);
			return (1.0 - s) * color[i] + s * color[i + 1];
		}
	}

	return color[5];
}

#endif // UTILS_H