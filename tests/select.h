/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * $Id: select.h 36 2003-03-11 21:36:59Z nas $ 
 * Copyright (C) 2002 Nathan Stitt  
 * See file COPYING for use and distribution permission.
 */ 
#ifndef _TESTS_SELECT_H_
#define _TESTS_SELECT_H_

#include "tmplsql/tmplsql.h"

#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>


namespace select_test {

	struct fixture : public CppUnit::TestFixture  {
		void init();
		void straight_select();
		void limited_select();
		void field_filter();
		void query_filter();
		void join();
		void reopen();
		void update();
		void boom();
	};

	#if (__GNUC__)
	__attribute__ ((unused))
	#endif
	static CppUnit::Test *suite(){
		CppUnit::TestSuite *suite = new CppUnit::TestSuite( "SELECT Tests" );
		
		suite->addTest( new CppUnit::TestCaller<fixture>( "straight_select",
								  &fixture::straight_select ) );
 		suite->addTest( new CppUnit::TestCaller<fixture>( "limited_select",
 								  &fixture::limited_select ) );
 		suite->addTest( new CppUnit::TestCaller<fixture>( "field_filter",
 								  &fixture::field_filter ) );
		suite->addTest( new CppUnit::TestCaller<fixture>( "query_filter",
								  &fixture::query_filter ) );
 		suite->addTest( new CppUnit::TestCaller<fixture>( "join",
 								  &fixture::join ) );
 		suite->addTest( new CppUnit::TestCaller<fixture>( "reopen",
 								  &fixture::reopen ) );
 		suite->addTest( new CppUnit::TestCaller<fixture>( "update",
 								  &fixture::update ) );
		return suite;
	}

}
#endif // _TESTS_SELECT_H_

