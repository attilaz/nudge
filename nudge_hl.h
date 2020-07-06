#ifndef NUDGE_NH_H_HEADER_GUARD
#define NUDGE_NH_H_HEADER_GUARD

#ifndef NUDGE_MAX_DYNAMIC_BODIES
#define NUDGE_MAX_DYNAMIC_BODIES	(1024)
#endif

#ifndef NUDGE_MAX_STATIC_BODIES
#define NUDGE_MAX_STATIC_BODIES		(1024)
#endif

#ifndef NUDGE_MAX_SPHERES
#define NUDGE_MAX_SPHERES			(1024)
#endif

#ifndef NUDGE_MAX_BOXES
#define NUDGE_MAX_BOXES				(1024)
#endif

#ifndef NUDGE_MAX_CONTACTS
#define NUDGE_MAX_CONTACTS			(1024)
#endif

#ifndef NUDGE_MAX_BODY_CONNECTIONS
#define NUDGE_MAX_BODY_CONNECTIONS	(1024)
#endif

#ifndef NUDGE_ARENA_SIZE
#define NUDGE_ARENA_SIZE			(4*1024*1024)
#endif

#define NUDGE_BODY_STATIC 	(1)
#define NUDGE_BODY_ACTIVE 	(2)


namespace nh
{
	typedef uint16_t BodyHandle;
	typedef uint32_t UserDataType; //can configure this

	struct WorldSettings
	{
		float gravity[3];
	};
	

	struct ShapeType
	{
		enum Enum
		{
			Box,
			Sphere
		};
	};
	
/*	TODO: Shape for compound interface
	struct Shape
	{
	 	rotation, offset transform;
	 	union
	 	{
	 		box params
	 		sphere params
	 	};
	};
	*/
	
	void init();
	void shutdown();
	
		// world
	void collide();
	void step(float timeStep, WorldSettings* _settings);
	
	BodyHandle createBox(float _xsize, float _ysize, float _zsize, float _mass, uint32_t _flags = 0);
	BodyHandle createSphere(float _radius, float _mass, uint32_t _flags = 0);
	//BodyHandle createCompound(Shape _shapes, uint16_t _shapeCount, float _mass, uint32_t flags);
	//BodyHandle createMesh(vec3* _positions, uint32_t* indices, float _mass, uint32_t flags);

	void destroy(BodyHandle _body);

	void setTransform(BodyHandle _body, float _px, float _py, float _pz, float _qx, float _qy, float _qz, float _qw);
	void setTransform(BodyHandle _body, const float _pos[3], const float _quat[4]);
	
	void getTransform(BodyHandle _body, float _pos[3], float _quat[4]);

	void setBodyData(BodyHandle _body, uint32_t _data);
	uint32_t setBodyData(BodyHandle _body);
	
//	const BodyTranform* getDynamicBodyTransforms();
	
		//contacts
	uint32_t getContactsCount();
	BodyHandle getContactBody(uint32_t _index, uint32_t _bodyIndex);
}

#ifdef NH_IMPLEMENTATION
#include "nudge.h"

#define NUDGE_MAX_BODIES (NUDGE_MAX_DYNAMIC_BODIES + NUDGE_MAX_STATIC_BODIES)
#define NUDGE_MAX_SHAPES (NUDGE_MAX_BOXES + NUDGE_MAX_SPHERES)

namespace nh
{
	template <int16_t HandleCount>
	struct HandleAlloc
	{
		HandleAlloc()
		{
			for(int16_t ii=0; ii<HandleCount; ++ii)
			{
				unusedHandles[ii] = HandleCount - ii - 1;
			}
			numUnusedHandles = HandleCount;
		}
		
		int16_t alloc()
		{
			int16_t handle = -1;
			if ( numUnusedHandles > 0)
			{
				handle = unusedHandles[--numUnusedHandles];
			}
			return handle;
		}
		
		void free(int16_t _handle)
		{
			unusedHandles[numUnusedHandles++] = _handle;
		}
		int16_t unusedHandles[HandleCount];
		int16_t numUnusedHandles;
	};

	struct StaticBody
	{
		uint8_t  shapeType;
		uint16_t shapeHandle;
	};
	
	struct StaticBodyData
	{
		StaticBody* bodies;
		uint32_t	count;
	};
	
