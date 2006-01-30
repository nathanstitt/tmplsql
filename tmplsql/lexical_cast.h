/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * $Id: lexical_cast.h 37 2003-03-12 22:12:43Z nas $ 
 * Copyright (C) 2002 Nathan Stitt  
 * See file COPYING for use and distribution permission.
 */

#ifndef _TMPLSQL_LEXICAL_CAST_H_
#define _TMPLSQL_LEXICAL_CAST_H_


#include <sstream>
#include <typeinfo>
#include <stdlib.h>
#include <string.h>
#include <iostream>

namespace tmplsql {

	//! converts data type Source to type Target
	/*! the lexical cast functions attempt to convert data from the Source type to the Target type using std::stringstream */
	template<typename Target, typename Source>
	inline Target lexical_cast(Source arg)   {
		std::stringstream interpreter;
		Target result;
		if( ! ( interpreter << arg) || !(interpreter >> result) || ! ( interpreter >> std::ws ).eof() ) {
			return 0;
		}
		return result;
	}

	//! specialization for const char*
	template <>
	inline const char* lexical_cast<const char*,const char* >(const char* arg ) {
		return arg;
	}
	//! specialization for std::string
	template <>
	inline std::string lexical_cast<std::string,const char* >(const char* arg ) {
		if ( arg ){
			return arg;
		} else {
			return std::string();
		}
	}
	//! specialization for const char* -> bool conversion. converts T, Y, and 1 to true
	template <>
	inline bool lexical_cast<bool,const char* >(const char* arg ) {
		bool ret_val = false;
		if ( arg && strlen(arg) ){
			if ( ( toupper(*arg) == 'T' ) || ( toupper(*arg) == 'Y' ) ||  ( *arg == '1' ) ) {
				ret_val = true;
			}
		}
		return ret_val;
	}

}

#endif // _TMPLSQL_LEXICAL_CAST_H_
