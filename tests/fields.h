/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * $Id: fields.h 36 2003-03-11 21:36:59Z nas $ 
 * Copyright (C) 2002 Nathan Stitt  
 * See file COPYING for use and distribution permission.
 */ 
#ifndef _TESTS_FIELDS_H_
#define _TESTS_FIELDS_H_

#include "tmplsql/fields.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>


namespace fields_test {

	struct fixture : public CppUnit::TestFixture  {
		void name();
		void table();
		void primary_key();
		void assign_comp();
		void modified();
	};

	#if (__GNUC__)
	__attribute__ ((unused))
	#endif
	static CppUnit::Test *suite(){
		CppUnit::TestSuite *suite = new CppUnit::TestSuite( "FIELDS Tests" );

		suite->addTest( new CppUnit::TestCaller<fixture>( "name()",
								  &fixture::name ) );
 		suite->addTest( new CppUnit::TestCaller<fixture>( "table()",
 								  &fixture::table ) );
  		suite->addTest( new CppUnit::TestCaller<fixture>( "primay_key()",
  								  &fixture::primary_key ) );
 		suite->addTest( new CppUnit::TestCaller<fixture>( "modified()",
 								  &fixture::modified ) );
 		suite->addTest( new CppUnit::TestCaller<fixture>( "assign_comp()",
 								  &fixture::assign_comp ) );
	return suite;
	}

}
#endif // _TESTS_FIELDS_H_

