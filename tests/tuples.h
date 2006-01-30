/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * $Id: tuples.h 6 2002-08-25 01:06:41Z nas $ 
 * Copyright (C) 2002 Nathan Stitt  
 * See file COPYING for use and distribution permission.
 */
#ifndef _TESTS_TUPLES_H_
#define _TESTS_TUPLES_H_


#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>


namespace tuples_test {
	struct fixture : public CppUnit::TestFixture  {
		void get();
		void length();
	};
	#if (__GNUC__)
	__attribute__ ((unused))
	#endif
	static CppUnit::Test *suite(){
		CppUnit::TestSuite *suite = new CppUnit::TestSuite( "tuples Tests" );

		suite->addTest( new CppUnit::TestCaller<fixture>( "get",
								  &fixture::get ) );


		suite->addTest( new CppUnit::TestCaller<fixture>( "length",
								  &fixture::length ) );

		return suite;
	}
}

#endif // _TESTS_TUPLES_H_

