// Pointer storage behind legacy 32-bit animdata fields.
//
// Animdata member offsets are baked into the shipped shape data
// (DONTPOINTS::flag and friends are byte offsets into the anim block),
// so pointer-sized fields inside these structs cannot be widened on
// 64-bit without breaking animation bindings. The field keeps the low
// 32 bits (truthiness checks and save fidelity); the full pointer lives
// in this side table, keyed by the field's own address. Load validates
// that the field still holds the stored pointer's low bits, so stale
// entries on recycled blocks resolve to null exactly like the original
// zeroed field would.
#pragma once

#include <unordered_map>
#include <cstdint>

namespace AnimLink
{
	inline std::unordered_map<void*, void*>& slots()
	{
		static std::unordered_map<void*, void*> m;
		return m;
	}

	inline void Store(uint32_t& field, const void* p)
	{
		field = (uint32_t)(uintptr_t)p;
		if (p)
			slots()[static_cast<void*>(&field)] = const_cast<void*>(p);
		else
			slots().erase(static_cast<void*>(&field));
	}

	inline void* Load(const uint32_t& field)
	{
		auto it = slots().find(const_cast<void*>(static_cast<const void*>(&field)));
		if (it == slots().end())
			return nullptr;
		if ((uint32_t)(uintptr_t)it->second != field)
			return nullptr;
		return it->second;
	}

	inline void Clear(uint32_t& field)
	{
		field = 0;
		slots().erase(static_cast<void*>(&field));
	}
}
