#pragma once
#ifndef _LIBMEM_H_
#define _LIBMEM_H_
#endif

#define allocation(var,type, ...) var=(type*)allocate_object(sizeof(type)); *var = type(__VA_ARGS__);

static char static_memory[32 * 1024 * 1024];
static unsigned int static_memory_pointer = 0;

#ifndef _DEBUG
__forceinline float sinf(const float x) {
    float r; _asm fld  dword ptr[x];
    _asm fsin;
    _asm fstp dword ptr[r];
    return r;
}
__forceinline float cosf(const float x) {
    float r; _asm fld  dword ptr[x];
    _asm fcos;
    _asm fstp dword ptr[r];
    return r;
}
__forceinline float sqrtf(const float x) {
    float r; _asm fld  dword ptr[x];
    _asm fsqrt;
    _asm fstp dword ptr[r];
    return r;
}
__forceinline float abs(const float x) {
    float r; _asm fld  dword ptr[x];
    _asm fabs;
    _asm fstp dword ptr[r];
    return r;
}
__forceinline float tanf(const float x) {
    float r; _asm fld  dword ptr[x];
    _asm fptan;
    _asm fstp st(0)
    _asm fstp dword ptr[r];
    return r;
}
__forceinline float acosf(const float x) {
    float r;
    __asm {
        fld x
        fpatan
        fsqrt
        fdiv
        fld1
        fsub
        fpatan
        fstp r
    }
    return r;
}

extern "C" void* memset(void* ptr, int value, size_t num) {
    unsigned char* p = static_cast<unsigned char*>(ptr);
    while (num--) {
        *p++ = static_cast<unsigned char>(value);
    }
    return ptr;
}
#endif


void* allocate_object(unsigned int size)
{
	void* ptr = &(static_memory[static_memory_pointer]);
	static_memory_pointer += size;
	return ptr;
}