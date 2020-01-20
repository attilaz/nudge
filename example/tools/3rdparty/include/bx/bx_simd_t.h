/*
 * Copyright 2010-2020 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_SIMD_T_H_HEADER_GUARD
#define BX_SIMD_T_H_HEADER_GUARD

/*
 * Copyright 2010-2020 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_PLATFORM_H_HEADER_GUARD
#define BX_PLATFORM_H_HEADER_GUARD

// Architecture
#define BX_ARCH_32BIT 0
#define BX_ARCH_64BIT 0

// Compiler
#define BX_COMPILER_CLANG          0
#define BX_COMPILER_CLANG_ANALYZER 0
#define BX_COMPILER_GCC            0
#define BX_COMPILER_MSVC           0

// Endianess
#define BX_CPU_ENDIAN_BIG    0
#define BX_CPU_ENDIAN_LITTLE 0

// CPU
#define BX_CPU_ARM   0
#define BX_CPU_JIT   0
#define BX_CPU_MIPS  0
#define BX_CPU_PPC   0
#define BX_CPU_RISCV 0
#define BX_CPU_X86   0

// C Runtime
#define BX_CRT_BIONIC 0
#define BX_CRT_BSD    0
#define BX_CRT_GLIBC  0
#define BX_CRT_LIBCXX 0
#define BX_CRT_MINGW  0
#define BX_CRT_MSVC   0
#define BX_CRT_NEWLIB 0

#ifndef BX_CRT_MUSL
#	define BX_CRT_MUSL 0
#endif // BX_CRT_MUSL

#ifndef BX_CRT_NONE
#	define BX_CRT_NONE 0
#endif // BX_CRT_NONE

// Platform
#define BX_PLATFORM_ANDROID    0
#define BX_PLATFORM_BSD        0
#define BX_PLATFORM_EMSCRIPTEN 0
#define BX_PLATFORM_HAIKU      0
#define BX_PLATFORM_HURD       0
#define BX_PLATFORM_IOS        0
#define BX_PLATFORM_LINUX      0
#define BX_PLATFORM_NX         0
#define BX_PLATFORM_OSX        0
#define BX_PLATFORM_PS4        0
#define BX_PLATFORM_RPI        0
#define BX_PLATFORM_STEAMLINK  0
#define BX_PLATFORM_WINDOWS    0
#define BX_PLATFORM_WINRT      0
#define BX_PLATFORM_XBOXONE    0

// http://sourceforge.net/apps/mediawiki/predef/index.php?title=Compilers
#if defined(__clang__)
// clang defines __GNUC__ or _MSC_VER
#	undef  BX_COMPILER_CLANG
#	define BX_COMPILER_CLANG (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#	if defined(__clang_analyzer__)
#		undef  BX_COMPILER_CLANG_ANALYZER
#		define BX_COMPILER_CLANG_ANALYZER 1
#	endif // defined(__clang_analyzer__)
#elif defined(_MSC_VER)
#	undef  BX_COMPILER_MSVC
#	define BX_COMPILER_MSVC _MSC_VER
#elif defined(__GNUC__)
#	undef  BX_COMPILER_GCC
#	define BX_COMPILER_GCC (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#else
#	error "BX_COMPILER_* is not defined!"
#endif //

// http://sourceforge.net/apps/mediawiki/predef/index.php?title=Architectures
#if defined(__arm__)     \
 || defined(__aarch64__) \
 || defined(_M_ARM)
#	undef  BX_CPU_ARM
#	define BX_CPU_ARM 1
#	define BX_CACHE_LINE_SIZE 64
#elif defined(__MIPSEL__)     \
 ||   defined(__mips_isa_rev) \
 ||   defined(__mips64)
#	undef  BX_CPU_MIPS
#	define BX_CPU_MIPS 1
#	define BX_CACHE_LINE_SIZE 64
#elif defined(_M_PPC)        \
 ||   defined(__powerpc__)   \
 ||   defined(__powerpc64__)
#	undef  BX_CPU_PPC
#	define BX_CPU_PPC 1
#	define BX_CACHE_LINE_SIZE 128
#elif defined(__riscv)   \
 ||   defined(__riscv__) \
 ||   defined(RISCVEL)
#	undef  BX_CPU_RISCV
#	define BX_CPU_RISCV 1
#	define BX_CACHE_LINE_SIZE 64
#elif defined(_M_IX86)    \
 ||   defined(_M_X64)     \
 ||   defined(__i386__)   \
 ||   defined(__x86_64__)
#	undef  BX_CPU_X86
#	define BX_CPU_X86 1
#	define BX_CACHE_LINE_SIZE 64
#else // PNaCl doesn't have CPU defined.
#	undef  BX_CPU_JIT
#	define BX_CPU_JIT 1
#	define BX_CACHE_LINE_SIZE 64
#endif //

#if defined(__x86_64__)    \
 || defined(_M_X64)        \
 || defined(__aarch64__)   \
 || defined(__64BIT__)     \
 || defined(__mips64)      \
 || defined(__powerpc64__) \
 || defined(__ppc64__)     \
 || defined(__LP64__)
#	undef  BX_ARCH_64BIT
#	define BX_ARCH_64BIT 64
#else
#	undef  BX_ARCH_32BIT
#	define BX_ARCH_32BIT 32
#endif //

#if BX_CPU_PPC
// __BIG_ENDIAN__ is gcc predefined macro
#	if defined(__BIG_ENDIAN__)
#		undef  BX_CPU_ENDIAN_BIG
#		define BX_CPU_ENDIAN_BIG 1
#	else
#		undef  BX_CPU_ENDIAN_LITTLE
#		define BX_CPU_ENDIAN_LITTLE 1
#	endif
#else
#	undef  BX_CPU_ENDIAN_LITTLE
#	define BX_CPU_ENDIAN_LITTLE 1
#endif // BX_CPU_PPC

// http://sourceforge.net/apps/mediawiki/predef/index.php?title=Operating_Systems
#if defined(_DURANGO) || defined(_XBOX_ONE)
#	undef  BX_PLATFORM_XBOXONE
#	define BX_PLATFORM_XBOXONE 1
#elif defined(_WIN32) || defined(_WIN64)
// http://msdn.microsoft.com/en-us/library/6sehtctf.aspx
#	ifndef NOMINMAX
#		define NOMINMAX
#	endif // NOMINMAX
//  If _USING_V110_SDK71_ is defined it means we are using the v110_xp or v120_xp toolset.
#	if defined(_MSC_VER) && (_MSC_VER >= 1700) && (!_USING_V110_SDK71_)
#		include <winapifamily.h>
#	endif // defined(_MSC_VER) && (_MSC_VER >= 1700) && (!_USING_V110_SDK71_)
#	if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
#		undef  BX_PLATFORM_WINDOWS
#		if !defined(WINVER) && !defined(_WIN32_WINNT)
#			if BX_ARCH_64BIT
//				When building 64-bit target Win7 and above.
#				define WINVER 0x0601
#				define _WIN32_WINNT 0x0601
#			else
//				Windows Server 2003 with SP1, Windows XP with SP2 and above
#				define WINVER 0x0502
#				define _WIN32_WINNT 0x0502
#			endif // BX_ARCH_64BIT
#		endif // !defined(WINVER) && !defined(_WIN32_WINNT)
#		define BX_PLATFORM_WINDOWS _WIN32_WINNT
#	else
#		undef  BX_PLATFORM_WINRT
#		define BX_PLATFORM_WINRT 1
#	endif
#elif defined(__ANDROID__)
// Android compiler defines __linux__
#	include <sys/cdefs.h> // Defines __BIONIC__ and includes android/api-level.h
#	undef  BX_PLATFORM_ANDROID
#	define BX_PLATFORM_ANDROID __ANDROID_API__
#elif defined(__STEAMLINK__)
// SteamLink compiler defines __linux__
#	undef  BX_PLATFORM_STEAMLINK
#	define BX_PLATFORM_STEAMLINK 1
#elif defined(__VCCOREVER__)
// RaspberryPi compiler defines __linux__
#	undef  BX_PLATFORM_RPI
#	define BX_PLATFORM_RPI 1
#elif  defined(__linux__)
#	undef  BX_PLATFORM_LINUX
#	define BX_PLATFORM_LINUX 1
#elif  defined(__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__) \
	|| defined(__ENVIRONMENT_TV_OS_VERSION_MIN_REQUIRED__)
#	undef  BX_PLATFORM_IOS
#	define BX_PLATFORM_IOS 1
#elif defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__)
#	undef  BX_PLATFORM_OSX
#	define BX_PLATFORM_OSX __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__
#elif defined(__EMSCRIPTEN__)
#	undef  BX_PLATFORM_EMSCRIPTEN
#	define BX_PLATFORM_EMSCRIPTEN 1
#elif defined(__ORBIS__)
#	undef  BX_PLATFORM_PS4
#	define BX_PLATFORM_PS4 1
#elif  defined(__FreeBSD__)        \
	|| defined(__FreeBSD_kernel__) \
	|| defined(__NetBSD__)         \
	|| defined(__OpenBSD__)        \
	|| defined(__DragonFly__)
#	undef  BX_PLATFORM_BSD
#	define BX_PLATFORM_BSD 1
#elif defined(__GNU__)
#	undef  BX_PLATFORM_HURD
#	define BX_PLATFORM_HURD 1
#elif defined(__NX__)
#	undef  BX_PLATFORM_NX
#	define BX_PLATFORM_NX 1
#elif defined(__HAIKU__)
#	undef  BX_PLATFORM_HAIKU
#	define BX_PLATFORM_HAIKU 1
#endif //

#if !BX_CRT_NONE
// https://sourceforge.net/p/predef/wiki/Libraries/
#	if defined(__BIONIC__)
#		undef  BX_CRT_BIONIC
#		define BX_CRT_BIONIC 1
#	elif defined(_MSC_VER)
#		undef  BX_CRT_MSVC
#		define BX_CRT_MSVC 1
#	elif defined(__GLIBC__)
#		undef  BX_CRT_GLIBC
#		define BX_CRT_GLIBC (__GLIBC__ * 10000 + __GLIBC_MINOR__ * 100)
#	elif defined(__MINGW32__) || defined(__MINGW64__)
#		undef  BX_CRT_MINGW
#		define BX_CRT_MINGW 1
#	elif defined(__apple_build_version__) || defined(__ORBIS__) || defined(__EMSCRIPTEN__) || defined(__llvm__) || defined(__HAIKU__)
#		undef  BX_CRT_LIBCXX
#		define BX_CRT_LIBCXX 1
#	elif BX_PLATFORM_BSD
#		undef  BX_CRT_BSD
#		define BX_CRT_BSD 1
#	endif //

#	if !BX_CRT_BIONIC \
	&& !BX_CRT_BSD    \
	&& !BX_CRT_GLIBC  \
	&& !BX_CRT_LIBCXX \
	&& !BX_CRT_MINGW  \
	&& !BX_CRT_MSVC   \
	&& !BX_CRT_MUSL   \
	&& !BX_CRT_NEWLIB
#		undef  BX_CRT_NONE
#		define BX_CRT_NONE 1
#	endif // BX_CRT_*
#endif // !BX_CRT_NONE

///
#define BX_PLATFORM_POSIX (0   \
	||  BX_PLATFORM_ANDROID    \
	||  BX_PLATFORM_BSD        \
	||  BX_PLATFORM_EMSCRIPTEN \
	||  BX_PLATFORM_HAIKU      \
	||  BX_PLATFORM_HURD       \
	||  BX_PLATFORM_IOS        \
	||  BX_PLATFORM_LINUX      \
	||  BX_PLATFORM_NX         \
	||  BX_PLATFORM_OSX        \
	||  BX_PLATFORM_PS4        \
	||  BX_PLATFORM_RPI        \
	||  BX_PLATFORM_STEAMLINK  \
	)

///
#define BX_PLATFORM_NONE !(0   \
	||  BX_PLATFORM_ANDROID    \
	||  BX_PLATFORM_BSD        \
	||  BX_PLATFORM_EMSCRIPTEN \
	||  BX_PLATFORM_HAIKU      \
	||  BX_PLATFORM_HURD       \
	||  BX_PLATFORM_IOS        \
	||  BX_PLATFORM_LINUX      \
	||  BX_PLATFORM_NX         \
	||  BX_PLATFORM_OSX        \
	||  BX_PLATFORM_PS4        \
	||  BX_PLATFORM_RPI        \
	||  BX_PLATFORM_STEAMLINK  \
	||  BX_PLATFORM_WINDOWS    \
	||  BX_PLATFORM_WINRT      \
	||  BX_PLATFORM_XBOXONE    \
	)

///
#define BX_PLATFORM_OS_CONSOLE  (0 \
	||  BX_PLATFORM_NX             \
	||  BX_PLATFORM_PS4            \
	||  BX_PLATFORM_WINRT          \
	||  BX_PLATFORM_XBOXONE        \
	)

///
#define BX_PLATFORM_OS_DESKTOP  (0 \
	||  BX_PLATFORM_BSD            \
	||  BX_PLATFORM_HAIKU          \
	||  BX_PLATFORM_HURD           \
	||  BX_PLATFORM_LINUX          \
	||  BX_PLATFORM_OSX            \
	||  BX_PLATFORM_WINDOWS        \
	)

///
#define BX_PLATFORM_OS_EMBEDDED (0 \
	||  BX_PLATFORM_RPI            \
	||  BX_PLATFORM_STEAMLINK      \
	)

///
#define BX_PLATFORM_OS_MOBILE   (0 \
	||  BX_PLATFORM_ANDROID        \
	||  BX_PLATFORM_IOS            \
	)

///
#define BX_PLATFORM_OS_WEB      (0 \
	||  BX_PLATFORM_EMSCRIPTEN     \
	)

///
#if BX_COMPILER_GCC
#	define BX_COMPILER_NAME "GCC "       \
		BX_STRINGIZE(__GNUC__) "."       \
		BX_STRINGIZE(__GNUC_MINOR__) "." \
		BX_STRINGIZE(__GNUC_PATCHLEVEL__)
#elif BX_COMPILER_CLANG
#	define BX_COMPILER_NAME "Clang "      \
		BX_STRINGIZE(__clang_major__) "." \
		BX_STRINGIZE(__clang_minor__) "." \
		BX_STRINGIZE(__clang_patchlevel__)
#elif BX_COMPILER_MSVC
#	if BX_COMPILER_MSVC >= 1920 // Visual Studio 2019
#		define BX_COMPILER_NAME "MSVC 16.0"
#	elif BX_COMPILER_MSVC >= 1910 // Visual Studio 2017
#		define BX_COMPILER_NAME "MSVC 15.0"
#	elif BX_COMPILER_MSVC >= 1900 // Visual Studio 2015
#		define BX_COMPILER_NAME "MSVC 14.0"
#	elif BX_COMPILER_MSVC >= 1800 // Visual Studio 2013
#		define BX_COMPILER_NAME "MSVC 12.0"
#	elif BX_COMPILER_MSVC >= 1700 // Visual Studio 2012
#		define BX_COMPILER_NAME "MSVC 11.0"
#	elif BX_COMPILER_MSVC >= 1600 // Visual Studio 2010
#		define BX_COMPILER_NAME "MSVC 10.0"
#	elif BX_COMPILER_MSVC >= 1500 // Visual Studio 2008
#		define BX_COMPILER_NAME "MSVC 9.0"
#	else
#		define BX_COMPILER_NAME "MSVC"
#	endif //
#endif // BX_COMPILER_

#if BX_PLATFORM_ANDROID
#	define BX_PLATFORM_NAME "Android " \
				BX_STRINGIZE(BX_PLATFORM_ANDROID)
#elif BX_PLATFORM_BSD
#	define BX_PLATFORM_NAME "BSD"
#elif BX_PLATFORM_EMSCRIPTEN
#	define BX_PLATFORM_NAME "asm.js "          \
		BX_STRINGIZE(__EMSCRIPTEN_major__) "." \
		BX_STRINGIZE(__EMSCRIPTEN_minor__) "." \
		BX_STRINGIZE(__EMSCRIPTEN_tiny__)
#elif BX_PLATFORM_HAIKU
#	define BX_PLATFORM_NAME "Haiku"
#elif BX_PLATFORM_HURD
#	define BX_PLATFORM_NAME "Hurd"
#elif BX_PLATFORM_IOS
#	define BX_PLATFORM_NAME "iOS"
#elif BX_PLATFORM_LINUX
#	define BX_PLATFORM_NAME "Linux"
#elif BX_PLATFORM_NONE
#	define BX_PLATFORM_NAME "None"
#elif BX_PLATFORM_NX
#	define BX_PLATFORM_NAME "NX"
#elif BX_PLATFORM_OSX
#	define BX_PLATFORM_NAME "OSX"
#elif BX_PLATFORM_PS4
#	define BX_PLATFORM_NAME "PlayStation 4"
#elif BX_PLATFORM_RPI
#	define BX_PLATFORM_NAME "RaspberryPi"
#elif BX_PLATFORM_STEAMLINK
#	define BX_PLATFORM_NAME "SteamLink"
#elif BX_PLATFORM_WINDOWS
#	define BX_PLATFORM_NAME "Windows"
#elif BX_PLATFORM_WINRT
#	define BX_PLATFORM_NAME "WinRT"
#elif BX_PLATFORM_XBOXONE
#	define BX_PLATFORM_NAME "Xbox One"
#else
#	error "Unknown BX_PLATFORM!"
#endif // BX_PLATFORM_

#if BX_CPU_ARM
#	define BX_CPU_NAME "ARM"
#elif BX_CPU_JIT
#	define BX_CPU_NAME "JIT-VM"
#elif BX_CPU_MIPS
#	define BX_CPU_NAME "MIPS"
#elif BX_CPU_PPC
#	define BX_CPU_NAME "PowerPC"
#elif BX_CPU_RISCV
#	define BX_CPU_NAME "RISC-V"
#elif BX_CPU_X86
#	define BX_CPU_NAME "x86"
#endif // BX_CPU_

#if BX_CRT_BIONIC
#	define BX_CRT_NAME "Bionic libc"
#elif BX_CRT_BSD
#	define BX_CRT_NAME "BSD libc"
#elif BX_CRT_GLIBC
#	define BX_CRT_NAME "GNU C Library"
#elif BX_CRT_MSVC
#	define BX_CRT_NAME "MSVC C Runtime"
#elif BX_CRT_MINGW
#	define BX_CRT_NAME "MinGW C Runtime"
#elif BX_CRT_LIBCXX
#	define BX_CRT_NAME "Clang C Library"
#elif BX_CRT_NEWLIB
#	define BX_CRT_NAME "Newlib"
#elif BX_CRT_MUSL
#	define BX_CRT_NAME "musl libc"
#elif BX_CRT_NONE
#	define BX_CRT_NAME "None"
#else
#	error "Unknown BX_CRT!"
#endif // BX_CRT_

#if BX_ARCH_32BIT
#	define BX_ARCH_NAME "32-bit"
#elif BX_ARCH_64BIT
#	define BX_ARCH_NAME "64-bit"
#endif // BX_ARCH_

#if BX_COMPILER_MSVC
#	define BX_CPP_NAME "C++MsvcUnknown"
#elif defined(__cplusplus)
#	if __cplusplus < 201103L
#		error "Pre-C++11 compiler is not supported!"
#	elif __cplusplus < 201402L
#		define BX_CPP_NAME "C++11"
#	elif __cplusplus < 201703L
#		define BX_CPP_NAME "C++14"
#	elif __cplusplus < 201704L
#		define BX_CPP_NAME "C++17"
#	else
// See: https://gist.github.com/bkaradzic/2e39896bc7d8c34e042b#orthodox-c
#		define BX_CPP_NAME "C++WayTooModern"
#	endif // BX_CPP_NAME
#else
#	define BX_CPP_NAME "C++Unknown"
#endif // defined(__cplusplus)

#endif // BX_PLATFORM_H_HEADER_GUARD
/*
 * Copyright 2010-2020 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_MACROS_H_HEADER_GUARD
#define BX_MACROS_H_HEADER_GUARD


///
#if BX_COMPILER_MSVC
// Workaround MSVS bug...
#	define BX_VA_ARGS_PASS(...) BX_VA_ARGS_PASS_1_ __VA_ARGS__ BX_VA_ARGS_PASS_2_
#	define BX_VA_ARGS_PASS_1_ (
#	define BX_VA_ARGS_PASS_2_ )
#else
#	define BX_VA_ARGS_PASS(...) (__VA_ARGS__)
#endif // BX_COMPILER_MSVC

#define BX_VA_ARGS_COUNT(...) BX_VA_ARGS_COUNT_ BX_VA_ARGS_PASS(__VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define BX_VA_ARGS_COUNT_(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10, _a11, _a12, _a13, _a14, _a15, _a16, _last, ...) _last

///
#define BX_MACRO_DISPATCHER(_func, ...) BX_MACRO_DISPATCHER_1_(_func, BX_VA_ARGS_COUNT(__VA_ARGS__) )
#define BX_MACRO_DISPATCHER_1_(_func, _argCount) BX_MACRO_DISPATCHER_2_(_func, _argCount)
#define BX_MACRO_DISPATCHER_2_(_func, _argCount) BX_CONCATENATE(_func, _argCount)

///
#define BX_MAKEFOURCC(_a, _b, _c, _d) ( ( (uint32_t)(_a) | ( (uint32_t)(_b) << 8) | ( (uint32_t)(_c) << 16) | ( (uint32_t)(_d) << 24) ) )

///
#define BX_STRINGIZE(_x) BX_STRINGIZE_(_x)
#define BX_STRINGIZE_(_x) #_x

///
#define BX_CONCATENATE(_x, _y) BX_CONCATENATE_(_x, _y)
#define BX_CONCATENATE_(_x, _y) _x ## _y

///
#define BX_FILE_LINE_LITERAL "" __FILE__ "(" BX_STRINGIZE(__LINE__) "): "

///
#define BX_ALIGN_MASK(_value, _mask) ( ( (_value)+(_mask) ) & ( (~0)&(~(_mask) ) ) )
#define BX_ALIGN_16(_value) BX_ALIGN_MASK(_value, 0xf)
#define BX_ALIGN_256(_value) BX_ALIGN_MASK(_value, 0xff)
#define BX_ALIGN_4096(_value) BX_ALIGN_MASK(_value, 0xfff)

///
#define BX_ALIGNOF(_type) __alignof(_type)

#if defined(__has_feature)
#	define BX_CLANG_HAS_FEATURE(_x) __has_feature(_x)
#else
#	define BX_CLANG_HAS_FEATURE(_x) 0
#endif // defined(__has_feature)

#if defined(__has_extension)
#	define BX_CLANG_HAS_EXTENSION(_x) __has_extension(_x)
#else
#	define BX_CLANG_HAS_EXTENSION(_x) 0
#endif // defined(__has_extension)

#if BX_COMPILER_GCC || BX_COMPILER_CLANG
#	define BX_ALIGN_DECL(_align, _decl) _decl __attribute__( (aligned(_align) ) )
#	define BX_ALLOW_UNUSED __attribute__( (unused) )
#	define BX_FORCE_INLINE inline __attribute__( (__always_inline__) )
#	define BX_FUNCTION __PRETTY_FUNCTION__
#	define BX_LIKELY(_x)   __builtin_expect(!!(_x), 1)
#	define BX_UNLIKELY(_x) __builtin_expect(!!(_x), 0)
#	define BX_NO_INLINE   __attribute__( (noinline) )
#	define BX_NO_RETURN   __attribute__( (noreturn) )
#	define BX_CONST_FUNC  __attribute__( (const) )

#	if BX_COMPILER_GCC >= 70000
#		define BX_FALLTHROUGH __attribute__( (fallthrough) )
#	else
#		define BX_FALLTHROUGH BX_NOOP()
#	endif // BX_COMPILER_GCC >= 70000

#	define BX_NO_VTABLE
#	define BX_PRINTF_ARGS(_format, _args) __attribute__( (format(__printf__, _format, _args) ) )

#	if BX_CLANG_HAS_FEATURE(cxx_thread_local)
#		define BX_THREAD_LOCAL __thread
#	endif // BX_COMPILER_CLANG

#	if (!BX_PLATFORM_OSX && (BX_COMPILER_GCC >= 40200)) || (BX_COMPILER_GCC >= 40500)
#		define BX_THREAD_LOCAL __thread
#	endif // BX_COMPILER_GCC

#	define BX_ATTRIBUTE(_x) __attribute__( (_x) )

#	if BX_CRT_MSVC
#		define __stdcall
#	endif // BX_CRT_MSVC
#elif BX_COMPILER_MSVC
#	define BX_ALIGN_DECL(_align, _decl) __declspec(align(_align) ) _decl
#	define BX_ALLOW_UNUSED
#	define BX_FORCE_INLINE __forceinline
#	define BX_FUNCTION __FUNCTION__
#	define BX_LIKELY(_x)   (_x)
#	define BX_UNLIKELY(_x) (_x)
#	define BX_NO_INLINE __declspec(noinline)
#	define BX_NO_RETURN
#	define BX_CONST_FUNC  __declspec(noalias)
#	define BX_FALLTHROUGH BX_NOOP()
#	define BX_NO_VTABLE __declspec(novtable)
#	define BX_PRINTF_ARGS(_format, _args)
#	define BX_THREAD_LOCAL __declspec(thread)
#	define BX_ATTRIBUTE(_x)
#else
#	error "Unknown BX_COMPILER_?"
#endif

/// The return value of the function is solely a function of the arguments.
///
#if __cplusplus < 201402
#	define BX_CONSTEXPR_FUNC BX_CONST_FUNC
#else
#	define BX_CONSTEXPR_FUNC constexpr BX_CONST_FUNC
#endif // __cplusplus < 201402

///
#define BX_STATIC_ASSERT(_condition, ...) static_assert(_condition, "" __VA_ARGS__)

///
#define BX_ALIGN_DECL_16(_decl) BX_ALIGN_DECL(16, _decl)
#define BX_ALIGN_DECL_256(_decl) BX_ALIGN_DECL(256, _decl)
#define BX_ALIGN_DECL_CACHE_LINE(_decl) BX_ALIGN_DECL(BX_CACHE_LINE_SIZE, _decl)

///
#define BX_MACRO_BLOCK_BEGIN for(;;) {
#define BX_MACRO_BLOCK_END break; }
#define BX_NOOP(...) BX_MACRO_BLOCK_BEGIN BX_MACRO_BLOCK_END

///
#define BX_UNUSED_1(_a1)                                              \
	BX_MACRO_BLOCK_BEGIN                                              \
		BX_PRAGMA_DIAGNOSTIC_PUSH();                                  \
		/*BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wuseless-cast");*/ \
		(void)(true ? (void)0 : ( (void)(_a1) ) );                    \
		BX_PRAGMA_DIAGNOSTIC_POP();                                   \
	BX_MACRO_BLOCK_END

