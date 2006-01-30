/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * $Id: recordset.h 7 2002-08-25 01:10:18Z nas $ 
 * Copyright (C) 2002 Nathan Stitt  
 * See file COPYING for use and distribution permission.
 */
#ifndef _TESTS_RECORDSET_H_
#define _TESTS_RECORDSET_H_


#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>


namespace recordset_test {
	struct fixture : public CppUnit::TestFixture  {
		void select();
		void get_values();
		void length();
	};

	#if (__GNUC__)
	__attribute__ ((unused))
	#endif
	static CppUnit::Test *
	suite(){
		CppUnit::TestSuite *suite = new CppUnit::TestSuite( "recordset Tests" );
		suite->addTest( new CppUnit::TestCaller<fixture>( "select",
								  &fixture::select ) );
		suite->addTest( new CppUnit::TestCaller<fixture>( "get_values",
								  &fixture::get_values ) );
		suite->addTest( new CppUnit::TestCaller<fixture>( "length",
								  &fixture::length ) );

		return suite;
	}
}

#endif // _TESTS_RECORDSET_H_

