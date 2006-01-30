/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * $Id: commas.h 5 2002-08-24 19:22:45Z nas $ 
 * Copyright (C) 2002 Nathan Stitt  
 * See file COPYING for use and distribution permission.
 */
#ifndef _TESTS_COMMAS_H_
#define _TESTS_COMMAS_H_


#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>


namespace commas_test {
	struct fixture : public CppUnit::TestFixture  {
		void get();
		void reset();
		void times_called();
	};
	#if (__GNUC__)
	__attribute__ ((unused))
	#endif
	static CppUnit::Test *
	suite(){
		CppUnit::TestSuite *suite = new CppUnit::TestSuite( "commas Tests" );

		suite->addTest( new CppUnit::TestCaller<fixture>( "get",
								  &fixture::get ) );
		suite->addTest( new CppUnit::TestCaller<fixture>( "reset",
								  &fixture::reset ) );
		suite->addTest( new CppUnit::TestCaller<fixture>( "times_called",
								  &fixture::times_called ) );
		return suite;
	}
}

#endif // _TESTS_COMMAS_H_