#define BX_UNUSED_2(_a1, _a2) BX_UNUSED_1(_a1); BX_UNUSED_1(_a2)
#define BX_UNUSED_3(_a1, _a2, _a3) BX_UNUSED_2(_a1, _a2); BX_UNUSED_1(_a3)
#define BX_UNUSED_4(_a1, _a2, _a3, _a4) BX_UNUSED_3(_a1, _a2, _a3); BX_UNUSED_1(_a4)
#define BX_UNUSED_5(_a1, _a2, _a3, _a4, _a5) BX_UNUSED_4(_a1, _a2, _a3, _a4); BX_UNUSED_1(_a5)
#define BX_UNUSED_6(_a1, _a2, _a3, _a4, _a5, _a6) BX_UNUSED_5(_a1, _a2, _a3, _a4, _a5); BX_UNUSED_1(_a6)
#define BX_UNUSED_7(_a1, _a2, _a3, _a4, _a5, _a6, _a7) BX_UNUSED_6(_a1, _a2, _a3, _a4, _a5, _a6); BX_UNUSED_1(_a7)
#define BX_UNUSED_8(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8) BX_UNUSED_7(_a1, _a2, _a3, _a4, _a5, _a6, _a7); BX_UNUSED_1(_a8)
#define BX_UNUSED_9(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9) BX_UNUSED_8(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8); BX_UNUSED_1(_a9)
#define BX_UNUSED_10(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10) BX_UNUSED_9(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9); BX_UNUSED_1(_a10)
#define BX_UNUSED_11(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10, _a11) BX_UNUSED_10(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10); BX_UNUSED_1(_a11)
#define BX_UNUSED_12(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10, _a11, _a12) BX_UNUSED_11(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10, _a11); BX_UNUSED_1(_a12)

#if BX_COMPILER_MSVC
// Workaround MSVS bug...
#	define BX_UNUSED(...) BX_MACRO_DISPATCHER(BX_UNUSED_, __VA_ARGS__) BX_VA_ARGS_PASS(__VA_ARGS__)
#else
#	define BX_UNUSED(...) BX_MACRO_DISPATCHER(BX_UNUSED_, __VA_ARGS__)(__VA_ARGS__)
#endif // BX_COMPILER_MSVC

///
#if BX_COMPILER_CLANG
#	define BX_PRAGMA_DIAGNOSTIC_PUSH_CLANG_()     _Pragma("clang diagnostic push")
#	define BX_PRAGMA_DIAGNOSTIC_POP_CLANG_()      _Pragma("clang diagnostic pop")
#	define BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG(_x) _Pragma(BX_STRINGIZE(clang diagnostic ignored _x) )
#else
#	define BX_PRAGMA_DIAGNOSTIC_PUSH_CLANG_()
#	define BX_PRAGMA_DIAGNOSTIC_POP_CLANG_()
#	define BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG(_x)
#endif // BX_COMPILER_CLANG

#if BX_COMPILER_GCC && BX_COMPILER_GCC >= 40600
#	define BX_PRAGMA_DIAGNOSTIC_PUSH_GCC_()       _Pragma("GCC diagnostic push")
#	define BX_PRAGMA_DIAGNOSTIC_POP_GCC_()        _Pragma("GCC diagnostic pop")
#	define BX_PRAGMA_DIAGNOSTIC_IGNORED_GCC(_x)   _Pragma(BX_STRINGIZE(GCC diagnostic ignored _x) )
#else
#	define BX_PRAGMA_DIAGNOSTIC_PUSH_GCC_()
#	define BX_PRAGMA_DIAGNOSTIC_POP_GCC_()
#	define BX_PRAGMA_DIAGNOSTIC_IGNORED_GCC(_x)
#endif // BX_COMPILER_GCC

#if BX_COMPILER_MSVC
#	define BX_PRAGMA_DIAGNOSTIC_PUSH_MSVC_()     __pragma(warning(push) )
#	define BX_PRAGMA_DIAGNOSTIC_POP_MSVC_()      __pragma(warning(pop) )
#	define BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(_x) __pragma(warning(disable:_x) )
#else
#	define BX_PRAGMA_DIAGNOSTIC_PUSH_MSVC_()
#	define BX_PRAGMA_DIAGNOSTIC_POP_MSVC_()
#	define BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(_x)
#endif // BX_COMPILER_CLANG

#if BX_COMPILER_CLANG
#	define BX_PRAGMA_DIAGNOSTIC_PUSH              BX_PRAGMA_DIAGNOSTIC_PUSH_CLANG_
#	define BX_PRAGMA_DIAGNOSTIC_POP               BX_PRAGMA_DIAGNOSTIC_POP_CLANG_
#	define BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG
#elif BX_COMPILER_GCC
#	define BX_PRAGMA_DIAGNOSTIC_PUSH              BX_PRAGMA_DIAGNOSTIC_PUSH_GCC_
#	define BX_PRAGMA_DIAGNOSTIC_POP               BX_PRAGMA_DIAGNOSTIC_POP_GCC_
#	define BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC BX_PRAGMA_DIAGNOSTIC_IGNORED_GCC
#elif BX_COMPILER_MSVC
#	define BX_PRAGMA_DIAGNOSTIC_PUSH              BX_PRAGMA_DIAGNOSTIC_PUSH_MSVC_
#	define BX_PRAGMA_DIAGNOSTIC_POP               BX_PRAGMA_DIAGNOSTIC_POP_MSVC_
#	define BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC(_x)
#endif // BX_COMPILER_

///
#define BX_CLASS_NO_DEFAULT_CTOR(_class) \
	private: _class()

#define BX_CLASS_NO_COPY(_class) \
	private: _class(const _class& _rhs)

#define BX_CLASS_NO_ASSIGNMENT(_class) \
	private: _class& operator=(const _class& _rhs)

#define BX_CLASS_ALLOCATOR(_class)              \
	public: void* operator new(size_t _size);   \
	public: void  operator delete(void* _ptr);  \
	public: void* operator new[](size_t _size); \
	public: void  operator delete[](void* _ptr)

#define BX_CLASS_1(_class, _a1) BX_CONCATENATE(BX_CLASS_, _a1)(_class)
#define BX_CLASS_2(_class, _a1, _a2) BX_CLASS_1(_class, _a1); BX_CLASS_1(_class, _a2)
#define BX_CLASS_3(_class, _a1, _a2, _a3) BX_CLASS_2(_class, _a1, _a2); BX_CLASS_1(_class, _a3)
#define BX_CLASS_4(_class, _a1, _a2, _a3, _a4) BX_CLASS_3(_class, _a1, _a2, _a3); BX_CLASS_1(_class, _a4)

#if BX_COMPILER_MSVC
#	define BX_CLASS(_class, ...) BX_MACRO_DISPATCHER(BX_CLASS_, __VA_ARGS__) BX_VA_ARGS_PASS(_class, __VA_ARGS__)
#else
#	define BX_CLASS(_class, ...) BX_MACRO_DISPATCHER(BX_CLASS_, __VA_ARGS__)(_class, __VA_ARGS__)
#endif // BX_COMPILER_MSVC

#ifndef BX_CHECK
#	define BX_CHECK(_condition, ...) BX_NOOP()
#endif // BX_CHECK

#ifndef BX_TRACE
#	define BX_TRACE(...) BX_NOOP()
#endif // BX_TRACE

#ifndef BX_WARN
#	define BX_WARN(_condition, ...) BX_NOOP()
#endif // BX_CHECK

// static_assert sometimes causes unused-local-typedef...
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG("-Wunused-local-typedef")

#endif // BX_MACROS_H_HEADER_GUARD

#define BX_SIMD_FORCE_INLINE BX_FORCE_INLINE
#define BX_SIMD_INLINE inline

#define BX_SIMD_AVX     0
#define BX_SIMD_LANGEXT 0
#define BX_SIMD_NEON    0
#define BX_SIMD_SSE     0

#define BX_CONFIG_SUPPORTS_SIMD 0

#if defined(__AVX__) || defined(__AVX2__)
#	include <immintrin.h>
#	undef  BX_SIMD_AVX
#	define BX_SIMD_AVX 1
#endif //

#if defined(__SSE2__) || (BX_COMPILER_MSVC && (BX_ARCH_64BIT || _M_IX86_FP >= 2) )
#	include <emmintrin.h> // __m128i
#	if defined(__SSE4_1__)
#		include <smmintrin.h>
#	endif // defined(__SSE4_1__)
#	include <xmmintrin.h> // __m128
#	undef  BX_SIMD_SSE
#	define BX_SIMD_SSE 1
#elif defined(__ARM_NEON__) && (!BX_COMPILER_CLANG || BX_CLANG_HAS_EXTENSION(attribute_ext_vector_type) )
#	include <arm_neon.h>
#	undef  BX_SIMD_NEON
#	define BX_SIMD_NEON 1
#elif   BX_COMPILER_CLANG \
	&& !BX_PLATFORM_EMSCRIPTEN \
	&& !BX_PLATFORM_IOS \
	&&  BX_CLANG_HAS_EXTENSION(attribute_ext_vector_type)
