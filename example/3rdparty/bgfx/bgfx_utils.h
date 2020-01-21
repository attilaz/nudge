/*
 * Copyright 2011-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#ifndef BGFX_UTILS_H_HEADER_GUARD
#define BGFX_UTILS_H_HEADER_GUARD

namespace bgfx
{
	/// Returns true if both internal transient index and vertex buffer have
	 /// enough space.
	 ///
	 /// @param[in] _numVertices Number of vertices.
	 /// @param[in] _layout Vertex layout.
	 /// @param[in] _numIndices Number of indices.
	 ///
	static inline bool checkAvailTransientBuffers(uint32_t _numVertices, const bgfx::VertexLayout& _layout, uint32_t _numIndices)
	{
		return _numVertices == bgfx::getAvailTransientVertexBuffer(_numVertices, _layout)
			&& (0 == _numIndices || _numIndices == bgfx::getAvailTransientIndexBuffer(_numIndices))
			;
	}
}

#endif
