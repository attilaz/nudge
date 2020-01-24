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

#include "../../nudge.h"

namespace
{

static const unsigned max_body_count = 2048;
static const unsigned max_box_count = 2048;
static const unsigned max_sphere_count = 2048;

static const nudge::Transform identity_transform = { {}, 0, { 0.0f, 0.0f, 0.0f, 1.0f } };

static nudge::Arena arena;
static nudge::BodyData bodies;
static nudge::ColliderData colliders;
static nudge::ContactData contact_data;
static nudge::ContactCache contact_cache;
static nudge::ActiveBodies active_bodies;

static inline void quaternion_concat(float r[4], const float a[4], const float b[4]) {
	r[0] = b[0] * a[3] + a[0] * b[3] + a[1] * b[2] - a[2] * b[1];
	r[1] = b[1] * a[3] + a[1] * b[3] + a[2] * b[0] - a[0] * b[2];
	r[2] = b[2] * a[3] + a[2] * b[3] + a[0] * b[1] - a[1] * b[0];
	r[3] = a[3] * b[3] - a[0] * b[0] - a[1] * b[1] - a[2] * b[2];
}

static inline void quaternion_transform(float r[3], const float a[4], const float b[3]) {
	float t[3];
	t[0] = a[1] * b[2] - a[2] * b[1];
	t[1] = a[2] * b[0] - a[0] * b[2];
	t[2] = a[0] * b[1] - a[1] * b[0];

	t[0] += t[0];
	t[1] += t[1];
	t[2] += t[2];

	r[0] = b[0] + a[3] * t[0] + a[1] * t[2] - a[2] * t[1];
	r[1] = b[1] + a[3] * t[1] + a[2] * t[0] - a[0] * t[2];
	r[2] = b[2] + a[3] * t[2] + a[0] * t[1] - a[1] * t[0];
}

static inline void matrix(float r[16], const float s[3], const float q[4], const float t[3]) {
	float kx = q[0] + q[0];
	float ky = q[1] + q[1];
	float kz = q[2] + q[2];

	float xx = kx * q[0];
	float yy = ky * q[1];
	float zz = kz * q[2];
	float xy = kx * q[1];
	float xz = kx * q[2];
	float yz = ky * q[2];
	float sx = kx * q[3];
	float sy = ky * q[3];
	float sz = kz * q[3];

	r[0] = (1.0f - yy - zz) * s[0];
	r[1] = (xy + sz) * s[0];
	r[2] = (xz - sy) * s[0];
	r[3] = 0.0f;

	r[4] = (xy - sz) * s[1];
	r[5] = (1.0f - xx - zz) * s[1];
	r[6] = (yz + sx) * s[1];
	r[7] = 0.0f;

	r[8] = (xz + sy) * s[2];
	r[9] = (yz - sx) * s[2];
	r[10] = (1.0f - xx - yy) * s[2];
	r[11] = 0.0f;

	r[12] = t[0];
	r[13] = t[1];
	r[14] = t[2];
	r[15] = 1.0f;
}

static inline unsigned add_box(float mass, float cx, float cy, float cz) {
	if (bodies.count == max_body_count || colliders.boxes.count == max_box_count)
		return 0;

	unsigned body = bodies.count++;
	unsigned collider = colliders.boxes.count++;

	float k = mass * (1.0f / 3.0f);

	float kcx2 = k * cx*cx;
	float kcy2 = k * cy*cy;
	float kcz2 = k * cz*cz;

	nudge::BodyProperties properties = {};
	properties.mass_inverse = 1.0f / mass;
	properties.inertia_inverse[0] = 1.0f / (kcy2 + kcz2);
	properties.inertia_inverse[1] = 1.0f / (kcx2 + kcz2);
	properties.inertia_inverse[2] = 1.0f / (kcx2 + kcy2);

	memset(&bodies.momentum[body], 0, sizeof(bodies.momentum[body]));
	bodies.idle_counters[body] = 0;
	bodies.properties[body] = properties;
	bodies.transforms[body] = identity_transform;

	colliders.boxes.transforms[collider] = identity_transform;
	colliders.boxes.transforms[collider].body = body;

	colliders.boxes.data[collider].size[0] = cx;
	colliders.boxes.data[collider].size[1] = cy;
	colliders.boxes.data[collider].size[2] = cz;
	colliders.boxes.tags[collider] = collider;

	return body;
}

static inline unsigned add_sphere(float mass, float radius) {
	if (bodies.count == max_body_count || colliders.spheres.count == max_sphere_count)
		return 0;

	unsigned body = bodies.count++;
	unsigned collider = colliders.spheres.count++;

	float k = 2.5f / (mass*radius*radius);

	nudge::BodyProperties properties = {};
	properties.mass_inverse = 1.0f / mass;
	properties.inertia_inverse[0] = k;
	properties.inertia_inverse[1] = k;
	properties.inertia_inverse[2] = k;

	memset(&bodies.momentum[body], 0, sizeof(bodies.momentum[body]));
	bodies.idle_counters[body] = 0;
	bodies.properties[body] = properties;
	bodies.transforms[body] = identity_transform;

	colliders.spheres.transforms[collider] = identity_transform;
	colliders.spheres.transforms[collider].body = body;

	colliders.spheres.data[collider].radius = radius;
	colliders.spheres.tags[collider] = collider + max_box_count;

	return body;
}


class ExampleSimulation : public entry::AppI
{
public:
	ExampleSimulation(const char* _name, const char* _description, const char* _url)
        : entry::AppI(_name, _description, _url)
	{
	}