	nudge::ActiveBodies 	activeBodies;
	nudge::ContactData 		contacts;
	nudge::BodyData 		bodies;
	StaticBodyData			staticBodies;
	UserDataType*			bodiesUserData;
	nudge::ColliderData 	shapes;
	nudge::BodyConnections  bodyConnections;
	nudge::ContactCache 	contactCache;
	nudge::Arena 			physicsMemoryArena;
	nudge::Arena 			physicsTempArena;

	HandleAlloc<NUDGE_MAX_DYNAMIC_BODIES> dynamicBodyHandleAlloc;
	HandleAlloc<NUDGE_MAX_STATIC_BODIES> staticBodyHandleAlloc;
	HandleAlloc<NUDGE_MAX_BOXES> boxHandleAlloc;
	HandleAlloc<NUDGE_MAX_SPHERES> sphereHandleAlloc;
	
	static const nudge::Transform identity_transform = { {}, 0, { 0.0f, 0.0f, 0.0f, 1.0f } };

		//aligned alloc/free from https://github.com/bkaradzic/bx/allocator.inl
	#define NUDGE_ALIGN_MASK(_value, _mask) ( ( (_value)+(_mask) ) & ( (~0)&(~(_mask) ) ) )
	
	inline void* alignPtr(void* _ptr, size_t _extra, size_t _align)
	{
		union { void* ptr; uintptr_t addr; } un;
		un.ptr = _ptr;
		uintptr_t unaligned = un.addr + _extra; // space for header
		uintptr_t mask = _align-1;
		uintptr_t aligned = NUDGE_ALIGN_MASK(unaligned, mask);
		un.addr = aligned;
		return un.ptr;
	}
	
	void* alignedAlloc(uint64_t _size, uint64_t _align = 64)
	{
		const size_t align = _align > sizeof(uint32_t) ? _align : sizeof(uint32_t);
		const size_t total = _size + align;
		uint8_t* ptr = (uint8_t*)malloc(total);
		uint8_t* aligned = (uint8_t*)alignPtr(ptr, sizeof(uint32_t), align);
		uint32_t* header = (uint32_t*)aligned - 1;
		*header = uint32_t(aligned - ptr);
		return aligned;
	}
	
	void alignedFree(void* _ptr)
	{
		uint8_t* aligned = (uint8_t*)_ptr;
		uint32_t* header = (uint32_t*)aligned - 1;
		uint8_t* ptr = aligned - *header;
		free(ptr);
	}
	
