/*
 * Copyright (c) 2012-2024 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/containers/types.h"
#include "python/py_evn.h"
#include "resource/types.h"
#include "world/types.h"

namespace crown
{
/// Manages a set of scripts in a world.
///
/// @ingroup World
struct ScriptWorld
{
	struct ScriptData
	{
		int module_ref;
	};

	struct InstanceData
	{
		UnitId unit;
		u32 script_i;
	};

	u32 _marker;
	Array<ScriptData> _script;
	Array<InstanceData> _data;
	HashMap<UnitId, u32> _map;
	HashMap<StringId64, u32> _cache;

	UnitManager *_unit_manager;
	ResourceManager *_resource_manager;
	PyWrapper *_py_wrapper;
	World *_world;
	UnitDestroyCallback _unit_destroy_callback;
	bool _disable_callbacks;

	///
	ScriptWorld(Allocator &a, UnitManager &um, ResourceManager &rm, PyWrapper &py, World &w);

	///
	~ScriptWorld();
};

namespace script_world
{
	/// Creates a new component for the @a unit and returns its id.
	ScriptInstance create(ScriptWorld &sw, UnitId unit, const ScriptDesc &desc);

	/// Destroys the component for the @a unit.
	void destroy(ScriptWorld &sw, UnitId unit, ScriptInstance i);

	/// Returns the component id for the @a unit.
	ScriptInstance instance(ScriptWorld &sw, UnitId unit);

	/// Calls the update function on all scripts.
	void update(ScriptWorld &sw, f32 dt);

	///
	void collision(ScriptWorld &sw, const PhysicsCollisionEvent &ev);

} // namespace script_world

} // namespace crown
