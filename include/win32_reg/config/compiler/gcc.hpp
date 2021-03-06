/*=============================================================================
  Copyright (C) 2015-2016 DxLibEx project
  https://github.com/Nagarei/DxLibEx/

  Distributed under the Boost Software License, Version 1.0.
  (See http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#ifndef WIN32_REG_INC_CONFIG_COMPILER_GCC_HPP_
#define WIN32_REG_INC_CONFIG_COMPILER_GCC_HPP_

/*
C++11
*/
#if (__GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 8) || (__GNUC__ == 4 && __GNUC_MINOR__ == 8 && __GNUC_PATCHLEVEL__ < 1 )|| !defined(__GXX_EXPERIMENTAL_CXX0X__))
#	define WIN32_REG_NO_CXX11_REF_QUALIFIERS
#endif

#if (__GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 8) || !defined(__GXX_EXPERIMENTAL_CXX0X__))
#	define WIN32_REG_NO_CXX11_ATTRIBUTES
#endif

#if (__GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 7) || !defined(__GXX_EXPERIMENTAL_CXX0X__))
#	define WIN32_REG_NO_CXX11_USER_DEFINED_LITERALS
#endif

#if (__GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 6) || !defined(__GXX_EXPERIMENTAL_CXX0X__))
#	define WIN32_REG_NO_CXX11_CONSTEXPR
#	define WIN32_REG_NO_CXX11_NOEXCEPT
#	define WIN32_REG_NO_CXX11_NOEXCEPT_EXPRESSION
#endif

#if (__GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 5) || !defined(__GXX_EXPERIMENTAL_CXX0X__))
#	define WIN32_REG_NO_CXX11_UNICODE_LITERALS
#endif

/*
C++14
*/
#if !defined(__cpp_constexpr) || (__cpp_constexpr < 201304)
#  define WIN32_REG_NO_CXX14_CONSTEXPR
#endif

#if !defined(__cpp_variable_templates) || (__cpp_variable_templates < 201304)
#	define WIN32_REG_NO_CXX14_VARIABLE_TEMPLATES
#endif

#if (__GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 9) || !defined(__GXX_EXPERIMENTAL_CXX0X__))
#	define WIN32_REG_NO_CXX14_ATTRIBUTE_DEPRECATED
#endif

#if __GNUC__ < 5
#	define WIN32_REG_NO_CXX14_UDLS_FOR_STRING_AND_CHRONO
#endif

/*
Proprietary extension
*/
#if ((__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)) && defined(__GXX_EXPERIMENTAL_CXX0X__))
#	define WIN32_REG_HAS_CONSTEXPR_CMATH_FUNCTION
#	define WIN32_REG_HAS_CONSTEXPR_COPYSIGN_FUNCTION
#	define WIN32_REG_HAS_CONSTEXPR_BIT_OPERATION
#endif

#endif	// #ifndef WIN32_REG_INC_CONFIG_COMPILER_GCC_HPP_
