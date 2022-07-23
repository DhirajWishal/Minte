// Copyright (c) 2022 Dhiraj Wishal

#pragma once

#include <cstdint>

namespace minte
{
	/**
	 * Point 2D structure.
	 * This holds information about a single 2D point using the X axis and Y axis coordinates.
	 *
	 * @tparam Type The data type.
	 */
	template<class Type>
	struct Point2D final
	{
		/**
		 * Default constructor.
		 */
		constexpr Point2D() = default;

		/**
		 * Explicit constructor.
		 *
		 * @param value The value to be set to all coordinates.
		 */
		explicit Point2D(Type value) : m_X(value), m_Y(value) {}

		/**
		 * Explicit constructor.
		 *
		 * @param x The X coordinate value.
		 * @param y The Y coordinate value.
		 */
		explicit Point2D(Type x, Type y) : m_X(x), m_Y(y) {}

		Type m_X = 0;
		Type m_Y = 0;
	};


	/**
	 * Point 3D structure.
	 * This holds information about a single 3D point using the X axis, Y axis and Z axis coordinates.
	 *
	 * @tparam Type The data type.
	 */
	template<class Type>
	struct Point3D final
	{
		/**
		 * Default constructor.
		 */
		constexpr Point3D() = default;

		/**
		 * Explicit constructor.
		 *
		 * @param value The value to be set to all coordinates.
		 */
		explicit constexpr Point3D(Type value) : m_X(value), m_Y(value), m_Z(value) {}

		/**
		 * Explicit constructor.
		 *
		 * @param x The X coordinate value.
		 * @param y The Y coordinate value.
		 * @param z The Z coordinate value.
		 */
		explicit constexpr Point3D(Type x, Type y, Type z) : m_X(x), m_Y(y), m_Z(z) {}

		Type m_X = 0;
		Type m_Y = 0;
		Type m_Z = 0;
	};

	/**
	 * Rectangle class.
	 * This class holds information of either any rectangle using the maximum and minimum points.
	 *
	 * @param Point The point data type.
	 */
	template<class Point>
	struct Rectangle final
	{
		/**
		 * Default constructor.
		 */
		constexpr Rectangle() = default;

		/**
		 * Explicit constructor.
		 *
		 * @param minimum The minimum point.
		 * @param maximum The maximum point.
		 */
		explicit constexpr Rectangle(Point minimum, Point maximum) : m_MinPoint(minimum), m_MaxPoint(maximum) {}

		Point m_MinPoint;
		Point m_MaxPoint;
	};

	using Point2D_UI32 = Point2D<uint32_t>;
	using Point3D_UI32 = Point3D<uint32_t>;

	using Rectangle2D = Rectangle<Point2D_UI32>;
	using Rectangle3D = Rectangle<Point3D_UI32>;

	/**
	 * Vertex structure.
	 * This contains information about a single vertex used by the library.
	 */
	struct Vertex final
	{
		Point2D<float> m_Position;			// X32Y32
		Point2D<float> m_TextureCoordinate;	// U32V32
		uint32_t m_Color;					// R8G8B8A8
	};

	using Index = uint32_t;	// Index type used by the index buffer.
}