	void init()
	{
		activeBodies.indices = (uint16_t*)alignedAlloc( sizeof(uint16_t) * NUDGE_MAX_DYNAMIC_BODIES );
		activeBodies.capacity = NUDGE_MAX_DYNAMIC_BODIES;
		activeBodies.count = 0;
		
		contacts.data = (nudge::Contact*) alignedAlloc( sizeof(nudge::Contact) * NUDGE_MAX_CONTACTS );
		contacts.bodies = (nudge::BodyPair*) alignedAlloc( sizeof(nudge::BodyPair) * NUDGE_MAX_CONTACTS);
		contacts.tags = (uint64_t*) alignedAlloc( sizeof(uint64_t) * NUDGE_MAX_CONTACTS);
		contacts.capacity = NUDGE_MAX_CONTACTS;
		contacts.count = 0;
		contacts.sleeping_pairs = (uint32_t*) alignedAlloc( sizeof(uint32_t) * NUDGE_MAX_CONTACTS);
		contacts.sleeping_count = 0;
		
		contactCache.capacity = NUDGE_MAX_DYNAMIC_BODIES*64;
		contactCache.data = (nudge::CachedContactImpulse*)alignedAlloc(sizeof(nudge::CachedContactImpulse)*contactCache.capacity);
		contactCache.tags = (uint64_t*)alignedAlloc(sizeof(uint64_t)*contactCache.capacity);

		
		bodies.transforms = (nudge::Transform*) alignedAlloc( sizeof(nudge::Transform) * NUDGE_MAX_DYNAMIC_BODIES );
		bodies.properties = (nudge::BodyProperties*) alignedAlloc( sizeof(nudge::BodyProperties) * NUDGE_MAX_DYNAMIC_BODIES );
		bodies.momentum = (nudge::BodyMomentum*) alignedAlloc( sizeof(nudge::BodyMomentum) * NUDGE_MAX_DYNAMIC_BODIES );
		bodies.idle_counters = (uint8_t*) alignedAlloc( sizeof(uint8_t) * NUDGE_MAX_DYNAMIC_BODIES );
		bodies.count = 0;

		staticBodies.bodies = (StaticBody*)alignedAlloc(sizeof(StaticBody) * NUDGE_MAX_STATIC_BODIES);
		staticBodies.count = 0;
		
		// 0th index in dynamic body is reserved for static shapes ( no body ).
		dynamicBodyHandleAlloc.alloc();
		bodies.count = 1;
		bodies.idle_counters[0] = 0;
		bodies.transforms[0] = identity_transform;
		memset(bodies.momentum, 0, sizeof(bodies.momentum[0]));
		memset(bodies.properties, 0, sizeof(bodies.properties[0]));
		
		bodiesUserData = (UserDataType*) alignedAlloc( sizeof(UserDataType) * NUDGE_MAX_BODIES);

		shapes.boxes.tags = (uint16_t*) alignedAlloc( sizeof(uint16_t) * NUDGE_MAX_BOXES );
		shapes.boxes.data = (nudge::BoxCollider*) alignedAlloc( sizeof(nudge::BoxCollider) * NUDGE_MAX_BOXES );
		shapes.boxes.transforms = (nudge::Transform*) alignedAlloc( sizeof(nudge::Transform) * NUDGE_MAX_BOXES );
		shapes.boxes.count = 0;

		shapes.spheres.tags = (uint16_t*) alignedAlloc( sizeof(uint16_t) * NUDGE_MAX_SPHERES );
		shapes.spheres.data = (nudge::SphereCollider*) alignedAlloc( sizeof(nudge::SphereCollider) * NUDGE_MAX_SPHERES );
		shapes.spheres.transforms = (nudge::Transform*) alignedAlloc( sizeof(nudge::Transform) * NUDGE_MAX_SPHERES );
		shapes.spheres.count = 0;

		bodyConnections.data = (nudge::BodyPair*) alignedAlloc( sizeof(nudge::BodyPair) * NUDGE_MAX_BODY_CONNECTIONS );
		bodyConnections.count = 0;
		
		physicsMemoryArena.data = malloc(NUDGE_ARENA_SIZE);
		physicsMemoryArena.size = NUDGE_ARENA_SIZE;
	}
	
	void shutdown()
	{
		//todo free memory
	}

	void collide()
	{
		nh::physicsTempArena = nh::physicsMemoryArena;
		
		nudge::collide(&nh::activeBodies, &nh::contacts, nh::bodies, nh::shapes, nh::bodyConnections, nh::physicsTempArena);
	}
	
	void step(float _timeStep, WorldSettings* _settings)
	{
		// Apply gravity and damping.
		float damping = 1.0f - _timeStep*0.25f;
		
		float gravityVel[3] = { _settings->gravity[0] * _timeStep,
								_settings->gravity[1] * _timeStep,
								_settings->gravity[2] * _timeStep};
		
		for (unsigned i = 0; i < nh::activeBodies.count; ++i) {
			unsigned index = nh::activeBodies.indices[i];
			
			bodies.momentum[index].velocity[0] += gravityVel[0];
			bodies.momentum[index].velocity[1] += gravityVel[1];
			bodies.momentum[index].velocity[2] += gravityVel[2];
			
			bodies.momentum[index].velocity[0] *= damping;
			bodies.momentum[index].velocity[1] *= damping;
			bodies.momentum[index].velocity[2] *= damping;
			
			bodies.momentum[index].angular_velocity[0] *= damping;
			bodies.momentum[index].angular_velocity[1] *= damping;
			bodies.momentum[index].angular_velocity[2] *= damping;
		}
		
		// Read previous impulses from contact cache.
		nudge::ContactImpulseData* contactImpulses = nudge::read_cached_impulses(contactCache, contacts, &physicsTempArena);
		
		// Setup contact constraints and apply the initial impulses.
		nudge::ContactConstraintData* contactConstraints = nudge::setup_contact_constraints(activeBodies, contacts, bodies, contactImpulses, &physicsTempArena);
		
		// Apply contact impulses. Increasing the number of iterations will improve stability.
		const int iterations = 20;
		for (unsigned i = 0; i < iterations; ++i) {
			nudge::apply_impulses(contactConstraints, bodies);
			// NOTE: Custom constraint impulses should be applied here.
		}
		
		// Update contact impulses.
		nudge::update_cached_impulses(contactConstraints, contactImpulses);
		
		// Write the updated contact impulses to the cache.
		nudge::write_cached_impulses(&contactCache, contacts, contactImpulses);
		
		// Move active bodies.
		nudge::advance(activeBodies, bodies, _timeStep);
	}


