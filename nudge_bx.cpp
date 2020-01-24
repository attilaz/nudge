//
// Copyright (c) 2017 Rasmus Barringer
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

// questions:
//	simd_madd	fma / _mm_fmadd_ps
//  slower... simd128::concat2x32<2,2,0,0>(sxycxy, dxy); -> simd_shuf_xyAB(simd_swiz_zzzz(sxycxy), simd_swiz_xxxx(dxy));
//            simd128::concat2x32<3,3,1,1>(sxycxy, dxy); -> simd_shuf_xyAB(simd_swiz_wwww(sxycxy), simd_swiz_yyyy(dxy));

#include "nudge.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "bx_simd_t.h"
using namespace bx;

#ifdef _WIN32
#include <intrin.h>
#define NUDGE_ALIGNED(n) __declspec(align(n))
#define NUDGE_FORCEINLINE __forceinline
#else
#define NUDGE_ALIGNED(n) __attribute__((aligned(n)))
#define NUDGE_FORCEINLINE inline __attribute__((always_inline))
#endif

#define NUDGE_ARENA_SCOPE(A) Arena& scope_arena_##A = A; Arena A = scope_arena_##A

namespace nudge {

constexpr float float_max = 3.402823e+38f;

static const float allowed_penetration = 1e-3f;
static const float bias_factor = 2.0f;

#define NUDGE_SIMDV_ALIGNED NUDGE_ALIGNED(16)
static const unsigned simdv_width32 = 4;
static const unsigned simdv_width32_log2 = 2;

#ifdef _WIN32
NUDGE_FORCEINLINE simd128_t operator + (simd128_t a, simd128_t b) {
	return simd_add(a, b);
}

NUDGE_FORCEINLINE simd128_t operator * (simd128_t a, simd128_t b) {
	return simd_mul(a, b);
}


NUDGE_FORCEINLINE simd128_t operator - (simd128_t a, simd128_t b) {
	return simd_sub(a, b);
}


#endif

typedef simd128_t simd4_float;
typedef simd128_t simd4_int32;

	
NUDGE_FORCEINLINE void simd_transpose32(simd4_float& x, simd4_float& y, simd4_float& z, simd4_float& w) {
	const simd128_t tmp0 = simd_shuf_xAyB(x, y);
	const simd128_t tmp2 = simd_shuf_xAyB(z, w);
	const simd128_t tmp1 = simd_shuf_zCwD(x, y);
	const simd128_t tmp3 = simd_shuf_zCwD(z, w);
	x = simd_shuf_ABxy(tmp2, tmp0);
	y = simd_shuf_CDzw(tmp2, tmp0);
	z = simd_shuf_ABxy(tmp3, tmp1);
	w = simd_shuf_CDzw(tmp3, tmp1);
}

#if 0
	//arm64 test any/all
	uint32_t vminvq_u32 (uint32x4_t a)
	uint32_t vmaxvq_u32 (uint32x4_t a)
#endif
	
	
	uint32_t as_int(float v)
	{
		union
		{
			float f;
			uint32_t i;
		};
		f = v;
		return i;
	}

namespace simd {
#if 0	//this is only correct for comparison result (same bit value for the whole component)
	uint32_t _mm_movemask_ps(float32x4_t x) {
		uint32x4_t mmA = vandq_u32(
								   vreinterpretq_u32_f32(x), (uint32x4_t) {0x1, 0x2, 0x4, 0x8}); // [0 1 2 3]
		uint32x4_t mmB = vextq_u32(mmA, mmA, 2);                        // [2 3 0 1]
		uint32x4_t mmC = vorrq_u32(mmA, mmB);                           // [0+2 1+3 0+2 1+3]
		uint32x4_t mmD = vextq_u32(mmC, mmC, 3);                        // [1+3 0+2 1+3 0+2]
		uint32x4_t mmE = vorrq_u32(mmC, mmD);                           // [0+1+2+3 ...]
		return vgetq_lane_u32(mmE, 0);
	}
#endif
	
#if 0
	static const uint32x4_t movemask = {1, 2, 4, 8};
	static const uint32x4_t highbit = {0x80000000, 0x80000000, 0x80000000,
		0x80000000};
	uint32x4_t t0 = vreinterpretq_u32_m128(a);
	uint32x4_t t1 = vtstq_u32(t0, highbit);	//this makes all bits equal
	uint32x4_t t2 = vandq_u32(t1, movemask);
	//A64 :  vaddv_u32
	uint32x2_t t3 = vorr_u32(vget_low_u32(t2), vget_high_u32(t2));
	return vget_lane_u32(t3, 0) | vget_lane_u32(t3, 1);
#endif
	
	NUDGE_FORCEINLINE unsigned cmpmask32(simd128_t x) {
		uint32_t i[4];
		i[0] = as_int(simd_x(x));
		i[1] = as_int(simd_y(x));
		i[2] = as_int(simd_z(x));
		i[3] = as_int(simd_w(x));
		assert(i[0] == 0x0 || i[0] == 0xffffffff);
		assert(i[1] == 0x0 || i[1] == 0xffffffff);
		assert(i[2] == 0x0 || i[2] == 0xffffffff);
		assert(i[3] == 0x0 || i[3] == 0xffffffff);
		return _mm_movemask_ps(x);
	}

	void test_cmpmask32()
	{	//precondition is that each component is 0 or 0xffffffff
		simd128_t a = simd_ild(0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff);
		assert( 0xf == cmpmask32(a));
		a = simd_ild(0x0, 0x0, 0x0, 0x0);
		assert( 0x0 == cmpmask32(a));
		
		a = simd_ild(0xffffffff, 0x0, 0xffffffff, 0x0);
		assert( 0x5 == cmpmask32(a));
		
		a = simd_ild(0x0, 0xffffffff, 0x0, 0xffffffff);
		assert( 0xa == cmpmask32(a));
	}

	NUDGE_FORCEINLINE unsigned signmask32(simd128_t x) {
		return _mm_movemask_ps(x);
	}

	void test_signmask32()
	{
		simd128_t a = simd_ild(0xffffffff, 0x80000000, 0xffffffff, 0x80000000);
		assert( 0xf == signmask32(a));

		a = simd_ild(0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff);
		assert( 0x0 == signmask32(a));
		
		a = simd_ild(0xffffffff, 0x0, 0x80000000, 0x0);
		assert( 0x5 == signmask32(a));
		
		a = simd_ild(0x0, 0x80000000, 0x0, 0xffffffff);
		assert( 0xa == signmask32(a));

	}

	
	NUDGE_FORCEINLINE simd128_t blendv32(simd128_t x, simd128_t y, simd128_t s) {
#if defined(__SSE4_1__) || defined(__AVX__)
#define NUDGE_NATIVE_BLENDV32
		return _mm_blendv_ps(x, y, s);
#else
		s = _mm_castsi128_ps(_mm_srai_epi32(_mm_castps_si128(s), 31));
		return _mm_or_ps(_mm_andnot_ps(s, x), _mm_and_ps(s, y));
#endif
	}
	
	void test_blendv32()
	{
		simd128_t a = simd_ld(1.0f, 2.0f, 3.0f, 4.0f);
		simd128_t b = simd_ld(5.0f, 6.0f, 7.0f, 8.0f);
		simd128_t s = simd_ild(0x80000000, 0xffffffff, 0x7fffffff, 0x0);

		simd128_t v = blendv32(a, b, s);
		assert(simd_x(v) == 5.0f);
		assert(simd_y(v) == 6.0f);
		assert(simd_z(v) == 3.0f);
		assert(simd_w(v) == 4.0f);
	}
}

namespace simd_float {

	inline BX_CONST_FUNC bool isNan(float _f)
	{
		union { float f; uint32_t ui; } u = { _f };
		const uint32_t tmp = u.ui & INT32_MAX;
		return tmp > UINT32_C(0x7f800000);
	}

	// Note: First operand is returned on NaN.
	NUDGE_FORCEINLINE simd4_float min_second_nan(simd4_float x, simd4_float y) {
		assert(!isNan(simd_x(x)));
		assert(!isNan(simd_y(x)));
		assert(!isNan(simd_z(x)));
		assert(!isNan(simd_w(x)));
		// Note: For SSE, second operand is returned on NaN.
		// todo: check Nan behaviour for neon
		return simd_min(y, x);
	}
	
	void test_min_second_nan()
	{
		simd128_t a = simd_ld(1.0f, 2.0f, 3.0f, 4.0f);
		simd128_t b = simd_ld(5.0f, NAN, 7.0f, 8.0f);

		simd128_t v = min_second_nan(a,b);
		assert(simd_x(v) == 1.0f);
		assert(simd_y(v) == 2.0f);
		assert(simd_z(v) == 3.0f);
		assert(simd_w(v) == 4.0f);
	}
}

//todo: these are slow implementations
BX_SIMD_FORCE_INLINE simd128_t simd_swiz_zzAA(simd128_t _a, simd128_t _b)
{
	return simd_shuf_xyAB(simd_swiz_zzzz(_a), simd_swiz_xxxx(_b));
}

BX_SIMD_FORCE_INLINE simd128_t simd_swiz_wwBB(simd128_t _a, simd128_t _b)
{
	return simd_shuf_xyAB(simd_swiz_wwww(_a), simd_swiz_yyyy(_b));
}

// these needs neon impl

// ext
//neon docs: http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0472k/chr1359125038862.html
//			 https://developer.arm.com/architectures/instruction-sets/simd-isas/neon/intrinsics
// to read:  https://people.xiph.org/~tterribe/daala/neon_tutorial.pdf

	//todo: add this to bx
	// neon: vmvn_u32 for result
BX_SIMD_FORCE_INLINE simd128_t simd_cmpneq(simd128_t x, simd128_t y) {
	return _mm_cmpneq_ps(x, y);
}

	//todo: add this to bx ??
	//neon: unaligned is same as aligned store
BX_SIMD_FORCE_INLINE void simd_stu(void* _ptr, simd128_t _a)
{
	_mm_storeu_ps(reinterpret_cast<float*>(_ptr), _a);
}

// neon int16x4_t vadd_s16 (int16x4_t a, int16x4_t b)
BX_SIMD_FORCE_INLINE simd128_t simd_i16_add(simd128_t _a, simd128_t _b)
{
	const __m128i a = _mm_castps_si128(_a);
	const __m128i b = _mm_castps_si128(_b);
	const __m128i add = _mm_add_epi16(a, b);
	const simd128_sse_t result = _mm_castsi128_ps(add);

	return result;
}
	
void test_simd_i16_add()
{
	simd128_t a = simd_ild(0x0000ffff, 0x00010002, 0x00008000, 0x7fff0000);
	simd128_t b = simd_ild(0x00000001, 0x00010002, 0x00008000, 0x7fff0000);
	
	simd128_t v = simd_i16_add(a,b);
	
	assert(as_int(simd_x(v)) == 0x0 );
	assert(as_int(simd_y(v)) == 0x00020004 );
	assert(as_int(simd_z(v)) == 0x0 );
	assert(as_int(simd_w(v)) == 0xfffe0000 );
}

// neon: uint16x4_t vceq_s16(int16x4_t a, int16x4_t b);
BX_SIMD_FORCE_INLINE simd128_t simd_i16_cmpeq(simd128_t _a, simd128_t _b)
{
	const __m128i a = _mm_castps_si128(_a);
	const __m128i b = _mm_castps_si128(_b);
	const __m128i cmp = _mm_cmpeq_epi16(a, b);
	const simd128_sse_t result = _mm_castsi128_ps(cmp);

	return result;
}
	
void test_simd_i16_cmpeq()
{
	simd128_t a = simd_ild(0x0000ffff, 0x00010002, 0x00008000, 0x7fff0000);
	simd128_t b = simd_ild(0x00000001, 0x00010002, 0x00038000, 0x7ffe0001);

	simd128_t v = simd_i16_cmpeq(a,b);
	
	assert(as_int(simd_x(v)) == 0xffff0000 );
	assert(as_int(simd_y(v)) == 0xffffffff );
	assert(as_int(simd_z(v)) == 0x0000ffff );
	assert(as_int(simd_w(v)) == 0x0 );

}


// neon: int16x4_t  vshr_n_s16(int16x4_t a, __constrange(1,16) int b);   // VSHR.S16 d0,d0,#16
// templated version for constant
BX_SIMD_FORCE_INLINE simd128_t simd_i16_srl(simd128_t _a, int _bits)
{
	const __m128i a = _mm_castps_si128(_a);
	const __m128i add = _mm_srli_epi16(a, _bits);
	const simd128_sse_t result = _mm_castsi128_ps(add);

	return result;
}

void test_simd_i16_srl()
{
	simd128_t a = simd_ild(0x0000ffff, 0x00010002, 0x00008000, 0x7fff0000);
	
	simd128_t v = simd_i16_srl(a,1);
	
	assert(as_int(simd_x(v)) == 0x00007fff );
	assert(as_int(simd_y(v)) == 0x00000001 );
	assert(as_int(simd_z(v)) == 0x00004000 );
	assert(as_int(simd_w(v)) == 0x3fff0000 );
}

	
// neon: int16x4_t  vmovn_s32(int32x4_t a);	//narrow to 16 bit
//   vcombine_s16
		//convert with signed saturation, but this only gets 0x0 and 0xffffffff here so neon one should support only that
BX_SIMD_FORCE_INLINE simd128_t simd_pack_i32_to_i16(simd128_t _a, simd128_t _b)
{
	const __m128i a = _mm_castps_si128(_a);
	const __m128i b = _mm_castps_si128(_b);
	const __m128i packed = _mm_packs_epi32(a, b);
	const simd128_sse_t result = _mm_castsi128_ps(packed);
#if 0
	uint32_t i[8];
	i[0] = as_int(simd_x(_a));
	i[1] = as_int(simd_y(_a));
	i[2] = as_int(simd_z(_a));
	i[3] = as_int(simd_w(_a));
	i[4] = as_int(simd_x(_b));
	i[5] = as_int(simd_y(_b));
	i[6] = as_int(simd_z(_b));
	i[7] = as_int(simd_w(_b));
	assert(i[0] == 0x0 || i[0] == 0xffffffff);
	assert(i[1] == 0x0 || i[1] == 0xffffffff);
	assert(i[2] == 0x0 || i[2] == 0xffffffff);
	assert(i[3] == 0x0 || i[3] == 0xffffffff);
	assert(i[4] == 0x0 || i[4] == 0xffffffff);
	assert(i[5] == 0x0 || i[5] == 0xffffffff);
	assert(i[6] == 0x0 || i[6] == 0xffffffff);
	assert(i[7] == 0x0 || i[7] == 0xffffffff);
#endif
	return result;
}
	
void test_simd_pack_i32_to_i16()
{
	simd128_t a = simd_ild(0x0001ffff, 0xffffffff, 0x80000004, 0xffffffff);
	simd128_t b = simd_ild(0x00000002, 0x00000003, 0xffffffff, 0xffffffff);

	simd128_t v = simd_pack_i32_to_i16(a,b);
		
	assert(as_int(simd_x(v)) == 0xffff7fff );
	assert(as_int(simd_y(v)) == 0xffff8000 );
	assert(as_int(simd_z(v)) == 0x00030002 );
	assert(as_int(simd_w(v)) == 0xffffffff );
}
	
	
// neon: int8x8_t  vmovn_s16(int16x8_t a);	//narrow to 16 bit
//   int8x16_t   vcombine_s8(int8x8_t low, int8x8_t high);
BX_SIMD_FORCE_INLINE simd128_t simd_pack_i16_to_i8(simd128_t _a, simd128_t _b)
{
	const __m128i a = _mm_castps_si128(_a);
	const __m128i b = _mm_castps_si128(_b);
	const __m128i packed = _mm_packs_epi16(a, b);
	const simd128_sse_t result = _mm_castsi128_ps(packed);

	return result;
}

	void test_simd_pack_i16_to_i8()
	{
		simd128_t a = simd_ild(0x0001ffff, 0xffffff00, 0x80000004, 0xffffffff);
		simd128_t b = simd_ild(0x00000002, 0x00000103, 0xffffffff, 0xffffffff);

		simd128_t v = simd_pack_i16_to_i8(a,b);
		
		assert(as_int(simd_x(v)) == 0xff8001ff );
		assert(as_int(simd_y(v)) == 0xffff8004 );
		assert(as_int(simd_z(v)) == 0x007f0002 );
		assert(as_int(simd_w(v)) == 0xffffffff );
	}

	
// Unpack and interleave 16 - bit integers from the low half of a and b, and store the results in dst.
// neon: int16x8_t   vcombine_s16(int16x4_t low, int16x4_t high);
	// int16x4_t   vget_high_s16(int16x8_t a);
	// int16x4_t   vget_low_s16(int16x8_t a);
BX_SIMD_FORCE_INLINE simd128_t simd_shuf_xAyBzCwD(simd128_t _a, simd128_t _b)
{
	const __m128i a = _mm_castps_si128(_a);
	const __m128i b = _mm_castps_si128(_b);
	const __m128i packed = _mm_unpacklo_epi16(a, b);
	const simd128_sse_t result = _mm_castsi128_ps(packed);

	return result;
}
	
	void test_simd_shuf_xAyBzCwD()
	{
		simd128_t a = simd_ild(0x0001ffff, 0xffffff00, 0x80000004, 0xffffffff);
		simd128_t b = simd_ild(0x00000002, 0x00000103, 0xffffffff, 0xffffffff);
		
		simd128_t v = simd_shuf_xAyBzCwD(a,b);
		
		assert(as_int(simd_x(v)) == 0x0002ffff );
		assert(as_int(simd_y(v)) == 0x00000001 );
		assert(as_int(simd_z(v)) == 0x0103ff00 );
		assert(as_int(simd_w(v)) == 0x0000ffff );
	}


// Create mask from the most significant bit of each 8-bit element in a, and store the result in dst.
BX_SIMD_FORCE_INLINE int simd_i8_mask(simd128_t _a)
{
	const __m128i a = _mm_castps_si128(_a);
	return _mm_movemask_epi8(a);
}
	
	void test_simd_i8_mask()
	{
		simd128_t a = simd_ild(0x0001ff80, 0xffff7f0f, 0x80002080, 0xffffffff);
		//						  0 0 1 1     1 1 0 0    1 0 0 1     1 1 1 1
		int v = simd_i8_mask(a);
		assert(v == 0xf9c3);
	}


#if 0
// NEON does not provide a version of this function, here is an article about some ways to repro the results.
// http://stackoverflow.com/questions/11870910/sse-mm-movemask-epi8-equivalent-method-for-arm-neon
// Creates a 16-bit mask from the most significant bits of the 16 signed or unsigned 8-bit integers in a and zero extends the upper bits. https://msdn.microsoft.com/en-us/library/vstudio/s090c8fk(v=vs.100).aspx
FORCE_INLINE int _mm_movemask_epi8(__m128i _a)
{
	uint8x16_t input = vreinterpretq_u8_m128i(_a);
	static const int8_t __attribute__((aligned(16))) xr[8] = { -7, -6, -5, -4, -3, -2, -1, 0 };
	uint8x8_t mask_and = vdup_n_u8(0x80);
	int8x8_t mask_shift = vld1_s8(xr);

	uint8x8_t lo = vget_low_u8(input);
	uint8x8_t hi = vget_high_u8(input);

	lo = vand_u8(lo, mask_and);
	lo = vshl_u8(lo, mask_shift);

	hi = vand_u8(hi, mask_and);
	hi = vshl_u8(hi, mask_shift);

	lo = vpadd_u8(lo, lo);
	lo = vpadd_u8(lo, lo);
	lo = vpadd_u8(lo, lo);

	hi = vpadd_u8(hi, hi);
	hi = vpadd_u8(hi, hi);
	hi = vpadd_u8(hi, hi);

	return ((hi[0] << 8) | (lo[0] & 0xFF));
}
#endif

	
	void test()
	{
		simd::test_cmpmask32();
		simd::test_signmask32();
		simd::test_blendv32();
		simd_float::test_min_second_nan();
		
		test_simd_i16_add();
		test_simd_i16_cmpeq();
		test_simd_i16_srl();
		test_simd_pack_i32_to_i16();
		test_simd_pack_i16_to_i8();
		test_simd_shuf_xAyBzCwD();
		test_simd_i8_mask();
	}

namespace simd_aos {
	NUDGE_FORCEINLINE simd4_float dot(simd4_float a, simd4_float b) {
		simd4_float c = a*b;
		return simd_swiz_xxxx(c) + simd_swiz_yyyy(c) + simd_swiz_zzzz(c);
	}
	
	NUDGE_FORCEINLINE simd4_float cross(simd4_float a, simd4_float b) {
		simd4_float c = simd_swiz_yzxx(a) * simd_swiz_zxyx(b);
		simd4_float d = simd_swiz_zxyx(a) * simd_swiz_yzxx(b);
		return simd_sub(c, d);
	}
}

namespace simd_soa {
	NUDGE_FORCEINLINE void cross(simd4_float ax, simd4_float ay, simd4_float az, simd4_float bx, simd4_float by, simd4_float bz, simd4_float& rx, simd4_float& ry, simd4_float& rz) {
		rx = simd_sub(ay*bz, az*by);
		ry = simd_sub(az*bx, ax*bz);
		rz = simd_sub(ax*by, ay*bx);
	}
	
	NUDGE_FORCEINLINE void normalize(simd4_float& x, simd4_float& y, simd4_float& z) {
		simd4_float f = simd_rsqrt_est(x*x + y*y + z*z);
		x = simd_mul(x,f);
		y = simd_mul(y, f);
		z = simd_mul(z, f);
	}
}

namespace {
	struct float3 {
		float x, y, z;
	};
	
	struct float3x3 {
		float3 c0, c1, c2;
	};
	
	struct Rotation {
		float3 v;
		float s;
	};
	
	struct AABB {
		float3 min;
		float unused0;
		float3 max;
		float unused1;
	};
	
	struct AABBV {
		float min_x[simdv_width32];
		float max_x[simdv_width32];
		float min_y[simdv_width32];
		float max_y[simdv_width32];
		float min_z[simdv_width32];
		float max_z[simdv_width32];
	};
	
	struct ContactSlotV {
		uint32_t indices[simdv_width32];
	};
	
	struct ContactPairV {
		uint32_t ab[simdv_width32];
	};
	
	struct ContactConstraintV {
		uint16_t a[simdv_width32];
		uint16_t b[simdv_width32];
		
		float pa_z[simdv_width32];
		float pa_x[simdv_width32];
		float pa_y[simdv_width32];
		
		float pb_z[simdv_width32];
		float pb_x[simdv_width32];
		float pb_y[simdv_width32];
		
		float n_x[simdv_width32];
		float u_x[simdv_width32];
		float v_x[simdv_width32];
		
		float n_y[simdv_width32];
		float u_y[simdv_width32];
		float v_y[simdv_width32];
		
		float n_z[simdv_width32];
		float u_z[simdv_width32];
		float v_z[simdv_width32];
		
		float bias[simdv_width32];
		float friction[simdv_width32];
		float normal_velocity_to_normal_impulse[simdv_width32];
		
		float friction_coefficient_x[simdv_width32];
		float friction_coefficient_y[simdv_width32];
		float friction_coefficient_z[simdv_width32];
		
		float na_x[simdv_width32];
		float na_y[simdv_width32];
		float na_z[simdv_width32];
		
		float nb_x[simdv_width32];
		float nb_y[simdv_width32];
		float nb_z[simdv_width32];
		
		float ua_x[simdv_width32];
		float ua_y[simdv_width32];
		float ua_z[simdv_width32];
		
		float va_x[simdv_width32];
		float va_y[simdv_width32];
		float va_z[simdv_width32];
		
		float ub_x[simdv_width32];
		float ub_y[simdv_width32];
		float ub_z[simdv_width32];
		
		float vb_x[simdv_width32];
		float vb_y[simdv_width32];
		float vb_z[simdv_width32];
	};
	
	struct ContactConstraintStateV {
		float applied_normal_impulse[simdv_width32];
		float applied_friction_impulse_x[simdv_width32];
		float applied_friction_impulse_y[simdv_width32];
	};
	