	void init(int32_t _argc, const char* const* _argv, uint32_t _width, uint32_t _height) override
	{
		Args args(_argc, _argv);

#if 0
#if BX_COMPILER_MSVC
		// Disable denormals for performance.
		_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
		_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
#endif
#endif
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

		// Allocate memory for simulation arena.
		arena.size = 64 * 1024 * 1024;
		arena.data = _mm_malloc(arena.size, 4096);

		// Allocate memory for bodies, colliders, and contacts.
		active_bodies.capacity = max_box_count;
		active_bodies.indices = static_cast<uint16_t*>(_mm_malloc(sizeof(uint16_t)*max_body_count, 64));

		bodies.idle_counters = static_cast<uint8_t*>(_mm_malloc(sizeof(uint8_t)*max_body_count, 64));
		bodies.transforms = static_cast<nudge::Transform*>(_mm_malloc(sizeof(nudge::Transform)*max_body_count, 64));
		bodies.momentum = static_cast<nudge::BodyMomentum*>(_mm_malloc(sizeof(nudge::BodyMomentum)*max_body_count, 64));
		bodies.properties = static_cast<nudge::BodyProperties*>(_mm_malloc(sizeof(nudge::BodyProperties)*max_body_count, 64));

		colliders.boxes.data = static_cast<nudge::BoxCollider*>(_mm_malloc(sizeof(nudge::BoxCollider)*max_box_count, 64));
		colliders.boxes.tags = static_cast<uint16_t*>(_mm_malloc(sizeof(uint16_t)*max_box_count, 64));
		colliders.boxes.transforms = static_cast<nudge::Transform*>(_mm_malloc(sizeof(nudge::Transform)*max_box_count, 64));

		colliders.spheres.data = static_cast<nudge::SphereCollider*>(_mm_malloc(sizeof(nudge::SphereCollider)*max_sphere_count, 64));
		colliders.spheres.tags = static_cast<uint16_t*>(_mm_malloc(sizeof(uint16_t)*max_sphere_count, 64));
		colliders.spheres.transforms = static_cast<nudge::Transform*>(_mm_malloc(sizeof(nudge::Transform)*max_sphere_count, 64));

		contact_data.capacity = max_body_count * 64;
		contact_data.bodies = static_cast<nudge::BodyPair*>(_mm_malloc(sizeof(nudge::BodyPair)*contact_data.capacity, 64));
		contact_data.data = static_cast<nudge::Contact*>(_mm_malloc(sizeof(nudge::Contact)*contact_data.capacity, 64));
		contact_data.tags = static_cast<uint64_t*>(_mm_malloc(sizeof(uint64_t)*contact_data.capacity, 64));
		contact_data.sleeping_pairs = static_cast<uint32_t*>(_mm_malloc(sizeof(uint32_t)*contact_data.capacity, 64));

		contact_cache.capacity = max_body_count * 64;
		contact_cache.data = static_cast<nudge::CachedContactImpulse*>(_mm_malloc(sizeof(nudge::CachedContactImpulse)*contact_cache.capacity, 64));
		contact_cache.tags = static_cast<uint64_t*>(_mm_malloc(sizeof(uint64_t)*contact_cache.capacity, 64));

		// The first body is the static world.
		bodies.count = 1;
		bodies.idle_counters[0] = 0;
		bodies.transforms[0] = identity_transform;
		memset(bodies.momentum, 0, sizeof(bodies.momentum[0]));
		memset(bodies.properties, 0, sizeof(bodies.properties[0]));

		// Add ground.
		{
			unsigned collider = colliders.boxes.count++;

			colliders.boxes.transforms[collider] = identity_transform;
			colliders.boxes.transforms[collider].position[1] -= 20.0f;

			colliders.boxes.data[collider].size[0] = 400.0f;
			colliders.boxes.data[collider].size[1] = 10.0f;
			colliders.boxes.data[collider].size[2] = 400.0f;
			colliders.boxes.tags[collider] = collider;
		}

		// Add boxes.
		for (unsigned i = 0; i < 512; ++i) {
			float sx = (float)rand() * (1.0f / (float)RAND_MAX) + 0.5f;
			float sy = (float)rand() * (1.0f / (float)RAND_MAX) + 0.5f;
			float sz = (float)rand() * (1.0f / (float)RAND_MAX) + 0.5f;

			unsigned body = add_box(8.0f*sx*sy*sz, sx, sy, sz);

			bodies.transforms[body].position[0] += (float)rand() * (1.0f / (float)RAND_MAX) * 10.0f - 5.0f;
			bodies.transforms[body].position[1] += (float)rand() * (1.0f / (float)RAND_MAX) * 150.0f;
			bodies.transforms[body].position[2] += (float)rand() * (1.0f / (float)RAND_MAX) * 10.0f - 5.0f;
		}

		// Add spheres.
		for (unsigned i = 0; i < 256; ++i) {
			float s = (float)rand() * (1.0f / (float)RAND_MAX) + 0.5f;

			unsigned body = add_sphere(4.18879f*s*s*s, s);

			bodies.transforms[body].position[0] += (float)rand() * (1.0f / (float)RAND_MAX) * 10.0f - 5.0f;
			bodies.transforms[body].position[1] += (float)rand() * (1.0f / (float)RAND_MAX) * 150.0f;
			bodies.transforms[body].position[2] += (float)rand() * (1.0f / (float)RAND_MAX) * 10.0f - 5.0f;
		}

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

			// simulate physics
			static const unsigned steps = 2;
			static const unsigned iterations = 20;

			float time_step = 1.0f / (60.0f * (float)steps);

			for (unsigned n = 0; n < steps; ++n) {
				// Setup a temporary memory arena. The same temporary memory is reused each iteration.
				nudge::Arena temporary = arena;

				// Find contacts.
				nudge::BodyConnections connections = {}; // NOTE: Custom constraints should be added as body connections.
				nudge::collide(&active_bodies, &contact_data, bodies, colliders, connections, temporary);

				// NOTE: Custom contacts can be added here, e.g., against the static environment.

				// Apply gravity and damping.
				float damping = 1.0f - time_step * 0.25f;

				for (unsigned i = 0; i < active_bodies.count; ++i) {
					unsigned index = active_bodies.indices[i];

					bodies.momentum[index].velocity[1] -= 9.82f * time_step;

					bodies.momentum[index].velocity[0] *= damping;
					bodies.momentum[index].velocity[1] *= damping;
					bodies.momentum[index].velocity[2] *= damping;

					bodies.momentum[index].angular_velocity[0] *= damping;
					bodies.momentum[index].angular_velocity[1] *= damping;
					bodies.momentum[index].angular_velocity[2] *= damping;
				}

				// Read previous impulses from contact cache.
				nudge::ContactImpulseData* contact_impulses = nudge::read_cached_impulses(contact_cache, contact_data, &temporary);

				// Setup contact constraints and apply the initial impulses.
				nudge::ContactConstraintData* contact_constraints = nudge::setup_contact_constraints(active_bodies, contact_data, bodies, contact_impulses, &temporary);

				// Apply contact impulses. Increasing the number of iterations will improve stability.
				for (unsigned i = 0; i < iterations; ++i) {
					nudge::apply_impulses(contact_constraints, bodies);
					// NOTE: Custom constraint impulses should be applied here.
				}

				// Update contact impulses.
				nudge::update_cached_impulses(contact_constraints, contact_impulses);

				// Write the updated contact impulses to the cache.
				nudge::write_cached_impulses(&contact_cache, contact_data, contact_impulses);

				// Move active bodies.
				nudge::advance(active_bodies, bodies, time_step);
			}

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

			static float time = 0.0f;
			time += deltaTime*timeScale;

			// Render boxes.
			for (unsigned i = 0; i < colliders.boxes.count; ++i) {
				unsigned body = colliders.boxes.transforms[i].body;

				float scale[3];
				float rotation[4];
				float position[3];

				memcpy(scale, colliders.boxes.data[i].size, sizeof(scale));

				quaternion_concat(rotation, bodies.transforms[body].rotation, colliders.boxes.transforms[i].rotation);
				quaternion_transform(position, bodies.transforms[body].rotation, colliders.boxes.transforms[i].position);

				position[0] += bodies.transforms[body].position[0];
				position[1] += bodies.transforms[body].position[1];
				position[2] += bodies.transforms[body].position[2];

				float m[16];
				matrix(m, scale, rotation, position);

				Aabb aabb;
				aabb.min = bx::Vec3(-1.0f, -1.0f, -1.0f);
				aabb.max = bx::Vec3( 1.0f,  1.0f,  1.0f);

				dde.pushTransform(m);
				dde.draw(aabb);
				dde.popTransform();
			}

			// Render spheres.
			for (unsigned i = 0; i < colliders.spheres.count; ++i) {
				unsigned body = colliders.spheres.transforms[i].body;

				float scale[3];
				float rotation[4];
				float position[3];

				scale[0] = scale[1] = scale[2] = colliders.spheres.data[i].radius;

				quaternion_concat(rotation, bodies.transforms[body].rotation, colliders.spheres.transforms[i].rotation);
				quaternion_transform(position, bodies.transforms[body].rotation, colliders.spheres.transforms[i].position);

				position[0] += bodies.transforms[body].position[0];
				position[1] += bodies.transforms[body].position[1];
				position[2] += bodies.transforms[body].position[2];

				float m[16];
				matrix(m, scale, rotation, position);

				Sphere sphere;
				sphere.center = bx::Vec3(position[0], position[1], position[2]);
				sphere.radius = 1.0f;

				dde.draw(sphere);
			}

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
	ExampleSimulation
	, "00-simulation"
	, "Simulation of dropping boxes and spheres."
	, "" //"https://bkaradzic.github.io/bgfx/examples.html#debugdraw"
	);


int32_t _main_(int32_t _argc, char** _argv)
{
	return 0;
}

