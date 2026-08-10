#pragma once
#include <cstdint>
bool Resolve_All(); // fills Offsets::* from live il2cpp via xdl
uintptr_t FindLibBase(const char* needle);