#	undef  BX_SIMD_LANGEXT
#	define BX_SIMD_LANGEXT 1
#endif //

namespace bx
{
#define ELEMx 0
#define ELEMy 1
#define ELEMz 2
#define ELEMw 3
#define BX_SIMD128_IMPLEMENT_SWIZZLE(_x, _y, _z, _w) \
			template<typename Ty> \
			Ty simd_swiz_##_x##_y##_z##_w(Ty _a);
/*
 * Copyright 2010-2015 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_SIMD_T_H_HEADER_GUARD
#	error "xmacro file, must be included from simd_*.h"
#endif // BX_FLOAT4_T_H_HEADER_GUARD

// included from float4_t.h
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, z, z)
// BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, w, w)

#undef BX_SIMD128_IMPLEMENT_SWIZZLE
#undef ELEMw
#undef ELEMz
#undef ELEMy
#undef ELEMx

#define BX_SIMD128_IMPLEMENT_TEST(_xyzw) \
			template<typename Ty> \
			BX_SIMD_FORCE_INLINE bool simd_test_any_##_xyzw(Ty _test); \
			\
			template<typename Ty> \
			BX_SIMD_FORCE_INLINE bool simd_test_all_##_xyzw(Ty _test)

BX_SIMD128_IMPLEMENT_TEST(x   );
BX_SIMD128_IMPLEMENT_TEST(y   );
BX_SIMD128_IMPLEMENT_TEST(xy  );
BX_SIMD128_IMPLEMENT_TEST(z   );
BX_SIMD128_IMPLEMENT_TEST(xz  );
BX_SIMD128_IMPLEMENT_TEST(yz  );
BX_SIMD128_IMPLEMENT_TEST(xyz );
BX_SIMD128_IMPLEMENT_TEST(w   );
BX_SIMD128_IMPLEMENT_TEST(xw  );
BX_SIMD128_IMPLEMENT_TEST(yw  );
BX_SIMD128_IMPLEMENT_TEST(xyw );
BX_SIMD128_IMPLEMENT_TEST(zw  );
BX_SIMD128_IMPLEMENT_TEST(xzw );
BX_SIMD128_IMPLEMENT_TEST(yzw );
BX_SIMD128_IMPLEMENT_TEST(xyzw);
#undef BX_SIMD128_IMPLEMENT_TEST

	template<typename Ty>
	Ty simd_shuf_xyAB(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_shuf_ABxy(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_shuf_CDzw(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_shuf_zwCD(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_shuf_xAyB(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_shuf_AxBy(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_shuf_zCwD(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_shuf_CzDw(Ty _a, Ty _b);

	template<typename Ty>
	float simd_x(Ty _a);

	template<typename Ty>
	float simd_y(Ty _a);

	template<typename Ty>
	float simd_z(Ty _a);

	template<typename Ty>
	float simd_w(Ty _a);

	template<typename Ty>
	Ty simd_ld(const void* _ptr);

	template<typename Ty>
	void simd_st(void* _ptr, Ty _a);

	template<typename Ty>
	void simd_stx(void* _ptr, Ty _a);

	template<typename Ty>
	void simd_stream(void* _ptr, Ty _a);

	template<typename Ty>
	Ty simd_ld(float _x, float _y, float _z, float _w);

	template<typename Ty>
	Ty simd_ld(float _x, float _y, float _z, float _w, float _a, float _b, float _c, float _d);

	template<typename Ty>
	Ty simd_ild(uint32_t _x, uint32_t _y, uint32_t _z, uint32_t _w);

	template<typename Ty>
	Ty simd_ild(uint32_t _x, uint32_t _y, uint32_t _z, uint32_t _w, uint32_t _a, uint32_t _b, uint32_t _c, uint32_t _d);

	template<typename Ty>
	Ty simd_splat(const void* _ptr);

	template<typename Ty>
	Ty simd_splat(float _a);

	template<typename Ty>
	Ty simd_isplat(uint32_t _a);

	template<typename Ty>
	Ty simd_zero();

	template<typename Ty>
	Ty simd_itof(Ty _a);

	template<typename Ty>
	Ty simd_ftoi(Ty _a);

	template<typename Ty>
	Ty simd_round(Ty _a);

	template<typename Ty>
	Ty simd_add(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_sub(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_mul(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_div(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_rcp_est(Ty _a);

	template<typename Ty>
	Ty simd_sqrt(Ty _a);

	template<typename Ty>
	Ty simd_rsqrt_est(Ty _a);

	template<typename Ty>
	Ty simd_dot3(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_dot(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_cmpeq(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_cmplt(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_cmple(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_cmpgt(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_cmpge(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_min(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_max(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_and(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_andc(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_or(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_xor(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_sll(Ty _a, int _count);

	template<typename Ty>
	Ty simd_srl(Ty _a, int _count);

	template<typename Ty>
	Ty simd_sra(Ty _a, int _count);

	template<typename Ty>
	Ty simd_icmpeq(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_icmplt(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_icmpgt(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_imin(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_imax(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_iadd(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_isub(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_shuf_xAzC(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_shuf_yBwD(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_rcp(Ty _a);

	template<typename Ty>
	Ty simd_orx(Ty _a);

	template<typename Ty>
	Ty simd_orc(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_neg(Ty _a);

	template<typename Ty>
	Ty simd_madd(Ty _a, Ty _b, Ty _c);

	template<typename Ty>
	Ty simd_nmsub(Ty _a, Ty _b, Ty _c);

	template<typename Ty>
	Ty simd_div_nr(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_selb(Ty _mask, Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_sels(Ty _test, Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_not(Ty _a);

	template<typename Ty>
	Ty simd_abs(Ty _a);

	template<typename Ty>
	Ty simd_clamp(Ty _a, Ty _min, Ty _max);

	template<typename Ty>
	Ty simd_lerp(Ty _a, Ty _b, Ty _s);

	template<typename Ty>
	Ty simd_rsqrt(Ty _a);

	template<typename Ty>
	Ty simd_rsqrt_nr(Ty _a);

	template<typename Ty>
	Ty simd_rsqrt_carmack(Ty _a);

	template<typename Ty>
	Ty simd_sqrt_nr(Ty _a);

	template<typename Ty>
	Ty simd_log2(Ty _a);

	template<typename Ty>
	Ty simd_exp2(Ty _a);

	template<typename Ty>
	Ty simd_pow(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_cross3(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_normalize3(Ty _a);

	template<typename Ty>
	Ty simd_ceil(Ty _a);

	template<typename Ty>
	Ty simd_floor(Ty _a);

	template<typename Ty>
	Ty simd_shuf_xAzC_ni(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_shuf_yBwD_ni(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_madd_ni(Ty _a, Ty _b, Ty _c);

	template<typename Ty>
	Ty simd_nmsub_ni(Ty _a, Ty _b, Ty _c);

	template<typename Ty>
	Ty simd_div_nr_ni(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_rcp_ni(Ty _a);

	template<typename Ty>
	Ty simd_orx_ni(Ty _a);

	template<typename Ty>
	Ty simd_orc_ni(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_neg_ni(Ty _a);

	template<typename Ty>
	Ty simd_selb_ni(Ty _mask, Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_sels_ni(Ty _test, Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_not_ni(Ty _a);

	template<typename Ty>
	Ty simd_min_ni(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_max_ni(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_abs_ni(Ty _a);

	template<typename Ty>
	Ty simd_imin_ni(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_imax_ni(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_clamp_ni(Ty _a, Ty _min, Ty _max);

	template<typename Ty>
	Ty simd_lerp_ni(Ty _a, Ty _b, Ty _s);

	template<typename Ty>
	Ty simd_sqrt_nr_ni(Ty _a);

	template<typename Ty>
	Ty simd_sqrt_nr1_ni(Ty _a);

	template<typename Ty>
	Ty simd_rsqrt_ni(Ty _a);

	template<typename Ty>
	Ty simd_rsqrt_nr_ni(Ty _a);

	template<typename Ty>
	Ty simd_rsqrt_carmack_ni(Ty _a);

	template<typename Ty>
	Ty simd_log2_ni(Ty _a);

	template<typename Ty>
	Ty simd_exp2_ni(Ty _a);

	template<typename Ty>
	Ty simd_pow_ni(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_dot3_ni(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_cross3_ni(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_normalize3_ni(Ty _a);

	template<typename Ty>
	Ty simd_dot_ni(Ty _a, Ty _b);

	template<typename Ty>
	Ty simd_ceil_ni(Ty _a);

	template<typename Ty>
	Ty simd_floor_ni(Ty _a);

	template<typename Ty>
	Ty simd_round_ni(Ty _a);

	template<typename Ty>
	bool simd_test_any_ni(Ty _a);

	template<typename Ty>
	bool simd_test_all_ni(Ty _a);

#if BX_SIMD_AVX
	typedef __m256 simd256_avx_t;
#endif // BX_SIMD_SSE

#if BX_SIMD_LANGEXT
	union simd128_langext_t
	{
		float    __attribute__((vector_size(16))) vf;
		int32_t  __attribute__((vector_size(16))) vi;
		uint32_t __attribute__((vector_size(16))) vu;
		float    fxyzw[4];
		int32_t  ixyzw[4];
		uint32_t uxyzw[4];

	};
#endif // BX_SIMD_LANGEXT

#if BX_SIMD_NEON
	typedef float32x4_t simd128_neon_t;
#endif // BX_SIMD_NEON

#if BX_SIMD_SSE
	typedef __m128 simd128_sse_t;
#endif // BX_SIMD_SSE

} // namespace bx

#if BX_SIMD_AVX
/*
 * Copyright 2010-2020 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_SIMD_T_H_HEADER_GUARD
#	error "Must be included from bx/simd_t.h!"
#endif // BX_SIMD_T_H_HEADER_GUARD

namespace bx
{
	template<>
	BX_SIMD_FORCE_INLINE simd256_avx_t simd_ld(const void* _ptr)
	{
		return _mm256_load_ps(reinterpret_cast<const float*>(_ptr) );
	}

	template<>
	BX_SIMD_FORCE_INLINE void simd_st(void* _ptr, simd256_avx_t _a)
	{
		_mm256_store_ps(reinterpret_cast<float*>(_ptr), _a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd256_avx_t simd_ld(float _x, float _y, float _z, float _w, float _A, float _B, float _C, float _D)
	{
		return _mm256_set_ps(_D, _C, _B, _A, _w, _z, _y, _x);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd256_avx_t simd_ild(uint32_t _x, uint32_t _y, uint32_t _z, uint32_t _w, uint32_t _A, uint32_t _B, uint32_t _C, uint32_t _D)
	{
		const __m256i set          = _mm256_set_epi32(_D, _C, _B, _A, _w, _z, _y, _x);
		const simd256_avx_t result = _mm256_castsi256_ps(set);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd256_avx_t simd_splat(float _a)
	{
		return _mm256_set1_ps(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd256_avx_t simd_isplat(uint32_t _a)
	{
		const __m256i splat        = _mm256_set1_epi32(_a);
		const simd256_avx_t result = _mm256_castsi256_ps(splat);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd256_avx_t simd_itof(simd256_avx_t _a)
	{
		const __m256i  itof        = _mm256_castps_si256(_a);
		const simd256_avx_t result = _mm256_cvtepi32_ps(itof);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd256_avx_t simd_ftoi(simd256_avx_t _a)
	{
		const __m256i ftoi         = _mm256_cvtps_epi32(_a);
		const simd256_avx_t result = _mm256_castsi256_ps(ftoi);

		return result;
	}

	typedef simd256_avx_t simd256_t;

} // namespace bx
#endif // BX_SIMD_AVX

#if BX_SIMD_LANGEXT
/*
 * Copyright 2010-2020 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_SIMD_T_H_HEADER_GUARD
#	error "Must be included from bx/simd_t.h!"
#endif // BX_SIMD_T_H_HEADER_GUARD

namespace bx
{
	BX_CONST_FUNC float sqrtRef(float);

#define ELEMx 0
#define ELEMy 1
#define ELEMz 2
#define ELEMw 3
#define BX_SIMD128_IMPLEMENT_SWIZZLE(_x, _y, _z, _w)                                                       \
			template<>                                                                                     \
			BX_SIMD_FORCE_INLINE simd128_langext_t simd_swiz_##_x##_y##_z##_w(simd128_langext_t _a)        \
			{                                                                                              \
				simd128_langext_t result;                                                                  \
				result.vf = __builtin_shufflevector(_a.vf, _a.vf, ELEM##_x, ELEM##_y, ELEM##_z, ELEM##_w); \
				return result;                                                                             \
			}

/*
 * Copyright 2010-2015 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_SIMD_T_H_HEADER_GUARD
#	error "xmacro file, must be included from simd_*.h"
#endif // BX_FLOAT4_T_H_HEADER_GUARD

// included from float4_t.h
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, z, z)
// BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, w, w)

#undef BX_SIMD128_IMPLEMENT_SWIZZLE
#undef ELEMw
#undef ELEMz
#undef ELEMy
#undef ELEMx

#define BX_SIMD128_IMPLEMENT_TEST(_xyzw, _mask)                                      \
			template<>                                                               \
			BX_SIMD_FORCE_INLINE bool simd_test_any_##_xyzw(simd128_langext_t _test) \
			{                                                                        \
				uint32_t tmp = ( (_test.uxyzw[3]>>31)<<3)                            \
				             | ( (_test.uxyzw[2]>>31)<<2)                            \
				             | ( (_test.uxyzw[1]>>31)<<1)                            \
				             | (  _test.uxyzw[0]>>31)                                \
				             ;                                                       \
				return 0 != (tmp&(_mask) );                                          \
			}                                                                        \
			                                                                         \
			template<>                                                               \
			BX_SIMD_FORCE_INLINE bool simd_test_all_##_xyzw(simd128_langext_t _test) \
			{                                                                        \
				uint32_t tmp = ( (_test.uxyzw[3]>>31)<<3)                            \
				             | ( (_test.uxyzw[2]>>31)<<2)                            \
				             | ( (_test.uxyzw[1]>>31)<<1)                            \
				             | (  _test.uxyzw[0]>>31)                                \
				             ;                                                       \
				return (_mask) == (tmp&(_mask) );                                    \
			}

BX_SIMD128_IMPLEMENT_TEST(x    , 0x1);
BX_SIMD128_IMPLEMENT_TEST(y    , 0x2);
BX_SIMD128_IMPLEMENT_TEST(xy   , 0x3);
BX_SIMD128_IMPLEMENT_TEST(z    , 0x4);
BX_SIMD128_IMPLEMENT_TEST(xz   , 0x5);
BX_SIMD128_IMPLEMENT_TEST(yz   , 0x6);
BX_SIMD128_IMPLEMENT_TEST(xyz  , 0x7);
BX_SIMD128_IMPLEMENT_TEST(w    , 0x8);
BX_SIMD128_IMPLEMENT_TEST(xw   , 0x9);
BX_SIMD128_IMPLEMENT_TEST(yw   , 0xa);
BX_SIMD128_IMPLEMENT_TEST(xyw  , 0xb);
BX_SIMD128_IMPLEMENT_TEST(zw   , 0xc);
BX_SIMD128_IMPLEMENT_TEST(xzw  , 0xd);
BX_SIMD128_IMPLEMENT_TEST(yzw  , 0xe);
BX_SIMD128_IMPLEMENT_TEST(xyzw , 0xf);

#undef BX_SIMD128_IMPLEMENT_TEST

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_shuf_xyAB(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vf = __builtin_shufflevector(_a.vf, _b.vf, 0, 1, 4, 5);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_shuf_ABxy(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vf = __builtin_shufflevector(_a.vf, _b.vf, 4, 5, 0, 1);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_shuf_CDzw(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vf = __builtin_shufflevector(_a.vf, _b.vf, 6, 7, 2, 3);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_shuf_zwCD(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vf = __builtin_shufflevector(_a.vf, _b.vf, 2, 3, 6, 7);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_shuf_xAyB(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vf = __builtin_shufflevector(_a.vf, _b.vf, 0, 4, 1, 5);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_shuf_AxBy(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vf = __builtin_shufflevector(_a.vf, _b.vf, 1, 5, 0, 4);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_shuf_zCwD(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vf = __builtin_shufflevector(_a.vf, _b.vf, 2, 6, 3, 7);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_shuf_CzDw(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vf = __builtin_shufflevector(_a.vf, _b.vf, 6, 2, 7, 3);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_shuf_xAzC(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vf = __builtin_shufflevector(_a.vf, _b.vf, 0, 4, 2, 6);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_shuf_yBwD(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vf = __builtin_shufflevector(_a.vf, _b.vf, 1, 5, 3, 7);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE float simd_x(simd128_langext_t _a)
	{
		return _a.fxyzw[0];
	}

	template<>
	BX_SIMD_FORCE_INLINE float simd_y(simd128_langext_t _a)
	{
		return _a.fxyzw[1];
	}

	template<>
	BX_SIMD_FORCE_INLINE float simd_z(simd128_langext_t _a)
	{
		return _a.fxyzw[2];
	}

	template<>
	BX_SIMD_FORCE_INLINE float simd_w(simd128_langext_t _a)
	{
		return _a.fxyzw[3];
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_ld(const void* _ptr)
	{
		const uint32_t* input = reinterpret_cast<const uint32_t*>(_ptr);
		simd128_langext_t result;
		result.uxyzw[0] = input[0];
		result.uxyzw[1] = input[1];
		result.uxyzw[2] = input[2];
		result.uxyzw[3] = input[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE void simd_st(void* _ptr, simd128_langext_t _a)
	{
		uint32_t* result = reinterpret_cast<uint32_t*>(_ptr);
		result[0] = _a.uxyzw[0];
		result[1] = _a.uxyzw[1];
		result[2] = _a.uxyzw[2];
		result[3] = _a.uxyzw[3];
	}

	template<>
	BX_SIMD_FORCE_INLINE void simd_stx(void* _ptr, simd128_langext_t _a)
	{
		uint32_t* result = reinterpret_cast<uint32_t*>(_ptr);
		result[0] = _a.uxyzw[0];
	}

	template<>
	BX_SIMD_FORCE_INLINE void simd_stream(void* _ptr, simd128_langext_t _a)
	{
		uint32_t* result = reinterpret_cast<uint32_t*>(_ptr);
		result[0] = _a.uxyzw[0];
		result[1] = _a.uxyzw[1];
		result[2] = _a.uxyzw[2];
		result[3] = _a.uxyzw[3];
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_ld(float _x, float _y, float _z, float _w)
	{
		simd128_langext_t result;
		result.vf = (float __attribute__((vector_size(16)))){ _x, _y, _z, _w };
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_ild(uint32_t _x, uint32_t _y, uint32_t _z, uint32_t _w)
	{
		simd128_langext_t result;
		result.vu = (uint32_t __attribute__((vector_size(16)))){ _x, _y, _z, _w };
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_splat(const void* _ptr)
	{
		const uint32_t val = *reinterpret_cast<const uint32_t*>(_ptr);
		simd128_langext_t result;
		result.vu = (uint32_t __attribute__((vector_size(16)))){ val, val, val, val };
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_splat(float _a)
	{
		return simd_ld<simd128_langext_t>(_a, _a, _a, _a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_isplat(uint32_t _a)
	{
		return simd_ild<simd128_langext_t>(_a, _a, _a, _a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_zero()
	{
		return simd_ild<simd128_langext_t>(0, 0, 0, 0);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_itof(simd128_langext_t _a)
	{
		simd128_langext_t result;
		result.vf = __builtin_convertvector(_a.vi, float __attribute__((vector_size(16))) );
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_ftoi(simd128_langext_t _a)
	{
		simd128_langext_t result;
		result.vi = __builtin_convertvector(_a.vf, int32_t __attribute__((vector_size(16))) );
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_round(simd128_langext_t _a)
	{
		const simd128_langext_t tmp    = simd_ftoi(_a);
		const simd128_langext_t result = simd_itof(tmp);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_add(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vf = _a.vf + _b.vf;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_sub(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vf = _a.vf - _b.vf;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_mul(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vf = _a.vf * _b.vf;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_div(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vf = _a.vf / _b.vf;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_sqrt(simd128_langext_t _a)
	{
		simd128_langext_t result;
		result.vf[0] = sqrtRef(_a.vf[0]);
		result.vf[1] = sqrtRef(_a.vf[1]);
		result.vf[2] = sqrtRef(_a.vf[2]);
		result.vf[3] = sqrtRef(_a.vf[3]);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_rsqrt_est(simd128_langext_t _a)
	{
		simd128_langext_t result;
		result.vf[0] = 1.0f / sqrtRef(_a.vf[0]);
		result.vf[1] = 1.0f / sqrtRef(_a.vf[1]);
		result.vf[2] = 1.0f / sqrtRef(_a.vf[2]);
		result.vf[3] = 1.0f / sqrtRef(_a.vf[3]);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_cmpeq(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vi = _a.vf == _b.vf;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_cmplt(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vi = _a.vf < _b.vf;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_cmple(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vi = _a.vf <= _b.vf;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_cmpgt(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vi = _a.vf > _b.vf;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_cmpge(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vi = _a.vf >= _b.vf;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_and(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vu = _a.vu & _b.vu;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_andc(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vu = _a.vu & ~_b.vu;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_or(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vu = _a.vu | _b.vu;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_xor(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vu = _a.vu ^ _b.vu;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_sll(simd128_langext_t _a, int _count)
	{
		simd128_langext_t result;
		const simd128_langext_t count = simd_isplat<simd128_langext_t>(_count);
		result.vu = _a.vu << count.vi;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_srl(simd128_langext_t _a, int _count)
	{
		simd128_langext_t result;
		const simd128_langext_t count = simd_isplat<simd128_langext_t>(_count);
		result.vu = _a.vu >> count.vi;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_sra(simd128_langext_t _a, int _count)
	{
		simd128_langext_t result;
		const simd128_langext_t count = simd_isplat<simd128_langext_t>(_count);
		result.vi = _a.vi >> count.vi;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_icmpeq(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vi = _a.vi == _b.vi;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_icmplt(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vi = _a.vi < _b.vi;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_icmpgt(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vi = _a.vi > _b.vi;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_iadd(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vi = _a.vi + _b.vi;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_isub(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vi = _a.vi - _b.vi;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_rcp(simd128_langext_t _a)
	{
		return simd_rcp_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_orx(simd128_langext_t _a)
	{
		return simd_orx_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_orc(simd128_langext_t _a, simd128_langext_t _b)
	{
		return simd_orc_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_neg(simd128_langext_t _a)
	{
		return simd_neg_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_madd(simd128_langext_t _a, simd128_langext_t _b, simd128_langext_t _c)
	{
		return simd_madd_ni(_a, _b, _c);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_nmsub(simd128_langext_t _a, simd128_langext_t _b, simd128_langext_t _c)
	{
		return simd_nmsub_ni(_a, _b, _c);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_div_nr(simd128_langext_t _a, simd128_langext_t _b)
	{
		return simd_div_nr_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_selb(simd128_langext_t _mask, simd128_langext_t _a, simd128_langext_t _b)
	{
		return simd_selb_ni(_mask, _a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_sels(simd128_langext_t _test, simd128_langext_t _a, simd128_langext_t _b)
	{
		return simd_sels_ni(_test, _a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_not(simd128_langext_t _a)
	{
		return simd_not_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_abs(simd128_langext_t _a)
	{
		return simd_abs_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_clamp(simd128_langext_t _a, simd128_langext_t _min, simd128_langext_t _max)
	{
		return simd_clamp_ni(_a, _min, _max);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_lerp(simd128_langext_t _a, simd128_langext_t _b, simd128_langext_t _s)
	{
		return simd_lerp_ni(_a, _b, _s);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_rcp_est(simd128_langext_t _a)
	{
		return simd_rcp_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_rsqrt(simd128_langext_t _a)
	{
		return simd_rsqrt_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_rsqrt_nr(simd128_langext_t _a)
	{
		return simd_rsqrt_nr_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_rsqrt_carmack(simd128_langext_t _a)
	{
		return simd_rsqrt_carmack_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_sqrt_nr(simd128_langext_t _a)
	{
		return simd_sqrt_nr_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_log2(simd128_langext_t _a)
	{
		return simd_log2_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_exp2(simd128_langext_t _a)
	{
		return simd_exp2_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_pow(simd128_langext_t _a, simd128_langext_t _b)
	{
		return simd_pow_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_cross3(simd128_langext_t _a, simd128_langext_t _b)
	{
		return simd_cross3_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_normalize3(simd128_langext_t _a)
	{
		return simd_normalize3_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_dot3(simd128_langext_t _a, simd128_langext_t _b)
	{
		return simd_dot3_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_dot(simd128_langext_t _a, simd128_langext_t _b)
	{
		return simd_dot_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_ceil(simd128_langext_t _a)
	{
		return simd_ceil_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_floor(simd128_langext_t _a)
	{
		return simd_floor_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_min(simd128_langext_t _a, simd128_langext_t _b)
	{
		return simd_min_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_max(simd128_langext_t _a, simd128_langext_t _b)
	{
		return simd_max_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_imin(simd128_langext_t _a, simd128_langext_t _b)
	{
		return simd_imin_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_imax(simd128_langext_t _a, simd128_langext_t _b)
	{
		return simd_imax_ni(_a, _b);
	}

	typedef simd128_langext_t simd128_t;

} // namespace bx
#endif // BX_SIMD_LANGEXT

#if BX_SIMD_NEON
/*
 * Copyright 2010-2020 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_SIMD_T_H_HEADER_GUARD
#	error "Must be included from bx/simd_t.h!"
#endif // BX_SIMD_T_H_HEADER_GUARD

namespace bx
{
#if BX_COMPILER_CLANG

#define SHUFFLE_A(_a,  _i0, _i1, _i2, _i3)	\
__builtin_shufflevector(_a, _a, _i0, _i1, _i2, _i3 )
#define SHUFFLE_AB(_a, _b, _i0, _i1, _i2, _i3)	\
__builtin_shufflevector(_a, _b, _i0, _i1, _i2, _i3 )
	
#else

#define SHUFFLE_A(_a,  _i0, _i1, _i2, _i3)	\
__builtin_shuffle(_a, (uint32x4_t){ _i0, _i1, _i2, _i3 })
#define SHUFFLE_AB(_a, _b, _i0, _i1, _i2, _i3)	\
__builtin_shuffle(_a, _b, (uint32x4_t){ _i0, _i1, _i2, _i3 })

#endif

	
#define ELEMx 0
#define ELEMy 1
#define ELEMz 2
#define ELEMw 3
#define BX_SIMD128_IMPLEMENT_SWIZZLE(_x, _y, _z, _w)                                                  \
			template<>                                                                                \
			BX_SIMD_FORCE_INLINE simd128_neon_t simd_swiz_##_x##_y##_z##_w(simd128_neon_t _a)         \
			{                                                                                         \
				return SHUFFLE_A(_a, ELEM##_x, ELEM##_y, ELEM##_z, ELEM##_w ); \
			}

/*
 * Copyright 2010-2015 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_SIMD_T_H_HEADER_GUARD
#	error "xmacro file, must be included from simd_*.h"
#endif // BX_FLOAT4_T_H_HEADER_GUARD

// included from float4_t.h
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, z, z)
// BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, w, w)

#undef BX_SIMD128_IMPLEMENT_SWIZZLE
#undef ELEMw
#undef ELEMz
#undef ELEMy
#undef ELEMx

#define BX_SIMD128_IMPLEMENT_TEST(_xyzw, _swizzle)                                \
			template<>                                                            \
			BX_SIMD_FORCE_INLINE bool simd_test_any_##_xyzw(simd128_neon_t _test) \
			{                                                                     \
				const simd128_neon_t tmp0 = simd_swiz_##_swizzle(_test);          \
				return simd_test_any_ni(tmp0);                                    \
			}                                                                     \
			                                                                      \
			template<>                                                            \
			BX_SIMD_FORCE_INLINE bool simd_test_all_##_xyzw(simd128_neon_t _test) \
			{                                                                     \
				const simd128_neon_t tmp0 = simd_swiz_##_swizzle(_test);          \
				return simd_test_all_ni(tmp0);                                    \
			}

BX_SIMD128_IMPLEMENT_TEST(x,   xxxx);
BX_SIMD128_IMPLEMENT_TEST(y,   yyyy);
BX_SIMD128_IMPLEMENT_TEST(xy,  xyyy);
BX_SIMD128_IMPLEMENT_TEST(z,   zzzz);
BX_SIMD128_IMPLEMENT_TEST(xz,  xzzz);
BX_SIMD128_IMPLEMENT_TEST(yz,  yzzz);
BX_SIMD128_IMPLEMENT_TEST(xyz, xyzz);
BX_SIMD128_IMPLEMENT_TEST(w,   wwww);
BX_SIMD128_IMPLEMENT_TEST(xw,  xwww);
BX_SIMD128_IMPLEMENT_TEST(yw,  ywww);
BX_SIMD128_IMPLEMENT_TEST(xyw, xyww);
BX_SIMD128_IMPLEMENT_TEST(zw,  zwww);
BX_SIMD128_IMPLEMENT_TEST(xzw, xzww);
BX_SIMD128_IMPLEMENT_TEST(yzw, yzww);
#undef BX_SIMD128_IMPLEMENT_TEST

	template<>
	BX_SIMD_FORCE_INLINE bool simd_test_any_xyzw(simd128_neon_t _test)
	{
		return simd_test_any_ni(_test);
	}

	template<>
	BX_SIMD_FORCE_INLINE bool simd_test_all_xyzw(simd128_neon_t _test)
	{
		return simd_test_all_ni(_test);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_shuf_xyAB(simd128_neon_t _a, simd128_neon_t _b)
	{
		return SHUFFLE_AB(_a, _b, 0, 1, 4, 5 );
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_shuf_ABxy(simd128_neon_t _a, simd128_neon_t _b)
	{
		return SHUFFLE_AB(_a, _b, 4, 5, 0, 1 );
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_shuf_CDzw(simd128_neon_t _a, simd128_neon_t _b)
	{
		return SHUFFLE_AB(_a, _b, 6, 7, 2, 3 );
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_shuf_zwCD(simd128_neon_t _a, simd128_neon_t _b)
	{
		return SHUFFLE_AB(_a, _b, 2, 3, 6, 7 );
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_shuf_xAyB(simd128_neon_t _a, simd128_neon_t _b)
	{
		return SHUFFLE_AB(_a, _b, 0, 4, 1, 5 );
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_shuf_AxBy(simd128_neon_t _a, simd128_neon_t _b)
	{
		return SHUFFLE_AB(_a, _b, 4, 0, 5, 1 );
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_shuf_zCwD(simd128_neon_t _a, simd128_neon_t _b)
	{
		return SHUFFLE_AB(_a, _b, 2, 6, 3, 7 );
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_shuf_CzDw(simd128_neon_t _a, simd128_neon_t _b)
	{
		return SHUFFLE_AB(_a, _b, 6, 2, 7, 3 );
	}
#undef SHUFFLE_A
#undef SHUFFLE_AB

	template<>
	BX_SIMD_FORCE_INLINE float simd_x(simd128_neon_t _a)
	{
		return vgetq_lane_f32(_a, 0);
	}

	template<>
	BX_SIMD_FORCE_INLINE float simd_y(simd128_neon_t _a)
	{
		return vgetq_lane_f32(_a, 1);
	}

	template<>
	BX_SIMD_FORCE_INLINE float simd_z(simd128_neon_t _a)
	{
		return vgetq_lane_f32(_a, 2);
	}

	template<>
	BX_SIMD_FORCE_INLINE float simd_w(simd128_neon_t _a)
	{
		return vgetq_lane_f32(_a, 3);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_ld(const void* _ptr)
	{
		return vld1q_f32( (const float32_t*)_ptr);
	}

	template<>
	BX_SIMD_FORCE_INLINE void simd_st(void* _ptr, simd128_neon_t _a)
	{
		vst1q_f32( (float32_t*)_ptr, _a);
	}

	template<>
	BX_SIMD_FORCE_INLINE void simd_stx(void* _ptr, simd128_neon_t _a)
	{
		vst1q_lane_f32( (float32_t*)_ptr, _a, 0);
	}

	template<>
	BX_SIMD_FORCE_INLINE void simd_stream(void* _ptr, simd128_neon_t _a)
	{
		vst1q_f32( (float32_t*)_ptr, _a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_ld(float _x, float _y, float _z, float _w)
	{
		const float32_t val[4] = {_x, _y, _z, _w};
		return simd_ld<simd128_neon_t>(val);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_ild(uint32_t _x, uint32_t _y, uint32_t _z, uint32_t _w)
	{
		const uint32_t   val[4]    = {_x, _y, _z, _w};
		const uint32x4_t tmp       = vld1q_u32(val);
		const simd128_neon_t result = vreinterpretq_f32_u32(tmp);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_splat(const void* _ptr)
	{
		const simd128_neon_t tmp0   = vld1q_f32( (const float32_t*)_ptr);
		const float32x2_t   tmp1   = vget_low_f32(tmp0);
		const simd128_neon_t result = vdupq_lane_f32(tmp1, 0);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_splat(float _a)
	{
		return vdupq_n_f32(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_isplat(uint32_t _a)
	{
		const int32x4_t tmp    = vdupq_n_s32(_a);
		const simd128_neon_t  result = vreinterpretq_f32_s32(tmp);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_zero()
	{
		return simd_isplat<simd128_neon_t>(0);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_itof(simd128_neon_t _a)
	{
		const int32x4_t itof   = vreinterpretq_s32_f32(_a);
		const simd128_neon_t  result = vcvtq_f32_s32(itof);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_ftoi(simd128_neon_t _a)
	{
		const int32x4_t ftoi  = vcvtq_s32_f32(_a);
		const simd128_neon_t result = vreinterpretq_f32_s32(ftoi);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_add(simd128_neon_t _a, simd128_neon_t _b)
	{
		return vaddq_f32(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_sub(simd128_neon_t _a, simd128_neon_t _b)
	{
		return vsubq_f32(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_mul(simd128_neon_t _a, simd128_neon_t _b)
	{
		return vmulq_f32(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_rcp_est(simd128_neon_t _a)
	{
		return vrecpeq_f32(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_rsqrt_est(simd128_neon_t _a)
	{
		return vrsqrteq_f32(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_cmpeq(simd128_neon_t _a, simd128_neon_t _b)
	{
		const uint32x4_t tmp    = vceqq_f32(_a, _b);
		const simd128_neon_t   result = vreinterpretq_f32_u32(tmp);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_cmplt(simd128_neon_t _a, simd128_neon_t _b)
	{
		const uint32x4_t tmp    = vcltq_f32(_a, _b);
		const simd128_neon_t   result = vreinterpretq_f32_u32(tmp);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_cmple(simd128_neon_t _a, simd128_neon_t _b)
	{
		const uint32x4_t tmp    = vcleq_f32(_a, _b);
		const simd128_neon_t   result = vreinterpretq_f32_u32(tmp);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_cmpgt(simd128_neon_t _a, simd128_neon_t _b)
	{
		const uint32x4_t tmp    = vcgtq_f32(_a, _b);
		const simd128_neon_t   result = vreinterpretq_f32_u32(tmp);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_cmpge(simd128_neon_t _a, simd128_neon_t _b)
	{
		const uint32x4_t tmp    = vcgeq_f32(_a, _b);
		const simd128_neon_t   result = vreinterpretq_f32_u32(tmp);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_min(simd128_neon_t _a, simd128_neon_t _b)
	{
		return vminq_f32(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_max(simd128_neon_t _a, simd128_neon_t _b)
	{
		return vmaxq_f32(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_and(simd128_neon_t _a, simd128_neon_t _b)
	{
		const int32x4_t tmp0   = vreinterpretq_s32_f32(_a);
		const int32x4_t tmp1   = vreinterpretq_s32_f32(_b);
		const int32x4_t tmp2   = vandq_s32(tmp0, tmp1);
		const simd128_neon_t  result = vreinterpretq_f32_s32(tmp2);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_andc(simd128_neon_t _a, simd128_neon_t _b)
	{
		const int32x4_t tmp0   = vreinterpretq_s32_f32(_a);
		const int32x4_t tmp1   = vreinterpretq_s32_f32(_b);
		const int32x4_t tmp2   = vbicq_s32(tmp0, tmp1);
		const simd128_neon_t  result = vreinterpretq_f32_s32(tmp2);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_or(simd128_neon_t _a, simd128_neon_t _b)
	{
		const int32x4_t tmp0   = vreinterpretq_s32_f32(_a);
		const int32x4_t tmp1   = vreinterpretq_s32_f32(_b);
		const int32x4_t tmp2   = vorrq_s32(tmp0, tmp1);
		const simd128_neon_t  result = vreinterpretq_f32_s32(tmp2);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_xor(simd128_neon_t _a, simd128_neon_t _b)
	{
		const int32x4_t tmp0   = vreinterpretq_s32_f32(_a);
		const int32x4_t tmp1   = vreinterpretq_s32_f32(_b);
		const int32x4_t tmp2   = veorq_s32(tmp0, tmp1);
		const simd128_neon_t  result = vreinterpretq_f32_s32(tmp2);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_sll(simd128_neon_t _a, int _count)
	{
#if !BX_COMPILER_CLANG
		if (__builtin_constant_p(_count) )
		{
			const uint32x4_t tmp0   = vreinterpretq_u32_f32(_a);
			const uint32x4_t tmp1   = vshlq_n_u32(tmp0, _count);
			const simd128_neon_t   result = vreinterpretq_f32_u32(tmp1);

			return result;
		}
#endif
		const uint32x4_t tmp0   = vreinterpretq_u32_f32(_a);
		const int32x4_t  shift  = vdupq_n_s32(_count);
		const uint32x4_t tmp1   = vshlq_u32(tmp0, shift);
		const simd128_neon_t   result = vreinterpretq_f32_u32(tmp1);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_srl(simd128_neon_t _a, int _count)
	{
#if !BX_COMPILER_CLANG
		if (__builtin_constant_p(_count) )
		{
			const uint32x4_t tmp0   = vreinterpretq_u32_f32(_a);
			const uint32x4_t tmp1   = vshrq_n_u32(tmp0, _count);
			const simd128_neon_t   result = vreinterpretq_f32_u32(tmp1);

			return result;
		}
#endif
		const uint32x4_t tmp0   = vreinterpretq_u32_f32(_a);
		const int32x4_t  shift  = vdupq_n_s32(-_count);
		const uint32x4_t tmp1   = vshlq_u32(tmp0, shift);
		const simd128_neon_t   result = vreinterpretq_f32_u32(tmp1);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_sra(simd128_neon_t _a, int _count)
	{
#if !BX_COMPILER_CLANG
		if (__builtin_constant_p(_count) )
		{
			const int32x4_t tmp0   = vreinterpretq_s32_f32(_a);
			const int32x4_t tmp1   = vshrq_n_s32(tmp0, _count);
			const simd128_neon_t  result = vreinterpretq_f32_s32(tmp1);

			return result;
		}
#endif
		const int32x4_t tmp0   = vreinterpretq_s32_f32(_a);
		const int32x4_t shift  = vdupq_n_s32(-_count);
		const int32x4_t tmp1   = vshlq_s32(tmp0, shift);
		const simd128_neon_t  result = vreinterpretq_f32_s32(tmp1);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_icmpeq(simd128_neon_t _a, simd128_neon_t _b)
	{
		const int32x4_t  tmp0   = vreinterpretq_s32_f32(_a);
		const int32x4_t  tmp1   = vreinterpretq_s32_f32(_b);
		const uint32x4_t tmp2   = vceqq_s32(tmp0, tmp1);
		const simd128_neon_t   result = vreinterpretq_f32_u32(tmp2);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_icmplt(simd128_neon_t _a, simd128_neon_t _b)
	{
		const int32x4_t  tmp0   = vreinterpretq_s32_f32(_a);
		const int32x4_t  tmp1   = vreinterpretq_s32_f32(_b);
		const uint32x4_t tmp2   = vcltq_s32(tmp0, tmp1);
		const simd128_neon_t   result = vreinterpretq_f32_u32(tmp2);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_icmpgt(simd128_neon_t _a, simd128_neon_t _b)
	{
		const int32x4_t  tmp0   = vreinterpretq_s32_f32(_a);
		const int32x4_t  tmp1   = vreinterpretq_s32_f32(_b);
		const uint32x4_t tmp2   = vcgtq_s32(tmp0, tmp1);
		const simd128_neon_t   result = vreinterpretq_f32_u32(tmp2);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_imin(simd128_neon_t _a, simd128_neon_t _b)
	{
		const int32x4_t tmp0   = vreinterpretq_s32_f32(_a);
		const int32x4_t tmp1   = vreinterpretq_s32_f32(_b);
		const int32x4_t tmp2   = vminq_s32(tmp0, tmp1);
		const simd128_neon_t  result = vreinterpretq_f32_s32(tmp2);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_imax(simd128_neon_t _a, simd128_neon_t _b)
	{
		const int32x4_t tmp0   = vreinterpretq_s32_f32(_a);
		const int32x4_t tmp1   = vreinterpretq_s32_f32(_b);
		const int32x4_t tmp2   = vmaxq_s32(tmp0, tmp1);
		const simd128_neon_t  result = vreinterpretq_f32_s32(tmp2);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_iadd(simd128_neon_t _a, simd128_neon_t _b)
	{
		const int32x4_t tmp0   = vreinterpretq_s32_f32(_a);
		const int32x4_t tmp1   = vreinterpretq_s32_f32(_b);
		const int32x4_t tmp2   = vaddq_s32(tmp0, tmp1);
		const simd128_neon_t  result = vreinterpretq_f32_s32(tmp2);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_isub(simd128_neon_t _a, simd128_neon_t _b)
	{
		const int32x4_t tmp0   = vreinterpretq_s32_f32(_a);
		const int32x4_t tmp1   = vreinterpretq_s32_f32(_b);
		const int32x4_t tmp2   = vsubq_s32(tmp0, tmp1);
		const simd128_neon_t  result = vreinterpretq_f32_s32(tmp2);

		return result;
	}

	template<>
	BX_SIMD_INLINE simd128_neon_t simd_shuf_xAzC(simd128_neon_t _a, simd128_neon_t _b)
	{
		return simd_shuf_xAzC_ni(_a, _b);
	}

	template<>
	BX_SIMD_INLINE simd128_neon_t simd_shuf_yBwD(simd128_neon_t _a, simd128_neon_t _b)
	{
		return simd_shuf_yBwD_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_rcp(simd128_neon_t _a)
	{
		return simd_rcp_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_orx(simd128_neon_t _a)
	{
		return simd_orx_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_orc(simd128_neon_t _a, simd128_neon_t _b)
	{
		return simd_orc_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_neg(simd128_neon_t _a)
	{
		return simd_neg_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_madd(simd128_neon_t _a, simd128_neon_t _b, simd128_neon_t _c)
	{
		return simd_madd_ni(_a, _b, _c);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_nmsub(simd128_neon_t _a, simd128_neon_t _b, simd128_neon_t _c)
	{
		return simd_nmsub_ni(_a, _b, _c);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_div_nr(simd128_neon_t _a, simd128_neon_t _b)
	{
		return simd_div_nr_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_div(simd128_neon_t _a, simd128_neon_t _b)
	{
		return simd_div_nr_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_selb(simd128_neon_t _mask, simd128_neon_t _a, simd128_neon_t _b)
	{
		return simd_selb_ni(_mask, _a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_sels(simd128_neon_t _test, simd128_neon_t _a, simd128_neon_t _b)
	{
		return simd_sels_ni(_test, _a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_not(simd128_neon_t _a)
	{
		return simd_not_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_abs(simd128_neon_t _a)
	{
		return simd_abs_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_clamp(simd128_neon_t _a, simd128_neon_t _min, simd128_neon_t _max)
	{
		return simd_clamp_ni(_a, _min, _max);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_lerp(simd128_neon_t _a, simd128_neon_t _b, simd128_neon_t _s)
	{
		return simd_lerp_ni(_a, _b, _s);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_rsqrt(simd128_neon_t _a)
	{
		return simd_rsqrt_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_rsqrt_nr(simd128_neon_t _a)
	{
		return simd_rsqrt_nr_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_rsqrt_carmack(simd128_neon_t _a)
	{
		return simd_rsqrt_carmack_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_sqrt_nr(simd128_neon_t _a)
	{
		return simd_sqrt_nr_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_sqrt(simd128_neon_t _a)
	{
		return simd_sqrt_nr_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_log2(simd128_neon_t _a)
	{
		return simd_log2_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_exp2(simd128_neon_t _a)
	{
		return simd_exp2_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_pow(simd128_neon_t _a, simd128_neon_t _b)
	{
		return simd_pow_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_cross3(simd128_neon_t _a, simd128_neon_t _b)
	{
		return simd_cross3_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_normalize3(simd128_neon_t _a)
	{
		return simd_normalize3_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_dot3(simd128_neon_t _a, simd128_neon_t _b)
	{
		return simd_dot3_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_dot(simd128_neon_t _a, simd128_neon_t _b)
	{
		return simd_dot_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_ceil(simd128_neon_t _a)
	{
		return simd_ceil_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_floor(simd128_neon_t _a)
	{
		return simd_floor_ni(_a);
	}

	typedef simd128_neon_t simd128_t;

} // namespace bx
#endif // BX_SIMD_NEON

#if BX_SIMD_SSE
/*
 * Copyright 2010-2020 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_SIMD_T_H_HEADER_GUARD
#	error "Must be included from bx/simd_t.h!"
#endif // BX_SIMD_T_H_HEADER_GUARD

namespace bx
{
#define ELEMx 0
#define ELEMy 1
#define ELEMz 2
#define ELEMw 3
#define BX_SIMD128_IMPLEMENT_SWIZZLE(_x, _y, _z, _w)                                                   \
			template<>                                                                                 \
			BX_SIMD_FORCE_INLINE simd128_sse_t simd_swiz_##_x##_y##_z##_w(simd128_sse_t _a)            \
			{                                                                                          \
				return _mm_shuffle_ps( _a, _a, _MM_SHUFFLE(ELEM##_w, ELEM##_z, ELEM##_y, ELEM##_x ) ); \
			}

/*
 * Copyright 2010-2015 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_SIMD_T_H_HEADER_GUARD
#	error "xmacro file, must be included from simd_*.h"
#endif // BX_FLOAT4_T_H_HEADER_GUARD

// included from float4_t.h
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, z, z)
// BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, w, w)

#undef BX_SIMD128_IMPLEMENT_SWIZZLE
#undef ELEMw
#undef ELEMz
#undef ELEMy
#undef ELEMx

#define BX_SIMD128_IMPLEMENT_TEST(_xyzw, _mask)                                  \
			template<>                                                           \
			BX_SIMD_FORCE_INLINE bool simd_test_any_##_xyzw(simd128_sse_t _test) \
			{                                                                    \
				return 0x0 != (_mm_movemask_ps(_test)&(_mask) );                 \
			}                                                                    \
			                                                                     \
			template<>                                                           \
			BX_SIMD_FORCE_INLINE bool simd_test_all_##_xyzw(simd128_sse_t _test) \
			{                                                                    \
				return (_mask) == (_mm_movemask_ps(_test)&(_mask) );             \
			}

BX_SIMD128_IMPLEMENT_TEST(x    , 0x1);
BX_SIMD128_IMPLEMENT_TEST(y    , 0x2);
BX_SIMD128_IMPLEMENT_TEST(xy   , 0x3);
BX_SIMD128_IMPLEMENT_TEST(z    , 0x4);
BX_SIMD128_IMPLEMENT_TEST(xz   , 0x5);
BX_SIMD128_IMPLEMENT_TEST(yz   , 0x6);
BX_SIMD128_IMPLEMENT_TEST(xyz  , 0x7);
BX_SIMD128_IMPLEMENT_TEST(w    , 0x8);
BX_SIMD128_IMPLEMENT_TEST(xw   , 0x9);
BX_SIMD128_IMPLEMENT_TEST(yw   , 0xa);
BX_SIMD128_IMPLEMENT_TEST(xyw  , 0xb);
BX_SIMD128_IMPLEMENT_TEST(zw   , 0xc);
BX_SIMD128_IMPLEMENT_TEST(xzw  , 0xd);
BX_SIMD128_IMPLEMENT_TEST(yzw  , 0xe);
BX_SIMD128_IMPLEMENT_TEST(xyzw , 0xf);

#undef BX_SIMD128_IMPLEMENT_TEST

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_shuf_xyAB(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_movelh_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_shuf_ABxy(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_movelh_ps(_b, _a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_shuf_CDzw(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_movehl_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_shuf_zwCD(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_movehl_ps(_b, _a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_shuf_xAyB(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_unpacklo_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_shuf_AxBy(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_unpacklo_ps(_b, _a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_shuf_zCwD(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_unpackhi_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_shuf_CzDw(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_unpackhi_ps(_b, _a);
	}

	template<>
	BX_SIMD_FORCE_INLINE float simd_x(simd128_sse_t _a)
	{
		return _mm_cvtss_f32(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE float simd_y(simd128_sse_t _a)
	{
		const simd128_sse_t yyyy = simd_swiz_yyyy(_a);
		const float result  = _mm_cvtss_f32(yyyy);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE float simd_z(simd128_sse_t _a)
	{
		const simd128_sse_t zzzz = simd_swiz_zzzz(_a);
		const float result  = _mm_cvtss_f32(zzzz);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE float simd_w(simd128_sse_t _a)
	{
		const simd128_sse_t wwww = simd_swiz_wwww(_a);
		const float result  = _mm_cvtss_f32(wwww);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_ld(const void* _ptr)
	{
		return _mm_load_ps(reinterpret_cast<const float*>(_ptr) );
	}

	template<>
	BX_SIMD_FORCE_INLINE void simd_st(void* _ptr, simd128_sse_t _a)
	{
		_mm_store_ps(reinterpret_cast<float*>(_ptr), _a);
	}

	template<>
	BX_SIMD_FORCE_INLINE void simd_stx(void* _ptr, simd128_sse_t _a)
	{
		_mm_store_ss(reinterpret_cast<float*>(_ptr), _a);
	}

	template<>
	BX_SIMD_FORCE_INLINE void simd_stream(void* _ptr, simd128_sse_t _a)
	{
		_mm_stream_ps(reinterpret_cast<float*>(_ptr), _a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_ld(float _x, float _y, float _z, float _w)
	{
		return _mm_set_ps(_w, _z, _y, _x);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_ild(uint32_t _x, uint32_t _y, uint32_t _z, uint32_t _w)
	{
		const __m128i set     = _mm_set_epi32(_w, _z, _y, _x);
		const simd128_sse_t result = _mm_castsi128_ps(set);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_splat(const void* _ptr)
	{
		const simd128_sse_t x___   = _mm_load_ss(reinterpret_cast<const float*>(_ptr) );
		const simd128_sse_t result = simd_swiz_xxxx(x___);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_splat(float _a)
	{
		return _mm_set1_ps(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_isplat(uint32_t _a)
	{
		const __m128i splat   = _mm_set1_epi32(_a);
		const simd128_sse_t result = _mm_castsi128_ps(splat);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_zero()
	{
		return _mm_setzero_ps();
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_itof(simd128_sse_t _a)
	{
		const __m128i  itof   = _mm_castps_si128(_a);
		const simd128_sse_t result = _mm_cvtepi32_ps(itof);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_ftoi(simd128_sse_t _a)
	{
		const __m128i ftoi    = _mm_cvtps_epi32(_a);
		const simd128_sse_t result = _mm_castsi128_ps(ftoi);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_round(simd128_sse_t _a)
	{
#if defined(__SSE4_1__)
		return _mm_round_ps(_a, _MM_FROUND_NINT);
#else
		const __m128i round   = _mm_cvtps_epi32(_a);
		const simd128_sse_t result = _mm_cvtepi32_ps(round);

		return result;
#endif // defined(__SSE4_1__)
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_add(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_add_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_sub(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_sub_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_mul(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_mul_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_div(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_div_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_rcp_est(simd128_sse_t _a)
	{
		return _mm_rcp_ps(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_sqrt(simd128_sse_t _a)
	{
		return _mm_sqrt_ps(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_rsqrt_est(simd128_sse_t _a)
	{
		return _mm_rsqrt_ps(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_dot3(simd128_sse_t _a, simd128_sse_t _b)
	{
#if defined(__SSE4_1__)
		return _mm_dp_ps(_a, _b, 0x77);
#else
		return simd_dot3_ni(_a, _b);
#endif // defined(__SSE4__)
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_dot(simd128_sse_t _a, simd128_sse_t _b)
	{
#if defined(__SSE4_1__)
		return _mm_dp_ps(_a, _b, 0xFF);
#else
		return simd_dot_ni(_a, _b);
#endif // defined(__SSE4__)
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_cmpeq(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_cmpeq_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_cmplt(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_cmplt_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_cmple(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_cmple_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_cmpgt(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_cmpgt_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_cmpge(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_cmpge_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_min(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_min_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_max(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_max_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_and(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_and_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_andc(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_andnot_ps(_b, _a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_or(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_or_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_xor(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_xor_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_sll(simd128_sse_t _a, int _count)
	{
		const __m128i a       = _mm_castps_si128(_a);
		const __m128i shift   = _mm_slli_epi32(a, _count);
		const simd128_sse_t result = _mm_castsi128_ps(shift);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_srl(simd128_sse_t _a, int _count)
	{
		const __m128i a       = _mm_castps_si128(_a);
		const __m128i shift   = _mm_srli_epi32(a, _count);
		const simd128_sse_t result = _mm_castsi128_ps(shift);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_sra(simd128_sse_t _a, int _count)
	{
		const __m128i a       = _mm_castps_si128(_a);
		const __m128i shift   = _mm_srai_epi32(a, _count);
		const simd128_sse_t result = _mm_castsi128_ps(shift);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_icmpeq(simd128_sse_t _a, simd128_sse_t _b)
	{
		const __m128i tmp0    = _mm_castps_si128(_a);
		const __m128i tmp1    = _mm_castps_si128(_b);
		const __m128i tmp2    = _mm_cmpeq_epi32(tmp0, tmp1);
		const simd128_sse_t result = _mm_castsi128_ps(tmp2);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_icmplt(simd128_sse_t _a, simd128_sse_t _b)
	{
		const __m128i tmp0    = _mm_castps_si128(_a);
		const __m128i tmp1    = _mm_castps_si128(_b);
		const __m128i tmp2    = _mm_cmplt_epi32(tmp0, tmp1);
		const simd128_sse_t result = _mm_castsi128_ps(tmp2);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_icmpgt(simd128_sse_t _a, simd128_sse_t _b)
	{
		const __m128i tmp0    = _mm_castps_si128(_a);
		const __m128i tmp1    = _mm_castps_si128(_b);
		const __m128i tmp2    = _mm_cmpgt_epi32(tmp0, tmp1);
		const simd128_sse_t result = _mm_castsi128_ps(tmp2);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_imin(simd128_sse_t _a, simd128_sse_t _b)
	{
#if defined(__SSE4_1__)
		const __m128i tmp0    = _mm_castps_si128(_a);
		const __m128i tmp1    = _mm_castps_si128(_b);
		const __m128i tmp2    = _mm_min_epi32(tmp0, tmp1);
		const simd128_sse_t result = _mm_castsi128_ps(tmp2);

		return result;
#else
		return simd_imin_ni(_a, _b);
#endif // defined(__SSE4_1__)
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_imax(simd128_sse_t _a, simd128_sse_t _b)
	{
#if defined(__SSE4_1__)
		const __m128i tmp0    = _mm_castps_si128(_a);
		const __m128i tmp1    = _mm_castps_si128(_b);
		const __m128i tmp2    = _mm_max_epi32(tmp0, tmp1);
		const simd128_sse_t result = _mm_castsi128_ps(tmp2);

		return result;
#else
		return simd_imax_ni(_a, _b);
#endif // defined(__SSE4_1__)
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_iadd(simd128_sse_t _a, simd128_sse_t _b)
	{
		const __m128i a       = _mm_castps_si128(_a);
		const __m128i b       = _mm_castps_si128(_b);
		const __m128i add     = _mm_add_epi32(a, b);
		const simd128_sse_t result = _mm_castsi128_ps(add);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_isub(simd128_sse_t _a, simd128_sse_t _b)
	{
		const __m128i a       = _mm_castps_si128(_a);
		const __m128i b       = _mm_castps_si128(_b);
		const __m128i sub     = _mm_sub_epi32(a, b);
		const simd128_sse_t result = _mm_castsi128_ps(sub);

		return result;
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_shuf_xAzC(simd128_sse_t _a, simd128_sse_t _b)
	{
		return simd_shuf_xAzC_ni(_a, _b);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_shuf_yBwD(simd128_sse_t _a, simd128_sse_t _b)
	{
		return simd_shuf_yBwD_ni(_a, _b);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_rcp(simd128_sse_t _a)
	{
		return simd_rcp_ni(_a);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_orx(simd128_sse_t _a)
	{
		return simd_orx_ni(_a);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_orc(simd128_sse_t _a, simd128_sse_t _b)
	{
		return simd_orc_ni(_a, _b);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_neg(simd128_sse_t _a)
	{
		return simd_neg_ni(_a);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_madd(simd128_sse_t _a, simd128_sse_t _b, simd128_sse_t _c)
	{
		return simd_madd_ni(_a, _b, _c);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_nmsub(simd128_sse_t _a, simd128_sse_t _b, simd128_sse_t _c)
	{
		return simd_nmsub_ni(_a, _b, _c);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_div_nr(simd128_sse_t _a, simd128_sse_t _b)
	{
		return simd_div_nr_ni(_a, _b);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_selb(simd128_sse_t _mask, simd128_sse_t _a, simd128_sse_t _b)
	{
		return simd_selb_ni(_mask, _a, _b);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_sels(simd128_sse_t _test, simd128_sse_t _a, simd128_sse_t _b)
	{
		return simd_sels_ni(_test, _a, _b);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_not(simd128_sse_t _a)
	{
		return simd_not_ni(_a);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_abs(simd128_sse_t _a)
	{
		return simd_abs_ni(_a);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_clamp(simd128_sse_t _a, simd128_sse_t _min, simd128_sse_t _max)
	{
		return simd_clamp_ni(_a, _min, _max);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_lerp(simd128_sse_t _a, simd128_sse_t _b, simd128_sse_t _s)
	{
		return simd_lerp_ni(_a, _b, _s);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_rsqrt(simd128_sse_t _a)
	{
		return simd_rsqrt_ni(_a);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_rsqrt_nr(simd128_sse_t _a)
	{
		return simd_rsqrt_nr_ni(_a);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_rsqrt_carmack(simd128_sse_t _a)
	{
		return simd_rsqrt_carmack_ni(_a);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_sqrt_nr(simd128_sse_t _a)
	{
		return simd_sqrt_nr_ni(_a);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_log2(simd128_sse_t _a)
	{
		return simd_log2_ni(_a);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_exp2(simd128_sse_t _a)
	{
		return simd_exp2_ni(_a);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_pow(simd128_sse_t _a, simd128_sse_t _b)
	{
		return simd_pow_ni(_a, _b);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_cross3(simd128_sse_t _a, simd128_sse_t _b)
	{
		return simd_cross3_ni(_a, _b);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_normalize3(simd128_sse_t _a)
	{
		return simd_normalize3_ni(_a);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_ceil(simd128_sse_t _a)
	{
		return simd_ceil_ni(_a);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_floor(simd128_sse_t _a)
	{
		return simd_floor_ni(_a);
	}

	typedef simd128_sse_t simd128_t;

} // namespace bx
#endif // BX_SIMD_SSE

#if (  BX_SIMD_LANGEXT \
	|| BX_SIMD_NEON    \
	|| BX_SIMD_SSE     \
	|| BX_SIMD_AVX     \
	)
#	undef  BX_CONFIG_SUPPORTS_SIMD
#	define BX_CONFIG_SUPPORTS_SIMD 1
#endif // BX_SIMD_*

namespace bx
{
	union simd128_ref_t
	{
		float    fxyzw[4];
		int32_t  ixyzw[4];
		uint32_t uxyzw[4];
	};

#ifndef BX_SIMD_WARN_REFERENCE_IMPL
#	define BX_SIMD_WARN_REFERENCE_IMPL 0
#endif // BX_SIMD_WARN_REFERENCE_IMPL

#if !BX_CONFIG_SUPPORTS_SIMD
#	if BX_SIMD_WARN_REFERENCE_IMPL
#		pragma message("*** Using SIMD128 reference implementation! ***")
#	endif // BX_SIMD_WARN_REFERENCE_IMPL

	typedef simd128_ref_t simd128_t;
#endif // BX_SIMD_REFERENCE

	struct simd256_ref_t
	{
#if BX_COMPILER_MSVC
		typedef simd128_ref_t type;
#else
		typedef simd128_t type;
#endif // BX_COMPILER_MSVC

		type simd128_0;
		type simd128_1;
	};

#if !BX_SIMD_AVX
#	if BX_SIMD_WARN_REFERENCE_IMPL
#		pragma message("*** Using SIMD256 reference implementation! ***")
#	endif // BX_SIMD_WARN_REFERENCE_IMPL

	typedef simd256_ref_t simd256_t;
#endif // !BX_SIMD_AVX

	simd128_t simd_zero();

	simd128_t simd_ld(const void* _ptr);

	simd128_t simd_ld(float _x, float _y, float _z, float _w);

	simd128_t simd_ild(uint32_t _x, uint32_t _y, uint32_t _z, uint32_t _w);

	simd128_t simd_splat(const void* _ptr);

	simd128_t simd_splat(float _a);

	simd128_t simd_isplat(uint32_t _a);

} // namespace bx

/*
 * Copyright 2010-2020 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_SIMD_T_H_HEADER_GUARD
#	error "Must be included from bx/simd_t.h!"
#endif // BX_SIMD_T_H_HEADER_GUARD

namespace bx
{
	BX_CONST_FUNC float sqrt(float);
	BX_CONST_FUNC float rsqrt(float);

#define ELEMx 0
#define ELEMy 1
#define ELEMz 2
#define ELEMw 3
#define BX_SIMD128_IMPLEMENT_SWIZZLE(_x, _y, _z, _w)                                        \
			template<>                                                                      \
			BX_SIMD_FORCE_INLINE simd128_ref_t simd_swiz_##_x##_y##_z##_w(simd128_ref_t _a) \
			{                                                                               \
				simd128_ref_t result;                                                       \
				result.ixyzw[0] = _a.ixyzw[ELEM##_x];                                       \
				result.ixyzw[1] = _a.ixyzw[ELEM##_y];                                       \
				result.ixyzw[2] = _a.ixyzw[ELEM##_z];                                       \
				result.ixyzw[3] = _a.ixyzw[ELEM##_w];                                       \
				return result;                                                              \
			}

/*
 * Copyright 2010-2015 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_SIMD_T_H_HEADER_GUARD
#	error "xmacro file, must be included from simd_*.h"
#endif // BX_FLOAT4_T_H_HEADER_GUARD

// included from float4_t.h
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, x, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, z, z)
// BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, y, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, z, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(x, w, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, x, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, y, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, z, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(y, w, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, x, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, y, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, z, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(z, w, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, x, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, y, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, z, w, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, x, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, x, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, x, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, x, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, y, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, y, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, y, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, y, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, z, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, z, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, z, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, z, w)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, w, x)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, w, y)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, w, z)
BX_SIMD128_IMPLEMENT_SWIZZLE(w, w, w, w)

#undef BX_SIMD128_IMPLEMENT_SWIZZLE
#undef ELEMw
#undef ELEMz
#undef ELEMy
#undef ELEMx

#define BX_SIMD128_IMPLEMENT_TEST(_xyzw, _mask)                                  \
			template<>                                                           \
			BX_SIMD_FORCE_INLINE bool simd_test_any_##_xyzw(simd128_ref_t _test) \
			{                                                                    \
				uint32_t tmp = ( (_test.uxyzw[3]>>31)<<3)                        \
				             | ( (_test.uxyzw[2]>>31)<<2)                        \
				             | ( (_test.uxyzw[1]>>31)<<1)                        \
				             | (  _test.uxyzw[0]>>31)                            \
				             ;                                                   \
				return 0 != (tmp&(_mask) );                                      \
			}                                                                    \
			                                                                     \
			template<>                                                           \
			BX_SIMD_FORCE_INLINE bool simd_test_all_##_xyzw(simd128_ref_t _test) \
			{                                                                    \
				uint32_t tmp = ( (_test.uxyzw[3]>>31)<<3)                        \
				             | ( (_test.uxyzw[2]>>31)<<2)                        \
				             | ( (_test.uxyzw[1]>>31)<<1)                        \
				             | (  _test.uxyzw[0]>>31)                            \
				             ;                                                   \
				return (_mask) == (tmp&(_mask) );                                \
			}

BX_SIMD128_IMPLEMENT_TEST(x    , 0x1);
BX_SIMD128_IMPLEMENT_TEST(y    , 0x2);
BX_SIMD128_IMPLEMENT_TEST(xy   , 0x3);
BX_SIMD128_IMPLEMENT_TEST(z    , 0x4);
BX_SIMD128_IMPLEMENT_TEST(xz   , 0x5);
BX_SIMD128_IMPLEMENT_TEST(yz   , 0x6);
BX_SIMD128_IMPLEMENT_TEST(xyz  , 0x7);
BX_SIMD128_IMPLEMENT_TEST(w    , 0x8);
BX_SIMD128_IMPLEMENT_TEST(xw   , 0x9);
BX_SIMD128_IMPLEMENT_TEST(yw   , 0xa);
BX_SIMD128_IMPLEMENT_TEST(xyw  , 0xb);
BX_SIMD128_IMPLEMENT_TEST(zw   , 0xc);
BX_SIMD128_IMPLEMENT_TEST(xzw  , 0xd);
BX_SIMD128_IMPLEMENT_TEST(yzw  , 0xe);
BX_SIMD128_IMPLEMENT_TEST(xyzw , 0xf);

#undef BX_SIMD128_IMPLEMENT_TEST

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_shuf_xyAB(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.uxyzw[0] = _a.uxyzw[0];
		result.uxyzw[1] = _a.uxyzw[1];
		result.uxyzw[2] = _b.uxyzw[0];
		result.uxyzw[3] = _b.uxyzw[1];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_shuf_ABxy(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.uxyzw[0] = _b.uxyzw[0];
		result.uxyzw[1] = _b.uxyzw[1];
		result.uxyzw[2] = _a.uxyzw[0];
		result.uxyzw[3] = _a.uxyzw[1];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_shuf_CDzw(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.uxyzw[0] = _b.uxyzw[2];
		result.uxyzw[1] = _b.uxyzw[3];
		result.uxyzw[2] = _a.uxyzw[2];
		result.uxyzw[3] = _a.uxyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_shuf_zwCD(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.uxyzw[0] = _a.uxyzw[2];
		result.uxyzw[1] = _a.uxyzw[3];
		result.uxyzw[2] = _b.uxyzw[2];
		result.uxyzw[3] = _b.uxyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_shuf_xAyB(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.uxyzw[0] = _a.uxyzw[0];
		result.uxyzw[1] = _b.uxyzw[0];
		result.uxyzw[2] = _a.uxyzw[1];
		result.uxyzw[3] = _b.uxyzw[1];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_shuf_AxBy(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.uxyzw[0] = _a.uxyzw[1];
		result.uxyzw[1] = _b.uxyzw[1];
		result.uxyzw[2] = _a.uxyzw[0];
		result.uxyzw[3] = _b.uxyzw[0];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_shuf_zCwD(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.uxyzw[0] = _a.uxyzw[2];
		result.uxyzw[1] = _b.uxyzw[2];
		result.uxyzw[2] = _a.uxyzw[3];
		result.uxyzw[3] = _b.uxyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_shuf_CzDw(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.uxyzw[0] = _b.uxyzw[2];
		result.uxyzw[1] = _a.uxyzw[2];
		result.uxyzw[2] = _b.uxyzw[3];
		result.uxyzw[3] = _a.uxyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE float simd_x(simd128_ref_t _a)
	{
		return _a.fxyzw[0];
	}

	template<>
	BX_SIMD_FORCE_INLINE float simd_y(simd128_ref_t _a)
	{
		return _a.fxyzw[1];
	}

	template<>
	BX_SIMD_FORCE_INLINE float simd_z(simd128_ref_t _a)
	{
		return _a.fxyzw[2];
	}

	template<>
	BX_SIMD_FORCE_INLINE float simd_w(simd128_ref_t _a)
	{
		return _a.fxyzw[3];
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_ld(const void* _ptr)
	{
		const uint32_t* input = reinterpret_cast<const uint32_t*>(_ptr);
		simd128_ref_t result;
		result.uxyzw[0] = input[0];
		result.uxyzw[1] = input[1];
		result.uxyzw[2] = input[2];
		result.uxyzw[3] = input[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE void simd_st(void* _ptr, simd128_ref_t _a)
	{
		uint32_t* result = reinterpret_cast<uint32_t*>(_ptr);
		result[0] = _a.uxyzw[0];
		result[1] = _a.uxyzw[1];
		result[2] = _a.uxyzw[2];
		result[3] = _a.uxyzw[3];
	}

	template<>
	BX_SIMD_FORCE_INLINE void simd_stx(void* _ptr, simd128_ref_t _a)
	{
		uint32_t* result = reinterpret_cast<uint32_t*>(_ptr);
		result[0] = _a.uxyzw[0];
	}

	template<>
	BX_SIMD_FORCE_INLINE void simd_stream(void* _ptr, simd128_ref_t _a)
	{
		uint32_t* result = reinterpret_cast<uint32_t*>(_ptr);
		result[0] = _a.uxyzw[0];
		result[1] = _a.uxyzw[1];
		result[2] = _a.uxyzw[2];
		result[3] = _a.uxyzw[3];
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_ld(float _x, float _y, float _z, float _w)
	{
		simd128_ref_t result;
		result.fxyzw[0] = _x;
		result.fxyzw[1] = _y;
		result.fxyzw[2] = _z;
		result.fxyzw[3] = _w;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_ild(uint32_t _x, uint32_t _y, uint32_t _z, uint32_t _w)
	{
		simd128_ref_t result;
		result.uxyzw[0] = _x;
		result.uxyzw[1] = _y;
		result.uxyzw[2] = _z;
		result.uxyzw[3] = _w;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_splat(const void* _ptr)
	{
		const uint32_t val = *reinterpret_cast<const uint32_t*>(_ptr);
		simd128_ref_t result;
		result.uxyzw[0] = val;
		result.uxyzw[1] = val;
		result.uxyzw[2] = val;
		result.uxyzw[3] = val;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_splat(float _a)
	{
		return simd_ld<simd128_ref_t>(_a, _a, _a, _a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_isplat(uint32_t _a)
	{
		return simd_ild<simd128_ref_t>(_a, _a, _a, _a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_zero()
	{
		return simd_ild<simd128_ref_t>(0, 0, 0, 0);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_itof(simd128_ref_t _a)
	{
		simd128_ref_t result;
		result.fxyzw[0] = (float)_a.ixyzw[0];
		result.fxyzw[1] = (float)_a.ixyzw[1];
		result.fxyzw[2] = (float)_a.ixyzw[2];
		result.fxyzw[3] = (float)_a.ixyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_ftoi(simd128_ref_t _a)
	{
		simd128_ref_t result;
		result.ixyzw[0] = (int)_a.fxyzw[0];
		result.ixyzw[1] = (int)_a.fxyzw[1];
		result.ixyzw[2] = (int)_a.fxyzw[2];
		result.ixyzw[3] = (int)_a.fxyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_round(simd128_ref_t _a)
	{
		return simd_round_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_add(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.fxyzw[0] = _a.fxyzw[0] + _b.fxyzw[0];
		result.fxyzw[1] = _a.fxyzw[1] + _b.fxyzw[1];
		result.fxyzw[2] = _a.fxyzw[2] + _b.fxyzw[2];
		result.fxyzw[3] = _a.fxyzw[3] + _b.fxyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_sub(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.fxyzw[0] = _a.fxyzw[0] - _b.fxyzw[0];
		result.fxyzw[1] = _a.fxyzw[1] - _b.fxyzw[1];
		result.fxyzw[2] = _a.fxyzw[2] - _b.fxyzw[2];
		result.fxyzw[3] = _a.fxyzw[3] - _b.fxyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_mul(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.fxyzw[0] = _a.fxyzw[0] * _b.fxyzw[0];
		result.fxyzw[1] = _a.fxyzw[1] * _b.fxyzw[1];
		result.fxyzw[2] = _a.fxyzw[2] * _b.fxyzw[2];
		result.fxyzw[3] = _a.fxyzw[3] * _b.fxyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_div(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.fxyzw[0] = _a.fxyzw[0] / _b.fxyzw[0];
		result.fxyzw[1] = _a.fxyzw[1] / _b.fxyzw[1];
		result.fxyzw[2] = _a.fxyzw[2] / _b.fxyzw[2];
		result.fxyzw[3] = _a.fxyzw[3] / _b.fxyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_rcp_est(simd128_ref_t _a)
	{
		simd128_ref_t result;
		result.fxyzw[0] = 1.0f / _a.fxyzw[0];
		result.fxyzw[1] = 1.0f / _a.fxyzw[1];
		result.fxyzw[2] = 1.0f / _a.fxyzw[2];
		result.fxyzw[3] = 1.0f / _a.fxyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_sqrt(simd128_ref_t _a)
	{
		simd128_ref_t result;
		result.fxyzw[0] = sqrt(_a.fxyzw[0]);
		result.fxyzw[1] = sqrt(_a.fxyzw[1]);
		result.fxyzw[2] = sqrt(_a.fxyzw[2]);
		result.fxyzw[3] = sqrt(_a.fxyzw[3]);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_rsqrt_est(simd128_ref_t _a)
	{
		simd128_ref_t result;
		result.fxyzw[0] = rsqrt(_a.fxyzw[0]);
		result.fxyzw[1] = rsqrt(_a.fxyzw[1]);
		result.fxyzw[2] = rsqrt(_a.fxyzw[2]);
		result.fxyzw[3] = rsqrt(_a.fxyzw[3]);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_cmpeq(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.ixyzw[0] = _a.fxyzw[0] == _b.fxyzw[0] ? 0xffffffff : 0x0;
		result.ixyzw[1] = _a.fxyzw[1] == _b.fxyzw[1] ? 0xffffffff : 0x0;
		result.ixyzw[2] = _a.fxyzw[2] == _b.fxyzw[2] ? 0xffffffff : 0x0;
		result.ixyzw[3] = _a.fxyzw[3] == _b.fxyzw[3] ? 0xffffffff : 0x0;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_cmplt(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.ixyzw[0] = _a.fxyzw[0] < _b.fxyzw[0] ? 0xffffffff : 0x0;
		result.ixyzw[1] = _a.fxyzw[1] < _b.fxyzw[1] ? 0xffffffff : 0x0;
		result.ixyzw[2] = _a.fxyzw[2] < _b.fxyzw[2] ? 0xffffffff : 0x0;
		result.ixyzw[3] = _a.fxyzw[3] < _b.fxyzw[3] ? 0xffffffff : 0x0;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_cmple(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.ixyzw[0] = _a.fxyzw[0] <= _b.fxyzw[0] ? 0xffffffff : 0x0;
		result.ixyzw[1] = _a.fxyzw[1] <= _b.fxyzw[1] ? 0xffffffff : 0x0;
		result.ixyzw[2] = _a.fxyzw[2] <= _b.fxyzw[2] ? 0xffffffff : 0x0;
		result.ixyzw[3] = _a.fxyzw[3] <= _b.fxyzw[3] ? 0xffffffff : 0x0;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_cmpgt(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.ixyzw[0] = _a.fxyzw[0] > _b.fxyzw[0] ? 0xffffffff : 0x0;
		result.ixyzw[1] = _a.fxyzw[1] > _b.fxyzw[1] ? 0xffffffff : 0x0;
		result.ixyzw[2] = _a.fxyzw[2] > _b.fxyzw[2] ? 0xffffffff : 0x0;
		result.ixyzw[3] = _a.fxyzw[3] > _b.fxyzw[3] ? 0xffffffff : 0x0;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_cmpge(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.ixyzw[0] = _a.fxyzw[0] >= _b.fxyzw[0] ? 0xffffffff : 0x0;
		result.ixyzw[1] = _a.fxyzw[1] >= _b.fxyzw[1] ? 0xffffffff : 0x0;
		result.ixyzw[2] = _a.fxyzw[2] >= _b.fxyzw[2] ? 0xffffffff : 0x0;
		result.ixyzw[3] = _a.fxyzw[3] >= _b.fxyzw[3] ? 0xffffffff : 0x0;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_min(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.fxyzw[0] = _a.fxyzw[0] < _b.fxyzw[0] ? _a.fxyzw[0] : _b.fxyzw[0];
		result.fxyzw[1] = _a.fxyzw[1] < _b.fxyzw[1] ? _a.fxyzw[1] : _b.fxyzw[1];
		result.fxyzw[2] = _a.fxyzw[2] < _b.fxyzw[2] ? _a.fxyzw[2] : _b.fxyzw[2];
		result.fxyzw[3] = _a.fxyzw[3] < _b.fxyzw[3] ? _a.fxyzw[3] : _b.fxyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_max(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.fxyzw[0] = _a.fxyzw[0] > _b.fxyzw[0] ? _a.fxyzw[0] : _b.fxyzw[0];
		result.fxyzw[1] = _a.fxyzw[1] > _b.fxyzw[1] ? _a.fxyzw[1] : _b.fxyzw[1];
		result.fxyzw[2] = _a.fxyzw[2] > _b.fxyzw[2] ? _a.fxyzw[2] : _b.fxyzw[2];
		result.fxyzw[3] = _a.fxyzw[3] > _b.fxyzw[3] ? _a.fxyzw[3] : _b.fxyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_and(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.uxyzw[0] = _a.uxyzw[0] & _b.uxyzw[0];
		result.uxyzw[1] = _a.uxyzw[1] & _b.uxyzw[1];
		result.uxyzw[2] = _a.uxyzw[2] & _b.uxyzw[2];
		result.uxyzw[3] = _a.uxyzw[3] & _b.uxyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_andc(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.uxyzw[0] = _a.uxyzw[0] & ~_b.uxyzw[0];
		result.uxyzw[1] = _a.uxyzw[1] & ~_b.uxyzw[1];
		result.uxyzw[2] = _a.uxyzw[2] & ~_b.uxyzw[2];
		result.uxyzw[3] = _a.uxyzw[3] & ~_b.uxyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_or(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.uxyzw[0] = _a.uxyzw[0] | _b.uxyzw[0];
		result.uxyzw[1] = _a.uxyzw[1] | _b.uxyzw[1];
		result.uxyzw[2] = _a.uxyzw[2] | _b.uxyzw[2];
		result.uxyzw[3] = _a.uxyzw[3] | _b.uxyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_xor(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.uxyzw[0] = _a.uxyzw[0] ^ _b.uxyzw[0];
		result.uxyzw[1] = _a.uxyzw[1] ^ _b.uxyzw[1];
		result.uxyzw[2] = _a.uxyzw[2] ^ _b.uxyzw[2];
		result.uxyzw[3] = _a.uxyzw[3] ^ _b.uxyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_sll(simd128_ref_t _a, int _count)
	{
		simd128_ref_t result;
		result.uxyzw[0] = _a.uxyzw[0] << _count;
		result.uxyzw[1] = _a.uxyzw[1] << _count;
		result.uxyzw[2] = _a.uxyzw[2] << _count;
		result.uxyzw[3] = _a.uxyzw[3] << _count;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_srl(simd128_ref_t _a, int _count)
	{
		simd128_ref_t result;
		result.uxyzw[0] = _a.uxyzw[0] >> _count;
		result.uxyzw[1] = _a.uxyzw[1] >> _count;
		result.uxyzw[2] = _a.uxyzw[2] >> _count;
		result.uxyzw[3] = _a.uxyzw[3] >> _count;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_sra(simd128_ref_t _a, int _count)
	{
		simd128_ref_t result;
		result.ixyzw[0] = _a.ixyzw[0] >> _count;
		result.ixyzw[1] = _a.ixyzw[1] >> _count;
		result.ixyzw[2] = _a.ixyzw[2] >> _count;
		result.ixyzw[3] = _a.ixyzw[3] >> _count;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_icmpeq(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.ixyzw[0] = _a.ixyzw[0] == _b.ixyzw[0] ? 0xffffffff : 0x0;
		result.ixyzw[1] = _a.ixyzw[1] == _b.ixyzw[1] ? 0xffffffff : 0x0;
		result.ixyzw[2] = _a.ixyzw[2] == _b.ixyzw[2] ? 0xffffffff : 0x0;
		result.ixyzw[3] = _a.ixyzw[3] == _b.ixyzw[3] ? 0xffffffff : 0x0;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_icmplt(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.ixyzw[0] = _a.ixyzw[0] < _b.ixyzw[0] ? 0xffffffff : 0x0;
		result.ixyzw[1] = _a.ixyzw[1] < _b.ixyzw[1] ? 0xffffffff : 0x0;
		result.ixyzw[2] = _a.ixyzw[2] < _b.ixyzw[2] ? 0xffffffff : 0x0;
		result.ixyzw[3] = _a.ixyzw[3] < _b.ixyzw[3] ? 0xffffffff : 0x0;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_icmpgt(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.ixyzw[0] = _a.ixyzw[0] > _b.ixyzw[0] ? 0xffffffff : 0x0;
		result.ixyzw[1] = _a.ixyzw[1] > _b.ixyzw[1] ? 0xffffffff : 0x0;
		result.ixyzw[2] = _a.ixyzw[2] > _b.ixyzw[2] ? 0xffffffff : 0x0;
		result.ixyzw[3] = _a.ixyzw[3] > _b.ixyzw[3] ? 0xffffffff : 0x0;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_imin(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.ixyzw[0] = _a.ixyzw[0] < _b.ixyzw[0] ? _a.ixyzw[0] : _b.ixyzw[0];
		result.ixyzw[1] = _a.ixyzw[1] < _b.ixyzw[1] ? _a.ixyzw[1] : _b.ixyzw[1];
		result.ixyzw[2] = _a.ixyzw[2] < _b.ixyzw[2] ? _a.ixyzw[2] : _b.ixyzw[2];
		result.ixyzw[3] = _a.ixyzw[3] < _b.ixyzw[3] ? _a.ixyzw[3] : _b.ixyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_imax(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.ixyzw[0] = _a.ixyzw[0] > _b.ixyzw[0] ? _a.ixyzw[0] : _b.ixyzw[0];
		result.ixyzw[1] = _a.ixyzw[1] > _b.ixyzw[1] ? _a.ixyzw[1] : _b.ixyzw[1];
		result.ixyzw[2] = _a.ixyzw[2] > _b.ixyzw[2] ? _a.ixyzw[2] : _b.ixyzw[2];
		result.ixyzw[3] = _a.ixyzw[3] > _b.ixyzw[3] ? _a.ixyzw[3] : _b.ixyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_iadd(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.ixyzw[0] = _a.ixyzw[0] + _b.ixyzw[0];
		result.ixyzw[1] = _a.ixyzw[1] + _b.ixyzw[1];
		result.ixyzw[2] = _a.ixyzw[2] + _b.ixyzw[2];
		result.ixyzw[3] = _a.ixyzw[3] + _b.ixyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_isub(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.ixyzw[0] = _a.ixyzw[0] - _b.ixyzw[0];
		result.ixyzw[1] = _a.ixyzw[1] - _b.ixyzw[1];
		result.ixyzw[2] = _a.ixyzw[2] - _b.ixyzw[2];
		result.ixyzw[3] = _a.ixyzw[3] - _b.ixyzw[3];
		return result;
	}

	BX_SIMD_FORCE_INLINE simd128_t simd_zero()
	{
		return simd_zero<simd128_t>();
	}

	BX_SIMD_FORCE_INLINE simd128_t simd_ld(const void* _ptr)
	{
		return simd_ld<simd128_t>(_ptr);
	}

	BX_SIMD_FORCE_INLINE simd128_t simd_ld(float _x, float _y, float _z, float _w)
	{
		return simd_ld<simd128_t>(_x, _y, _z, _w);
	}

	BX_SIMD_FORCE_INLINE simd128_t simd_ild(uint32_t _x, uint32_t _y, uint32_t _z, uint32_t _w)
	{
		return simd_ild<simd128_t>(_x, _y, _z, _w);
	}

	BX_SIMD_FORCE_INLINE simd128_t simd_splat(const void* _ptr)
	{
		return simd_splat<simd128_t>(_ptr);
	}

	BX_SIMD_FORCE_INLINE simd128_t simd_splat(float _a)
	{
		return simd_splat<simd128_t>(_a);
	}

	BX_SIMD_FORCE_INLINE simd128_t simd_isplat(uint32_t _a)
	{
		return simd_isplat<simd128_t>(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_shuf_xAzC(simd128_ref_t _a, simd128_ref_t _b)
	{
		return simd_shuf_xAzC_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_shuf_yBwD(simd128_ref_t _a, simd128_ref_t _b)
	{
		return simd_shuf_yBwD_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_rcp(simd128_ref_t _a)
	{
		return simd_rcp_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_orx(simd128_ref_t _a)
	{
		return simd_orx_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_orc(simd128_ref_t _a, simd128_ref_t _b)
	{
		return simd_orc_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_neg(simd128_ref_t _a)
	{
		return simd_neg_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_madd(simd128_ref_t _a, simd128_ref_t _b, simd128_ref_t _c)
	{
		return simd_madd_ni(_a, _b, _c);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_nmsub(simd128_ref_t _a, simd128_ref_t _b, simd128_ref_t _c)
	{
		return simd_nmsub_ni(_a, _b, _c);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_div_nr(simd128_ref_t _a, simd128_ref_t _b)
	{
		return simd_div_nr_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_selb(simd128_ref_t _mask, simd128_ref_t _a, simd128_ref_t _b)
	{
		return simd_selb_ni(_mask, _a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_sels(simd128_ref_t _test, simd128_ref_t _a, simd128_ref_t _b)
	{
		return simd_sels_ni(_test, _a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_not(simd128_ref_t _a)
	{
		return simd_not_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_abs(simd128_ref_t _a)
	{
		return simd_abs_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_clamp(simd128_ref_t _a, simd128_ref_t _min, simd128_ref_t _max)
	{
		return simd_clamp_ni(_a, _min, _max);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_lerp(simd128_ref_t _a, simd128_ref_t _b, simd128_ref_t _s)
	{
		return simd_lerp_ni(_a, _b, _s);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_rsqrt(simd128_ref_t _a)
	{
		return simd_rsqrt_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_rsqrt_nr(simd128_ref_t _a)
	{
		return simd_rsqrt_nr_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_rsqrt_carmack(simd128_ref_t _a)
	{
		return simd_rsqrt_carmack_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_sqrt_nr(simd128_ref_t _a)
	{
		return simd_sqrt_nr_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_log2(simd128_ref_t _a)
	{
		return simd_log2_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_exp2(simd128_ref_t _a)
	{
		return simd_exp2_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_pow(simd128_ref_t _a, simd128_ref_t _b)
	{
		return simd_pow_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_cross3(simd128_ref_t _a, simd128_ref_t _b)
	{
		return simd_cross3_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_normalize3(simd128_ref_t _a)
	{
		return simd_normalize3_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_dot3(simd128_ref_t _a, simd128_ref_t _b)
	{
		return simd_dot3_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_dot(simd128_ref_t _a, simd128_ref_t _b)
	{
		return simd_dot_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_ceil(simd128_ref_t _a)
	{
		return simd_ceil_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_floor(simd128_ref_t _a)
	{
		return simd_floor_ni(_a);
	}

} // namespace bx
/*
 * Copyright 2010-2020 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_SIMD_T_H_HEADER_GUARD
#	error "Must be included from bx/simd_t.h!"
#endif // BX_SIMD_T_H_HEADER_GUARD

namespace bx
{
	template<>
	BX_SIMD_FORCE_INLINE simd256_ref_t simd_ld(const void* _ptr)
	{
		const simd256_ref_t::type* ptr = reinterpret_cast<const simd256_ref_t::type*>(_ptr);
		simd256_ref_t result;
		result.simd128_0 = simd_ld<simd256_ref_t::type>(&ptr[0]);
		result.simd128_1 = simd_ld<simd256_ref_t::type>(&ptr[1]);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE void simd_st(void* _ptr, simd256_ref_t& _a)
	{
		simd256_ref_t* result = reinterpret_cast<simd256_ref_t*>(_ptr);
		simd_st<simd256_ref_t::type>(&result[0], _a.simd128_0);
		simd_st<simd256_ref_t::type>(&result[1], _a.simd128_1);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd256_ref_t simd_ld(float _x, float _y, float _z, float _w, float _a, float _b, float _c, float _d)
	{
		simd256_ref_t result;
		result.simd128_0 = simd_ld<simd256_ref_t::type>(_x, _y, _z, _w);
		result.simd128_1 = simd_ld<simd256_ref_t::type>(_a, _b, _c, _d);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd256_ref_t simd_ild(uint32_t _x, uint32_t _y, uint32_t _z, uint32_t _w, uint32_t _a, uint32_t _b, uint32_t _c, uint32_t _d)
	{
		simd256_ref_t result;
		result.simd128_0 = simd_ild<simd256_ref_t::type>(_x, _y, _z, _w);
		result.simd128_1 = simd_ild<simd256_ref_t::type>(_a, _b, _c, _d);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd256_ref_t simd_splat(float _a)
	{
		simd256_ref_t result;
		result.simd128_0 = simd_splat<simd256_ref_t::type>(_a);
		result.simd128_1 = simd_splat<simd256_ref_t::type>(_a);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd256_ref_t simd_isplat(uint32_t _a)
	{
		simd256_ref_t result;
		result.simd128_0 = simd_isplat<simd256_ref_t::type>(_a);
		result.simd128_1 = simd_isplat<simd256_ref_t::type>(_a);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd256_ref_t simd_itof(simd256_ref_t _a)
	{
		simd256_ref_t result;
		result.simd128_0 = simd_itof(_a.simd128_0);
		result.simd128_1 = simd_itof(_a.simd128_1);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd256_ref_t simd_ftoi(simd256_ref_t _a)
	{
		simd256_ref_t result;
		result.simd128_0 = simd_ftoi(_a.simd128_0);
		result.simd128_1 = simd_ftoi(_a.simd128_1);
		return result;
	}

} // namespace bx

/*
 * Copyright 2010-2020 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

namespace bx
{
	template<typename Ty>
	BX_SIMD_INLINE Ty simd_shuf_xAzC_ni(Ty _a, Ty _b)
	{
		const Ty xAyB   = simd_shuf_xAyB(_a, _b);
		const Ty zCwD   = simd_shuf_zCwD(_a, _b);
		const Ty result = simd_shuf_xyAB(xAyB, zCwD);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_shuf_yBwD_ni(Ty _a, Ty _b)
	{
		const Ty xAyB   = simd_shuf_xAyB(_a, _b);
		const Ty zCwD   = simd_shuf_zCwD(_a, _b);
		const Ty result = simd_shuf_zwCD(xAyB, zCwD);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_madd_ni(Ty _a, Ty _b, Ty _c)
	{
		const Ty mul    = simd_mul(_a, _b);
		const Ty result = simd_add(mul, _c);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_nmsub_ni(Ty _a, Ty _b, Ty _c)
	{
		const Ty mul    = simd_mul(_a, _b);
		const Ty result = simd_sub(_c, mul);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_div_nr_ni(Ty _a, Ty _b)
	{
		const Ty oneish  = simd_isplat<Ty>(0x3f800001);
		const Ty est     = simd_rcp_est(_b);
		const Ty iter0   = simd_mul(_a, est);
		const Ty tmp1    = simd_nmsub(_b, est, oneish);
		const Ty result  = simd_madd(tmp1, iter0, iter0);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_rcp_ni(Ty _a)
	{
		const Ty one    = simd_splat<Ty>(1.0f);
		const Ty result = simd_div(one, _a);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_orx_ni(Ty _a)
	{
		const Ty zwxy   = simd_swiz_zwxy(_a);
		const Ty tmp0   = simd_or(_a, zwxy);
		const Ty tmp1   = simd_swiz_yyyy(_a);
		const Ty tmp2   = simd_or(tmp0, tmp1);
		const Ty mf000  = simd_ild<Ty>(UINT32_MAX, 0, 0, 0);
		const Ty result = simd_and(tmp2, mf000);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_orc_ni(Ty _a, Ty _b)
	{
		const Ty aorb   = simd_or(_a, _b);
		const Ty mffff  = simd_isplat<Ty>(UINT32_MAX);
		const Ty result = simd_xor(aorb, mffff);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_neg_ni(Ty _a)
	{
		const Ty zero   = simd_zero<Ty>();
		const Ty result = simd_sub(zero, _a);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_selb_ni(Ty _mask, Ty _a, Ty _b)
	{
		const Ty sel_a  = simd_and(_a, _mask);
		const Ty sel_b  = simd_andc(_b, _mask);
		const Ty result = simd_or(sel_a, sel_b);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_sels_ni(Ty _test, Ty _a, Ty _b)
	{
		const Ty mask   = simd_sra(_test, 31);
		const Ty result = simd_selb(mask, _a, _b);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_not_ni(Ty _a)
	{
		const Ty mffff  = simd_isplat<Ty>(UINT32_MAX);
		const Ty result = simd_xor(_a, mffff);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_min_ni(Ty _a, Ty _b)
	{
		const Ty mask   = simd_cmplt(_a, _b);
		const Ty result = simd_selb(mask, _a, _b);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_max_ni(Ty _a, Ty _b)
	{
		const Ty mask   = simd_cmpgt(_a, _b);
		const Ty result = simd_selb(mask, _a, _b);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_abs_ni(Ty _a)
	{
		const Ty a_neg  = simd_neg(_a);
		const Ty result = simd_max(a_neg, _a);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_imin_ni(Ty _a, Ty _b)
	{
		const Ty mask   = simd_icmplt(_a, _b);
		const Ty result = simd_selb(mask, _a, _b);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_imax_ni(Ty _a, Ty _b)
	{
		const Ty mask   = simd_icmpgt(_a, _b);
		const Ty result = simd_selb(mask, _a, _b);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_clamp_ni(Ty _a, Ty _min, Ty _max)
	{
		const Ty tmp    = simd_min(_a, _max);
		const Ty result = simd_max(tmp, _min);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_lerp_ni(Ty _a, Ty _b, Ty _s)
	{
		const Ty ba     = simd_sub(_b, _a);
		const Ty result = simd_madd(_s, ba, _a);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_sqrt_nr_ni(Ty _a)
	{
		const Ty half   = simd_splat<Ty>(0.5f);
		const Ty one    = simd_splat<Ty>(1.0f);
		const Ty tmp0   = simd_rsqrt_est(_a);
		const Ty tmp1   = simd_mul(tmp0, _a);
		const Ty tmp2   = simd_mul(tmp1, half);
		const Ty tmp3   = simd_nmsub(tmp0, tmp1, one);
		const Ty result = simd_madd(tmp3, tmp2, tmp1);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_sqrt_nr1_ni(Ty _a)
	{
		const Ty half = simd_splat<Ty>(0.5f);

		Ty result = _a;
		for (uint32_t ii = 0; ii < 11; ++ii)
		{
			const Ty tmp1 = simd_div(_a, result);
			const Ty tmp2 = simd_add(tmp1, result);
			result        = simd_mul(tmp2, half);
		}

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_rsqrt_ni(Ty _a)
	{
		const Ty one    = simd_splat<Ty>(1.0f);
		const Ty sqrt   = simd_sqrt(_a);
		const Ty result = simd_div(one, sqrt);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_rsqrt_nr_ni(Ty _a)
	{
		const Ty rsqrt           = simd_rsqrt_est(_a);
		const Ty iter0           = simd_mul(_a, rsqrt);
		const Ty iter1           = simd_mul(iter0, rsqrt);
		const Ty half            = simd_splat<Ty>(0.5f);
		const Ty half_rsqrt      = simd_mul(half, rsqrt);
		const Ty three           = simd_splat<Ty>(3.0f);
		const Ty three_sub_iter1 = simd_sub(three, iter1);
		const Ty result          = simd_mul(half_rsqrt, three_sub_iter1);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_rsqrt_carmack_ni(Ty _a)
	{
		const Ty half    = simd_splat<Ty>(0.5f);
		const Ty ah      = simd_mul(half, _a);
		const Ty ashift  = simd_sra(_a, 1);
		const Ty magic   = simd_isplat<Ty>(0x5f3759df);
		const Ty msuba   = simd_isub(magic, ashift);
		const Ty msubasq = simd_mul(msuba, msuba);
		const Ty tmp0    = simd_splat<Ty>(1.5f);
		const Ty tmp1    = simd_mul(ah, msubasq);
		const Ty tmp2    = simd_sub(tmp0, tmp1);
		const Ty result  = simd_mul(msuba, tmp2);

		return result;
	}

	namespace simd_logexp_detail
	{
		template<typename Ty>
		BX_SIMD_INLINE Ty simd_poly1(Ty _a, float _b, float _c)
		{
			const Ty bbbb   = simd_splat<Ty>(_b);
			const Ty cccc   = simd_splat<Ty>(_c);
			const Ty result = simd_madd(cccc, _a, bbbb);

			return result;
		}

		template<typename Ty>
		BX_SIMD_INLINE Ty simd_poly2(Ty _a, float _b, float _c, float _d)
		{
			const Ty bbbb   = simd_splat<Ty>(_b);
			const Ty poly   = simd_poly1(_a, _c, _d);
			const Ty result = simd_madd(poly, _a, bbbb);

			return result;
		}

		template<typename Ty>
		BX_SIMD_INLINE Ty simd_poly3(Ty _a, float _b, float _c, float _d, float _e)
		{
			const Ty bbbb   = simd_splat<Ty>(_b);
			const Ty poly   = simd_poly2(_a, _c, _d, _e);
			const Ty result = simd_madd(poly, _a, bbbb);

			return result;
		}

		template<typename Ty>
		BX_SIMD_INLINE Ty simd_poly4(Ty _a, float _b, float _c, float _d, float _e, float _f)
		{
			const Ty bbbb   = simd_splat<Ty>(_b);
			const Ty poly   = simd_poly3(_a, _c, _d, _e, _f);
			const Ty result = simd_madd(poly, _a, bbbb);

			return result;
		}

		template<typename Ty>
		BX_SIMD_INLINE Ty simd_poly5(Ty _a, float _b, float _c, float _d, float _e, float _f, float _g)
		{
			const Ty bbbb   = simd_splat<Ty>(_b);
			const Ty poly   = simd_poly4(_a, _c, _d, _e, _f, _g);
			const Ty result = simd_madd(poly, _a, bbbb);

			return result;
		}

		template<typename Ty>
		BX_SIMD_INLINE Ty simd_logpoly(Ty _a)
		{
#if 1
			const Ty result = simd_poly5(_a
				, 3.11578814719469302614f, -3.32419399085241980044f
				, 2.59883907202499966007f, -1.23152682416275988241f
				, 0.318212422185251071475f, -0.0344359067839062357313f
				);
#elif 0
			const Ty result = simd_poly4(_a
				, 2.8882704548164776201f, -2.52074962577807006663f
				, 1.48116647521213171641f, -0.465725644288844778798f
				, 0.0596515482674574969533f
				);
#elif 0
			const Ty result = simd_poly3(_a
				, 2.61761038894603480148f, -1.75647175389045657003f
				, 0.688243882994381274313f, -0.107254423828329604454f
				);
#else
			const Ty result = simd_poly2(_a
				, 2.28330284476918490682f, -1.04913055217340124191f
				, 0.204446009836232697516f
				);
#endif

			return result;
		}

		template<typename Ty>
		BX_SIMD_INLINE Ty simd_exppoly(Ty _a)
		{
#if 1
			const Ty result = simd_poly5(_a
				, 9.9999994e-1f, 6.9315308e-1f
				, 2.4015361e-1f, 5.5826318e-2f
				, 8.9893397e-3f, 1.8775767e-3f
				);
#elif 0
			const Ty result = simd_poly4(_a
				, 1.0000026f, 6.9300383e-1f
				, 2.4144275e-1f, 5.2011464e-2f
				, 1.3534167e-2f
				);
#elif 0
			const Ty result = simd_poly3(_a
				, 9.9992520e-1f, 6.9583356e-1f
				, 2.2606716e-1f, 7.8024521e-2f
				);
#else
			const Ty result = simd_poly2(_a
				, 1.0017247f, 6.5763628e-1f
				, 3.3718944e-1f
				);
#endif // 0

			return result;
		}
	} // namespace simd_internal

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_log2_ni(Ty _a)
	{
		const Ty expmask  = simd_isplat<Ty>(0x7f800000);
		const Ty mantmask = simd_isplat<Ty>(0x007fffff);
		const Ty one      = simd_splat<Ty>(1.0f);

		const Ty c127     = simd_isplat<Ty>(127);
		const Ty aexp     = simd_and(_a, expmask);
		const Ty aexpsr   = simd_srl(aexp, 23);
		const Ty tmp0     = simd_isub(aexpsr, c127);
		const Ty exp      = simd_itof(tmp0);

		const Ty amask    = simd_and(_a, mantmask);
		const Ty mant     = simd_or(amask, one);

		const Ty poly     = simd_logexp_detail::simd_logpoly(mant);

		const Ty mandiff  = simd_sub(mant, one);
		const Ty result   = simd_madd(poly, mandiff, exp);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_exp2_ni(Ty _a)
	{
		const Ty min      = simd_splat<Ty>( 129.0f);
		const Ty max      = simd_splat<Ty>(-126.99999f);
		const Ty tmp0     = simd_min(_a, min);
		const Ty aaaa     = simd_max(tmp0, max);

		const Ty half     = simd_splat<Ty>(0.5f);
		const Ty tmp2     = simd_sub(aaaa, half);
		const Ty ipart    = simd_ftoi(tmp2);
		const Ty iround   = simd_itof(ipart);
		const Ty fpart    = simd_sub(aaaa, iround);

		const Ty c127     = simd_isplat<Ty>(127);
		const Ty tmp5     = simd_iadd(ipart, c127);
		const Ty expipart = simd_sll(tmp5, 23);

		const Ty expfpart = simd_logexp_detail::simd_exppoly(fpart);

		const Ty result   = simd_mul(expipart, expfpart);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_pow_ni(Ty _a, Ty _b)
	{
		const Ty alog2  = simd_log2(_a);
		const Ty alog2b = simd_mul(alog2, _b);
		const Ty result = simd_exp2(alog2b);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_dot3_ni(Ty _a, Ty _b)
	{
		const Ty xyzw   = simd_mul(_a, _b);
		const Ty xxxx   = simd_swiz_xxxx(xyzw);
		const Ty yyyy   = simd_swiz_yyyy(xyzw);
		const Ty zzzz   = simd_swiz_zzzz(xyzw);
		const Ty tmp1   = simd_add(xxxx, yyyy);
		const Ty result = simd_add(zzzz, tmp1);
		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_cross3_ni(Ty _a, Ty _b)
	{
		// a.yzx * b.zxy - a.zxy * b.yzx == (a * b.yzx - a.yzx * b).yzx
#if 0
		const Ty a_yzxw = simd_swiz_yzxw(_a);
		const Ty a_zxyw = simd_swiz_zxyw(_a);
		const Ty b_zxyw = simd_swiz_zxyw(_b);
		const Ty b_yzxw = simd_swiz_yzxw(_b);
		const Ty tmp    = simd_mul(a_yzxw, b_zxyw);
		const Ty result = simd_nmsub(a_zxyw, b_yzxw, tmp);
#else
		const Ty a_yzxw = simd_swiz_yzxw(_a);
		const Ty b_yzxw = simd_swiz_yzxw(_b);
		const Ty tmp0   = simd_mul(_a, b_yzxw);
		const Ty tmp1   = simd_nmsub(a_yzxw, _b, tmp0);
		const Ty result = simd_swiz_yzxw(tmp1);
#endif

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_normalize3_ni(Ty _a)
	{
		const Ty dot3    = simd_dot3(_a, _a);
		const Ty invSqrt = simd_rsqrt(dot3);
		const Ty result  = simd_mul(_a, invSqrt);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_dot_ni(Ty _a, Ty _b)
	{
		const Ty xyzw   = simd_mul(_a, _b);
		const Ty yzwx   = simd_swiz_yzwx(xyzw);
		const Ty tmp0   = simd_add(xyzw, yzwx);
		const Ty zwxy   = simd_swiz_zwxy(tmp0);
		const Ty result = simd_add(tmp0, zwxy);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_ceil_ni(Ty _a)
	{
		const Ty tmp0   = simd_ftoi(_a);
		const Ty tmp1   = simd_itof(tmp0);
		const Ty mask   = simd_cmplt(tmp1, _a);
		const Ty one    = simd_splat<Ty>(1.0f);
		const Ty tmp2   = simd_and(one, mask);
		const Ty result = simd_add(tmp1, tmp2);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_floor_ni(Ty _a)
	{
		const Ty tmp0   = simd_ftoi(_a);
		const Ty tmp1   = simd_itof(tmp0);
		const Ty mask   = simd_cmpgt(tmp1, _a);
		const Ty one    = simd_splat<Ty>(1.0f);
		const Ty tmp2   = simd_and(one, mask);
		const Ty result = simd_sub(tmp1, tmp2);

		return result;
	}

	template<typename Ty>
	BX_SIMD_FORCE_INLINE Ty simd_round_ni(Ty _a)
	{
		const Ty tmp    = simd_ftoi(_a);
		const Ty result = simd_itof(tmp);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE bool simd_test_any_ni(Ty _a)
	{
		const Ty mask = simd_sra(_a, 31);
		const Ty zwxy = simd_swiz_zwxy(mask);
		const Ty tmp0 = simd_or(mask, zwxy);
		const Ty tmp1 = simd_swiz_yyyy(tmp0);
		const Ty tmp2 = simd_or(tmp0, tmp1);
		int res;
		simd_stx(&res, tmp2);
		return 0 != res;
	}

	template<typename Ty>
	BX_SIMD_INLINE bool simd_test_all_ni(Ty _a)
	{
		const Ty bits  = simd_sra(_a, 31);
		const Ty m1248 = simd_ild<Ty>(1, 2, 4, 8);
		const Ty mask  = simd_and(bits, m1248);
		const Ty zwxy  = simd_swiz_zwxy(mask);
		const Ty tmp0  = simd_or(mask, zwxy);
		const Ty tmp1  = simd_swiz_yyyy(tmp0);
		const Ty tmp2  = simd_or(tmp0, tmp1);
		int res;
		simd_stx(&res, tmp2);
		return 0xf == res;
	}

} // namespace bx

#endif // BX_SIMD_T_H_HEADER_GUARD
