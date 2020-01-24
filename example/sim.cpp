/*
 * Copyright 2011-2020 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include <bx/timer.h>
#include <common-bgfx/args.h>
#include <common-bgfx/entry/cmd.h>
#include <common-bgfx/entry/input.h>
#include <debugdraw/debugdraw.h>
#include <common-bgfx/camera.h>
#include <common-bgfx/imgui/imgui.h>

#include <bx/uint32_t.h>

namespace
{

class ExampleDebugDraw : public entry::AppI
{
public:
    ExampleDebugDraw(const char* _name, const char* _description, const char* _url)
        : entry::AppI(_name, _description, _url)
	{
	}

	void init(int32_t _argc, const char* const* _argv, uint32_t _width, uint32_t _height) override
	{
		Args args(_argc, _argv);

		m_width  = _width;
		m_height = _height;
		m_debug  = BGFX_DEBUG_NONE;
		m_reset  = BGFX_RESET_VSYNC | BGFX_RESET_MSAA_X16;

		bgfx::Init init;
		init.type     = args.m_type;
		init.vendorId = args.m_pciId;
		init.resolution.width  = m_width;
		init.resolution.height = m_height;
		init.resolution.reset  = m_reset;
		bgfx::init(init);

		// Enable m_debug text.
		bgfx::setDebug(m_debug);

		// Set view 0 clear state.
		bgfx::setViewClear(0
				, BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH
				, 0x303030ff
				, 1.0f
				, 0
				);

		m_timeOffset = bx::getHPCounter();

		cameraCreate();

		cameraSetPosition({ 0.0f, 2.0f, -12.0f });
		cameraSetVerticalAngle(0.0f);

		ddInit();

		imguiCreate();
	}

	virtual int shutdown() override
	{
		imguiDestroy();

		ddShutdown();

		cameraDestroy();

		// Shutdown bgfx.
		bgfx::shutdown();

		return 0;
	}


	bool update() override
	{
		if (!entry::processEvents(m_width, m_height, m_debug, m_reset, &m_mouseState) )
		{
			imguiBeginFrame(
				   m_mouseState.m_mx
				,  m_mouseState.m_my
				, (m_mouseState.m_buttons[entry::MouseButton::Left  ] ? IMGUI_MBUT_LEFT   : 0)
				| (m_mouseState.m_buttons[entry::MouseButton::Right ] ? IMGUI_MBUT_RIGHT  : 0)
				| (m_mouseState.m_buttons[entry::MouseButton::Middle] ? IMGUI_MBUT_MIDDLE : 0)
				,  m_mouseState.m_mz
				, uint16_t(m_width)
				, uint16_t(m_height)
				);

			showExampleDialog(this);

			ImGui::SetNextWindowPos(
				  ImVec2(m_width - m_width / 5.0f - 10.0f, 10.0f)
				, ImGuiCond_FirstUseEver
				);
			ImGui::SetNextWindowSize(
				  ImVec2(m_width / 5.0f, m_height / 3.0f)
				, ImGuiCond_FirstUseEver
				);
			ImGui::Begin("Settings"
				, NULL
				, 0
				);

			static float timeScale = 1.0f;
			ImGui::SliderFloat("T scale", &timeScale, -1.0f, 1.0f);

			ImGui::End();

			imguiEndFrame();

			int64_t now = bx::getHPCounter() - m_timeOffset;
			static int64_t last = now;
			const int64_t frameTime = now - last;
			last = now;
			const double freq = double(bx::getHPFrequency() );
			const float deltaTime = float(frameTime/freq);

			// Update camera.
			cameraUpdate(deltaTime, m_mouseState);

			float view[16];
			cameraGetViewMtx(view);

			float proj[16];

			// Set view and projection matrix for view 0.
			{
				bx::mtxProj(proj, 60.0f, float(m_width)/float(m_height), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);

				bgfx::setViewTransform(0, view, proj);
				bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height) );
			}

			float mtxVp[16];
			bx::mtxMul(mtxVp, view, proj);

			float mtxInvVp[16];
			bx::mtxInverse(mtxInvVp, mtxVp);

			const bx::Vec3 at  = { 0.0f,  0.0f, 0.0f };
			const bx::Vec3 eye = { 5.0f, 10.0f, 5.0f };
			bx::mtxLookAt(view, eye, at);
			bx::mtxProj(proj, 45.0f, float(m_width)/float(m_height), 1.0f, 15.0f, bgfx::getCaps()->homogeneousDepth);
			bx::mtxMul(mtxVp, view, proj);

			DebugDrawEncoder dde;

			dde.begin(0);
			dde.drawAxis(0.0f, 0.0f, 0.0f);

			dde.push();
				Aabb aabb =
				{
					{  5.0f, 1.0f, 1.0f },
					{ 10.0f, 5.0f, 5.0f },
				};
				dde.setColor(0xff00ff00);
				dde.draw(aabb);
			
			Sphere sphere = {
				{ 5.0f, 5.0f, 1.0f},
				1.0f,
			};
			dde.draw(sphere);
			dde.pop();

			static float time = 0.0f;
			time += deltaTime*timeScale;

			dde.setColor(0xffffffff);

			dde.push();

				float mtx[16];
				bx::mtxSRT(mtx
					, 1.0f, 1.0f, 1.0f
					, 0.0f, time, time*0.53f
					, -10.0f, 1.0f, 10.0f
					);

				Cylinder cylinder =
				{
					{ -10.0f, 1.0f, 10.0f },
					{   0.0f, 0.0f,  0.0f },
					1.0f
				};

				cylinder.end = bx::mul({ 0.0f, 4.0f, 0.0f }, mtx);
				dde.setColor(0xffffffff);
				dde.draw(cylinder);

				dde.push();
					toAabb(aabb, cylinder);
					dde.setWireframe(true);
					dde.setColor(0xff0000ff);
					dde.draw(aabb);
				dde.pop();

			dde.pop();

			dde.end();

			// Advance to next frame. Rendering thread will be kicked to
			// process submitted rendering primitives.
			bgfx::frame();

			return true;
		}

		return false;
	}

	entry::MouseState m_mouseState;

	int64_t m_timeOffset;

	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_debug;
	uint32_t m_reset;
};

} // namespace

ENTRY_IMPLEMENT_MAIN(
	  ExampleDebugDraw
	, "29-debugdraw"
	, "Debug draw."
	, "https://bkaradzic.github.io/bgfx/examples.html#debugdraw"
	);