	void set_float3(float* _v, float _x, float _y, float _z)
	{
		_v[0] = _x;
		_v[1] = _y;
		_v[2] = _z;
	}
	
	void set_float4(float* _v, float _x, float _y, float _z, float _w)
	{
		_v[0] = _x;
		_v[1] = _y;
		_v[2] = _z;
		_v[3] = _w;
	}
	
	BodyHandle createBody(uint32_t _flags)
	{
		int16_t idx = 0;
		if ( 0 == (_flags & NUDGE_BODY_STATIC))
		{
			idx = dynamicBodyHandleAlloc.alloc();
			if ( idx < 0 ) return idx;
			
			bodies.transforms[idx] = identity_transform;
			set_float3(bodies.momentum[idx].velocity, 0.0f, 0.0f, 0.0f);
			set_float3(bodies.momentum[idx].angular_velocity, 0.0f, 0.0f, 0.0f);
			bodies.idle_counters[idx] = (0 == (_flags & NUDGE_BODY_ACTIVE)) ? 0 : 0xff;
			
			bodies.count++;
		}
		else
		{
			idx = staticBodyHandleAlloc.alloc();
			if ( idx < 0 ) return idx;
			
			idx += NUDGE_MAX_DYNAMIC_BODIES;
		}
		
		bodiesUserData[idx] = 0;
		
		return idx;
	}
	
	BodyHandle createBox(float _xsize, float _ysize, float _zsize, float _mass, uint32_t _flags)
	{
		if ( boxHandleAlloc.numUnusedHandles == 0 )
			return 0;
		
		BodyHandle body = createBody(_flags);
		if ( body < 0 )
			return 0;

		int16_t shape = boxHandleAlloc.alloc();
		set_float3(shapes.boxes.data[shape].size, _xsize*0.5f, _ysize*0.5f, _zsize*0.5f);
		
		shapes.boxes.transforms[shape] = identity_transform;
		shapes.boxes.transforms[shape].body = ((0 == _flags) & NUDGE_BODY_STATIC) ? body : 0;
		shapes.boxes.count++;

		if ( body < NUDGE_MAX_DYNAMIC_BODIES)
		{
			float k = _mass * (1.0f/12.0f);
			
			float kcx2 = k*_xsize*_xsize;
			float kcy2 = k*_ysize*_ysize;
			float kcz2 = k*_zsize*_zsize;

			nudge::BodyProperties* properties = &bodies.properties[body];
			properties->mass_inverse = 1.0f / _mass;
			properties->inertia_inverse[0] = 1.0f / (kcy2+kcz2);
			properties->inertia_inverse[1] = 1.0f / (kcx2+kcz2);
			properties->inertia_inverse[2] = 1.0f / (kcx2+kcy2);
		}
		else
		{
			StaticBody* staticBody = &staticBodies.bodies[body-NUDGE_MAX_DYNAMIC_BODIES];
			staticBody->shapeHandle = shape;
			staticBody->shapeType = ShapeType::Box;
		}

		return body;
	}
	