	struct InertiaTransform {
		float xx;
		float yy;
		float zz;
		float unused0;
		float xy;
		float xz;
		float yz;
		float unused1;
	};
}

#ifdef _WIN32
static inline unsigned first_set_bit(unsigned x) {
	unsigned long r = 0;
	_BitScanForward(&r, x);
	return r;
}
#else	// this is fine for clang/gcc
static inline unsigned first_set_bit(unsigned x) {
	return __builtin_ctz(x);
}
#endif

static inline void* align(Arena* arena, uintptr_t alignment) {
	uintptr_t data = (uintptr_t)arena->data;
	uintptr_t end = data + arena->size;
	uintptr_t mask = alignment-1;
	
	data = (data + mask) & ~mask;
	
	arena->data = (void*)data;
	arena->size = end - data;
	
	assert((intptr_t)arena->size >= 0); // Out of memory.
	
	return arena->data;
}

static inline void* allocate(Arena* arena, uintptr_t size) {
	test();
	
	void* data = arena->data;
	arena->data = (void*)((uintptr_t)data + size);
	arena->size -= size;
	
	assert((intptr_t)arena->size >= 0); // Out of memory.
	
	return data;
}

static inline void* allocate(Arena* arena, uintptr_t size, uintptr_t alignment) {
	align(arena, alignment);
	
	void* data = arena->data;
	arena->data = (void*)((uintptr_t)data + size);
	arena->size -= size;
	
	assert((intptr_t)arena->size >= 0); // Out of memory.
	
	return data;
}

template<class T>
static inline T* allocate_struct(Arena* arena, uintptr_t alignment) {
	return static_cast<T*>(allocate(arena, sizeof(T), alignment));
}

template<class T>
static inline T* allocate_array(Arena* arena, uintptr_t count, uintptr_t alignment) {
	return static_cast<T*>(allocate(arena, sizeof(T)*count, alignment));
}

static inline void* reserve(Arena* arena, uintptr_t size, uintptr_t alignment) {
	align(arena, alignment);
	assert(size <= arena->size); // Cannot reserve this amount.
	return arena->data;
}

static inline void commit(Arena* arena, uintptr_t size) {
	allocate(arena, size);
}

template<class T>
static inline T* reserve_array(Arena* arena, uintptr_t count, uintptr_t alignment) {
	return static_cast<T*>(reserve(arena, sizeof(T)*count, alignment));
}

template<class T>
static inline void commit_array(Arena* arena, uintptr_t count) {
	commit(arena, sizeof(T)*count);
}

static inline Rotation make_rotation(const float q[4]) {
	Rotation r = { { q[0], q[1], q[2] }, q[3] };
	return r;
}

static inline float3 make_float3(const float x[3]) {
	float3 r = { x[0], x[1], x[2] };
	return r;
}

static inline float3 make_float3(float x, float y, float z) {
	float3 r = { x, y, z };
	return r;
}

static inline float3 make_float3(float x) {
	float3 r = { x, x, x };
	return r;
}

static inline float3 operator + (float3 a, float3 b) {
	float3 r = { a.x + b.x, a.y + b.y, a.z + b.z };
	return r;
}

static inline float3 operator - (float3 a, float3 b) {
	float3 r = { a.x - b.x, a.y - b.y, a.z - b.z };
	return r;
}

static inline float3 operator * (float a, float3 b) {
	float3 r = { a * b.x, a * b.y, a * b.z };
	return r;
}

static inline float3 operator * (float3 a, float b) {
	float3 r = { a.x * b, a.y * b, a.z * b };
	return r;
}

static inline float3& operator *= (float3& a, float b) {
	a.x *= b;
	a.y *= b;
	a.z *= b;
	return a;
}

static inline float dot(float3 a, float3 b) {
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

static inline float length2(float3 a) {
	return dot(a, a);
}

static inline float3 cross(float3 a, float3 b) {
	float3 v = { a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x };
	return v;
}

static inline float3 operator * (Rotation lhs, float3 rhs) {
	float3 t = 2.0f * cross(lhs.v, rhs);
	return rhs + lhs.s * t + cross(lhs.v, t);
}

static inline Rotation operator * (Rotation lhs, Rotation rhs) {
	float3 v = rhs.v*lhs.s + lhs.v*rhs.s + cross(lhs.v, rhs.v);
	Rotation r = { v, lhs.s*rhs.s - dot(lhs.v, rhs.v) };
	return r;
}

static inline Rotation normalize(Rotation r) {
	float f = 1.0f / sqrtf(r.s*r.s + r.v.x*r.v.x + r.v.y*r.v.y + r.v.z*r.v.z);
	r.v *= f;
	r.s *= f;
	return r;
}

static inline Rotation inverse(Rotation r) {
	r.v.x = -r.v.x;
	r.v.y = -r.v.y;
	r.v.z = -r.v.z;
	return r;
}

static inline float3x3 matrix(Rotation q) {
	float kx = q.v.x + q.v.x;
	float ky = q.v.y + q.v.y;
	float kz = q.v.z + q.v.z;
	
	float xx = kx*q.v.x;
	float yy = ky*q.v.y;
	float zz = kz*q.v.z;
	float xy = kx*q.v.y;
	float xz = kx*q.v.z;
	float yz = ky*q.v.z;
	float sx = kx*q.s;
	float sy = ky*q.s;
	float sz = kz*q.s;
	
	float3x3 m = {
		{ 1.0f - yy - zz, xy + sz, xz - sy },
		{ xy - sz, 1.0f - xx - zz, yz + sx },
		{ xz + sy, yz - sx, 1.0f - xx - yy },
	};
	return m;
}

static inline Transform operator * (Transform lhs, Transform rhs) {
	float3 p = make_rotation(lhs.rotation) * make_float3(rhs.position) + make_float3(lhs.position);
	Rotation q = make_rotation(lhs.rotation) * make_rotation(rhs.rotation);
	
	Transform r = {
		{ p.x, p.y, p.z },
		rhs.body,
		{ q.v.x, q.v.y, q.v.z, q.s },
	};
	return r;
}

static unsigned box_box_collide(uint32_t* pairs, unsigned pair_count, BoxCollider* colliders, Transform* transforms, Contact* contacts, BodyPair* bodies, uint64_t* tags, Arena temporary) {
	// TODO: We may want to batch/chunk this for better cache behavior for repeatedly accessed data.
	// TODO: We should make use of 8-wide SIMD here as well.
	
	float* feature_penetrations = allocate_array<float>(&temporary, pair_count + 7, 32); // Padding is required.
	uint32_t* features = allocate_array<uint32_t>(&temporary, pair_count + 7, 32);
	
	unsigned count = 0;
	
	// Determine most separating face and reject pairs separated by a face.
	{
		pairs[pair_count+0] = 0; // Padding.
		pairs[pair_count+1] = 0;
		pairs[pair_count+2] = 0;
		
		unsigned added = 0;
		
		// Transform each box into the local space of the other in order to quickly determine per-face penetration.
		for (unsigned i = 0; i < pair_count; i += 4) {
			// Load pairs.
			unsigned pair0 = pairs[i+0];
			unsigned pair1 = pairs[i+1];
			unsigned pair2 = pairs[i+2];
			unsigned pair3 = pairs[i+3];
			
			unsigned a0_index = pair0 & 0xffff;
			unsigned b0_index = pair0 >> 16;
			
			unsigned a1_index = pair1 & 0xffff;
			unsigned b1_index = pair1 >> 16;
			
			unsigned a2_index = pair2 & 0xffff;
			unsigned b2_index = pair2 >> 16;
			
			unsigned a3_index = pair3 & 0xffff;
			unsigned b3_index = pair3 >> 16;
			
			// Load rotations.
			simd4_float a_rotation_x = simd_ld(transforms[a0_index].rotation);
			simd4_float a_rotation_y = simd_ld(transforms[a1_index].rotation);
			simd4_float a_rotation_z = simd_ld(transforms[a2_index].rotation);
			simd4_float a_rotation_s = simd_ld(transforms[a3_index].rotation);
			
			simd4_float b_rotation_x = simd_ld(transforms[b0_index].rotation);
			simd4_float b_rotation_y = simd_ld(transforms[b1_index].rotation);
			simd4_float b_rotation_z = simd_ld(transforms[b2_index].rotation);
			simd4_float b_rotation_s = simd_ld(transforms[b3_index].rotation);
			
			simd_transpose32(a_rotation_x, a_rotation_y, a_rotation_z, a_rotation_s);
			simd_transpose32(b_rotation_x, b_rotation_y, b_rotation_z, b_rotation_s);
			
			// Determine quaternion for rotation from a to b.
			simd4_float t_x, t_y, t_z;
			simd_soa::cross(b_rotation_x, b_rotation_y, b_rotation_z, a_rotation_x, a_rotation_y, a_rotation_z, t_x, t_y, t_z);
			
			simd4_float relative_rotation_x = simd_sub(a_rotation_x * b_rotation_s, simd_madd(b_rotation_x, a_rotation_s, t_x));
			simd4_float relative_rotation_y = simd_sub(a_rotation_y * b_rotation_s, simd_madd(b_rotation_y, a_rotation_s, t_y));
			simd4_float relative_rotation_z = simd_sub(a_rotation_z * b_rotation_s, simd_madd(b_rotation_z, a_rotation_s, t_z));
			simd4_float relative_rotation_s = (a_rotation_x * b_rotation_x +
											   a_rotation_y * b_rotation_y +
											   a_rotation_z * b_rotation_z +
											   a_rotation_s * b_rotation_s);
			
			// Compute the corresponding matrix.
			// Note that the b to a matrix is simply the transpose of a to b.
			simd4_float kx = relative_rotation_x + relative_rotation_x;
			simd4_float ky = relative_rotation_y + relative_rotation_y;
			simd4_float kz = relative_rotation_z + relative_rotation_z;
			
			simd4_float xx = kx * relative_rotation_x;
			simd4_float yy = ky * relative_rotation_y;
			simd4_float zz = kz * relative_rotation_z;
			simd4_float xy = kx * relative_rotation_y;
			simd4_float xz = kx * relative_rotation_z;
			simd4_float yz = ky * relative_rotation_z;
			simd4_float sx = kx * relative_rotation_s;
			simd4_float sy = ky * relative_rotation_s;
			simd4_float sz = kz * relative_rotation_s;
			
			simd4_float one = simd_splat(1.0f);
			
			simd4_float vx_x = simd_sub(one, simd_add(yy, zz));
			simd4_float vx_y = xy + sz;
			simd4_float vx_z = simd_sub(xz, sy);
			
			simd4_float vy_x = simd_sub(xy, sz);
			simd4_float vy_y = simd_sub(one, simd_add(xx, zz));
			simd4_float vy_z = yz + sx;
			
			simd4_float vz_x = xz + sy;
			simd4_float vz_y = simd_sub(yz, sx);
			simd4_float vz_z = simd_sub(one, simd_add(xx, yy));
			
			// Load sizes.
			simd4_float a_size_x = simd_ld(colliders[a0_index].size);
			simd4_float a_size_y = simd_ld(colliders[a1_index].size);
			simd4_float a_size_z = simd_ld(colliders[a2_index].size);
			simd4_float a_size_w = simd_ld(colliders[a3_index].size);
			
			simd4_float b_size_x = simd_ld(colliders[b0_index].size);
			simd4_float b_size_y = simd_ld(colliders[b1_index].size);
			simd4_float b_size_z = simd_ld(colliders[b2_index].size);
			simd4_float b_size_w = simd_ld(colliders[b3_index].size);
			
			simd_transpose32(a_size_x, a_size_y, a_size_z, a_size_w);
			simd_transpose32(b_size_x, b_size_y, b_size_z, b_size_w);
			
			// Compute the penetration.
			vx_x = simd_abs(vx_x);
			vx_y = simd_abs(vx_y);
			vx_z = simd_abs(vx_z);
			
			vy_x = simd_abs(vy_x);
			vy_y = simd_abs(vy_y);
			vy_z = simd_abs(vy_z);
			
			vz_x = simd_abs(vz_x);
			vz_y = simd_abs(vz_y);
			vz_z = simd_abs(vz_z);
			
			simd4_float pax = b_size_x + vx_x*a_size_x + vy_x*a_size_y + vz_x*a_size_z;
			simd4_float pay = b_size_y + vx_y*a_size_x + vy_y*a_size_y + vz_y*a_size_z;
			simd4_float paz = b_size_z + vx_z*a_size_x + vy_z*a_size_y + vz_z*a_size_z;
			
			simd4_float pbx = a_size_x + vx_x*b_size_x + vx_y*b_size_y + vx_z*b_size_z;
			simd4_float pby = a_size_y + vy_x*b_size_x + vy_y*b_size_y + vy_z*b_size_z;
			simd4_float pbz = a_size_z + vz_x*b_size_x + vz_y*b_size_y + vz_z*b_size_z;
			
			// Load positions.
			simd4_float a_position_x = simd_ld(transforms[a0_index].position);
			simd4_float a_position_y = simd_ld(transforms[a1_index].position);
			simd4_float a_position_z = simd_ld(transforms[a2_index].position);
			simd4_float a_position_w = simd_ld(transforms[a3_index].position);
			
			simd4_float b_position_x = simd_ld(transforms[b0_index].position);
			simd4_float b_position_y = simd_ld(transforms[b1_index].position);
			simd4_float b_position_z = simd_ld(transforms[b2_index].position);
			simd4_float b_position_w = simd_ld(transforms[b3_index].position);
			
			// Compute relative positions and offset the penetrations.
			simd4_float delta_x = simd_sub(a_position_x, b_position_x);
			simd4_float delta_y = simd_sub(a_position_y, b_position_y);
			simd4_float delta_z = simd_sub(a_position_z, b_position_z);
			simd4_float delta_w = simd_sub(a_position_w, b_position_w);
			
			simd_transpose32(delta_x, delta_y, delta_z, delta_w);
			
			simd_soa::cross(b_rotation_x, b_rotation_y, b_rotation_z, delta_x, delta_y, delta_z, t_x, t_y, t_z);
			t_x = simd_add(t_x, t_x);
			t_y = simd_add(t_y, t_y);
			t_z = simd_add(t_z, t_z);
			
			simd4_float u_x, u_y, u_z;
			simd_soa::cross(b_rotation_x, b_rotation_y, b_rotation_z, t_x, t_y, t_z, u_x, u_y, u_z);
			
			simd4_float a_offset_x = simd_sub(u_x + delta_x, b_rotation_s * t_x);
			simd4_float a_offset_y = simd_sub(u_y + delta_y, b_rotation_s * t_y);
			simd4_float a_offset_z = simd_sub(u_z + delta_z, b_rotation_s * t_z);
			
			pax = simd_sub(pax, simd_abs(a_offset_x));
			pay = simd_sub(pay, simd_abs(a_offset_y));
			paz = simd_sub(paz, simd_abs(a_offset_z));
			
			simd_soa::cross(delta_x, delta_y, delta_z, a_rotation_x, a_rotation_y, a_rotation_z, t_x, t_y, t_z);
			t_x = simd_add(t_x, t_x);
			t_y = simd_add(t_y, t_y);
			t_z = simd_add(t_z, t_z);

			simd_soa::cross(a_rotation_x, a_rotation_y, a_rotation_z, t_x, t_y, t_z, u_x, u_y, u_z);
			
			simd4_float b_offset_x = simd_sub(u_x, simd_madd(a_rotation_s, t_x, delta_x));
			simd4_float b_offset_y = simd_sub(u_y, simd_madd(a_rotation_s, t_y, delta_y));
			simd4_float b_offset_z = simd_sub(u_z, simd_madd(a_rotation_s, t_z, delta_z));
			
			pbx = simd_sub(pbx, simd_abs(b_offset_x));
			pby = simd_sub(pby, simd_abs(b_offset_y));
			pbz = simd_sub(pbz, simd_abs(b_offset_z));
			
			// Reduce face penetrations.
			simd4_float payz = simd_min(pay, paz);
			simd4_float pbyz = simd_min(pby, pbz);
			
			simd4_float pa = simd_min(pax, payz);
			simd4_float pb = simd_min(pbx, pbyz);
			
			simd4_float p = simd_min(pa, pb);
			
			// Determine the best aligned face for each collider.
			simd4_float aymf = simd_cmpeq(payz, pa);
			simd4_float azmf = simd_cmpeq(paz, pa);
			
			simd4_float bymf = simd_cmpeq(pbyz, pb);
			simd4_float bzmf = simd_cmpeq(pbz, pb);
			
			simd4_int32 aymi = simd_and(aymf, simd_isplat(1));
			simd4_int32 azmi = simd_and(azmf, simd_isplat(1));
			
			simd4_int32 bymi = simd_and(bymf, simd_isplat(1));
			simd4_int32 bzmi = simd_and(bzmf, simd_isplat(1));
			
			simd4_int32 aface = simd_iadd(aymi, azmi);
			simd4_int32 bface = simd_iadd(bymi, bzmi);
			
			// Swap so that collider a has the most separating face.
			simd4_float swap = simd_cmpeq(pa, p);
			
			simd4_float pair_a_b = simd_ld((const int32_t*)(pairs + i));
			simd4_float pair_b_a = simd_or(simd_sll(pair_a_b, 16), simd_srl(pair_a_b, 16));
			
			simd4_float face = simd::blendv32(bface, aface, swap);
			simd4_float pair = simd::blendv32(pair_a_b, pair_b_a, swap);
			
			// Store data for pairs with positive penetration.
			unsigned mask = simd::cmpmask32(simd_cmpgt(p, simd_zero()));
			
			NUDGE_ALIGNED(16) float face_penetration_array[4];
			NUDGE_ALIGNED(16) uint32_t face_array[4];
			NUDGE_ALIGNED(16) uint32_t pair_array[4];
			
			simd_st(face_penetration_array, p);
			simd_st((float*)face_array, face);
			simd_st((float*)pair_array, pair);
			
			while (mask) {
				unsigned index = first_set_bit(mask);
				mask &= mask-1;
				
				feature_penetrations[added] = face_penetration_array[index];
				features[added] = face_array[index];
				pairs[added] = pair_array[index];
				
				++added;
			}
		}
		
		// Erase padding.
		while (added && !pairs[added-1])
			--added;
		
		pair_count = added;
	}
	
	// Check if edge pairs are more separating.
	// Do face-face test if not.
	{
		pairs[pair_count+0] = 0; // Padding.
		pairs[pair_count+1] = 0;
		pairs[pair_count+2] = 0;
		
		feature_penetrations[pair_count+0] = 0.0f;
		feature_penetrations[pair_count+1] = 0.0f;
		feature_penetrations[pair_count+2] = 0.0f;
		
		unsigned added = 0;
		
		for (unsigned pair_offset = 0; pair_offset < pair_count; pair_offset += 4) {
			// Load pairs.
			unsigned pair0 = pairs[pair_offset+0];
			unsigned pair1 = pairs[pair_offset+1];
			unsigned pair2 = pairs[pair_offset+2];
			unsigned pair3 = pairs[pair_offset+3];
			
			unsigned a0_index = pair0 & 0xffff;
			unsigned b0_index = pair0 >> 16;
			
			unsigned a1_index = pair1 & 0xffff;
			unsigned b1_index = pair1 >> 16;
			
			unsigned a2_index = pair2 & 0xffff;
			unsigned b2_index = pair2 >> 16;
			
			unsigned a3_index = pair3 & 0xffff;
			unsigned b3_index = pair3 >> 16;
			
			// Load rotations.
			simd4_float a_rotation_x = simd_ld(transforms[a0_index].rotation);
			simd4_float a_rotation_y = simd_ld(transforms[a1_index].rotation);
			simd4_float a_rotation_z = simd_ld(transforms[a2_index].rotation);
			simd4_float a_rotation_s = simd_ld(transforms[a3_index].rotation);
			
			simd4_float b_rotation_x = simd_ld(transforms[b0_index].rotation);
			simd4_float b_rotation_y = simd_ld(transforms[b1_index].rotation);
			simd4_float b_rotation_z = simd_ld(transforms[b2_index].rotation);
			simd4_float b_rotation_s = simd_ld(transforms[b3_index].rotation);
			
			simd_transpose32(a_rotation_x, a_rotation_y, a_rotation_z, a_rotation_s);
			simd_transpose32(b_rotation_x, b_rotation_y, b_rotation_z, b_rotation_s);
			
			// Determine quaternion for rotation from a to b.
			simd4_float t_x, t_y, t_z;
			simd_soa::cross(b_rotation_x, b_rotation_y, b_rotation_z, a_rotation_x, a_rotation_y, a_rotation_z, t_x, t_y, t_z);
			
			simd4_float relative_rotation_x = simd_sub(a_rotation_x * b_rotation_s, simd_madd(b_rotation_x, a_rotation_s, t_x));
			simd4_float relative_rotation_y = simd_sub(a_rotation_y * b_rotation_s, simd_madd(b_rotation_y, a_rotation_s, t_y));
			simd4_float relative_rotation_z = simd_sub(a_rotation_z * b_rotation_s, simd_madd(b_rotation_z, a_rotation_s, t_z));
			simd4_float relative_rotation_s = (a_rotation_x * b_rotation_x +
											   a_rotation_y * b_rotation_y +
											   a_rotation_z * b_rotation_z +
											   a_rotation_s * b_rotation_s);
			
			// Compute the corresponding matrix.
			// Note that the b to a matrix is simply the transpose of a to b.
			simd4_float kx = relative_rotation_x + relative_rotation_x;
			simd4_float ky = relative_rotation_y + relative_rotation_y;
			simd4_float kz = relative_rotation_z + relative_rotation_z;
			
			simd4_float xx = kx * relative_rotation_x;
			simd4_float yy = ky * relative_rotation_y;
			simd4_float zz = kz * relative_rotation_z;
			simd4_float xy = kx * relative_rotation_y;
			simd4_float xz = kx * relative_rotation_z;
			simd4_float yz = ky * relative_rotation_z;
			simd4_float sx = kx * relative_rotation_s;
			simd4_float sy = ky * relative_rotation_s;
			simd4_float sz = kz * relative_rotation_s;
			
			simd4_float one = simd_splat(1.0f);
			
			simd4_float vx_x = simd_sub(one, simd_add(yy, zz));
			simd4_float vx_y = xy + sz;
			simd4_float vx_z = simd_sub(xz, sy);
			
			simd4_float vy_x = simd_sub(xy, sz);
			simd4_float vy_y = simd_sub(one, simd_add(xx, zz));
			simd4_float vy_z = yz + sx;
			
			simd4_float vz_x = xz + sy;
			simd4_float vz_y = simd_sub(yz, sx);
			simd4_float vz_z = simd_sub(one, simd_add(xx, yy));
			
			NUDGE_ALIGNED(16) float a_to_b[4*9];
			
			simd_st(a_to_b + 0, vx_x);
			simd_st(a_to_b + 4, vx_y);
			simd_st(a_to_b + 8, vx_z);
			
			simd_st(a_to_b + 12, vy_x);
			simd_st(a_to_b + 16, vy_y);
			simd_st(a_to_b + 20, vy_z);
			
			simd_st(a_to_b + 24, vz_x);
			simd_st(a_to_b + 28, vz_y);
			simd_st(a_to_b + 32, vz_z);
			
			// Load sizes.
			simd4_float a_size_x = simd_ld(colliders[a0_index].size);
			simd4_float a_size_y = simd_ld(colliders[a1_index].size);
			simd4_float a_size_z = simd_ld(colliders[a2_index].size);
			simd4_float a_size_w = simd_ld(colliders[a3_index].size);
			
			simd4_float b_size_x = simd_ld(colliders[b0_index].size);
			simd4_float b_size_y = simd_ld(colliders[b1_index].size);
			simd4_float b_size_z = simd_ld(colliders[b2_index].size);
			simd4_float b_size_w = simd_ld(colliders[b3_index].size);
			
			simd_transpose32(a_size_x, a_size_y, a_size_z, a_size_w);
			simd_transpose32(b_size_x, b_size_y, b_size_z, b_size_w);
			
			// Load positions.
			simd4_float a_position_x = simd_ld(transforms[a0_index].position);
			simd4_float a_position_y = simd_ld(transforms[a1_index].position);
			simd4_float a_position_z = simd_ld(transforms[a2_index].position);
			simd4_float a_position_w = simd_ld(transforms[a3_index].position);
			
			simd4_float b_position_x = simd_ld(transforms[b0_index].position);
			simd4_float b_position_y = simd_ld(transforms[b1_index].position);
			simd4_float b_position_z = simd_ld(transforms[b2_index].position);
			simd4_float b_position_w = simd_ld(transforms[b3_index].position);
			
			// Compute relative positions and offset the penetrations.
			simd4_float delta_x = simd_sub(a_position_x, b_position_x);
			simd4_float delta_y = simd_sub(a_position_y, b_position_y);
			simd4_float delta_z = simd_sub(a_position_z, b_position_z);
			simd4_float delta_w = simd_sub(a_position_w, b_position_w);
			
			simd_transpose32(delta_x, delta_y, delta_z, delta_w);
			
			simd_soa::cross(delta_x, delta_y, delta_z, a_rotation_x, a_rotation_y, a_rotation_z, t_x, t_y, t_z);
			t_x = simd_add(t_x, t_x);
			t_y = simd_add(t_y, t_y);
			t_z = simd_add(t_z, t_z);

			simd4_float u_x, u_y, u_z;
			simd_soa::cross(a_rotation_x, a_rotation_y, a_rotation_z, t_x, t_y, t_z, u_x, u_y, u_z);
			
			simd4_float b_offset_x = simd_sub(u_x, simd_madd(a_rotation_s, t_x, delta_x));
			simd4_float b_offset_y = simd_sub(u_y, simd_madd(a_rotation_s, t_y, delta_y));
			simd4_float b_offset_z = simd_sub(u_z, simd_madd(a_rotation_s, t_z, delta_z));
			
			NUDGE_ALIGNED(16) float b_offset_array[3*4];
			
			simd_st(b_offset_array + 0, b_offset_x);
			simd_st(b_offset_array + 4, b_offset_y);
			simd_st(b_offset_array + 8, b_offset_z);
			
			simd4_float face_penetration = simd_ld(feature_penetrations + pair_offset);
			
			// Is an edge pair more separating?
			NUDGE_ALIGNED(16) float edge_penetration_a[4*9];
			NUDGE_ALIGNED(16) float edge_penetration_b[4*9];
			
			for (unsigned i = 0; i < 3; ++i) {
				simd4_float acx = simd_ld(a_to_b + (0*3 + i)*4);
				simd4_float acy = simd_ld(a_to_b + (1*3 + i)*4);
				simd4_float acz = simd_ld(a_to_b + (2*3 + i)*4);
				
				simd4_float bcx = simd_ld(a_to_b + (i*3 + 0)*4);
				simd4_float bcy = simd_ld(a_to_b + (i*3 + 1)*4);
				simd4_float bcz = simd_ld(a_to_b + (i*3 + 2)*4);
				
				simd4_float ac2x = acx*acx;
				simd4_float ac2y = acy*acy;
				simd4_float ac2z = acz*acz;
				
				simd4_float bc2x = bcx*bcx;
				simd4_float bc2y = bcy*bcy;
				simd4_float bc2z = bcz*bcz;
				
				simd4_float aacx = simd_abs(acx);
				simd4_float aacy = simd_abs(acy);
				simd4_float aacz = simd_abs(acz);
				
				simd4_float abcx = simd_abs(bcx);
				simd4_float abcy = simd_abs(bcy);
				simd4_float abcz = simd_abs(bcz);
				
				simd4_float r_a0 = ac2y + ac2z;
				simd4_float r_a1 = ac2z + ac2x;
				simd4_float r_a2 = ac2x + ac2y;
				
				simd4_float r_b0 = bc2y + bc2z;
				simd4_float r_b1 = bc2z + bc2x;
				simd4_float r_b2 = bc2x + bc2y;
				
				simd4_float nan_threshold = simd_splat(1e-3f);
				
				r_a0 = simd_or(simd_rsqrt_est(r_a0), simd_cmple(r_a0, nan_threshold));
				r_a1 = simd_or(simd_rsqrt_est(r_a1), simd_cmple(r_a1, nan_threshold));
				r_a2 = simd_or(simd_rsqrt_est(r_a2), simd_cmple(r_a2, nan_threshold));
				
				r_b0 = simd_or(simd_rsqrt_est(r_b0), simd_cmple(r_b0, nan_threshold));
				r_b1 = simd_or(simd_rsqrt_est(r_b1), simd_cmple(r_b1, nan_threshold));
				r_b2 = simd_or(simd_rsqrt_est(r_b2), simd_cmple(r_b2, nan_threshold));
				
				simd4_float pa0 = aacy*a_size_z + aacz*a_size_y;
				simd4_float pa1 = aacz*a_size_x + aacx*a_size_z;
				simd4_float pa2 = aacx*a_size_y + aacy*a_size_x;
				
				simd4_float pb0 = abcy*b_size_z + abcz*b_size_y;
				simd4_float pb1 = abcz*b_size_x + abcx*b_size_z;
				simd4_float pb2 = abcx*b_size_y + abcy*b_size_x;
				
				simd4_float o0 = simd_abs(simd_sub(acy*b_offset_z, acz*b_offset_y));
				simd4_float o1 = simd_abs(simd_sub(acz*b_offset_x, acx*b_offset_z));
				simd4_float o2 = simd_abs(simd_sub(acx*b_offset_y, acy*b_offset_x));
				
				simd_st(edge_penetration_a + (i*3 + 0)*4, simd_sub(pa0, o0) * r_a0);
				simd_st(edge_penetration_a + (i*3 + 1)*4, simd_sub(pa1, o1) * r_a1);
				simd_st(edge_penetration_a + (i*3 + 2)*4, simd_sub(pa2, o2) * r_a2);
				
				simd_st(edge_penetration_b + (i*3 + 0)*4, pb0 * r_b0);
				simd_st(edge_penetration_b + (i*3 + 1)*4, pb1 * r_b1);
				simd_st(edge_penetration_b + (i*3 + 2)*4, pb2 * r_b2);
			}
			
			simd4_int32 a_edge = simd_isplat(0);
			simd4_int32 b_edge = simd_isplat(0);
			
			simd4_float penetration = face_penetration;
			
			for (unsigned i = 0; i < 3; ++i) {
				for (unsigned j = 0; j < 3; ++j) {
					simd4_float p = simd_ld(edge_penetration_a + (i*3 + j)*4) + simd_ld(edge_penetration_b + (j*3 + i)*4);
					
					simd4_float mask = simd_cmpgt(penetration, p);
					
					penetration = simd_float::min_second_nan(penetration, p); // Note: First operand is returned on NaN.
					a_edge = simd::blendv32(a_edge, simd_isplat(j), mask);
					b_edge = simd::blendv32(b_edge, simd_isplat(i), mask);
				}
			}
			
			simd4_float face_bias = simd_splat(1e-3f);
			
			unsigned edge = simd::cmpmask32(simd_cmpgt(face_penetration, penetration + face_bias));
			unsigned overlapping = simd::cmpmask32(simd_cmpgt(penetration, simd_zero()));
			
			unsigned face = ~edge;
			
			edge &= overlapping;
			face &= overlapping;
			
			NUDGE_ALIGNED(16) float penetration_array[4];
			NUDGE_ALIGNED(16) int32_t a_edge_array[4];
			NUDGE_ALIGNED(16) int32_t b_edge_array[4];
			
			simd_st(penetration_array, penetration);
			simd_st(a_edge_array, a_edge);
			simd_st(b_edge_array, b_edge);
			
			// Do face-face tests.
			while (face) {
				unsigned index = first_set_bit(face);
				face &= face-1;
				
				unsigned pair = pairs[pair_offset + index];
				unsigned a_face = features[pair_offset + index];
				
				unsigned a_index = pair & 0xffff;
				unsigned b_index = pair >> 16;
				
				// Gather.
				simd4_float dirs = simd_ld(a_to_b[(a_face*3 + 0)*4 + index],
										   a_to_b[(a_face*3 + 1)*4 + index],
										   a_to_b[(a_face*3 + 2)*4 + index],
										   0.0f);
				
				simd4_float c0 = simd_ld(a_to_b[(0*3 + 0)*4 + index],
										 a_to_b[(1*3 + 0)*4 + index],
										 a_to_b[(2*3 + 0)*4 + index],
										 0.0f);
				
				simd4_float c1 = simd_ld(a_to_b[(0*3 + 1)*4 + index],
										 a_to_b[(1*3 + 1)*4 + index],
										 a_to_b[(2*3 + 1)*4 + index],
										 0.0f);
				
				simd4_float c2 = simd_ld(a_to_b[(0*3 + 2)*4 + index],
										 a_to_b[(1*3 + 2)*4 + index],
										 a_to_b[(2*3 + 2)*4 + index],
										 0.0f);
				
				simd4_float b_offset = simd_ld(b_offset_array[0*4 + index],
											   b_offset_array[1*4 + index],
											   b_offset_array[2*4 + index],
											   0.0f);
				
				// Load sizes.
				simd4_float a_size = simd_ld(colliders[a_index].size);
				simd4_float b_size = simd_ld(colliders[b_index].size);
				
				// Find most aligned face of b.
				dirs = simd_abs(dirs);
				
				simd4_float max_dir = simd_max(simd_swiz_xzyw(dirs), simd_swiz_xxxx(dirs));
				
				unsigned dir_mask = simd::cmpmask32(simd_cmpge(dirs, max_dir));
				
				// Compute the coordinates of the two quad faces.
				c0 = simd_mul(c0, simd_swiz_xxxx(b_size));
				c1 = simd_mul(c1, simd_swiz_yyyy(b_size));
				c2 = simd_mul(c2, simd_swiz_zzzz(b_size));
				
				unsigned b_face = 0;
				
				if (dir_mask & 4) {
					simd4_float t = c0;
					c0 = c2;
					c2 = c1;
					c1 = t;
					b_face = 2;
				}
				else if (dir_mask & 2) {
					simd4_float t = c0;
					c0 = c1;
					c1 = c2;
					c2 = t;
					b_face = 1;
				}
				
				simd4_float c = c0;
				simd4_float dx = c1;
				simd4_float dy = c2;
				
				unsigned b_positive_face_bit = simd::signmask32(simd_xor(b_offset, c)) & (1 << a_face);
				unsigned b_offset_neg = simd::signmask32(b_offset) & (1 << a_face);
				
				if (!b_positive_face_bit)
					c = simd_neg(c);
				
				c = simd_add(c, b_offset);
				
				// Quad coordinate packing:
				// Size of quad a, center of quad b, x-axis of quad b, y-axis of quad b.
				// a.size.x, c.x, dx.x, dy.x
				// a.size.y, c.y, dx.y, dy.y
				// a.size.z, c.z, dx.z, dy.z
				NUDGE_ALIGNED(16) float quads[4*3];
				
				simd4_float q0 = simd_shuf_xAyB(a_size, c);
				simd4_float q1 = simd_shuf_zCwD(a_size, c);
				simd4_float q2 = simd_shuf_xAyB(dx, dy);
				simd4_float q3 = simd_shuf_zCwD(dx, dy);
				
				simd_st(quads + 0, simd_shuf_xyAB(q0, q2));
				simd_st(quads + 4, simd_shuf_zwCD(q0, q2));
				simd_st(quads + 8, simd_shuf_xyAB(q1, q3));
				
				// Transform so that overlap testing can be done in two dimensions.
				const float* transformed_x = quads + 4*((a_face+1) % 3);
				const float* transformed_y = quads + 4*((a_face+2) % 3);
				const float* transformed_z = quads + 4*a_face;
				
				// Find support points for the overlap between the quad faces in two dimensions.
				NUDGE_ALIGNED(32) float support[16*3];
				NUDGE_ALIGNED(32) uint32_t support_tags[16];
				unsigned mask; // Indicates valid points.
				{
					float* support_x = support + 0;
					float* support_y = support + 16;
					
					simd4_float tx = simd_ld(transformed_x);
					simd4_float ty = simd_ld(transformed_y);
					
					simd4_float sxycxy = simd_shuf_xAyB(tx, ty);
					simd4_float dxy = simd_shuf_zCwD(tx, ty);
					
					simd4_float sx = simd_swiz_xxxx(sxycxy);
					simd4_float sy = simd_swiz_yyyy(sxycxy);
					simd4_float cx = simd_swiz_zzzz(sxycxy);
					simd4_float cy = simd_swiz_wwww(sxycxy);
					
					simd4_float sign_npnp = simd_ld(-0.0f, 0.0f, -0.0f, 0.0f);
					
					// Add corner points to the support if they are part of the intersection.
					simd128_t corner_mask;
					simd128_t edge_mask;
					{
						simd4_float sign_pnpn = simd_ld(0.0f, -0.0f, 0.0f, -0.0f);
						simd4_float sign_nnpp = simd_ld(-0.0f, -0.0f, 0.0f, 0.0f);
						
						simd4_float corner0x = simd_xor(sx, sign_pnpn);
						simd4_float corner0y = simd_xor(sy, sign_nnpp);
						
						simd4_float corner1x = cx + simd_xor(simd_swiz_xxxx(dxy), sign_npnp) + simd_xor(simd_swiz_zzzz(dxy), sign_nnpp);
						simd4_float corner1y = cy + simd_xor(simd_swiz_yyyy(dxy), sign_npnp) + simd_xor(simd_swiz_wwww(dxy), sign_nnpp);
						
						simd4_float k = (simd_sub( simd_swiz_zzAA(sxycxy, dxy) * simd_swiz_wywy(dxy),
												   simd_swiz_wwBB(sxycxy, dxy) * simd_swiz_zxzx(dxy)));
						
						simd4_float ox = simd_swiz_xxxx(k);
						simd4_float oy = simd_swiz_yyyy(k);
						simd4_float delta_max = simd_abs(simd_swiz_zzzz(k));
						
						simd4_float sdxy = dxy * simd_swiz_yxyx(sxycxy);
						
						simd4_float delta_x = ox + simd_xor(simd_swiz_zzzz(sdxy), sign_nnpp) + simd_xor(simd_swiz_wwww(sdxy), sign_npnp);
						simd4_float delta_y = oy + simd_xor(simd_swiz_xxxx(sdxy), sign_nnpp) + simd_xor(simd_swiz_yyyy(sdxy), sign_npnp);
						
						simd4_float inside_x = simd_cmple(simd_abs(corner1x), sx);
						simd4_float inside_y = simd_cmple(simd_abs(corner1y), sy);
						
						simd4_float mask0 = simd_cmple(simd_max(simd_abs(delta_x), simd_abs(delta_y)), delta_max);
						simd4_float mask1 = simd_and(inside_x, inside_y);
						
						corner_mask = simd_pack_i32_to_i16(mask0, mask1);
						
						// Don't allow edge intersections if both vertices are inside.
						edge_mask = simd_pack_i32_to_i16(simd_and(simd_swiz_wzxz(mask0), simd_swiz_yxyw(mask0)),
														 simd_and(simd_swiz_ywzw(mask1), simd_swiz_xzxy(mask1)));
						
						simd_st(support_x + 0, corner0x);
						simd_st(support_y + 0, corner0y);
						simd_st(support_x + 4, corner1x);
						simd_st(support_y + 4, corner1y);
					}
					
					// Find additional support points by intersecting the edges of the second quad against the bounds of the first.
					unsigned edge_axis_near;
					unsigned edge_axis_far;
					{
						simd4_float one = simd_splat(1.0f);
						simd4_float rdxy = simd_div(one, dxy);
						
						simd4_float offset_x = simd_swiz_xxzz(dxy);
						simd4_float offset_y = simd_swiz_yyww(dxy);
						
						simd4_float pivot_x = cx + simd_xor(simd_swiz_zzxx(dxy), sign_npnp);
						simd4_float pivot_y = cy + simd_xor(simd_swiz_wwyy(dxy), sign_npnp);
						
						simd4_float sign_mask = simd_splat(-0.0f);
						simd4_float pos_x = simd_or(simd_and(offset_x, sign_mask), sx); // Copy sign.
						simd4_float pos_y = simd_or(simd_and(offset_y, sign_mask), sy);
						
						simd4_float rx = simd_swiz_xxzz(rdxy);
						simd4_float ry = simd_swiz_yyww(rdxy);
						
						simd4_float near_x = (pos_x + pivot_x) * rx;
						simd4_float far_x = simd_sub(pos_x, pivot_x) * rx;
						
						simd4_float near_y = (pos_y + pivot_y) * ry;
						simd4_float far_y = simd_sub(pos_y, pivot_y) * ry;
						
						simd4_float a = simd_float::min_second_nan(one, near_x); // First operand is returned on NaN.
						simd4_float b = simd_float::min_second_nan(one, far_x);
						
						edge_axis_near = simd::cmpmask32(simd_cmpgt(a, near_y));
						edge_axis_far = simd::cmpmask32(simd_cmpgt(b, far_y));
						
						a = simd_min(a, near_y);
						b = simd_min(b, far_y);
						
						simd4_float ax = simd_sub(pivot_x, offset_x * a);
						simd4_float ay = simd_sub(pivot_y, offset_y * a);
						simd4_float bx = simd_sub(pivot_x, offset_x * b);
						simd4_float by = simd_sub(pivot_y, offset_y * b);
						
						simd4_float mask = simd_cmpgt(a + b, simd_zero()); // Make sure -a < b.
						
						simd4_float mask_a = simd_cmpneq(a, one);
						simd4_float mask_b = simd_cmpneq(b, one);
						
						mask_a = simd_and(mask_a, mask);
						mask_b = simd_and(mask_b, mask);
						
						edge_mask = simd_and(simd_not(edge_mask), simd_pack_i32_to_i16(mask_a, mask_b));
						
						simd_st(support_x + 8, ax);
						simd_st(support_y + 8, ay);
						simd_st(support_x + 12, bx);
						simd_st(support_y + 12, by);
					}
					
					mask = simd_i8_mask(simd_pack_i16_to_i8(corner_mask, edge_mask));
					
					// Calculate and store vertex labels.
					// The 8 vertices are tagged using the sign bit of each axis.
					// Bit rotation is used to "transform" the coordinates.
					unsigned a_sign_face_bit = b_offset_neg ? (1 << a_face) : 0;
					unsigned b_sign_face_bit = b_positive_face_bit ? 0 : (1 << b_face);
					
					unsigned a_vertices = 0x12003624 >> (3 - a_face); // Rotates all vertices in parallel.
					unsigned b_vertices = 0x00122436 >> (3 - b_face);
					
					unsigned a_face_bits = 0xffff0000 | a_sign_face_bit;
					unsigned b_face_bits = 0x0000ffff | (b_sign_face_bit << 16);
					
					support_tags[0] = ((a_vertices >>  0) & 0x7) | a_face_bits;
					support_tags[1] = ((a_vertices >>  8) & 0x7) | a_face_bits;
					support_tags[2] = ((a_vertices >> 16) & 0x7) | a_face_bits;
					support_tags[3] = ((a_vertices >> 24) & 0x7) | a_face_bits;
					
					support_tags[4] = ((b_vertices << 16) & 0x70000) | b_face_bits;
					support_tags[5] = ((b_vertices <<  8) & 0x70000) | b_face_bits;
					support_tags[6] = ((b_vertices >>  0) & 0x70000) | b_face_bits;
					support_tags[7] = ((b_vertices >>  8) & 0x70000) | b_face_bits;
					
					// Calculate edge numbers in the local coordinate frame.
					unsigned edge_axis_winding = simd::signmask32(dxy);
					
					unsigned y_near0 = (edge_axis_near >> 0) & 1;
					unsigned y_near1 = (edge_axis_near >> 1) & 1;
					unsigned y_near2 = (edge_axis_near >> 2) & 1;
					unsigned y_near3 = (edge_axis_near >> 3) & 1;
					
					unsigned y_far0 = (edge_axis_far >> 0) & 1;
					unsigned y_far1 = (edge_axis_far >> 1) & 1;
					unsigned y_far2 = (edge_axis_far >> 2) & 1;
					unsigned y_far3 = (edge_axis_far >> 3) & 1;
					
					unsigned a_near_edge0 = y_near0*2 + ((edge_axis_winding >> (0 + y_near0)) & 1);
					unsigned a_near_edge1 = y_near1*2 + ((edge_axis_winding >> (0 + y_near1)) & 1);
					unsigned a_near_edge2 = y_near2*2 + ((edge_axis_winding >> (2 + y_near2)) & 1);
					unsigned a_near_edge3 = y_near3*2 + ((edge_axis_winding >> (2 + y_near3)) & 1);
					
					edge_axis_winding ^= 0xf;
					
					unsigned a_far_edge0 = y_far0*2 + ((edge_axis_winding >> (0 + y_far0)) & 1);
					unsigned a_far_edge1 = y_far1*2 + ((edge_axis_winding >> (0 + y_far1)) & 1);
					unsigned a_far_edge2 = y_far2*2 + ((edge_axis_winding >> (2 + y_far2)) & 1);
					unsigned a_far_edge3 = y_far3*2 + ((edge_axis_winding >> (2 + y_far3)) & 1);
					
					// Map local edges to labels (so that faces can share an edge).
					// The 12 edges are tagged using two ordered points.
					// We use the same trick as the vertex transform but do it for pairs of vertices (in correct order).
					uint64_t a_edge_map = 0x1200362424003612llu >> (3 - a_face);
					uint64_t b_edge_map = 0x2400361212003624llu >> (3 - b_face);
					
					unsigned face_bits = a_sign_face_bit | (a_sign_face_bit << 8) | (b_sign_face_bit << 16) | (b_sign_face_bit << 24);
					
					unsigned b_edge0 = ((unsigned)((b_edge_map >> (0<<4)) & 0x0707) << 16) | face_bits;
					unsigned b_edge1 = ((unsigned)((b_edge_map >> (1<<4)) & 0x0707) << 16) | face_bits;
					unsigned b_edge2 = ((unsigned)((b_edge_map >> (2<<4)) & 0x0707) << 16) | face_bits;
					unsigned b_edge3 = ((unsigned)((b_edge_map >> (3<<4)) & 0x0707) << 16) | face_bits;
					
					support_tags[ 8] = (unsigned)((a_edge_map >> (a_near_edge0<<4)) & 0x0707) | b_edge0;
					support_tags[ 9] = (unsigned)((a_edge_map >> (a_near_edge1<<4)) & 0x0707) | b_edge1;
					support_tags[10] = (unsigned)((a_edge_map >> (a_near_edge2<<4)) & 0x0707) | b_edge2;
					support_tags[11] = (unsigned)((a_edge_map >> (a_near_edge3<<4)) & 0x0707) | b_edge3;
					
					support_tags[12] = (unsigned)((a_edge_map >> (a_far_edge0<<4)) & 0x0707) | b_edge0;
					support_tags[13] = (unsigned)((a_edge_map >> (a_far_edge1<<4)) & 0x0707) | b_edge1;
					support_tags[14] = (unsigned)((a_edge_map >> (a_far_edge2<<4)) & 0x0707) | b_edge2;
					support_tags[15] = (unsigned)((a_edge_map >> (a_far_edge3<<4)) & 0x0707) | b_edge3;
				}
				
				// Compute z-plane through face b and calculate z for the support points.
				simd4_float a_size_transformed = simd_ld(transformed_x);
				simd4_float c_transformed = simd_ld(transformed_y);
				simd4_float dx_transformed = simd_ld(transformed_z);
				simd4_float dy_transformed = simd_zero();
				
				simd_transpose32(a_size_transformed, c_transformed, dx_transformed, dy_transformed);
				
				simd4_float zn = simd_aos::cross(dx_transformed, dy_transformed);
				simd4_float plane = simd_shuf_xyAB(simd_xor(zn, simd_splat(-0.0f)), simd_aos::dot(c_transformed, zn));
				plane = simd_mul(plane, simd_div(simd_splat(1.0f) , simd_swiz_zzzz(zn)));
				
				NUDGE_ALIGNED(32) float penetrations[16];
				
				simd4_float z_sign = simd_zero();
				
				if (b_offset_neg)
					z_sign = simd_splat(-0.0f);
				
				simd4_float penetration_offset = simd_swiz_zzzz(a_size_transformed);
				unsigned penetration_mask = 0;
				
				for (unsigned i = 0; i < 16; i += simdv_width32) {
					simd4_float x = simd_ld(support + 0 + i);
					simd4_float y = simd_ld(support + 16 + i);
					simd4_float z = x*simd_swiz_xxxx(plane) + y*simd_swiz_yyyy(plane) + simd_swiz_zzzz(plane);
					
					simd4_float penetration = simd_sub(penetration_offset, simd_xor(z, z_sign));
					
					z = simd_add(z, penetration * simd_xor(simd_splat(0.5f), z_sign));
					
					penetration_mask |= simd::cmpmask32(simd_cmpgt(penetration, simd_zero())) << i;
					
					simd_st(penetrations + i, penetration);
					simd_st(support + 32 + i, z);
				}
				
				mask &= penetration_mask;
				
				// Inverse transform.
				unsigned a_face_inverse = (a_face ^ 1) ^ (a_face >> 1);
				
				const float* support_x = support + 16*((a_face_inverse+1) % 3);
				const float* support_y = support + 16*((a_face_inverse+2) % 3);
				const float* support_z = support + 16*a_face_inverse;
				
				// Setup rotation matrix from a to world.
				simd4_float a_to_world0, a_to_world1, a_to_world2;
				{
					simd4_float qx_qy_qz_qs = simd_ld(transforms[a_index].rotation);
					simd4_float kx_ky_kz_ks = qx_qy_qz_qs + qx_qy_qz_qs;
					
					// Make ks negative so that we can create +sx from kx*qs and -sx from ks*qx.
					kx_ky_kz_ks = simd_xor(kx_ky_kz_ks, simd_ld(0.0f, 0.0f, 0.0f, -0.0f));
					
					//  1.0f - yy - zz, xy + sz, xz - sy
					a_to_world0 = (simd_swiz_yxxw(kx_ky_kz_ks) * simd_swiz_yyzw(qx_qy_qz_qs) +
								   simd_swiz_zzww(kx_ky_kz_ks) * simd_swiz_zwyw(qx_qy_qz_qs));
					
					// xy - sz, 1.0f - zz - xx, yz + sx
					a_to_world1 = (simd_swiz_xzyw(kx_ky_kz_ks) * simd_swiz_yzzw(qx_qy_qz_qs) +
								   simd_swiz_wxxw(kx_ky_kz_ks) * simd_swiz_zxww(qx_qy_qz_qs));
					
					// xz + sy, yz - sx, 1.0f - xx - yy
					a_to_world2 = (simd_swiz_xyxw(kx_ky_kz_ks) * simd_swiz_zzxw(qx_qy_qz_qs) +
								   simd_swiz_ywyw(kx_ky_kz_ks) * simd_swiz_wxyw(qx_qy_qz_qs));
					
					a_to_world0 = simd_sub(a_to_world0, simd_ld(1.0f, 0.0f, 0.0f, 0.0f));
					a_to_world1 = simd_sub(a_to_world1, simd_ld(0.0f, 1.0f, 0.0f, 0.0f));
					a_to_world2 = simd_sub(a_to_world2, simd_ld(0.0f, 0.0f, 1.0f, 0.0f));
					
					a_to_world0 = simd_xor(a_to_world0, simd_ld(-0.0f, 0.0f, 0.0f, 0.0f));
					a_to_world1 = simd_xor(a_to_world1, simd_ld(0.0f, -0.0f, 0.0f, 0.0f));
					a_to_world2 = simd_xor(a_to_world2, simd_ld(0.0f, 0.0f, -0.0f, 0.0f));
				}
				
				// Add valid support points as contacts.
				simd4_float wn = a_face == 0 ? a_to_world0 : (a_face == 1 ? a_to_world1 : a_to_world2);
				
				if (b_offset_neg)
					wn = simd_xor(wn, simd_splat(-0.0f));
				
				simd4_float a_position = simd_ld(transforms[a_index].position);
				
				uint16_t a_body = (uint16_t)transforms[a_index].body;
				uint16_t b_body = (uint16_t)transforms[b_index].body;
				
				a_index = transforms[a_index].body >> 16;
				b_index = transforms[b_index].body >> 16;
				
				unsigned tag_swap = 0;
				
				if (b_index > a_index) {
					unsigned tc = a_index;
					uint16_t tb = a_body;
					
					a_index = b_index;
					b_index = tc;
					
					a_body = b_body;
					b_body = tb;
					
					tag_swap = 16;
					
					wn = simd_xor(wn, simd_splat(-0.0f));
				}
				
				uint64_t high_tag = ((uint64_t)a_index << 32) | ((uint64_t)b_index << 48);
				
				while (mask) {
					unsigned index = first_set_bit(mask);
					mask &= mask-1;
					
					simd4_float wp = (a_to_world0 * simd_splat(support_x + index) +
									  a_to_world1 * simd_splat(support_y + index) +
									  a_to_world2 * simd_splat(support_z + index) + a_position);

					float penetration = penetrations[index];
					
					simd_st(contacts[count].position, wp);
					simd_st(contacts[count].normal, wn);
					
					contacts[count].penetration = penetration;
					contacts[count].friction = 0.5f;
					bodies[count].a = a_body;
					bodies[count].b = b_body;
					tags[count] = (uint32_t)(support_tags[index] >> tag_swap) | (uint32_t)(support_tags[index] << tag_swap) | high_tag;
					
					++count;
				}
			}
			
			// Batch edge pairs.
			// Note: We need to output the edge pairs after handling the faces since we read from the pairs array during face processing.
			while (edge) {
				unsigned index = first_set_bit(edge);
				edge &= edge-1;
				
				unsigned pair = pairs[pair_offset + index];
				unsigned edge_a = a_edge_array[index];
				unsigned edge_b = b_edge_array[index];
				
				unsigned a = pair & 0xffff;
				unsigned b = pair >> 16;
				
				a = transforms[a].body >> 16;
				b = transforms[b].body >> 16;
				
				feature_penetrations[added] = penetration_array[index];
				features[added] = a > b ? edge_a | (edge_b << 16) : edge_b | (edge_a << 16);
				pairs[added] = a > b ? pair : (pair >> 16) | (pair << 16);
				
				++added;
			}
		}
		
		assert(!added || pairs[added-1]); // There should be no padding.
		
		pair_count = added;
	}
	
	// Do edge-edge tests.
	{
		pairs[pair_count+0] = 0; // Padding.
		pairs[pair_count+1] = 0;
		pairs[pair_count+2] = 0;
		
		features[pair_count+0] = 0;
		features[pair_count+1] = 0;
		features[pair_count+2] = 0;
		
		feature_penetrations[pair_count+0] = 0.0f;
		feature_penetrations[pair_count+1] = 0.0f;
		feature_penetrations[pair_count+2] = 0.0f;
		
		for (unsigned i = 0; i < pair_count; i += 4) {
			// Load pairs.
			unsigned pair0 = pairs[i + 0];
			unsigned pair1 = pairs[i + 1];
			unsigned pair2 = pairs[i + 2];
			unsigned pair3 = pairs[i + 3];
			
			unsigned a0_index = pair0 & 0xffff;
			unsigned b0_index = pair0 >> 16;
			
			unsigned a1_index = pair1 & 0xffff;
			unsigned b1_index = pair1 >> 16;
			
			unsigned a2_index = pair2 & 0xffff;
			unsigned b2_index = pair2 >> 16;
			
			unsigned a3_index = pair3 & 0xffff;
			unsigned b3_index = pair3 >> 16;
			
			// Load rotations.
			simd4_float a_rotation_x = simd_ld(transforms[a0_index].rotation);
			simd4_float a_rotation_y = simd_ld(transforms[a1_index].rotation);
			simd4_float a_rotation_z = simd_ld(transforms[a2_index].rotation);
			simd4_float a_rotation_s = simd_ld(transforms[a3_index].rotation);
			
			simd4_float b_rotation_x = simd_ld(transforms[b0_index].rotation);
			simd4_float b_rotation_y = simd_ld(transforms[b1_index].rotation);
			simd4_float b_rotation_z = simd_ld(transforms[b2_index].rotation);
			simd4_float b_rotation_s = simd_ld(transforms[b3_index].rotation);
			
			simd_transpose32(a_rotation_x, a_rotation_y, a_rotation_z, a_rotation_s);
			simd_transpose32(b_rotation_x, b_rotation_y, b_rotation_z, b_rotation_s);
			
			// Compute rotation matrices.
			simd4_float a_basis_xx, a_basis_xy, a_basis_xz;
			simd4_float a_basis_yx, a_basis_yy, a_basis_yz;
			simd4_float a_basis_zx, a_basis_zy, a_basis_zz;
			{
				simd4_float kx = a_rotation_x + a_rotation_x;
				simd4_float ky = a_rotation_y + a_rotation_y;
				simd4_float kz = a_rotation_z + a_rotation_z;
				
				simd4_float xx = kx*a_rotation_x;
				simd4_float yy = ky*a_rotation_y;
				simd4_float zz = kz*a_rotation_z;
				simd4_float xy = kx*a_rotation_y;
				simd4_float xz = kx*a_rotation_z;
				simd4_float yz = ky*a_rotation_z;
				simd4_float sx = kx*a_rotation_s;
				simd4_float sy = ky*a_rotation_s;
				simd4_float sz = kz*a_rotation_s;
				
				simd4_float one = simd_splat(1.0f);

				a_basis_xx = simd_sub(one, simd_add(yy, zz));
				a_basis_xy = xy + sz;
				a_basis_xz = simd_sub(xz, sy);
				
				a_basis_yx = simd_sub(xy, sz);
				a_basis_yy = simd_sub(one, simd_add(xx, zz));
				a_basis_yz = yz + sx;
				
				a_basis_zx = xz + sy;
				a_basis_zy = simd_sub(yz, sx);
				a_basis_zz = simd_sub(one, simd_add(xx, yy));
			}
			
			simd4_float b_basis_xx, b_basis_xy, b_basis_xz;
			simd4_float b_basis_yx, b_basis_yy, b_basis_yz;
			simd4_float b_basis_zx, b_basis_zy, b_basis_zz;
			{
				simd4_float kx = b_rotation_x + b_rotation_x;
				simd4_float ky = b_rotation_y + b_rotation_y;
				simd4_float kz = b_rotation_z + b_rotation_z;
				
				simd4_float xx = kx*b_rotation_x;
				simd4_float yy = ky*b_rotation_y;
				simd4_float zz = kz*b_rotation_z;
				simd4_float xy = kx*b_rotation_y;
				simd4_float xz = kx*b_rotation_z;
				simd4_float yz = ky*b_rotation_z;
				simd4_float sx = kx*b_rotation_s;
				simd4_float sy = ky*b_rotation_s;
				simd4_float sz = kz*b_rotation_s;
				
				b_basis_xx = simd_splat(1.0f) - yy - zz;
				b_basis_xy = xy + sz;
				b_basis_xz = xz - sy;
				
				b_basis_yx = xy - sz;
				b_basis_yy = simd_splat(1.0f) - xx - zz;
				b_basis_yz = yz + sx;
				
				b_basis_zx = xz + sy;
				b_basis_zy = yz - sx;
				b_basis_zz = simd_splat(1.0f) - xx - yy;
			}
			
			// Load edges.
			simd4_int32 edge = simd_ld((const int32_t*)(features + i));
			
			// Select edge directions.
#ifdef NUDGE_NATIVE_BLENDV32
			simd4_int32 a_select_y = simd_sll(edge, 32-1); // Shifts the relevant bit to the top.
			simd4_int32 a_select_z = simd_sll(edge, 32-2);
			
			simd4_int32 b_select_y = simd_sll(edge, 16-1);
			simd4_int32 b_select_z = simd_sll(edge, 16-2);
			
			simd4_float u_x = simd::blendv32(a_basis_xx, a_basis_yx, a_select_y);
			simd4_float u_y = simd::blendv32(a_basis_xy, a_basis_yy, a_select_y);
			simd4_float u_z = simd::blendv32(a_basis_xz, a_basis_yz, a_select_y);
			
			simd4_float v_x = simd::blendv32(b_basis_xx, b_basis_yx, b_select_y);
			simd4_float v_y = simd::blendv32(b_basis_xy, b_basis_yy, b_select_y);
			simd4_float v_z = simd::blendv32(b_basis_xz, b_basis_yz, b_select_y);
			
			u_x = simd::blendv32(u_x, a_basis_zx, a_select_z);
			u_y = simd::blendv32(u_y, a_basis_zy, a_select_z);
			u_z = simd::blendv32(u_z, a_basis_zz, a_select_z);
			
			v_x = simd::blendv32(v_x, b_basis_zx, b_select_z);
			v_y = simd::blendv32(v_y, b_basis_zy, b_select_z);
			v_z = simd::blendv32(v_z, b_basis_zz, b_select_z);
#else
			simd4_int32 a_edge = simd_and(edge, simd_isplat(0xffff));
			simd4_int32 b_edge = simd_srl(edge, 16);
			
			simd4_float a_select_x = simd_icmpeq(a_edge, simd_zero());
			simd4_float a_select_y = simd_icmpeq(a_edge, simd_isplat(1));
			simd4_float a_select_z = simd_icmpeq(a_edge, simd_isplat(2));
			
			simd4_float b_select_x = simd_icmpeq(b_edge, simd_zero());
			simd4_float b_select_y = simd_icmpeq(b_edge, simd_isplat(1));
			simd4_float b_select_z = simd_icmpeq(b_edge, simd_isplat(2));
			
			simd4_float u_x = simd_and(a_basis_xx, a_select_x);
			simd4_float u_y = simd_and(a_basis_xy, a_select_x);
			simd4_float u_z = simd_and(a_basis_xz, a_select_x);
			
			simd4_float v_x = simd_and(b_basis_xx, b_select_x);
			simd4_float v_y = simd_and(b_basis_xy, b_select_x);
			simd4_float v_z = simd_and(b_basis_xz, b_select_x);
			
			u_x = simd_or(u_x, simd_and(a_basis_yx, a_select_y));
			u_y = simd_or(u_y, simd_and(a_basis_yy, a_select_y));
			u_z = simd_or(u_z, simd_and(a_basis_yz, a_select_y));
			
			v_x = simd_or(v_x, simd_and(b_basis_yx, b_select_y));
			v_y = simd_or(v_y, simd_and(b_basis_yy, b_select_y));
			v_z = simd_or(v_z, simd_and(b_basis_yz, b_select_y));
			
			u_x = simd_or(u_x, simd_and(a_basis_zx, a_select_z));
			u_y = simd_or(u_y, simd_and(a_basis_zy, a_select_z));
			u_z = simd_or(u_z, simd_and(a_basis_zz, a_select_z));
			
			v_x = simd_or(v_x, simd_and(b_basis_zx, b_select_z));
			v_y = simd_or(v_y, simd_and(b_basis_zy, b_select_z));
			v_z = simd_or(v_z, simd_and(b_basis_zz, b_select_z));
#endif
			
			// Compute axis.
			simd4_float n_x, n_y, n_z;
			simd_soa::cross(u_x, u_y, u_z, v_x, v_y, v_z, n_x, n_y, n_z);
			
			// Load positions.
			simd4_float a_position_x = simd_ld(transforms[a0_index].position);
			simd4_float a_position_y = simd_ld(transforms[a1_index].position);
			simd4_float a_position_z = simd_ld(transforms[a2_index].position);
			simd4_float a_position_w = simd_ld(transforms[a3_index].position);
			
			simd4_float b_position_x = simd_ld(transforms[b0_index].position);
			simd4_float b_position_y = simd_ld(transforms[b1_index].position);
			simd4_float b_position_z = simd_ld(transforms[b2_index].position);
			simd4_float b_position_w = simd_ld(transforms[b3_index].position);
			
			simd_transpose32(a_position_x, a_position_y, a_position_z, a_position_w);
			simd_transpose32(b_position_x, b_position_y, b_position_z, b_position_w);
			
			// Compute relative position.
			simd4_float delta_x = b_position_x - a_position_x;
			simd4_float delta_y = b_position_y - a_position_y;
			simd4_float delta_z = b_position_z - a_position_z;
			
			// Flip normal?
			simd4_float sign_mask = simd_splat(-0.0f);
			simd4_float flip_sign = simd_and(n_x*delta_x + n_y*delta_y + n_z*delta_z, sign_mask);
			
			n_x = simd_xor(n_x, flip_sign);
			n_y = simd_xor(n_y, flip_sign);
			n_z = simd_xor(n_z, flip_sign);
			
			// Load sizes.
			simd4_float a_size_x = simd_ld(colliders[a0_index].size);
			simd4_float a_size_y = simd_ld(colliders[a1_index].size);
			simd4_float a_size_z = simd_ld(colliders[a2_index].size);
			simd4_float a_size_w = simd_ld(colliders[a3_index].size);
			
			simd4_float b_size_x = simd_ld(colliders[b0_index].size);
			simd4_float b_size_y = simd_ld(colliders[b1_index].size);
			simd4_float b_size_z = simd_ld(colliders[b2_index].size);
			simd4_float b_size_w = simd_ld(colliders[b3_index].size);
			
			simd_transpose32(a_size_x, a_size_y, a_size_z, a_size_w);
			simd_transpose32(b_size_x, b_size_y, b_size_z, b_size_w);
			
			// Compute direction to the edge.
			simd4_float a_sign_x = a_basis_xx*n_x + a_basis_xy*n_y + a_basis_xz*n_z;
			simd4_float a_sign_y = a_basis_yx*n_x + a_basis_yy*n_y + a_basis_yz*n_z;
			simd4_float a_sign_z = a_basis_zx*n_x + a_basis_zy*n_y + a_basis_zz*n_z;
			
			simd4_float b_sign_x = b_basis_xx*n_x + b_basis_xy*n_y + b_basis_xz*n_z;
			simd4_float b_sign_y = b_basis_yx*n_x + b_basis_yy*n_y + b_basis_yz*n_z;
			simd4_float b_sign_z = b_basis_zx*n_x + b_basis_zy*n_y + b_basis_zz*n_z;
			
			a_sign_x = simd_and(a_sign_x, sign_mask);
			a_sign_y = simd_and(a_sign_y, sign_mask);
			a_sign_z = simd_and(a_sign_z, sign_mask);
			
			b_sign_x = simd_and(b_sign_x, sign_mask);
			b_sign_y = simd_and(b_sign_y, sign_mask);
			b_sign_z = simd_and(b_sign_z, sign_mask);
			
			simd4_int32 edge_x = simd_or(simd_srl(a_sign_x, 31-0), simd_srl(simd_xor(b_sign_x, simd_splat(-0.0f)), 31-16));
			simd4_int32 edge_y = simd_or(simd_srl(a_sign_y, 31-1), simd_srl(simd_xor(b_sign_y, simd_splat(-0.0f)), 31-17));
			simd4_int32 edge_z = simd_or(simd_srl(a_sign_z, 31-2), simd_srl(simd_xor(b_sign_z, simd_splat(-0.0f)), 31-18));
			simd4_int32 edge_w = simd_i16_add(simd_i16_add(edge, simd_isplat((1 << 16) | 1)), simd_i16_srl(edge, 1)); // Calculates 1 << edge (valid for 0-2).

			simd4_int32 edge_xy = simd_or(edge_x, edge_y);
			simd4_int32 edge_zw = simd_or(edge_z, edge_w);
			
			simd4_int32 tag_hi = simd_or(edge_xy, edge_zw);
			simd4_int32 tag_lo = simd_and(simd_not(edge_w), tag_hi);
			tag_hi = simd_sll(tag_hi, 8);
			
			simd4_int32 tag = simd_or(tag_lo, tag_hi);
			
			a_size_x = simd_xor(a_size_x, a_sign_x);
			a_size_y = simd_xor(a_size_y, a_sign_y);
			a_size_z = simd_xor(a_size_z, a_sign_z);
			
			b_size_x = simd_xor(b_size_x, b_sign_x);
			b_size_y = simd_xor(b_size_y, b_sign_y);
			b_size_z = simd_xor(b_size_z, b_sign_z);
			
			a_basis_xx = simd_mul(a_basis_xx, a_size_x);
			a_basis_xy = simd_mul(a_basis_xy, a_size_x);
			a_basis_xz = simd_mul(a_basis_xz, a_size_x);
			
			a_basis_yx = simd_mul(a_basis_yx, a_size_y);
			a_basis_yy = simd_mul(a_basis_yy, a_size_y);
			a_basis_yz = simd_mul(a_basis_yz, a_size_y);
			
			a_basis_zx = simd_mul(a_basis_zx, a_size_z);
			a_basis_zy = simd_mul(a_basis_zy, a_size_z);
			a_basis_zz = simd_mul(a_basis_zz, a_size_z);
			
			b_basis_xx = simd_mul(b_basis_xx, b_size_x);
			b_basis_xy = simd_mul(b_basis_xy, b_size_x);
			b_basis_xz = simd_mul(b_basis_xz, b_size_x);
			
			b_basis_yx = simd_mul(b_basis_yx, b_size_y);
			b_basis_yy = simd_mul(b_basis_yy, b_size_y);
			b_basis_yz = simd_mul(b_basis_yz, b_size_y);
			
			b_basis_zx = simd_mul(b_basis_zx, b_size_z);
			b_basis_zy = simd_mul(b_basis_zy, b_size_z);
			b_basis_zz = simd_mul(b_basis_zz, b_size_z);
			
			simd4_float ca_x = a_basis_xx + a_basis_yx + a_basis_zx + a_position_x;
			simd4_float ca_y = a_basis_xy + a_basis_yy + a_basis_zy + a_position_y;
			simd4_float ca_z = a_basis_xz + a_basis_yz + a_basis_zz + a_position_z;
			
			simd4_float cb_x = b_basis_xx + b_basis_yx + b_basis_zx - b_position_x; // Note that cb really is negated to save some operations.
			simd4_float cb_y = b_basis_xy + b_basis_yy + b_basis_zy - b_position_y;
			simd4_float cb_z = b_basis_xz + b_basis_yz + b_basis_zz - b_position_z;
			
			// Calculate closest point between the two lines.
			simd4_float o_x = ca_x + cb_x;
			simd4_float o_y = ca_y + cb_y;
			simd4_float o_z = ca_z + cb_z;
			
			simd4_float ia = u_x*u_x + u_y*u_y + u_z*u_z;
			simd4_float ib = u_x*v_x + u_y*v_y + u_z*v_z;
			simd4_float ic = v_x*v_x + v_y*v_y + v_z*v_z;
			simd4_float id = o_x*u_x + o_y*u_y + o_z*u_z;
			simd4_float ie = o_x*v_x + o_y*v_y + o_z*v_z;
			
			simd4_float half = simd_splat(0.5f);
			simd4_float ir = simd_div(half, (ia*ic - ib*ib));
			
			simd4_float sa = (ib*ie - ic*id) * ir;
			simd4_float sb = (ia*ie - ib*id) * ir;
			
			simd4_float p_x = (ca_x - cb_x)*half + u_x*sa + v_x*sb;
			simd4_float p_y = (ca_y - cb_y)*half + u_y*sa + v_y*sb;
			simd4_float p_z = (ca_z - cb_z)*half + u_z*sa + v_z*sb;
			
			simd_soa::normalize(n_x, n_y, n_z);
			
			simd4_float p_w = simd_ld(feature_penetrations + i);
			simd4_float n_w = simd_splat(0.5f);
			
			simd_transpose32(p_x, p_y, p_z, p_w);
			simd_transpose32(n_x, n_y, n_z, n_w);
			
			simd_st(contacts[count + 0].position, p_x);
			simd_st(contacts[count + 0].normal, n_x);
			simd_st(contacts[count + 1].position, p_y);
			simd_st(contacts[count + 1].normal, n_y);
			simd_st(contacts[count + 2].position, p_z);
			simd_st(contacts[count + 2].normal, n_z);
			simd_st(contacts[count + 3].position, p_w);
			simd_st(contacts[count + 3].normal, n_w);
			
			simd4_float body_pair = simd_or(simd_and(a_position_w, simd_isplat(0xffff)), simd_sll(b_position_w, 16));
			simd_stu((float*)(bodies + count), body_pair);
			
			simd4_int32 pair = simd_or(simd_and(b_position_w, simd_isplat(0xffff0000)), simd_srl(a_position_w, 16));
			
			simd_stu((int32_t*)tags + count*2 + 0, simd_shuf_xAyB(tag, pair));
			simd_stu((int32_t*)tags + count*2 + 4, simd_shuf_xAyB(tag, pair));
			
			count += 4;
		}
		
		// Get rid of padding.
		while (count && bodies[count-1].a == bodies[count-1].b)
			--count;
	}
	
	return count;
}

static inline unsigned sphere_sphere_collide(SphereCollider a, SphereCollider b, Transform a_transform, Transform b_transform, Contact* contacts, BodyPair* bodies) {
	float r = a.radius + b.radius;
	
	float3 dp = make_float3(b_transform.position) - make_float3(a_transform.position);
	float l2 = length2(dp);
	
	if (l2 > r*r)
		return 0;
	
	float3 n;
	float l = sqrtf(l2);
	
	if (l2 > 1e-4f)
		n = dp * (1.0f / l);
	else
		n = make_float3(1.0f, 0.0f, 0.0f);
	
	float3 p = make_float3(a_transform.position) + n * (l - b.radius);
	
	contacts[0].position[0] = p.x;
	contacts[0].position[1] = p.y;
	contacts[0].position[2] = p.z;
	contacts[0].penetration = r - l;
	contacts[0].normal[0] = n.x;
	contacts[0].normal[1] = n.y;
	contacts[0].normal[2] = n.z;
	contacts[0].friction = 0.5f;
	
	bodies[0].a = (uint16_t)a_transform.body;
	bodies[0].b = (uint16_t)b_transform.body;
	
	return 1;
}

static inline unsigned box_sphere_collide(BoxCollider a, SphereCollider b, Transform a_transform, Transform b_transform, Contact* contacts, BodyPair* bodies) {
	Rotation a_to_world = make_rotation(a_transform.rotation);
	Rotation world_to_a = inverse(a_to_world);
	float3 offset_b = world_to_a * (make_float3(b_transform.position) - make_float3(a_transform.position));
	
	float dx = fabsf(offset_b.x);
	float dy = fabsf(offset_b.y);
	float dz = fabsf(offset_b.z);
	
	float w = a.size[0] + b.radius;
	float h = a.size[1] + b.radius;
	float d = a.size[2] + b.radius;
	
	if (dx >= w || dy >= h || dz >= d)
		return 0;
	
	float3 n;
	float penetration;
	
	float r = b.radius;
	
	unsigned outside_x = dx > a.size[0];
	unsigned outside_y = dy > a.size[1];
	unsigned outside_z = dz > a.size[2];
	
	if (outside_x + outside_y + outside_z >= 2) {
		float3 corner = {
			outside_x ? (offset_b.x > 0.0f ? a.size[0] : -a.size[0]) : offset_b.x,
			outside_y ? (offset_b.y > 0.0f ? a.size[1] : -a.size[1]) : offset_b.y,
			outside_z ? (offset_b.z > 0.0f ? a.size[2] : -a.size[2]) : offset_b.z,
		};
		
		float3 dp = offset_b - corner;
		float l2 = length2(dp);
		
		if (l2 > r*r)
			return 0;
		
		float l = sqrtf(l2);
		float m = 1.0f / l;
		
		n = dp * m;
		penetration = r - l;
	}
	else if (w - dx < h - dy && w - dx < d - dz) {
		n.x = offset_b.x > 0.0f ? 1.0f : -1.0f;
		n.y = 0.0f;
		n.z = 0.0f;
		penetration = w - dx;
	}
	else if (h - dy < d - dz) {
		n.x = 0.0f;
		n.y = offset_b.y > 0.0f ? 1.0f : -1.0f;
		n.z = 0.0f;
		penetration = h - dy;
	}
	else {
		n.x = 0.0f;
		n.y = 0.0f;
		n.z = offset_b.z > 0.0f ? 1.0f : -1.0f;
		penetration = d - dz;
	}
	
	float3 p = offset_b - n*r;
	
	p = a_to_world * p + make_float3(a_transform.position);
	n = a_to_world * n;
	
	contacts[0].position[0] = p.x;
	contacts[0].position[1] = p.y;
	contacts[0].position[2] = p.z;
	contacts[0].penetration = penetration;
	contacts[0].normal[0] = n.x;
	contacts[0].normal[1] = n.y;
	contacts[0].normal[2] = n.z;
	contacts[0].friction = 0.5f;
	
	bodies[0].a = (uint16_t)a_transform.body;
	bodies[0].b = (uint16_t)b_transform.body;
	
	return 1;
}

template<unsigned offset>
static inline void dilate_3(simd4_int32 x, simd4_int32& lo32, simd4_int32& hi32) {
	simd4_int32 mask0 = simd_isplat(0xff);
	simd4_int32 mask1 = simd_isplat(0x0f00f00f);
	simd4_int32 mask2 = simd_isplat(0xc30c30c3);
	simd4_int32 mask3 = simd_isplat(0x49249249);
	
	simd4_int32 lo24 = x;
	simd4_int32 hi24 = simd_srl(x, 8);
	lo24 = simd_and(lo24, mask0);
	hi24 = simd_and(hi24, mask0);
	
	lo24 = simd_or(lo24, simd_sll(lo24, 8));
	hi24 = simd_or(hi24, simd_sll(hi24, 8));
	lo24 = simd_and(lo24, mask1);
	hi24 = simd_and(hi24, mask1);
	
	lo24 = simd_or(lo24, simd_sll(lo24, 4));
	hi24 = simd_or(hi24, simd_sll(hi24, 4));
	lo24 = simd_and(lo24, mask2);
	hi24 = simd_and(hi24, mask2);
	
	lo24 = simd_or(lo24, simd_sll(lo24, 2));
	hi24 = simd_or(hi24, simd_sll(hi24, 2));
	lo24 = simd_and(lo24, mask3);
	hi24 = simd_and(hi24, mask3);
	
	lo32 = simd_or(simd_sll(lo24, offset), simd_sll(hi24, 24+offset));
	hi32 = simd_srl(hi24, 8-offset);
}

static inline void morton(simd4_int32 x, simd4_int32 y, simd4_int32 z, simd4_int32& lo32, simd4_int32& hi32) {
	simd4_int32 lx, hx, ly, hy, lz, hz;
	dilate_3<2>(x, lx, hx);
	dilate_3<1>(y, ly, hy);
	dilate_3<0>(z, lz, hz);
	
	lo32 = simd_or(simd_or(lx, ly), lz);
	hi32 = simd_or(simd_or(hx, hy), hz);
}

static inline void radix_sort_uint64_low48(uint64_t* data, unsigned count, Arena temporary) {
	uint64_t* temp = allocate_array<uint64_t>(&temporary, count, 16);
	
	unsigned buckets0[257] = {};
	unsigned buckets1[257] = {};
	unsigned buckets2[257] = {};
	unsigned buckets3[257] = {};
	unsigned buckets4[257] = {};
	unsigned buckets5[257] = {};
	
	unsigned* histogram0 = buckets0+1;
	unsigned* histogram1 = buckets1+1;
	unsigned* histogram2 = buckets2+1;
	unsigned* histogram3 = buckets3+1;
	unsigned* histogram4 = buckets4+1;
	unsigned* histogram5 = buckets5+1;
	
	for (unsigned i = 0; i < count; ++i) {
		uint64_t d = data[i];
		
		++histogram0[(d >> (0 << 3)) & 0xff];
		++histogram1[(d >> (1 << 3)) & 0xff];
		++histogram2[(d >> (2 << 3)) & 0xff];
		++histogram3[(d >> (3 << 3)) & 0xff];
		++histogram4[(d >> (4 << 3)) & 0xff];
		++histogram5[(d >> (5 << 3)) & 0xff];
	}
	
	for (unsigned i = 1; i < 256; ++i) {
		buckets0[i] += buckets0[i-1];
		buckets1[i] += buckets1[i-1];
		buckets2[i] += buckets2[i-1];
		buckets3[i] += buckets3[i-1];
		buckets4[i] += buckets4[i-1];
		buckets5[i] += buckets5[i-1];
	}
	
	for (unsigned i = 0; i < count; ++i) {
		uint64_t d = data[i];
		unsigned index = buckets0[(d >> (0 << 3)) & 0xff]++;
		temp[index] = d;
	}
	
	for (unsigned i = 0; i < count; ++i) {
		uint64_t d = temp[i];
		unsigned index = buckets1[(d >> (1 << 3)) & 0xff]++;
		data[index] = d;
	}
	
	for (unsigned i = 0; i < count; ++i) {
		uint64_t d = data[i];
		unsigned index = buckets2[(d >> (2 << 3)) & 0xff]++;
		temp[index] = d;
	}
	
	for (unsigned i = 0; i < count; ++i) {
		uint64_t d = temp[i];
		unsigned index = buckets3[(d >> (3 << 3)) & 0xff]++;
		data[index] = d;
	}
	
	for (unsigned i = 0; i < count; ++i) {
		uint64_t d = data[i];
		unsigned index = buckets4[(d >> (4 << 3)) & 0xff]++;
		temp[index] = d;
	}
	
	for (unsigned i = 0; i < count; ++i) {
		uint64_t d = temp[i];
		unsigned index = buckets5[(d >> (5 << 3)) & 0xff]++;
		data[index] = d;
	}
}

static inline void radix_sort_uint32_x2(uint32_t* data, uint32_t* data2, unsigned count, Arena temporary) {
	uint32_t* temp = allocate_array<uint32_t>(&temporary, count, 16);
	uint32_t* temp2 = allocate_array<uint32_t>(&temporary, count, 16);
	
	unsigned buckets0[257] = {};
	unsigned buckets1[257] = {};
	unsigned buckets2[257] = {};
	unsigned buckets3[257] = {};
	
	unsigned* histogram0 = buckets0+1;
	unsigned* histogram1 = buckets1+1;
	unsigned* histogram2 = buckets2+1;
	unsigned* histogram3 = buckets3+1;
	
	for (unsigned i = 0; i < count; ++i) {
		uint32_t d = data[i];
		
		++histogram0[(d >> (0 << 3)) & 0xff];
		++histogram1[(d >> (1 << 3)) & 0xff];
		++histogram2[(d >> (2 << 3)) & 0xff];
		++histogram3[(d >> (3 << 3)) & 0xff];
	}
	
	for (unsigned i = 1; i < 256; ++i) {
		buckets0[i] += buckets0[i-1];
		buckets1[i] += buckets1[i-1];
		buckets2[i] += buckets2[i-1];
		buckets3[i] += buckets3[i-1];
	}
	
	for (unsigned i = 0; i < count; ++i) {
		uint32_t d = data[i];
		uint32_t d2 = data2[i];
		unsigned index = buckets0[(d >> (0 << 3)) & 0xff]++;
		temp[index] = d;
		temp2[index] = d2;
	}
	
	for (unsigned i = 0; i < count; ++i) {
		uint32_t d = temp[i];
		uint32_t d2 = temp2[i];
		unsigned index = buckets1[(d >> (1 << 3)) & 0xff]++;
		data[index] = d;
		data2[index] = d2;
	}
	
	for (unsigned i = 0; i < count; ++i) {
		uint32_t d = data[i];
		uint32_t d2 = data2[i];
		unsigned index = buckets2[(d >> (2 << 3)) & 0xff]++;
		temp[index] = d;
		temp2[index] = d2;
	}
	
	for (unsigned i = 0; i < count; ++i) {
		uint32_t d = temp[i];
		uint32_t d2 = temp2[i];
		unsigned index = buckets3[(d >> (3 << 3)) & 0xff]++;
		data[index] = d;
		data2[index] = d2;
	}
}

static inline void radix_sort_uint32(uint32_t* data, unsigned count, Arena temporary) {
	uint32_t* temp = allocate_array<uint32_t>(&temporary, count, 16);
	
	unsigned buckets0[257] = {};
	unsigned buckets1[257] = {};
	unsigned buckets2[257] = {};
	unsigned buckets3[257] = {};
	
	unsigned* histogram0 = buckets0+1;
	unsigned* histogram1 = buckets1+1;
	unsigned* histogram2 = buckets2+1;
	unsigned* histogram3 = buckets3+1;
	
	for (unsigned i = 0; i < count; ++i) {
		uint32_t d = data[i];
		
		++histogram0[(d >> (0 << 3)) & 0xff];
		++histogram1[(d >> (1 << 3)) & 0xff];
		++histogram2[(d >> (2 << 3)) & 0xff];
		++histogram3[(d >> (3 << 3)) & 0xff];
	}
	
	for (unsigned i = 1; i < 256; ++i) {
		buckets0[i] += buckets0[i-1];
		buckets1[i] += buckets1[i-1];
		buckets2[i] += buckets2[i-1];
		buckets3[i] += buckets3[i-1];
	}
	
	for (unsigned i = 0; i < count; ++i) {
		uint32_t d = data[i];
		unsigned index = buckets0[(d >> (0 << 3)) & 0xff]++;
		temp[index] = d;
	}
	
	for (unsigned i = 0; i < count; ++i) {
		uint32_t d = temp[i];
		unsigned index = buckets1[(d >> (1 << 3)) & 0xff]++;
		data[index] = d;
	}
	
	for (unsigned i = 0; i < count; ++i) {
		uint32_t d = data[i];
		unsigned index = buckets2[(d >> (2 << 3)) & 0xff]++;
		temp[index] = d;
	}
	
	for (unsigned i = 0; i < count; ++i) {
		uint32_t d = temp[i];
		unsigned index = buckets3[(d >> (3 << 3)) & 0xff]++;
		data[index] = d;
	}
}

template<unsigned data_stride, unsigned index_stride, class T>
NUDGE_FORCEINLINE static void load4(const float* data, const T* indices,
									simd4_float& d0, simd4_float& d1, simd4_float& d2, simd4_float& d3) {
	static const unsigned stride_in_floats = data_stride/sizeof(float);
	
	unsigned i0 = indices[0*index_stride];
	unsigned i1 = indices[1*index_stride];
	unsigned i2 = indices[2*index_stride];
	unsigned i3 = indices[3*index_stride];
	
	d0 = simd_ld(data + i0*stride_in_floats);
	d1 = simd_ld(data + i1*stride_in_floats);
	d2 = simd_ld(data + i2*stride_in_floats);
	d3 = simd_ld(data + i3*stride_in_floats);
	
	simd_transpose32(d0, d1, d2, d3);
}

template<unsigned data_stride, unsigned index_stride, class T>
NUDGE_FORCEINLINE static void load8(const float* data, const T* indices,
									simd4_float& d0, simd4_float& d1, simd4_float& d2, simd4_float& d3,
									simd4_float& d4, simd4_float& d5, simd4_float& d6, simd4_float& d7) {
	static const unsigned stride_in_floats = data_stride/sizeof(float);
	
	unsigned i0 = indices[0*index_stride];
	unsigned i1 = indices[1*index_stride];
	unsigned i2 = indices[2*index_stride];
	unsigned i3 = indices[3*index_stride];
	
	d0 = simd_ld(data + i0*stride_in_floats);
	d1 = simd_ld(data + i1*stride_in_floats);
	d2 = simd_ld(data + i2*stride_in_floats);
	d3 = simd_ld(data + i3*stride_in_floats);
	
	d4 = simd_ld(data + i0*stride_in_floats + 4);
	d5 = simd_ld(data + i1*stride_in_floats + 4);
	d6 = simd_ld(data + i2*stride_in_floats + 4);
	d7 = simd_ld(data + i3*stride_in_floats + 4);
	
	simd_transpose32(d0, d1, d2, d3);
	simd_transpose32(d4, d5, d6, d7);
}

template<unsigned data_stride, unsigned index_stride, class T>
NUDGE_FORCEINLINE static void store8(float* data, const T* indices,
									 simd4_float d0, simd4_float d1, simd4_float d2, simd4_float d3,
									 simd4_float d4, simd4_float d5, simd4_float d6, simd4_float d7) {
	static const unsigned stride_in_floats = data_stride/sizeof(float);
	
	simd_transpose32(d0, d1, d2, d3);
	simd_transpose32(d4, d5, d6, d7);
	
	unsigned i0 = indices[0*index_stride];
	unsigned i1 = indices[1*index_stride];
	unsigned i2 = indices[2*index_stride];
	unsigned i3 = indices[3*index_stride];
	
	simd_st(data + i0*stride_in_floats, d0);
	simd_st(data + i1*stride_in_floats, d1);
	simd_st(data + i2*stride_in_floats, d2);
	simd_st(data + i3*stride_in_floats, d3);
	
	simd_st(data + i0*stride_in_floats + 4, d4);
	simd_st(data + i1*stride_in_floats + 4, d5);
	simd_st(data + i2*stride_in_floats + 4, d6);
	simd_st(data + i3*stride_in_floats + 4, d7);
}

void collide(ActiveBodies* active_bodies, ContactData* contacts, BodyData bodies, ColliderData colliders, BodyConnections body_connections, Arena temporary) {
	contacts->count = 0;
	contacts->sleeping_count = 0;
	active_bodies->count = 0;
	
	const Transform* body_transforms = bodies.transforms;
	
	unsigned count = colliders.spheres.count + colliders.boxes.count;
	unsigned aligned_count = (count + 7) & (~7);
	
	assert(count <= (1 << 13)); // Too many colliders. 2^13 is currently the maximum.
	
	AABB* aos_bounds = allocate_array<AABB>(&temporary, aligned_count, 32);
	
	unsigned box_bounds_offset = 0;
	unsigned sphere_bounds_offset = colliders.boxes.count;
	
	Transform* transforms = allocate_array<Transform>(&temporary, count, 32);
	uint16_t* collider_tags = allocate_array<uint16_t>(&temporary, count, 32);
	uint16_t* collider_bodies = allocate_array<uint16_t>(&temporary, count, 32);
	
	if (colliders.boxes.count) {
		for (unsigned i = 0; i < colliders.boxes.count; ++i) {
			Transform transform = colliders.boxes.transforms[i];
			transform = body_transforms[transform.body] * transform;
			transform.body |= (uint32_t)colliders.boxes.tags[i] << 16;
			
			float3x3 m = matrix(make_rotation(transform.rotation));
			
			m.c0 *= colliders.boxes.data[i].size[0];
			m.c1 *= colliders.boxes.data[i].size[1];
			m.c2 *= colliders.boxes.data[i].size[2];
			
			float3 size = {
				fabsf(m.c0.x) + fabsf(m.c1.x) + fabsf(m.c2.x),
				fabsf(m.c0.y) + fabsf(m.c1.y) + fabsf(m.c2.y),
				fabsf(m.c0.z) + fabsf(m.c1.z) + fabsf(m.c2.z),
			};
			
			float3 min = make_float3(transform.position) - size;
			float3 max = make_float3(transform.position) + size;
			
			AABB aabb = {
				min, 0.0f,
				max, 0.0f,
			};
			
			transforms[i + box_bounds_offset] = transform;
			aos_bounds[i + box_bounds_offset] = aabb;
			collider_tags[i + box_bounds_offset] = colliders.boxes.tags[i];
			collider_bodies[i + box_bounds_offset] = colliders.boxes.transforms[i].body;
		}
		
		colliders.boxes.transforms = transforms + box_bounds_offset;
	}
	
	if (colliders.spheres.count) {
		for (unsigned i = 0; i < colliders.spheres.count; ++i) {
			Transform transform = colliders.spheres.transforms[i];
			transform = body_transforms[transform.body] * transform;
			transform.body |= (uint32_t)colliders.spheres.tags[i] << 16;
			
			float radius = colliders.spheres.data[i].radius;
			
			float3 min = make_float3(transform.position) - make_float3(radius);
			float3 max = make_float3(transform.position) + make_float3(radius);
			
			AABB aabb = {
				min, 0.0f,
				max, 0.0f,
			};
			
			transforms[i + sphere_bounds_offset] = transform;
			aos_bounds[i + sphere_bounds_offset] = aabb;
			collider_tags[i + sphere_bounds_offset] = colliders.spheres.tags[i];
			collider_bodies[i + sphere_bounds_offset] = colliders.spheres.transforms[i].body;
		}
		
		colliders.spheres.transforms = transforms + sphere_bounds_offset;
	}
	
	for (unsigned i = count; i < aligned_count; ++i) {
		AABB zero = {};
		aos_bounds[i] = zero;
	}
	
	// Morton order using the min corner should improve coherence: After some point, all BBs' min points will be outside a BB's max.
	simd4_float scene_min128 = simd_ld(&aos_bounds[0].min.x);
	simd4_float scene_max128 = scene_min128;
	
	for (unsigned i = 1; i < count; ++i) {
		simd4_float p = simd_ld(&aos_bounds[i].min.x);
		scene_min128 = simd_min(scene_min128, p);
		scene_max128 = simd_max(scene_max128, p);
	}
	
	simd4_float scene_scale128 = simd_splat((1<<16)-1) * simd_rcp_est(scene_max128 - scene_min128);
	
	scene_scale128 = simd_min(simd_swiz_xyzz(scene_scale128), simd_swiz_zzxy(scene_scale128));
	scene_scale128 = simd_min(scene_scale128, simd_swiz_yxwz(scene_scale128));
	scene_min128 = scene_min128 * scene_scale128;
	
#ifdef DEBUG
	if (simd_x(scene_scale128) < 2.0f)
		printf("Warning: World bounds are very large, which may decrease performance. Perhaps there's a body in free fall?\n");
#endif
	
	simd4_float scene_min = scene_min128;
	simd4_float scene_scale = scene_scale128;
	simd4_int32 index = simd_ild(0 << 16, 1 << 16, 2 << 16, 3 << 16);
	
	simd4_float scene_min_x = simd_swiz_xxxx(scene_min);
	simd4_float scene_min_y = simd_swiz_yyyy(scene_min);
	simd4_float scene_min_z = simd_swiz_zzzz(scene_min);
	
	uint64_t* morton_codes = allocate_array<uint64_t>(&temporary, aligned_count, 32);
	
	for (unsigned i = 0; i < count; i += simdv_width32) {
		simd4_float pos_x = simd_ld(&aos_bounds[i+0].min.x);
		simd4_float pos_y = simd_ld(&aos_bounds[i+1].min.x);
		simd4_float pos_z = simd_ld(&aos_bounds[i+2].min.x);
		simd4_float pos_w = simd_ld(&aos_bounds[i+3].min.x);
		
		simd_transpose32(pos_x, pos_y, pos_z, pos_w);
		
		pos_x = simd_nmsub(pos_x, scene_scale, scene_min_x);
		pos_y = simd_nmsub(pos_y, scene_scale, scene_min_y);
		pos_z = simd_nmsub(pos_z, scene_scale, scene_min_z);
		
		simd4_int32 lm, hm;
		morton(simd_ftoi(pos_x), simd_ftoi(pos_y), simd_ftoi(pos_z), lm, hm);
		hm = simd_or(hm, index);
		
		simd4_int32 mi0 = simd_shuf_xAyB(lm, hm);
		simd4_int32 mi1 = simd_shuf_zCwD(lm, hm);
		
		simd_st((int32_t*)(morton_codes + i) + 0, mi0);
		simd_st((int32_t*)(morton_codes + i) + 4, mi1);
		
		index = simd_iadd(index, simd_isplat(simdv_width32 << 16));
	}
	
	radix_sort_uint64_low48(morton_codes, count, temporary);
	uint16_t* sorted_indices = allocate_array<uint16_t>(&temporary, aligned_count, 32);
	
	for (unsigned i = 0; i < count; ++i)
		sorted_indices[i] = (uint16_t)(morton_codes[i] >> 48);
	
	for (unsigned i = count; i < aligned_count; ++i)
		sorted_indices[i] = 0;
	
	unsigned bounds_count = aligned_count >> simdv_width32_log2;
	AABBV* bounds = allocate_array<AABBV>(&temporary, bounds_count, 32);
	
	for (unsigned i = 0; i < count; i += simdv_width32) {
		simd4_float min_x, min_y, min_z, min_w;
		simd4_float max_x, max_y, max_z, max_w;
		load8<sizeof(aos_bounds[0]), 1>(&aos_bounds[0].min.x, sorted_indices + i,
										min_x, min_y, min_z, min_w,
										max_x, max_y, max_z, max_w);
		
		simd_st(bounds[i >> simdv_width32_log2].min_x, min_x);
		simd_st(bounds[i >> simdv_width32_log2].max_x, max_x);
		simd_st(bounds[i >> simdv_width32_log2].min_y, min_y);
		simd_st(bounds[i >> simdv_width32_log2].max_y, max_y);
		simd_st(bounds[i >> simdv_width32_log2].min_z, min_z);
		simd_st(bounds[i >> simdv_width32_log2].max_z, max_z);
	}
	
	for (unsigned i = count; i < aligned_count; ++i) {
		unsigned bounds_group = i >> simdv_width32_log2;
		unsigned bounds_lane = i & (simdv_width32-1);
		
		bounds[bounds_group].min_x[bounds_lane] = -float_max;
		bounds[bounds_group].max_x[bounds_lane] =  float_max;
		bounds[bounds_group].min_y[bounds_lane] = -float_max;
		bounds[bounds_group].max_y[bounds_lane] =  float_max;
		bounds[bounds_group].min_z[bounds_lane] = -float_max;
		bounds[bounds_group].max_z[bounds_lane] =  float_max;
	}
	
	// Pack each set of 8 consecutive AABBs into coarse AABBs.
	unsigned coarse_count = aligned_count >> 3;
	unsigned aligned_coarse_count = (coarse_count + (simdv_width32-1)) & (~(simdv_width32-1));
	
	unsigned coarse_bounds_count = aligned_coarse_count >> simdv_width32_log2;
	AABBV* coarse_bounds = allocate_array<AABBV>(&temporary, coarse_bounds_count, 32);
	
	for (unsigned i = 0; i < coarse_count; ++i) {
		unsigned start = i << (3 - simdv_width32_log2);
		
		simd4_float coarse_min_x = simd_ld(bounds[start].min_x);
		simd4_float coarse_max_x = simd_ld(bounds[start].max_x);
		simd4_float coarse_min_y = simd_ld(bounds[start].min_y);
		simd4_float coarse_max_y = simd_ld(bounds[start].max_y);
		simd4_float coarse_min_z = simd_ld(bounds[start].min_z);
		simd4_float coarse_max_z = simd_ld(bounds[start].max_z);
		
		// Note the last padded bounds are +/- FLOAT_MAX, so the earlier bounds should be in the first operand.
		coarse_min_x = simd_min(coarse_min_x, simd_ld(bounds[start+1].min_x));
		coarse_max_x = simd_max(coarse_max_x, simd_ld(bounds[start+1].max_x));
		coarse_min_y = simd_min(coarse_min_y, simd_ld(bounds[start+1].min_y));
		coarse_max_y = simd_max(coarse_max_y, simd_ld(bounds[start+1].max_y));
		coarse_min_z = simd_min(coarse_min_z, simd_ld(bounds[start+1].min_z));
		coarse_max_z = simd_max(coarse_max_z, simd_ld(bounds[start+1].max_z));
		
		coarse_min_x = simd_min(coarse_min_x, simd_swiz_zwxy(coarse_min_x));
		coarse_max_x = simd_max(coarse_max_x, simd_swiz_zwxy(coarse_max_x));
		coarse_min_y = simd_min(coarse_min_y, simd_swiz_zwxy(coarse_min_y));
		coarse_max_y = simd_max(coarse_max_y, simd_swiz_zwxy(coarse_max_y));
		coarse_min_z = simd_min(coarse_min_z, simd_swiz_zwxy(coarse_min_z));
		coarse_max_z = simd_max(coarse_max_z, simd_swiz_zwxy(coarse_max_z));
		
		coarse_min_x = simd_min(coarse_min_x, simd_swiz_yxwz(coarse_min_x));
		coarse_max_x = simd_max(coarse_max_x, simd_swiz_yxwz(coarse_max_x));
		coarse_min_y = simd_min(coarse_min_y, simd_swiz_yxwz(coarse_min_y));
		coarse_max_y = simd_max(coarse_max_y, simd_swiz_yxwz(coarse_max_y));
		coarse_min_z = simd_min(coarse_min_z, simd_swiz_yxwz(coarse_min_z));
		coarse_max_z = simd_max(coarse_max_z, simd_swiz_yxwz(coarse_max_z));
		
		unsigned bounds_group = i >> simdv_width32_log2;
		unsigned bounds_lane = i & (simdv_width32-1);
		
		coarse_bounds[bounds_group].min_x[bounds_lane] = simd_x(coarse_min_x);
		coarse_bounds[bounds_group].max_x[bounds_lane] = simd_x(coarse_max_x);
		coarse_bounds[bounds_group].min_y[bounds_lane] = simd_x(coarse_min_y);
		coarse_bounds[bounds_group].max_y[bounds_lane] = simd_x(coarse_max_y);
		coarse_bounds[bounds_group].min_z[bounds_lane] = simd_x(coarse_min_z);
		coarse_bounds[bounds_group].max_z[bounds_lane] = simd_x(coarse_max_z);
	}
	
	for (unsigned i = coarse_count; i < aligned_coarse_count; ++i) {
		unsigned bounds_group = i >> simdv_width32_log2;
		unsigned bounds_lane = i & (simdv_width32-1);
		
		coarse_bounds[bounds_group].min_x[bounds_lane] = -float_max;
		coarse_bounds[bounds_group].max_x[bounds_lane] =  float_max;
		coarse_bounds[bounds_group].min_y[bounds_lane] = -float_max;
		coarse_bounds[bounds_group].max_y[bounds_lane] =  float_max;
		coarse_bounds[bounds_group].min_z[bounds_lane] = -float_max;
		coarse_bounds[bounds_group].max_z[bounds_lane] =  float_max;
	}
	
	// Test all coarse groups against each other and generate pairs with potential overlap.
	uint32_t* coarse_groups = reserve_array<uint32_t>(&temporary, coarse_count*coarse_count, 32);
	unsigned coarse_group_count = 0;
	
	for (unsigned i = 0; i < coarse_count; ++i) {
		unsigned bounds_group = i >> simdv_width32_log2;
		unsigned bounds_lane = i & (simdv_width32-1);
		
		simd4_float min_a_x = simd_splat(coarse_bounds[bounds_group].min_x + bounds_lane);
		simd4_float max_a_x = simd_splat(coarse_bounds[bounds_group].max_x + bounds_lane);
		simd4_float min_a_y = simd_splat(coarse_bounds[bounds_group].min_y + bounds_lane);
		simd4_float max_a_y = simd_splat(coarse_bounds[bounds_group].max_y + bounds_lane);
		simd4_float min_a_z = simd_splat(coarse_bounds[bounds_group].min_z + bounds_lane);
		simd4_float max_a_z = simd_splat(coarse_bounds[bounds_group].max_z + bounds_lane);
		
		unsigned first = coarse_group_count;
		
		// Maximum number of colliders is 2^13, i.e., 13 bit indices.
		// i needs 10 bits.
		// j needs 7 or 8 bits.
		// mask needs 4 or 8 bits.
		unsigned ij_bits = (bounds_group << 8) | (i << 16);
		
		for (unsigned j = bounds_group; j < coarse_bounds_count; ++j) {
			simd4_float min_b_x = simd_ld(coarse_bounds[j].min_x);
			simd4_float max_b_x = simd_ld(coarse_bounds[j].max_x);
			simd4_float min_b_y = simd_ld(coarse_bounds[j].min_y);
			simd4_float max_b_y = simd_ld(coarse_bounds[j].max_y);
			simd4_float min_b_z = simd_ld(coarse_bounds[j].min_z);
			simd4_float max_b_z = simd_ld(coarse_bounds[j].max_z);
			
			simd4_float inside_x = simd_and(simd_cmpgt(max_b_x, min_a_x), simd_cmpgt(max_a_x, min_b_x));
			simd4_float inside_y = simd_and(simd_cmpgt(max_b_y, min_a_y), simd_cmpgt(max_a_y, min_b_y));
			simd4_float inside_z = simd_and(simd_cmpgt(max_b_z, min_a_z), simd_cmpgt(max_a_z, min_b_z));
			
			unsigned mask = simd::cmpmask32(simd_and(simd_and(inside_x, inside_y), inside_z));
			
			coarse_groups[coarse_group_count] = mask | ij_bits;
			coarse_group_count += mask != 0;
			
			ij_bits += 1 << 8;
		}
		
		// Mask out collisions already handled.
		coarse_groups[first] &= ~((1 << bounds_lane) - 1);
	}
	
	commit_array<uint32_t>(&temporary, coarse_group_count);
	
	uint32_t* coarse_pairs = reserve_array<uint32_t>(&temporary, coarse_group_count*simdv_width32, 32);
	unsigned coarse_pair_count = 0;
	
	for (unsigned i = 0; i < coarse_group_count; ++i) {
		unsigned group = coarse_groups[i];
		unsigned mask = group & 0xff;
		
		unsigned batch = (group & 0xff00) >> (8 - simdv_width32_log2);
		unsigned other = group & 0xffff0000;
		
		while (mask) {
			unsigned index = first_set_bit(mask);
			mask &= mask-1;
			
			coarse_pairs[coarse_pair_count++] = other | (batch + index);
		}
	}
	
	commit_array<uint32_t>(&temporary, coarse_pair_count);
	
	// Test AABBs within the coarse pairs.
	uint32_t* groups = reserve_array<uint32_t>(&temporary, coarse_pair_count*16, 32);
	unsigned group_count = 0;
	
	// TODO: This version is currently much worse than the 256-bit version. We should fix it.
	for (unsigned n = 0; n < coarse_pair_count; ++n) {
		unsigned pair = coarse_pairs[n];
		
		unsigned a = pair >> 16;
		unsigned b = pair & 0xffff;
		
		unsigned a_start = a << 3;
		unsigned a_end = a_start + (1 << 3);
		
		if (a_end > count)
			a_end = count;
		
		unsigned b_start = b << (3 - simdv_width32_log2);
		unsigned b_end = b_start + (1 << (3 - simdv_width32_log2));
		
		if (b_end > bounds_count)
			b_end = bounds_count;
		
		for (unsigned i = a_start; i < a_end; ++i) {
			unsigned bounds_group = i >> simdv_width32_log2;
			unsigned bounds_lane = i & (simdv_width32-1);
			
			simd4_float min_a_x = simd_splat(bounds[bounds_group].min_x + bounds_lane);
			simd4_float max_a_x = simd_splat(bounds[bounds_group].max_x + bounds_lane);
			simd4_float min_a_y = simd_splat(bounds[bounds_group].min_y + bounds_lane);
			simd4_float max_a_y = simd_splat(bounds[bounds_group].max_y + bounds_lane);
			simd4_float min_a_z = simd_splat(bounds[bounds_group].min_z + bounds_lane);
			simd4_float max_a_z = simd_splat(bounds[bounds_group].max_z + bounds_lane);
			
			unsigned first = group_count;
			
			unsigned start = (i+1) >> simdv_width32_log2;
			
			if (start < b_start)
				start = b_start;
			
			// Maximum number of colliders is 2^13, i.e., 13 bit indices.
			// i needs 13 bits.
			// j needs 10 or 11 bits.
			// mask needs 4 or 8 bits.
			unsigned ij_bits = (start << 8) | (i << 19);
			
			for (unsigned j = start; j < b_end; ++j) {
				simd4_float min_b_x = simd_ld(bounds[j].min_x);
				simd4_float max_b_x = simd_ld(bounds[j].max_x);
				simd4_float min_b_y = simd_ld(bounds[j].min_y);
				simd4_float max_b_y = simd_ld(bounds[j].max_y);
				simd4_float min_b_z = simd_ld(bounds[j].min_z);
				simd4_float max_b_z = simd_ld(bounds[j].max_z);
				
				simd4_float inside_x = simd_and(simd_cmpgt(max_b_x, min_a_x), simd_cmpgt(max_a_x, min_b_x));
				simd4_float inside_y = simd_and(simd_cmpgt(max_b_y, min_a_y), simd_cmpgt(max_a_y, min_b_y));
				simd4_float inside_z = simd_and(simd_cmpgt(max_b_z, min_a_z), simd_cmpgt(max_a_z, min_b_z));
				
				unsigned mask = simd::cmpmask32(simd_and(simd_and(inside_x, inside_y), inside_z));
				
				groups[group_count] = mask | ij_bits;
				group_count += mask != 0;
				
				ij_bits += 1 << 8;
			}
			
			// Mask out collisions already handled.
			if (first < group_count && (groups[first] & 0x7ff00) == (bounds_group << 8))
				groups[first] &= ~((2 << bounds_lane) - 1);
		}
	}
	
	commit_array<uint32_t>(&temporary, group_count);
	
	uint32_t* pairs = reserve_array<uint32_t>(&temporary, group_count*simdv_width32, 32);
	unsigned pair_count = 0;
	
	for (unsigned i = 0; i < group_count; ++i) {
		unsigned group = groups[i];
		unsigned mask = group & 0xff;
		
		unsigned batch = (group & 0x7ff00) >> (8 - simdv_width32_log2);
		unsigned base = ((uint32_t)(group >> 19) << 16) | batch;
		
		while (mask) {
			unsigned index = first_set_bit(mask);
			mask &= mask-1;
			
			pairs[pair_count++] = base + index;
		}
	}
	
	commit_array<uint32_t>(&temporary, pair_count);
	
	for (unsigned i = 0; i < pair_count; ++i) {
		unsigned pair = pairs[i];
		pairs[i] = sorted_indices[pair & 0xffff] | ((uint32_t)sorted_indices[pair >> 16] << 16);
	}
	
	radix_sort_uint32(pairs, pair_count, temporary);
	
	// Discard islands of inactive objects at a coarse level, before detailed collisions.
	{
		NUDGE_ARENA_SCOPE(temporary);
		
		// Find connected sets.
		uint16_t* heights = allocate_array<uint16_t>(&temporary, bodies.count, 16);
		uint16_t* parents = allocate_array<uint16_t>(&temporary, bodies.count, 16);
		
		memset(heights, 0, sizeof(heights[0])*bodies.count);
		memset(parents, 0xff, sizeof(parents[0])*bodies.count);
		
		for (unsigned i = 0; i < body_connections.count; ++i) {
			BodyPair pair = body_connections.data[i];
			
			unsigned a = pair.a;
			unsigned b = pair.b;
			
			// Body 0 is the static world and is ignored.
			if (!a || !b)
				continue;
			
			// Determine the root of a and b.
			unsigned a_root = a;
			unsigned a_parent = parents[a];
			
			for (unsigned parent = a_parent; parent != 0xffff; parent = parents[a_root])
				a_root = parent;
			
			unsigned b_root = b;
			unsigned b_parent = parents[b];
			
			for (unsigned parent = b_parent; parent != 0xffff; parent = parents[b_root])
				b_root = parent;
			
			if (a_root == b_root)
				continue;
			
			// Put a and b under the same root.
			unsigned a_height = heights[a_root];
			unsigned b_height = heights[b_root];
			
			unsigned root;
			
			if (a_height < b_height) {
				parents[a_root] = b_root;
				root = b_root;
			}
			else {
				parents[b_root] = a_root;
				root = a_root;
			}
			
			if (a_height == b_height) // Height of subtree increased.
				heights[a_root] = a_height+1;
			
			// Propagate the root to make subsequent iterations faster.
			if (a_root != a) {
				while (a_parent != a_root) {
					unsigned next = parents[a_parent];
					parents[a] = root;
					
					a = a_parent;
					a_parent = next;
				}
			}
			
			if (b_root != b) {
				while (b_parent != b_root) {
					unsigned next = parents[b_parent];
					parents[b] = root;
					
					b = b_parent;
					b_parent = next;
				}
			}
		}
		
		for (unsigned i = 0; i < pair_count; ++i) {
			unsigned pair = pairs[i];
			
			unsigned a = collider_bodies[pair & 0xffff];
			unsigned b = collider_bodies[pair >> 16];
			
			// Body 0 is the static world and is ignored.
			if (!a || !b)
				continue;
			
			// Determine the root of a and b.
			unsigned a_root = a;
			unsigned a_parent = parents[a];
			
			for (unsigned parent = a_parent; parent != 0xffff; parent = parents[a_root])
				a_root = parent;
			
			unsigned b_root = b;
			unsigned b_parent = parents[b];
			
			for (unsigned parent = b_parent; parent != 0xffff; parent = parents[b_root])
				b_root = parent;
			
			if (a_root == b_root)
				continue;
			
			// Put a and b under the same root.
			unsigned a_height = heights[a_root];
			unsigned b_height = heights[b_root];
			
			unsigned root;
			
			if (a_height < b_height) {
				parents[a_root] = b_root;
				root = b_root;
			}
			else {
				parents[b_root] = a_root;
				root = a_root;
			}
			
			if (a_height == b_height) // Height of subtree increased.
				heights[a_root] = a_height+1;
			
			// Propagate the root to make subsequent iterations faster.
			if (a_root != a) {
				while (a_parent != a_root) {
					unsigned next = parents[a_parent];
					parents[a] = root;
					
					a = a_parent;
					a_parent = next;
				}
			}
			
			if (b_root != b) {
				while (b_parent != b_root) {
					unsigned next = parents[b_parent];
					parents[b] = root;
					
					b = b_parent;
					b_parent = next;
				}
			}
		}
		
		// Identify a numbered set for each body.
		unsigned set_count = 0;
		uint16_t* sets = heights;
		memset(sets, 0xff, sizeof(sets[0])*bodies.count);
		
		for (unsigned i = 1; i < bodies.count; ++i) {
			unsigned root = parents[i];
			
			for (unsigned parent = root; parent != 0xffff; parent = parents[root])
				root = parent;
			
			if (root == 0xffff)
				root = i;
			
			if (sets[root] == 0xffff)
				sets[root] = set_count++;
			
			sets[i] = sets[root];
		}
		
		sets[0] = 0;
		
		// Determine active sets.
		uint8_t* active = allocate_array<uint8_t>(&temporary, set_count, 16);
		memset(active, 0, sizeof(active[0])*set_count);
		
		for (unsigned i = 1; i < bodies.count; ++i) {
			if (bodies.idle_counters[i] != 0xff)
				active[sets[i]] = 1;
		}
		
		// Remove inactive pairs.
		unsigned removed = 0;
		
		for (unsigned i = 0; i < pair_count; ++i) {
			unsigned pair = pairs[i];
			
			unsigned a = collider_bodies[pair & 0xffff];
			unsigned b = collider_bodies[pair >> 16];
			
			if (a == b) {
				++removed;
				continue;
			}
			
			unsigned set = sets[a] | sets[b];
			
			if (active[set]) {
				pairs[i-removed] = pair;
			}
			else {
				unsigned a = collider_tags[pair & 0xffff];
				unsigned b = collider_tags[pair >> 16];
				
				contacts->sleeping_pairs[contacts->sleeping_count++] = a > b ? a | (b << 16): b | (a << 16);
				++removed;
			}
		}
		
		pair_count -= removed;
	}
	
	uint32_t bucket_sizes[4] = {};
	
	for (unsigned i = 0; i < pair_count; ++i) {
		unsigned pair = pairs[i];
		
		unsigned a = pair & 0xffff;
		unsigned b = pair >> 16;
		
		a = a >= colliders.boxes.count ? 1 : 0;
		b = b >= colliders.boxes.count ? 2 : 0;
		
		unsigned ab = a | b;
		
		++bucket_sizes[ab];
	}
	
	uint32_t bucket_offsets[4] = {
		0,
		((bucket_sizes[0] + 7) & ~3),
		((bucket_sizes[0] + 7) & ~3) + bucket_sizes[1],
		((bucket_sizes[0] + 7) & ~3) + bucket_sizes[1] + bucket_sizes[2],
	};
	
	uint32_t written_per_bucket[4] = { bucket_offsets[0], bucket_offsets[1], bucket_offsets[2], bucket_offsets[3] };
	
	uint32_t* partitioned_pairs = allocate_array<uint32_t>(&temporary, pair_count + 7, 16); // Padding is required.
	
	for (unsigned i = 0; i < pair_count; ++i) {
		unsigned pair = pairs[i];
		
		unsigned a = pair & 0xffff;
		unsigned b = pair >> 16;
		
		a = a >= colliders.boxes.count ? 1 : 0;
		b = b >= colliders.boxes.count ? 2 : 0;
		
		unsigned ab = a | b;
		
		partitioned_pairs[written_per_bucket[ab]++] = pair;
	}
	
	for (unsigned i = 0; i < bucket_sizes[2]; ++i) {
		unsigned index = bucket_offsets[2] + i;
		unsigned pair = partitioned_pairs[index];
		
		partitioned_pairs[index] = (pair >> 16) | (pair << 16);
	}
	
	contacts->count += box_box_collide(partitioned_pairs, bucket_sizes[0], colliders.boxes.data, colliders.boxes.transforms, contacts->data + contacts->count, contacts->bodies + contacts->count, contacts->tags + contacts->count, temporary);
	
	// TODO: SIMD-optimize this loop.
	for (unsigned i = 0; i < bucket_sizes[1] + bucket_sizes[2]; ++i) {
		unsigned pair = partitioned_pairs[bucket_offsets[1] + i];
		
		unsigned a = pair >> 16;
		unsigned b = pair & 0xffff;
		
		b -= colliders.boxes.count;
		
		BoxCollider box = colliders.boxes.data[a];
		SphereCollider sphere = colliders.spheres.data[b];
		
		contacts->tags[contacts->count] = (uint64_t)((colliders.boxes.transforms[a].body >> 16) | (colliders.spheres.transforms[b].body & 0xffff0000)) << 32;
		contacts->count += box_sphere_collide(box, sphere, colliders.boxes.transforms[a], colliders.spheres.transforms[b], contacts->data + contacts->count, contacts->bodies + contacts->count);
	}
	
	// TODO: SIMD-optimize this loop.
	for (unsigned i = 0; i < bucket_sizes[3]; ++i) {
		unsigned pair = partitioned_pairs[bucket_offsets[3] + i];
		
		unsigned a = pair >> 16;
		unsigned b = pair & 0xffff;
		
		a -= colliders.boxes.count;
		b -= colliders.boxes.count;
		
		SphereCollider sphere_a = colliders.spheres.data[a];
		SphereCollider sphere_b = colliders.spheres.data[b];
		
		contacts->tags[contacts->count] = (uint64_t)((colliders.spheres.transforms[a].body >> 16) | (colliders.spheres.transforms[b].body & 0xffff0000)) << 32;
		contacts->count += sphere_sphere_collide(sphere_a, sphere_b, colliders.spheres.transforms[a], colliders.spheres.transforms[b], contacts->data + contacts->count, contacts->bodies + contacts->count);
	}
	
	// Discard islands of inactive objects at a fine level.
	{
		NUDGE_ARENA_SCOPE(temporary);
		
		// Find connected sets.
		uint16_t* heights = allocate_array<uint16_t>(&temporary, bodies.count, 16);
		uint16_t* parents = allocate_array<uint16_t>(&temporary, bodies.count, 16);
		
		memset(heights, 0, sizeof(heights[0])*bodies.count);
		memset(parents, 0xff, sizeof(parents[0])*bodies.count);
		
		for (unsigned i = 0; i < body_connections.count; ++i) {
			BodyPair pair = body_connections.data[i];
			
			unsigned a = pair.a;
			unsigned b = pair.b;
			
			// Body 0 is the static world and is ignored.
			if (!a || !b)
				continue;
			
			// Determine the root of a and b.
			unsigned a_root = a;
			unsigned a_parent = parents[a];
			
			for (unsigned parent = a_parent; parent != 0xffff; parent = parents[a_root])
				a_root = parent;
			
			unsigned b_root = b;
			unsigned b_parent = parents[b];
			
			for (unsigned parent = b_parent; parent != 0xffff; parent = parents[b_root])
				b_root = parent;
			
			if (a_root == b_root)
				continue;
			
			// Put a and b under the same root.
			unsigned a_height = heights[a_root];
			unsigned b_height = heights[b_root];
			
			unsigned root;
			
			if (a_height < b_height) {
				parents[a_root] = b_root;
				root = b_root;
			}
			else {
				parents[b_root] = a_root;
				root = a_root;
			}
			
			if (a_height == b_height) // Height of subtree increased.
				heights[a_root] = a_height+1;
			
			// Propagate the root to make subsequent iterations faster.
			if (a_root != a) {
				while (a_parent != a_root) {
					unsigned next = parents[a_parent];
					parents[a] = root;
					
					a = a_parent;
					a_parent = next;
				}
			}
			
			if (b_root != b) {
				while (b_parent != b_root) {
					unsigned next = parents[b_parent];
					parents[b] = root;
					
					b = b_parent;
					b_parent = next;
				}
			}
		}
		
		for (unsigned i = 0; i < contacts->count; ) {
			unsigned a = contacts->bodies[i].a;
			unsigned b = contacts->bodies[i].b;
			
			do {
				++i;
			}
			while (i < contacts->count && contacts->bodies[i].a == a && contacts->bodies[i].b == b);
			
			// Body 0 is the static world and is ignored.
			if (!a || !b)
				continue;
			
			// Determine the root of a and b.
			unsigned a_root = a;
			unsigned a_parent = parents[a];
			
			for (unsigned parent = a_parent; parent != 0xffff; parent = parents[a_root])
				a_root = parent;
			
			unsigned b_root = b;
			unsigned b_parent = parents[b];
			
			for (unsigned parent = b_parent; parent != 0xffff; parent = parents[b_root])
				b_root = parent;
			
			if (a_root == b_root)
				continue;
			
			// Put a and b under the same root.
			unsigned a_height = heights[a_root];
			unsigned b_height = heights[b_root];
			
			unsigned root;
			
			if (a_height < b_height) {
				parents[a_root] = b_root;
				root = b_root;
			}
			else {
				parents[b_root] = a_root;
				root = a_root;
			}
			
			if (a_height == b_height) // Height of subtree increased.
				heights[a_root] = a_height+1;
			
			// Propagate the root to make subsequent iterations faster.
			if (a_root != a) {
				while (a_parent != a_root) {
					unsigned next = parents[a_parent];
					parents[a] = root;
					
					a = a_parent;
					a_parent = next;
				}
			}
			
			if (b_root != b) {
				while (b_parent != b_root) {
					unsigned next = parents[b_parent];
					parents[b] = root;
					
					b = b_parent;
					b_parent = next;
				}
			}
		}
		
		// Identify a numbered set for each body.
		unsigned set_count = 0;
		uint16_t* sets = heights;
		memset(sets, 0xff, sizeof(sets[0])*bodies.count);
		
		for (unsigned i = 1; i < bodies.count; ++i) {
			unsigned root = parents[i];
			
			for (unsigned parent = root; parent != 0xffff; parent = parents[root])
				root = parent;
			
			if (root == 0xffff)
				root = i;
			
			if (sets[root] == 0xffff)
				sets[root] = set_count++;
			
			sets[i] = sets[root];
		}
		
		sets[0] = 0;
		
		// Determine active sets.
		uint8_t* active = allocate_array<uint8_t>(&temporary, set_count, 16);
		memset(active, 0, sizeof(active[0])*set_count);
		
		for (unsigned i = 1; i < bodies.count; ++i) {
			if (bodies.idle_counters[i] != 0xff)
				active[sets[i]] = 1;
		}
		
		// Determine active bodies.
		for (unsigned i = 1; i < bodies.count; ++i) {
			unsigned set = sets[i];
			
			if (active[set])
				active_bodies->indices[active_bodies->count++] = i;
		}
		
		// Remove inactive contacts.
		unsigned removed = 0;
		
		for (unsigned i = 0; i < contacts->count; ) {
			unsigned a = contacts->bodies[i].a;
			unsigned b = contacts->bodies[i].b;
			unsigned tag = contacts->tags[i] >> 32;
			
			unsigned span = 0;
			
			do {
				++span;
			}
			while (i+span < contacts->count && (contacts->tags[i+span] >> 32) == tag);
			
			unsigned set = sets[a] | sets[b];
			
			if (active[set]) {
				for (unsigned j = 0; j < span; ++j) {
					contacts->tags[i+j-removed] = contacts->tags[i+j];
					contacts->data[i+j-removed] = contacts->data[i+j];
					contacts->bodies[i+j-removed] = contacts->bodies[i+j];
				}
			}
			else {
				contacts->sleeping_pairs[contacts->sleeping_count++] = tag;
				removed += span;
			}
			
			i += span;
		}
		
		contacts->count -= removed;
	}
	
	radix_sort_uint32(contacts->sleeping_pairs, contacts->sleeping_count, temporary);
}

struct ContactImpulseData {
	uint32_t* sorted_contacts;
	
	CachedContactImpulse* culled_data;
	uint64_t* culled_tags;
	unsigned culled_count;
	
	CachedContactImpulse* data;
};

ContactImpulseData* read_cached_impulses(ContactCache contact_cache, ContactData contacts, Arena* memory) {
	ContactImpulseData* data = allocate_struct<ContactImpulseData>(memory, 64);
	
	// Sort contacts based on tag so that they can be quickly matched against the contact cache.
	uint32_t* sorted_contacts = allocate_array<uint32_t>(memory, contacts.count, 16);
	data->sorted_contacts = sorted_contacts;
	{
		Arena temporary = *memory;
		uint32_t* contact_keys = allocate_array<uint32_t>(&temporary, contacts.count, 16);
		
		for (unsigned i = 0; i < contacts.count; ++i) {
			sorted_contacts[i] = i;
			contact_keys[i] = (uint32_t)contacts.tags[i];
		}
		
		radix_sort_uint32_x2(contact_keys, sorted_contacts, contacts.count, temporary);
		
		for (unsigned i = 0; i < contacts.count; ++i) {
			unsigned index = sorted_contacts[i];
			contact_keys[i] = (uint32_t)(contacts.tags[index] >> 32);
		}
		
		radix_sort_uint32_x2(contact_keys, sorted_contacts, contacts.count, temporary);
	}
	
	// Gather warm start impulses and store away culled impulses for sleeping pairs.
	CachedContactImpulse* culled_data = allocate_array<CachedContactImpulse>(memory, contact_cache.count, 16);
	uint64_t* culled_tags = allocate_array<uint64_t>(memory, contact_cache.count, 16);
	unsigned culled_count = 0;
	
	CachedContactImpulse* contact_impulses = allocate_array<CachedContactImpulse>(memory, contacts.count, 32);
	data->data = contact_impulses;
	
	unsigned cached_contact_offset = 0;
	unsigned sleeping_pair_offset = 0;
	
	for (unsigned i = 0; i < contacts.count; ++i) {
		unsigned index = sorted_contacts[i];
		uint64_t tag = contacts.tags[index];
		
		CachedContactImpulse cached_impulse = {};
		
		uint64_t cached_tag;
		while (cached_contact_offset < contact_cache.count && (cached_tag = contact_cache.tags[cached_contact_offset]) < tag) {
			unsigned cached_pair = cached_tag >> 32;
			
			while (sleeping_pair_offset < contacts.sleeping_count && contacts.sleeping_pairs[sleeping_pair_offset] < cached_pair)
				++sleeping_pair_offset;
			
			if (sleeping_pair_offset < contacts.sleeping_count && contacts.sleeping_pairs[sleeping_pair_offset] == cached_pair) {
				culled_data[culled_count] = contact_cache.data[cached_contact_offset];
				culled_tags[culled_count] = contact_cache.tags[cached_contact_offset];
				++culled_count;
			}
			
			++cached_contact_offset;
		}
		
		if (cached_contact_offset < contact_cache.count && contact_cache.tags[cached_contact_offset] == tag)
			cached_impulse = contact_cache.data[cached_contact_offset];
		
		contact_impulses[index] = cached_impulse;
	}
	
	for (; cached_contact_offset < contact_cache.count && sleeping_pair_offset < contacts.sleeping_count; ) {
		unsigned a = contact_cache.tags[cached_contact_offset] >> 32;
		unsigned b = contacts.sleeping_pairs[sleeping_pair_offset];
		
		if (a < b) {
			++cached_contact_offset;
		}
		else if (a == b) {
			culled_data[culled_count] = contact_cache.data[cached_contact_offset];
			culled_tags[culled_count] = contact_cache.tags[cached_contact_offset];
			++culled_count;
			++cached_contact_offset;
		}
		else {
			++sleeping_pair_offset;
		}
	}
	
	data->culled_data = culled_data;
	data->culled_tags = culled_tags;
	data->culled_count = culled_count;
	
	return data;
}

void write_cached_impulses(ContactCache* contact_cache, ContactData contacts, ContactImpulseData* contact_impulses) {
	uint32_t* sorted_contacts = contact_impulses->sorted_contacts;
	
	CachedContactImpulse* culled_data = contact_impulses->culled_data;
	uint64_t* culled_tags = contact_impulses->culled_tags;
	unsigned culled_count = contact_impulses->culled_count;
	
	// Cache impulses.
	assert(contact_cache->capacity >= contacts.count + culled_count); // Out of space in contact cache.
	contact_cache->count = contacts.count + culled_count;
	{
		// Pick sort from contacts and culled impulses.
		unsigned i = 0, j = 0, k = 0;
		
		while (i < contacts.count && j < culled_count) {
			unsigned index = sorted_contacts[i];
			
			uint64_t a = contacts.tags[index];
			uint64_t b = culled_tags[j];
			
			if (a < b) {
				contact_cache->tags[k] = contacts.tags[index];
				contact_cache->data[k] = contact_impulses->data[index];
				++i;
			}
			else {
				contact_cache->tags[k] = culled_tags[j];
				contact_cache->data[k] = culled_data[j];
				++j;
			}
			
			++k;
		}
		
		for (; i < contacts.count; ++i) {
			unsigned index = sorted_contacts[i];
			
			contact_cache->tags[k] = contacts.tags[index];
			contact_cache->data[k] = contact_impulses->data[index];
			++k;
		}
		
		for (; j < culled_count; ++j) {
			contact_cache->tags[k] = culled_tags[j];
			contact_cache->data[k] = culled_data[j];
			++k;
		}
	}
}

struct ContactConstraintData {
	unsigned contact_count;
	InertiaTransform* momentum_to_velocity;
	uint32_t* constraint_to_contact;
	
	ContactConstraintV* constraints;
	ContactConstraintStateV* constraint_states;
	unsigned constraint_batches;
};

ContactConstraintData* setup_contact_constraints(ActiveBodies active_bodies, ContactData contacts, BodyData bodies, ContactImpulseData* contact_impulses, Arena* memory) {
	// TODO: We should investigate better evaluation order for contacts.
	uint32_t* contact_order = contact_impulses->sorted_contacts;
	
	ContactConstraintData* data = allocate_struct<ContactConstraintData>(memory, 64);
	data->contact_count = contacts.count;
	
	InertiaTransform* momentum_to_velocity = allocate_array<InertiaTransform>(memory, bodies.count, 32);
	data->momentum_to_velocity = momentum_to_velocity;
	
	// TODO: Consider SIMD-optimizing this loop.
	// TODO: Don't compute anything for inactive bodies.
	for (unsigned i = 0; i < bodies.count; ++i) {
		Rotation rotation = make_rotation(bodies.transforms[i].rotation);
		float3 inertia_inverse = make_float3(bodies.properties[i].inertia_inverse);
		
		float3x3 m = matrix(rotation);
		
		InertiaTransform transform = {};
		
		transform.xx = inertia_inverse.x*m.c0.x*m.c0.x + inertia_inverse.y*m.c1.x*m.c1.x + inertia_inverse.z*m.c2.x*m.c2.x;
		transform.yy = inertia_inverse.x*m.c0.y*m.c0.y + inertia_inverse.y*m.c1.y*m.c1.y + inertia_inverse.z*m.c2.y*m.c2.y;
		transform.zz = inertia_inverse.x*m.c0.z*m.c0.z + inertia_inverse.y*m.c1.z*m.c1.z + inertia_inverse.z*m.c2.z*m.c2.z;
		transform.xy = inertia_inverse.x*m.c0.x*m.c0.y + inertia_inverse.y*m.c1.x*m.c1.y + inertia_inverse.z*m.c2.x*m.c2.y;
		transform.xz = inertia_inverse.x*m.c0.x*m.c0.z + inertia_inverse.y*m.c1.x*m.c1.z + inertia_inverse.z*m.c2.x*m.c2.z;
		transform.yz = inertia_inverse.x*m.c0.y*m.c0.z + inertia_inverse.y*m.c1.y*m.c1.z + inertia_inverse.z*m.c2.y*m.c2.z;
		
		momentum_to_velocity[i] = transform;
		bodies.momentum[i].unused0 = bodies.properties[i].mass_inverse;
	}
	
	CachedContactImpulse* impulses = contact_impulses->data;
	
	uint32_t* constraint_to_contact = allocate_array<uint32_t>(memory, contacts.count*simdv_width32, 32);
	data->constraint_to_contact = constraint_to_contact;
	
	// Schedule contacts so there are no conflicts within a SIMD width.
	ContactSlotV* contact_slots = reserve_array<ContactSlotV>(memory, contacts.count, 32);
	unsigned contact_slot_count = 0;
	{
		Arena temporary = *memory;
		commit_array<ContactSlotV>(&temporary, contacts.count);
		
		static const unsigned bucket_count = 16;
		
		ContactPairV* vacant_pair_buckets[bucket_count];
		ContactSlotV* vacant_slot_buckets[bucket_count];
		unsigned bucket_vacancy_count[bucket_count] = {};
		
		simd4_int32 invalid_index = simd_isplat(~0u);
		
		for (unsigned i = 0; i < bucket_count; ++i) {
			vacant_pair_buckets[i] = allocate_array<ContactPairV>(&temporary, contacts.count+1, 32);
			vacant_slot_buckets[i] = allocate_array<ContactSlotV>(&temporary, contacts.count, 32);
			
			// Add padding with invalid data so we don't have to range check.
			simd_st((int32_t*)vacant_pair_buckets[i]->ab, invalid_index);
		}
		
		for (unsigned i = 0; i < contacts.count; ++i) {
			unsigned index = contact_order[i];
			BodyPair bodies = contacts.bodies[index];
			
			unsigned bucket = i % bucket_count;
			ContactPairV* vacant_pairs = vacant_pair_buckets[bucket];
			ContactSlotV* vacant_slots = vacant_slot_buckets[bucket];
			unsigned vacancy_count = bucket_vacancy_count[bucket];
			
			// Ignore dependencies on body 0.
			unsigned ca = bodies.a ? bodies.a : bodies.b;
			unsigned cb = bodies.b ? bodies.b : bodies.a;
			
			simd128_t a = simd_isplat((ca << 16) | ca);
			simd128_t b = simd_isplat((cb << 16) | cb);
			
			simd128_t scheduled_a_b;
			
			unsigned j = 0;
			
			for (;; ++j) {
				scheduled_a_b = simd_ld(vacant_pairs[j].ab);
				
				simd128_t conflict = simd_pack_i16_to_i8(simd_i16_cmpeq(a, scheduled_a_b), simd_i16_cmpeq(b, scheduled_a_b));
				
				if (!simd_i8_mask(conflict))
					break;
			}
			
			unsigned lane = first_set_bit((unsigned)simd::cmpmask32(simd_icmpeq(scheduled_a_b, invalid_index)));
			ContactSlotV* slot = vacant_slots + j;
			ContactPairV* pair = vacant_pairs + j;
			
			slot->indices[lane] = index;
			
			simd_stx((float*)pair->ab + lane, simd_shuf_xAyBzCwD(a, b));
			
			if (j == vacancy_count) {
				++vacancy_count;
			}
			else if (lane == simdv_width32-1) {
				simd4_int32 indices = simd_ld((const int32_t*)slot->indices);
				
				--vacancy_count;
				
				ContactPairV* last_pair = vacant_pairs + vacancy_count;
				ContactSlotV* last_slot = vacant_slots + vacancy_count;
				
				simd_st((int32_t*)contact_slots[contact_slot_count++].indices, indices);
				
				*pair = *last_pair;
				*slot = *last_slot;
			}
			else {
				continue;
			}
			
			// Store count and maintain padding.
			bucket_vacancy_count[bucket] = vacancy_count;
			simd_st((int32_t*)vacant_pairs[vacancy_count].ab, invalid_index);
		}
		
		for (unsigned i = 0; i < bucket_count; ++i) {
			ContactPairV* vacant_pairs = vacant_pair_buckets[i];
			ContactSlotV* vacant_slots = vacant_slot_buckets[i];
			unsigned vacancy_count = bucket_vacancy_count[i];
			
			// Replace any unset indices with the first one, which is always valid.
			// This is safe because the slots will just overwrite each other.
			for (unsigned i = 0; i < vacancy_count; ++i) {
				simd4_int32 ab = simd_ld((int32_t*)vacant_pairs[i].ab);
				simd4_int32 indices = simd_ld((const int32_t*)vacant_slots[i].indices);
				
				simd4_int32 mask = simd_icmpeq(ab, invalid_index);
				simd4_int32 first_index = simd_swiz_xxxx(indices);
				
				indices = simd::blendv32(indices, first_index, mask);
				
				simd_st((int32_t*)contact_slots[contact_slot_count++].indices, indices);
			}
		}
	}
	commit_array<ContactSlotV>(memory, contact_slot_count);
	
	ContactConstraintV* constraints = allocate_array<ContactConstraintV>(memory, contact_slot_count, 32);
	ContactConstraintStateV* constraint_states = allocate_array<ContactConstraintStateV>(memory, contact_slot_count, 32);
	
	data->constraints = constraints;
	data->constraint_states = constraint_states;
	
	memset(constraint_states, 0, sizeof(ContactConstraintStateV)*contact_slot_count);
	
	for (unsigned i = 0; i < contact_slot_count; ++i) {
		ContactSlotV slot = contact_slots[i];
		
		for (unsigned j = 0; j < simdv_width32; ++j)
			constraint_to_contact[i*simdv_width32 + j] = slot.indices[j];
		
		simd4_float position_x, position_y, position_z, penetration;
		simd4_float normal_x, normal_y, normal_z, friction;
		load8<sizeof(contacts.data[0]), 1>((const float*)contacts.data, slot.indices,
										   position_x, position_y, position_z, penetration,
										   normal_x, normal_y, normal_z, friction);
		
		NUDGE_SIMDV_ALIGNED uint16_t ab_array[simdv_width32*2];
		
		for (unsigned j = 0; j < simdv_width32; ++j) {
			BodyPair pair = contacts.bodies[slot.indices[j]];
			ab_array[j*2 + 0] = pair.a;
			ab_array[j*2 + 1] = pair.b;
		}
		
		unsigned a0 = ab_array[0]; unsigned a1 = ab_array[2]; unsigned a2 = ab_array[4]; unsigned a3 = ab_array[6];
		unsigned b0 = ab_array[1]; unsigned b1 = ab_array[3]; unsigned b2 = ab_array[5]; unsigned b3 = ab_array[7];
		
		simd4_float a_mass_inverse = simd_ld(bodies.momentum[a0].unused0, bodies.momentum[a1].unused0, bodies.momentum[a2].unused0, bodies.momentum[a3].unused0);
		simd4_float b_mass_inverse = simd_ld(bodies.momentum[b0].unused0, bodies.momentum[b1].unused0, bodies.momentum[b2].unused0, bodies.momentum[b3].unused0);
		
		simd4_float a_position_x, a_position_y, a_position_z, a_position_w;
		simd4_float b_position_x, b_position_y, b_position_z, b_position_w;
		load4<sizeof(bodies.transforms[0]), 2>(bodies.transforms[0].position, ab_array,
											   a_position_x, a_position_y, a_position_z, a_position_w);
		load4<sizeof(bodies.transforms[0]), 2>(bodies.transforms[0].position, ab_array + 1,
											   b_position_x, b_position_y, b_position_z, b_position_w);
		
		simd4_float pa_x = position_x - a_position_x;
		simd4_float pa_y = position_y - a_position_y;
		simd4_float pa_z = position_z - a_position_z;
		
		simd4_float pb_x = position_x - b_position_x;
		simd4_float pb_y = position_y - b_position_y;
		simd4_float pb_z = position_z - b_position_z;
		
		simd4_float a_momentum_to_velocity_xx, a_momentum_to_velocity_yy, a_momentum_to_velocity_zz, a_momentum_to_velocity_u0;
		simd4_float a_momentum_to_velocity_xy, a_momentum_to_velocity_xz, a_momentum_to_velocity_yz, a_momentum_to_velocity_u1;
		load8<sizeof(momentum_to_velocity[0]), 2>((const float*)momentum_to_velocity, ab_array,
												  a_momentum_to_velocity_xx, a_momentum_to_velocity_yy, a_momentum_to_velocity_zz, a_momentum_to_velocity_u0,
												  a_momentum_to_velocity_xy, a_momentum_to_velocity_xz, a_momentum_to_velocity_yz, a_momentum_to_velocity_u1);
		
		simd4_float na_xt, na_yt, na_zt;
		simd_soa::cross(pa_x, pa_y, pa_z, normal_x, normal_y, normal_z, na_xt, na_yt, na_zt);
		
		simd4_float na_x = a_momentum_to_velocity_xx*na_xt + a_momentum_to_velocity_xy*na_yt + a_momentum_to_velocity_xz*na_zt;
		simd4_float na_y = a_momentum_to_velocity_xy*na_xt + a_momentum_to_velocity_yy*na_yt + a_momentum_to_velocity_yz*na_zt;
		simd4_float na_z = a_momentum_to_velocity_xz*na_xt + a_momentum_to_velocity_yz*na_yt + a_momentum_to_velocity_zz*na_zt;
		
		simd4_float b_momentum_to_velocity_xx, b_momentum_to_velocity_yy, b_momentum_to_velocity_zz, b_momentum_to_velocity_u0;
		simd4_float b_momentum_to_velocity_xy, b_momentum_to_velocity_xz, b_momentum_to_velocity_yz, b_momentum_to_velocity_u1;
		load8<sizeof(momentum_to_velocity[0]), 2>((const float*)momentum_to_velocity, ab_array + 1,
												  b_momentum_to_velocity_xx, b_momentum_to_velocity_yy, b_momentum_to_velocity_zz, b_momentum_to_velocity_u0,
												  b_momentum_to_velocity_xy, b_momentum_to_velocity_xz, b_momentum_to_velocity_yz, b_momentum_to_velocity_u1);
		
		simd4_float nb_xt, nb_yt, nb_zt;
		simd_soa::cross(pb_x, pb_y, pb_z, normal_x, normal_y, normal_z, nb_xt, nb_yt, nb_zt);
		
		simd4_float nb_x = b_momentum_to_velocity_xx*nb_xt + b_momentum_to_velocity_xy*nb_yt + b_momentum_to_velocity_xz*nb_zt;
		simd4_float nb_y = b_momentum_to_velocity_xy*nb_xt + b_momentum_to_velocity_yy*nb_yt + b_momentum_to_velocity_yz*nb_zt;
		simd4_float nb_z = b_momentum_to_velocity_xz*nb_xt + b_momentum_to_velocity_yz*nb_yt + b_momentum_to_velocity_zz*nb_zt;
		
		simd_soa::cross(na_x, na_y, na_z, pa_x, pa_y, pa_z, na_xt, na_yt, na_zt);
		simd_soa::cross(nb_x, nb_y, nb_z, pb_x, pb_y, pb_z, nb_xt, nb_yt, nb_zt);
		
		simd4_float normal_impulse_to_rotational_velocity_x = na_xt + nb_xt;
		simd4_float normal_impulse_to_rotational_velocity_y = na_yt + nb_yt;
		simd4_float normal_impulse_to_rotational_velocity_z = na_zt + nb_zt;
		
		simd4_float r_dot_n = normal_impulse_to_rotational_velocity_x*normal_x + normal_impulse_to_rotational_velocity_y*normal_y + normal_impulse_to_rotational_velocity_z*normal_z;
		
		simd4_float mass_inverse = a_mass_inverse + b_mass_inverse;
		simd4_float normal_velocity_to_normal_impulse = mass_inverse + r_dot_n;
		
		simd4_float nonzero = simd_cmpneq(normal_velocity_to_normal_impulse, simd_zero());
		normal_velocity_to_normal_impulse = simd_and(simd_div(simd_splat(-1.0f), normal_velocity_to_normal_impulse), nonzero);
		
		simd4_float bias = simd_splat(-bias_factor) * simd_max(penetration - simd_splat(allowed_penetration), simd_zero()) * normal_velocity_to_normal_impulse;
		
		// Compute a tangent from the normal. Care is taken to compute a smoothly varying basis to improve stability.
		simd4_float s = simd_abs(normal_x);
		
		simd4_float u_x = normal_z*s;
		simd4_float u_y = u_x - normal_z;
		simd4_float u_z = simd_madd(normal_x - normal_y, s, normal_y);
		
		u_x = simd_xor(u_x, simd_splat(-0.0f));
		simd_soa::normalize(u_x, u_y, u_z);
		
		// Compute the rest of the basis.
		simd4_float v_x, v_y, v_z;
		simd_soa::cross(u_x, u_y, u_z, normal_x, normal_y, normal_z, v_x, v_y, v_z);
		
		simd4_float ua_x, ua_y, ua_z, va_x, va_y, va_z;
		simd_soa::cross(pa_x, pa_y, pa_z, u_x, u_y, u_z, ua_x, ua_y, ua_z);
		simd_soa::cross(pa_x, pa_y, pa_z, v_x, v_y, v_z, va_x, va_y, va_z);
		
		simd4_float ub_x, ub_y, ub_z, vb_x, vb_y, vb_z;
		simd_soa::cross(pb_x, pb_y, pb_z, u_x, u_y, u_z, ub_x, ub_y, ub_z);
		simd_soa::cross(pb_x, pb_y, pb_z, v_x, v_y, v_z, vb_x, vb_y, vb_z);
		
		simd4_float a_duu = a_momentum_to_velocity_xx*ua_x*ua_x + a_momentum_to_velocity_yy*ua_y*ua_y + a_momentum_to_velocity_zz*ua_z*ua_z;
		simd4_float a_dvv = a_momentum_to_velocity_xx*va_x*va_x + a_momentum_to_velocity_yy*va_y*va_y + a_momentum_to_velocity_zz*va_z*va_z;
		simd4_float a_duv = a_momentum_to_velocity_xx*ua_x*va_x + a_momentum_to_velocity_yy*ua_y*va_y + a_momentum_to_velocity_zz*ua_z*va_z;
		
		simd4_float a_suu = a_momentum_to_velocity_xy*ua_x*ua_y + a_momentum_to_velocity_xz*ua_x*ua_z + a_momentum_to_velocity_yz*ua_y*ua_z;
		simd4_float a_svv = a_momentum_to_velocity_xy*va_x*va_y + a_momentum_to_velocity_xz*va_x*va_z + a_momentum_to_velocity_yz*va_y*va_z;
		simd4_float a_suv = a_momentum_to_velocity_xy*(ua_x*va_y + ua_y*va_x) + a_momentum_to_velocity_xz*(ua_x*va_z + ua_z*va_x) + a_momentum_to_velocity_yz*(ua_y*va_z + ua_z*va_y);
		
		simd4_float b_duu = b_momentum_to_velocity_xx*ub_x*ub_x + b_momentum_to_velocity_yy*ub_y*ub_y + b_momentum_to_velocity_zz*ub_z*ub_z;
		simd4_float b_dvv = b_momentum_to_velocity_xx*vb_x*vb_x + b_momentum_to_velocity_yy*vb_y*vb_y + b_momentum_to_velocity_zz*vb_z*vb_z;
		simd4_float b_duv = b_momentum_to_velocity_xx*ub_x*vb_x + b_momentum_to_velocity_yy*ub_y*vb_y + b_momentum_to_velocity_zz*ub_z*vb_z;
		
		simd4_float b_suu = b_momentum_to_velocity_xy*ub_x*ub_y + b_momentum_to_velocity_xz*ub_x*ub_z + b_momentum_to_velocity_yz*ub_y*ub_z;
		simd4_float b_svv = b_momentum_to_velocity_xy*vb_x*vb_y + b_momentum_to_velocity_xz*vb_x*vb_z + b_momentum_to_velocity_yz*vb_y*vb_z;
		simd4_float b_suv = b_momentum_to_velocity_xy*(ub_x*vb_y + ub_y*vb_x) + b_momentum_to_velocity_xz*(ub_x*vb_z + ub_z*vb_x) + b_momentum_to_velocity_yz*(ub_y*vb_z + ub_z*vb_y);
		
		simd4_float friction_x = mass_inverse + a_duu + a_suu + a_suu + b_duu + b_suu + b_suu;
		simd4_float friction_y = mass_inverse + a_dvv + a_svv + a_svv + b_dvv + b_svv + b_svv;
		simd4_float friction_z = a_duv + a_duv + a_suv + a_suv + b_duv + b_duv + b_suv + b_suv;
		
		simd4_float ua_xt = a_momentum_to_velocity_xx*ua_x + a_momentum_to_velocity_xy*ua_y + a_momentum_to_velocity_xz*ua_z;
		simd4_float ua_yt = a_momentum_to_velocity_xy*ua_x + a_momentum_to_velocity_yy*ua_y + a_momentum_to_velocity_yz*ua_z;
		simd4_float ua_zt = a_momentum_to_velocity_xz*ua_x + a_momentum_to_velocity_yz*ua_y + a_momentum_to_velocity_zz*ua_z;
		
		simd4_float va_xt = a_momentum_to_velocity_xx*va_x + a_momentum_to_velocity_xy*va_y + a_momentum_to_velocity_xz*va_z;
		simd4_float va_yt = a_momentum_to_velocity_xy*va_x + a_momentum_to_velocity_yy*va_y + a_momentum_to_velocity_yz*va_z;
		simd4_float va_zt = a_momentum_to_velocity_xz*va_x + a_momentum_to_velocity_yz*va_y + a_momentum_to_velocity_zz*va_z;
		
		simd4_float ub_xt = b_momentum_to_velocity_xx*ub_x + b_momentum_to_velocity_xy*ub_y + b_momentum_to_velocity_xz*ub_z;
		simd4_float ub_yt = b_momentum_to_velocity_xy*ub_x + b_momentum_to_velocity_yy*ub_y + b_momentum_to_velocity_yz*ub_z;
		simd4_float ub_zt = b_momentum_to_velocity_xz*ub_x + b_momentum_to_velocity_yz*ub_y + b_momentum_to_velocity_zz*ub_z;
		
		simd4_float vb_xt = b_momentum_to_velocity_xx*vb_x + b_momentum_to_velocity_xy*vb_y + b_momentum_to_velocity_xz*vb_z;
		simd4_float vb_yt = b_momentum_to_velocity_xy*vb_x + b_momentum_to_velocity_yy*vb_y + b_momentum_to_velocity_yz*vb_z;
		simd4_float vb_zt = b_momentum_to_velocity_xz*vb_x + b_momentum_to_velocity_yz*vb_y + b_momentum_to_velocity_zz*vb_z;
		
		constraints[i].a[0] = a0; constraints[i].a[1] = a1; constraints[i].a[2] = a2; constraints[i].a[3] = a3;
		constraints[i].b[0] = b0; constraints[i].b[1] = b1; constraints[i].b[2] = b2; constraints[i].b[3] = b3;
		
		simd_st(constraints[i].n_x, normal_x);
		simd_st(constraints[i].n_y, normal_y);
		simd_st(constraints[i].n_z, normal_z);
		
		simd_st(constraints[i].pa_x, pa_x);
		simd_st(constraints[i].pa_y, pa_y);
		simd_st(constraints[i].pa_z, pa_z);
		
		simd_st(constraints[i].pb_x, pb_x);
		simd_st(constraints[i].pb_y, pb_y);
		simd_st(constraints[i].pb_z, pb_z);
		
		simd_st(constraints[i].normal_velocity_to_normal_impulse, normal_velocity_to_normal_impulse);
		
		simd_st(constraints[i].bias, bias);
		simd_st(constraints[i].friction, friction);
		
		simd_st(constraints[i].u_x, u_x);
		simd_st(constraints[i].u_y, u_y);
		simd_st(constraints[i].u_z, u_z);
		
		simd_st(constraints[i].v_x, v_x);
		simd_st(constraints[i].v_y, v_y);
		simd_st(constraints[i].v_z, v_z);
		
		simd_st(constraints[i].friction_coefficient_x, friction_x);
		simd_st(constraints[i].friction_coefficient_y, friction_y);
		simd_st(constraints[i].friction_coefficient_z, friction_z);
		
		simd_st(constraints[i].ua_x, simd_neg(ua_xt));
		simd_st(constraints[i].ua_y, simd_neg(ua_yt));
		simd_st(constraints[i].ua_z, simd_neg(ua_zt));
		
		simd_st(constraints[i].va_x, simd_neg(va_xt));
		simd_st(constraints[i].va_y, simd_neg(va_yt));
		simd_st(constraints[i].va_z, simd_neg(va_zt));
		
		simd_st(constraints[i].na_x, simd_neg(na_x));
		simd_st(constraints[i].na_y, simd_neg(na_y));
		simd_st(constraints[i].na_z, simd_neg(na_z));
		
		simd_st(constraints[i].ub_x, ub_xt);
		simd_st(constraints[i].ub_y, ub_yt);
		simd_st(constraints[i].ub_z, ub_zt);
		
		simd_st(constraints[i].vb_x, vb_xt);
		simd_st(constraints[i].vb_y, vb_yt);
		simd_st(constraints[i].vb_z, vb_zt);
		
		simd_st(constraints[i].nb_x, nb_x);
		simd_st(constraints[i].nb_y, nb_y);
		simd_st(constraints[i].nb_z, nb_z);
		
		simd4_float cached_impulse_x, cached_impulse_y, cached_impulse_z, unused0;
		load4<sizeof(impulses[0]), 1>((const float*)impulses, slot.indices,
									  cached_impulse_x, cached_impulse_y, cached_impulse_z, unused0);
		
		simd4_float a_velocity_x, a_velocity_y, a_velocity_z;
		simd4_float a_angular_velocity_x, a_angular_velocity_y, a_angular_velocity_z, a_angular_velocity_w;
		load8<sizeof(bodies.momentum[0]), 1>((const float*)bodies.momentum, constraints[i].a,
											 a_velocity_x, a_velocity_y, a_velocity_z, a_mass_inverse,
											 a_angular_velocity_x, a_angular_velocity_y, a_angular_velocity_z, a_angular_velocity_w);
		
		simd4_float b_velocity_x, b_velocity_y, b_velocity_z;
		simd4_float b_angular_velocity_x, b_angular_velocity_y, b_angular_velocity_z, b_angular_velocity_w;
		load8<sizeof(bodies.momentum[0]), 1>((const float*)bodies.momentum, constraints[i].b,
											 b_velocity_x, b_velocity_y, b_velocity_z, b_mass_inverse,
											 b_angular_velocity_x, b_angular_velocity_y, b_angular_velocity_z, b_angular_velocity_w);
		
		simd4_float normal_impulse = simd_max(normal_x*cached_impulse_x + normal_y*cached_impulse_y + normal_z*cached_impulse_z, simd_zero());
		simd4_float max_friction_impulse = normal_impulse * friction;
		
		simd4_float friction_impulse_x = u_x*cached_impulse_x + u_y*cached_impulse_y + u_z*cached_impulse_z;
		simd4_float friction_impulse_y = v_x*cached_impulse_x + v_y*cached_impulse_y + v_z*cached_impulse_z;
		
		simd4_float friction_clamp_scale = friction_impulse_x*friction_impulse_x + friction_impulse_y*friction_impulse_y;
		
		friction_clamp_scale = simd_rsqrt_est(friction_clamp_scale);
		friction_clamp_scale = friction_clamp_scale * max_friction_impulse;
		friction_clamp_scale = simd_float::min_second_nan(simd_splat(1.0f), friction_clamp_scale); // Note: First operand is returned on NaN.
		
		friction_impulse_x = friction_impulse_x * friction_clamp_scale;
		friction_impulse_y = friction_impulse_y * friction_clamp_scale;
		
		simd4_float linear_impulse_x = friction_impulse_x*u_x + friction_impulse_y*v_x + normal_x * normal_impulse;
		simd4_float linear_impulse_y = friction_impulse_x*u_y + friction_impulse_y*v_y + normal_y * normal_impulse;
		simd4_float linear_impulse_z = friction_impulse_x*u_z + friction_impulse_y*v_z + normal_z * normal_impulse;
		
		simd4_float a_angular_impulse_x = friction_impulse_x * simd_ld(constraints[i].ua_x) + friction_impulse_y * simd_ld(constraints[i].va_x) + normal_impulse * simd_ld(constraints[i].na_x);
		simd4_float a_angular_impulse_y = friction_impulse_x * simd_ld(constraints[i].ua_y) + friction_impulse_y * simd_ld(constraints[i].va_y) + normal_impulse * simd_ld(constraints[i].na_y);
		simd4_float a_angular_impulse_z = friction_impulse_x * simd_ld(constraints[i].ua_z) + friction_impulse_y * simd_ld(constraints[i].va_z) + normal_impulse * simd_ld(constraints[i].na_z);
		
		simd4_float b_angular_impulse_x = friction_impulse_x * simd_ld(constraints[i].ub_x) + friction_impulse_y * simd_ld(constraints[i].vb_x) + normal_impulse * simd_ld(constraints[i].nb_x);
		simd4_float b_angular_impulse_y = friction_impulse_x * simd_ld(constraints[i].ub_y) + friction_impulse_y * simd_ld(constraints[i].vb_y) + normal_impulse * simd_ld(constraints[i].nb_y);
		simd4_float b_angular_impulse_z = friction_impulse_x * simd_ld(constraints[i].ub_z) + friction_impulse_y * simd_ld(constraints[i].vb_z) + normal_impulse * simd_ld(constraints[i].nb_z);
		
		a_velocity_x = simd_sub(a_velocity_x, linear_impulse_x * a_mass_inverse);
		a_velocity_y = simd_sub(a_velocity_y, linear_impulse_y * a_mass_inverse);
		a_velocity_z = simd_sub(a_velocity_z, linear_impulse_z * a_mass_inverse);
		
		a_angular_velocity_x = simd_add(a_angular_velocity_x, a_angular_impulse_x);
		a_angular_velocity_y = simd_add(a_angular_velocity_y, a_angular_impulse_y);
		a_angular_velocity_z = simd_add(a_angular_velocity_z, a_angular_impulse_z);
		
		b_velocity_x = simd_madd(linear_impulse_x, b_mass_inverse, b_velocity_x);
		b_velocity_y = simd_madd(linear_impulse_y, b_mass_inverse, b_velocity_y);
		b_velocity_z = simd_madd(linear_impulse_z, b_mass_inverse, b_velocity_z);
		
		b_angular_velocity_x = simd_add(b_angular_velocity_x, b_angular_impulse_x);
		b_angular_velocity_y = simd_add(b_angular_velocity_y, b_angular_impulse_y);
		b_angular_velocity_z = simd_add(b_angular_velocity_z, b_angular_impulse_z);
		
		simd_st(constraint_states[i].applied_normal_impulse, normal_impulse);
		simd_st(constraint_states[i].applied_friction_impulse_x, friction_impulse_x);
		simd_st(constraint_states[i].applied_friction_impulse_y, friction_impulse_y);
		
		store8<sizeof(bodies.momentum[0]), 1>((float*)bodies.momentum, constraints[i].a,
											  a_velocity_x, a_velocity_y, a_velocity_z, a_mass_inverse,
											  a_angular_velocity_x, a_angular_velocity_y, a_angular_velocity_z, a_angular_velocity_w);
		
		store8<sizeof(bodies.momentum[0]), 1>((float*)bodies.momentum, constraints[i].b,
											  b_velocity_x, b_velocity_y, b_velocity_z, b_mass_inverse,
											  b_angular_velocity_x, b_angular_velocity_y, b_angular_velocity_z, b_angular_velocity_w);
	}
	
	data->constraint_batches = contact_slot_count;
	
	return data;
}

void apply_impulses(ContactConstraintData* data, BodyData bodies) {
	ContactConstraintV* constraints = data->constraints;
	ContactConstraintStateV* constraint_states = data->constraint_states;
	
	unsigned constraint_batches = data->constraint_batches;
	
	for (unsigned i = 0; i < constraint_batches; ++i) {
		const ContactConstraintV& constraint = constraints[i];
		
		simd4_float a_velocity_x, a_velocity_y, a_velocity_z, a_mass_inverse;
		simd4_float a_angular_velocity_x, a_angular_velocity_y, a_angular_velocity_z, a_angular_velocity_w;
		load8<sizeof(bodies.momentum[0]), 1>((const float*)bodies.momentum, constraint.a,
											 a_velocity_x, a_velocity_y, a_velocity_z, a_mass_inverse,
											 a_angular_velocity_x, a_angular_velocity_y, a_angular_velocity_z, a_angular_velocity_w);
		
		simd4_float pa_z = simd_ld(constraint.pa_z);
		simd4_float pa_x = simd_ld(constraint.pa_x);
		simd4_float pa_y = simd_ld(constraint.pa_y);
		
		simd4_float v_xa = simd_madd(a_angular_velocity_y, pa_z, a_velocity_x);
		simd4_float v_ya = simd_madd(a_angular_velocity_z, pa_x, a_velocity_y);
		simd4_float v_za = simd_madd(a_angular_velocity_x, pa_y, a_velocity_z);
		
		simd4_float b_velocity_x, b_velocity_y, b_velocity_z, b_mass_inverse;
		simd4_float b_angular_velocity_x, b_angular_velocity_y, b_angular_velocity_z, b_angular_velocity_w;
		load8<sizeof(bodies.momentum[0]), 1>((const float*)bodies.momentum, constraint.b,
											 b_velocity_x, b_velocity_y, b_velocity_z, b_mass_inverse,
											 b_angular_velocity_x, b_angular_velocity_y, b_angular_velocity_z, b_angular_velocity_w);
		
		simd4_float pb_z = simd_ld(constraint.pb_z);
		simd4_float pb_x = simd_ld(constraint.pb_x);
		simd4_float pb_y = simd_ld(constraint.pb_y);
		
		simd4_float v_xb = simd_madd(b_angular_velocity_y, pb_z, b_velocity_x);
		simd4_float v_yb = simd_madd(b_angular_velocity_z, pb_x, b_velocity_y);
		simd4_float v_zb = simd_madd(b_angular_velocity_x, pb_y, b_velocity_z);
		
		v_xa = simd_madd(b_angular_velocity_z, pb_y, v_xa);
		v_ya = simd_madd(b_angular_velocity_x, pb_z, v_ya);
		v_za = simd_madd(b_angular_velocity_y, pb_x, v_za);
		
		simd4_float n_x = simd_ld(constraint.n_x);
		simd4_float fu_x = simd_ld(constraint.u_x);
		simd4_float fv_x = simd_ld(constraint.v_x);
		
		v_xb = simd_madd(a_angular_velocity_z, pa_y, v_xb);
		v_yb = simd_madd(a_angular_velocity_x, pa_z, v_yb);
		v_zb = simd_madd(a_angular_velocity_y, pa_x, v_zb);
		
		simd4_float n_y = simd_ld(constraint.n_y);
		simd4_float fu_y = simd_ld(constraint.u_y);
		simd4_float fv_y = simd_ld(constraint.v_y);
		
		simd4_float v_x = v_xb - v_xa;
		simd4_float v_y = v_yb - v_ya;
		simd4_float v_z = v_zb - v_za;
		
		simd4_float t_z = n_x * v_x;
		simd4_float t_x = v_x * fu_x;
		simd4_float t_y = v_x * fv_x;
		
		simd4_float n_z = simd_ld(constraint.n_z);
		simd4_float fu_z = simd_ld(constraint.u_z);
		simd4_float fv_z = simd_ld(constraint.v_z);
		
		simd4_float normal_bias = simd_ld(constraint.bias);
		simd4_float old_normal_impulse = simd_ld(constraint_states[i].applied_normal_impulse);
		simd4_float normal_factor = simd_ld(constraint.normal_velocity_to_normal_impulse);
		
		t_z = simd_madd(n_y, v_y, t_z);
		t_x = simd_madd(v_y, fu_y, t_x);
		t_y = simd_madd(v_y, fv_y, t_y);
		
		normal_bias = normal_bias + old_normal_impulse;
		
		t_z = simd_madd(n_z, v_z, t_z);
		t_x = simd_madd(v_z, fu_z, t_x);
		t_y = simd_madd(v_z, fv_z, t_y);
		
		simd4_float normal_impulse = simd_madd(normal_factor, t_z, normal_bias);
		
		simd4_float t_xx = t_x*t_x;
		simd4_float t_yy = t_y*t_y;
		simd4_float t_xy = t_x*t_y;
		simd4_float tl2 = t_xx + t_yy;
		
		normal_impulse = simd_max(normal_impulse, simd_zero());
		
		t_x = simd_mul(t_x, tl2);
		t_y = simd_mul(t_y, tl2);
		
		simd_st(constraint_states[i].applied_normal_impulse, normal_impulse);
		
		simd4_float max_friction_impulse = normal_impulse * simd_ld(constraint.friction);
		normal_impulse = normal_impulse - old_normal_impulse;
		
		simd4_float friction_x = simd_ld(constraint.friction_coefficient_x);
		simd4_float friction_factor = t_xx * friction_x;
		simd4_float linear_impulse_x = n_x * normal_impulse;
		
		simd4_float friction_y = simd_ld(constraint.friction_coefficient_y);
		friction_factor = simd_madd(t_yy, friction_y, friction_factor);
		simd4_float linear_impulse_y = n_y * normal_impulse;
		
		simd4_float friction_z = simd_ld(constraint.friction_coefficient_z);
		friction_factor = simd_madd(t_xy, friction_z, friction_factor);
		simd4_float linear_impulse_z = n_z * normal_impulse;
		
		friction_factor = simd_rcp_est(friction_factor);
		
		simd4_float na_x = simd_ld(constraint.na_x);
		simd4_float na_y = simd_ld(constraint.na_y);
		simd4_float na_z = simd_ld(constraint.na_z);
		
		a_angular_velocity_x = simd_madd(na_x, normal_impulse, a_angular_velocity_x);
		a_angular_velocity_y = simd_madd(na_y, normal_impulse, a_angular_velocity_y);
		a_angular_velocity_z = simd_madd(na_z, normal_impulse, a_angular_velocity_z);
		
		simd4_float old_friction_impulse_x = simd_ld(constraint_states[i].applied_friction_impulse_x);
		simd4_float old_friction_impulse_y = simd_ld(constraint_states[i].applied_friction_impulse_y);
		
		friction_factor = simd_float::min_second_nan(simd_splat(1e+6f), friction_factor); // Note: First operand is returned on NaN.
		
		simd4_float friction_impulse_x = t_x*friction_factor;
		simd4_float friction_impulse_y = t_y*friction_factor;
		
		friction_impulse_x = old_friction_impulse_x - friction_impulse_x; // Note: Friction impulse has the wrong sign until this point. This is really an addition.
		friction_impulse_y = old_friction_impulse_y - friction_impulse_y;
		
		simd4_float friction_clamp_scale = friction_impulse_x*friction_impulse_x + friction_impulse_y*friction_impulse_y;
		
		simd4_float nb_x = simd_ld(constraint.nb_x);
		simd4_float nb_y = simd_ld(constraint.nb_y);
		simd4_float nb_z = simd_ld(constraint.nb_z);
		
		friction_clamp_scale = simd_rsqrt_est(friction_clamp_scale);
		
		b_angular_velocity_x = simd_madd(nb_x, normal_impulse, b_angular_velocity_x);
		b_angular_velocity_y = simd_madd(nb_y, normal_impulse, b_angular_velocity_y);
		b_angular_velocity_z = simd_madd(nb_z, normal_impulse, b_angular_velocity_z);
		
		friction_clamp_scale = friction_clamp_scale * max_friction_impulse;
		friction_clamp_scale = simd_float::min_second_nan(simd_splat(1.0f), friction_clamp_scale); // Note: First operand is returned on NaN.
		
		friction_impulse_x = friction_impulse_x * friction_clamp_scale;
		friction_impulse_y = friction_impulse_y * friction_clamp_scale;
		
		simd_st(constraint_states[i].applied_friction_impulse_x, friction_impulse_x);
		simd_st(constraint_states[i].applied_friction_impulse_y, friction_impulse_y);
		
		friction_impulse_x = simd_sub(friction_impulse_x, old_friction_impulse_x);
		friction_impulse_y = simd_sub(friction_impulse_y, old_friction_impulse_y);
		
		linear_impulse_x = simd_madd(fu_x, friction_impulse_x, linear_impulse_x);
		linear_impulse_y = simd_madd(fu_y, friction_impulse_x, linear_impulse_y);
		linear_impulse_z = simd_madd(fu_z, friction_impulse_x, linear_impulse_z);
		
		linear_impulse_x = simd_madd(fv_x, friction_impulse_y, linear_impulse_x);
		linear_impulse_y = simd_madd(fv_y, friction_impulse_y, linear_impulse_y);
		linear_impulse_z = simd_madd(fv_z, friction_impulse_y, linear_impulse_z);
		
		simd4_float a_mass_inverse_neg = simd_xor(a_mass_inverse, simd_splat(-0.0f));
		
		a_velocity_x = simd_madd(linear_impulse_x, a_mass_inverse_neg, a_velocity_x);
		a_velocity_y = simd_madd(linear_impulse_y, a_mass_inverse_neg, a_velocity_y);
		a_velocity_z = simd_madd(linear_impulse_z, a_mass_inverse_neg, a_velocity_z);
		
		simd4_float ua_x = simd_ld(constraint.ua_x);
		simd4_float ua_y = simd_ld(constraint.ua_y);
		simd4_float ua_z = simd_ld(constraint.ua_z);
		
		a_angular_velocity_x = simd_madd(ua_x, friction_impulse_x, a_angular_velocity_x);
		a_angular_velocity_y = simd_madd(ua_y, friction_impulse_x, a_angular_velocity_y);
		a_angular_velocity_z = simd_madd(ua_z, friction_impulse_x, a_angular_velocity_z);
		
		simd4_float va_x = simd_ld(constraint.va_x);
		simd4_float va_y = simd_ld(constraint.va_y);
		simd4_float va_z = simd_ld(constraint.va_z);
		
		a_angular_velocity_x = simd_madd(va_x, friction_impulse_y, a_angular_velocity_x);
		a_angular_velocity_y = simd_madd(va_y, friction_impulse_y, a_angular_velocity_y);
		a_angular_velocity_z = simd_madd(va_z, friction_impulse_y, a_angular_velocity_z);
		
		a_angular_velocity_w = simd_zero(); // Reduces register pressure.
		
		store8<sizeof(bodies.momentum[0]), 1>((float*)bodies.momentum, constraint.a,
											  a_velocity_x, a_velocity_y, a_velocity_z, a_mass_inverse,
											  a_angular_velocity_x, a_angular_velocity_y, a_angular_velocity_z, a_angular_velocity_w);
		
		b_velocity_x = simd_madd(linear_impulse_x, b_mass_inverse, b_velocity_x);
		b_velocity_y = simd_madd(linear_impulse_y, b_mass_inverse, b_velocity_y);
		b_velocity_z = simd_madd(linear_impulse_z, b_mass_inverse, b_velocity_z);
		
		simd4_float ub_x = simd_ld(constraint.ub_x);
		simd4_float ub_y = simd_ld(constraint.ub_y);
		simd4_float ub_z = simd_ld(constraint.ub_z);
		
		b_angular_velocity_x = simd_madd(ub_x, friction_impulse_x, b_angular_velocity_x);
		b_angular_velocity_y = simd_madd(ub_y, friction_impulse_x, b_angular_velocity_y);
		b_angular_velocity_z = simd_madd(ub_z, friction_impulse_x, b_angular_velocity_z);
		
		simd4_float vb_x = simd_ld(constraint.vb_x);
		simd4_float vb_y = simd_ld(constraint.vb_y);
		simd4_float vb_z = simd_ld(constraint.vb_z);
		
		b_angular_velocity_x = simd_madd(vb_x, friction_impulse_y, b_angular_velocity_x);
		b_angular_velocity_y = simd_madd(vb_y, friction_impulse_y, b_angular_velocity_y);
		b_angular_velocity_z = simd_madd(vb_z, friction_impulse_y, b_angular_velocity_z);
		
		b_angular_velocity_w = simd_zero(); // Reduces register pressure.
		
		store8<sizeof(bodies.momentum[0]), 1>((float*)bodies.momentum, constraint.b,
											  b_velocity_x, b_velocity_y, b_velocity_z, b_mass_inverse,
											  b_angular_velocity_x, b_angular_velocity_y, b_angular_velocity_z, b_angular_velocity_w);
	}
}

void update_cached_impulses(ContactConstraintData* data, ContactImpulseData* contact_impulses) {
	uint32_t* constraint_to_contact = data->constraint_to_contact;
	
	ContactConstraintV* constraints = data->constraints;
	ContactConstraintStateV* constraint_states = data->constraint_states;
	unsigned constraint_count = data->constraint_batches * simdv_width32;
	
	for (unsigned i = 0; i < constraint_count; ++i) {
		unsigned contact = constraint_to_contact[i];
		
		unsigned b = i >> simdv_width32_log2;
		unsigned l = i & (simdv_width32-1);
		
		float* impulse = contact_impulses->data[contact].impulse;
		
		impulse[0] = (constraint_states[b].applied_normal_impulse[l] * constraints[b].n_x[l] +
					  constraint_states[b].applied_friction_impulse_x[l] * constraints[b].u_x[l] +
					  constraint_states[b].applied_friction_impulse_y[l] * constraints[b].v_x[l]);
		
		impulse[1] = (constraint_states[b].applied_normal_impulse[l] * constraints[b].n_y[l] +
					  constraint_states[b].applied_friction_impulse_x[l] * constraints[b].u_y[l] +
					  constraint_states[b].applied_friction_impulse_y[l] * constraints[b].v_y[l]);
		
		impulse[2] = (constraint_states[b].applied_normal_impulse[l] * constraints[b].n_z[l] +
					  constraint_states[b].applied_friction_impulse_x[l] * constraints[b].u_z[l] +
					  constraint_states[b].applied_friction_impulse_y[l] * constraints[b].v_z[l]);
	}
}

void advance(ActiveBodies active_bodies, BodyData bodies, float time_step) {
	float half_time_step = 0.5f * time_step;
	
	// TODO: Consider SIMD-optimizing this loop.
	for (unsigned n = 0; n < active_bodies.count; ++n) {
		unsigned i = active_bodies.indices[n];
		
		float3 velocity = make_float3(bodies.momentum[i].velocity);
		float3 angular_velocity = make_float3(bodies.momentum[i].angular_velocity);
		
		if (length2(velocity) < 1e-2f && length2(angular_velocity) < 1e-1f) {
			if (bodies.idle_counters[i] < 0xff)
				++bodies.idle_counters[i];
		}
		else {
			bodies.idle_counters[i] = 0;
		}
		
		Rotation dr = { angular_velocity };
		
		dr = dr * make_rotation(bodies.transforms[i].rotation);
		dr.v *= half_time_step;
		dr.s *= half_time_step;
		
		bodies.transforms[i].position[0] += velocity.x * time_step;
		bodies.transforms[i].position[1] += velocity.y * time_step;
		bodies.transforms[i].position[2] += velocity.z * time_step;
		
		bodies.transforms[i].rotation[0] += dr.v.x;
		bodies.transforms[i].rotation[1] += dr.v.y;
		bodies.transforms[i].rotation[2] += dr.v.z;
		bodies.transforms[i].rotation[3] += dr.s;
		
		Rotation rotation = normalize(make_rotation(bodies.transforms[i].rotation));
		
		bodies.transforms[i].rotation[0] = rotation.v.x;
		bodies.transforms[i].rotation[1] = rotation.v.y;
		bodies.transforms[i].rotation[2] = rotation.v.z;
		bodies.transforms[i].rotation[3] = rotation.s;
	}
}

}
