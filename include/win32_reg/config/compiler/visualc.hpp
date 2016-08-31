/*=============================================================================
  Copyright (C) 2015-2016 DxLibEx project
  https://github.com/Nagarei/DxLibEx/

  Distributed under the Boost Software License, Version 1.0.
  (See http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#ifndef WIN32_REG_INC_CONFIG_COMPILER_VISUALC_HPP_
#define WIN32_REG_INC_CONFIG_COMPILER_VISUALC_HPP_

/*
C++11
*/
#if _MSC_FULL_VER < 190023506//Visual Studio 2015 Update 1
//Visual Studio 2013 November CTP and Visual Stduio 2015 has too many bugs for constexpr. We will never support these.
#	define WIN32_REG_NO_CXX11_CONSTEXPR
#endif

#if _MSC_FULL_VER < 190023026//Visual Studio 2015
#	define WIN32_REG_NO_CXX11_NOEXCEPT_EXPRESSION
#	define WIN32_REG_NO_CXX11_USER_DEFINED_LITERALS
#	define WIN32_REG_NO_CXX11_UNICODE_LITERALS
#	define WIN32_REG_NO_CXX11_ATTRIBUTES
#endif
#if (_MSC_VER < 1900) && (_MSC_FULL_VER != 180021114)//Visual Studio 2013 Nobemver CTP or Visual Studio2015 or later
#	define WIN32_REG_NO_CXX11_NOEXCEPT
#	define WIN32_REG_NO_CXX11_REF_QUALIFIERS
#endif

/*
C++14
*/
#if !defined(__cpp_constexpr) || (__cpp_constexpr < 201304)
#  define WIN32_REG_NO_CXX14_CONSTEXPR
#endif

#if _MSC_FULL_VER < 190023824//Visual Studio 2015 Update2 RC
#  define WIN32_REG_NO_CXX14_VARIABLE_TEMPLATES
#endif

#if _MSC_FULL_VER < 190023026//Visual Studio 2015
#	define WIN32_REG_NO_CXX14_ATTRIBUTE_DEPRECATED
#	define WIN32_REG_NO_CXX14_UDLS_FOR_STRING_AND_CHRONO
#endif

#endif	// #ifndef WIN32_REG_INC_CONFIG_COMPILER_VISUALC_HPP_