	BodyHandle createSphere(float _radius, float _mass, uint32_t _flags)
	{
		if ( sphereHandleAlloc.numUnusedHandles == 0 )
			return 0;
		
		BodyHandle body = createBody(_flags);
		if ( body < 0 )
			return 0;
		
		int16_t shape = sphereHandleAlloc.alloc();
		shapes.spheres.data[shape].radius = _radius;
		
		shapes.spheres.transforms[shape] = identity_transform;
		shapes.spheres.transforms[shape].body = ((0 == _flags) & NUDGE_BODY_STATIC) ? body : 0;
		shapes.spheres.count++;
		
		if ( body < NUDGE_MAX_DYNAMIC_BODIES)
		{
			float k = 2.5f / (_mass*_radius*_radius);
			
			nudge::BodyProperties* properties = &bodies.properties[body];
			properties->mass_inverse = 1.0f / _mass;
			properties->inertia_inverse[0] = k;
			properties->inertia_inverse[1] = k;
			properties->inertia_inverse[2] = k;
		}
		else
		{
			StaticBody* staticBody = &staticBodies.bodies[body-NUDGE_MAX_DYNAMIC_BODIES];
			staticBody->shapeHandle = shape;
			staticBody->shapeType = ShapeType::Sphere;
		}
		
		return body;
	}

	void destroy(BodyHandle _body)
	{
		if (_body == 0 ) return;
//		unusedBodyIndices[numUnusedBodyIndices++] = _body;

#if 0
		if (_shape == 0 ) return;
		
		int idx = shapeHandles[_shape];
		shapeHandles[_shape] = -1;
		unusedBoxHandles[numUnusedBoxHandles++] = _shape;
		
		int swapHandle = shapes.boxes.tags[shapes.boxes.count-1];
		shapeHandles[swapHandle] = idx;
		shapes.boxes.transforms[idx] = shapes.boxes.transforms[shapes.boxes.count-1];
		shapes.boxes.tags[idx] = shapes.boxes.tags[shapes.boxes.count-1];
		shapes.boxes.data[idx] = shapes.boxes.data[shapes.boxes.count-1];
		shapes.boxes.count--;
#endif
		//todo: destroy shapes
	}
	
	void setTransform(BodyHandle _body, float _px, float _py, float _pz, float _qx, float _qy, float _qz, float _qw)
	{
		nudge::Transform* transform = NULL;
		if ( _body < NUDGE_MAX_DYNAMIC_BODIES )
		{
			transform = &nh::bodies.transforms[_body];
		}
		else
		{	//todo: shape types needed
			uint16_t shape = nh::staticBodies.bodies[_body - NUDGE_MAX_DYNAMIC_BODIES].shapeHandle;
			transform = &shapes.boxes.transforms[shape];
		}

		if ( transform )
		{
			set_float3(transform->position, _px, _py, _pz);
			set_float4(transform->rotation, _qx, _qy, _qz, _qw);
		}
	}
	void setTransform(BodyHandle _body, const float _pos[3], const float _quat[4])
	{
		nudge::Transform* transform = NULL;
		if ( _body < NUDGE_MAX_DYNAMIC_BODIES )
		{
			transform = &nh::bodies.transforms[_body];
		}
		else
		{
			uint16_t shape = nh::staticBodies.bodies[_body - NUDGE_MAX_DYNAMIC_BODIES].shapeHandle;
			transform = &shapes.boxes.transforms[shape];
		}
		if ( transform )
		{
			memcpy(transform->position, _pos, sizeof(float)*3);
			memcpy(transform->rotation, _quat, sizeof(float)*4);
		}
	}
	
	void getTransform(BodyHandle _body, float _pos[3], float _quat[4])
	{
		nudge::Transform* transform = NULL;
		if ( _body < NUDGE_MAX_DYNAMIC_BODIES )
		{
			transform = &nh::bodies.transforms[_body];
		}
		else
		{
			uint16_t shape = nh::staticBodies.bodies[_body - NUDGE_MAX_DYNAMIC_BODIES].shapeHandle;
			transform = &shapes.boxes.transforms[shape];
		}
		if ( transform )
		{
			memcpy(_pos, transform->position, sizeof(float)*3);
			memcpy(_quat, transform->rotation, sizeof(float)*4);
		}
	}

	void setBodyData(BodyHandle _body, UserDataType _data)
	{
		bodiesUserData[_body] = _data;
	}

	UserDataType getBodyData(BodyHandle _body)
	{
		return bodiesUserData[_body];
	}

	//contacts
	uint32_t getContactsCount()
	{
		return contacts.count;
	}
	
	BodyHandle getContactBody(uint32_t _index, uint32_t _bodyIndex)
	{
		return _bodyIndex == 0 ? contacts.bodies[_index].a + 1 : contacts.bodies[_index].b + 1;
	}
}

#endif


#endif /* NUDGE_NH_H_HEADER_GUARD */
