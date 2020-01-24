/*
 * Copyright 2011-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#ifndef BGFX_RENDERDOC_H_HEADER_GUARD
#define BGFX_RENDERDOC_H_HEADER_GUARD

namespace bgfx
{
	inline void* loadRenderDoc() {
		return NULL;
	}
	inline void unloadRenderDoc(void*) {}
	inline void renderDocTriggerCapture() {}

} // namespace bgfx

#endif // BGFX_RENDERDOC_H_HEADER_GUARD
