/*=============================================================================
  Copyright (C) 2015-2016 DxLibEx project
  https://github.com/Nagarei/DxLibEx/

  Distributed under the Boost Software License, Version 1.0.
  (See http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#ifndef WIN32_REG_INC_CONFIG_COMPILER_CLANG_HPP_
#define WIN32_REG_INC_CONFIG_COMPILER_CLANG_HPP_

/*
C++11
*/
#if !__has_feature(cxx_constexpr)
#	define WIN32_REG_NO_CXX11_CONSTEXPR
#endif

#if !__has_feature(cxx_noexcept)
#	define WIN32_REG_NO_CXX11_NOEXCEPT
#	define WIN32_REG_NO_CXX11_NOEXCEPT_EXPRESSION
#endif

#if !__has_feature(cxx_unicode_literals)
#	define WIN32_REG_NO_CXX11_UNICODE_LITERALS
#endif

#if __clang_major__ < 3 || (__clang_major__ == 3 && __clang_minor__ < 3)
#	define WIN32_REG_NO_CXX11_ATTRIBUTES
#endif

#if __clang_major__ < 3 || (__clang_major__ == 3 && __clang_minor__ < 1)
#	define WIN32_REG_NO_CXX11_USER_DEFINED_LITERALS
#endif

#if __clang_major__ < 2 || (__clang_major__ == 2 && __clang_minor__ < 9)
#	define WIN32_REG_NO_CXX11_REF_QUALIFIERS
#endif

/*
C++14
*/
#if !__has_feature(cxx_relaxed_constexpr)
#	define WIN32_REG_NO_CXX14_CONSTEXPR
#endif

#if !__has_feature(cxx_variable_templates)
#	define WIN32_REG_NO_CXX14_VARIABLE_TEMPLATES
#endif

#if __clang_major__ < 3 || (__clang_major__ == 3 && __clang_minor__ < 4)
#	define WIN32_REG_NO_CXX14_ATTRIBUTE_DEPRECATED
#	define WIN32_REG_NO_CXX14_UDLS_FOR_STRING_AND_CHRONO
#endif

/*
Proprietary extension
*/
#if (__clang_major__ > 3 || (__clang_major__ == 3 && __clang_minor__ >= 0))
#	define WIN32_REG_HAS_CONSTEXPR_COPYSIGN_FUNCTION
#endif

#endif	// #ifndef WIN32_REG_INC_CONFIG_COMPILER_CLANG_HPP_
