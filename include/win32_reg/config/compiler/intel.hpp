/*=============================================================================
  Copyright (C) 2015-2016 DxLibEx project
  https://github.com/Nagarei/DxLibEx/

  Distributed under the Boost Software License, Version 1.0.
  (See http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#ifndef WIN32_REG_CONFIG_COMPILER_INTEL_HPP_
#define WIN32_REG_CONFIG_COMPILER_INTEL_HPP_

#include "common_edg.hpp"

#if defined(__INTEL_COMPILER)
#	define WIN32_REG_INTEL_CXX_VERSION __INTEL_COMPILER
#elif defined(__ICL)
#	define WIN32_REG_INTEL_CXX_VERSION __ICL
#elif defined(__ICC)
#	define WIN32_REG_INTEL_CXX_VERSION __ICC
#elif defined(__ECC)
#	define WIN32_REG_INTEL_CXX_VERSION __ECC
#endif

#if (!(defined(_WIN32) || defined(_WIN64)) && defined(__STDC_HOSTED__) && (__STDC_HOSTED__ && (WIN32_REG_INTEL_CXX_VERSION <= 1200))) || defined(__GXX_EXPERIMENTAL_CPP0X__) || defined(__GXX_EXPERIMENTAL_CXX0X__)
#	define WIN32_REG_INTEL_STDCXX11
#endif
#if defined(_MSC_VER) && (_MSC_VER >= 1600)
#	define WIN32_REG_INTEL_STDCXX11
#endif

#if defined(SPROUT_INTEL_STDCXX11) && (SPROUT_INTEL_CXX_VERSION >= 1200)
#	undef WIN32_REG_NO_CXX11_DELETED_FUNCTIONS
#	undef WIN32_REG_NO_CXX11_DEFAULTED_FUNCTIONS
#endif

#if defined(SPROUT_INTEL_STDCXX11) && (SPROUT_INTEL_CXX_VERSION > 1200)
#	undef WIN32_REG_NO_CXX11_TEMPLATE_ALIASES
#endif

#if defined(SPROUT_INTEL_STDCXX11) && (SPROUT_INTEL_CXX_VERSION >= 1400) && !defined(_MSC_VER)
#	undef WIN32_REG_NO_CXX11_UNICODE_LITERALS
#	undef WIN32_REG_NO_CXX11_CONSTEXPR
#endif

#if defined(_MSC_VER) && (_MSC_VER <= 1700)
#	define WIN32_REG_NO_CXX11_DELETED_FUNCTIONS
#	define WIN32_REG_NO_CXX11_DEFAULTED_FUNCTIONS
#	define WIN32_REG_NO_CXX11_TEMPLATE_ALIASES
#endif

#endif	// #ifndef WIN32_REG_CONFIG_COMPILER_INTEL_HPP_
