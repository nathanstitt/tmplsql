/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * $Id: rdms.h 13 2002-09-09 22:29:58Z nas $ 
 * Copyright (C) 2002 Nathan Stitt  
 * See file COPYING for use and distribution permission.
 */

#ifndef _TESTS_RDMS_H_
#define _TESTS_RDMS_H_

#include "tmplsql/rdms.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>


namespace rdms_test {

	struct fixture : public CppUnit::TestFixture  {
		tmplsql::handle get_handle();
		void test_connection();
		void handle_release();
		void stream();
		void errors();
		void exec();
		void insert();
		void single_value();
		void streams();
		void transactions();
		void quote();
		void epoch_date();
		void result_sets();
	};

	#if (__GNUC__)
	__attribute__ ((unused))
	#endif
	static CppUnit::Test *suite(){
		CppUnit::TestSuite *suite = new CppUnit::TestSuite( "RDMS Tests" );
		
		suite->addTest( new CppUnit::TestCaller<fixture>( "connection",
							      &fixture::test_connection ) );
		suite->addTest( new CppUnit::TestCaller<fixture>( "handle_release", 
							      &fixture::handle_release ) );
		suite->addTest( new CppUnit::TestCaller<fixture>( "stream", 
							      &fixture::stream ) );
		suite->addTest( new CppUnit::TestCaller<fixture>( "errors", 
							      &fixture::errors ) );
		suite->addTest( new CppUnit::TestCaller<fixture>( "exec", 
							      &fixture::exec ) );
		suite->addTest( new CppUnit::TestCaller<fixture>( "insert", 
							      &fixture::insert ) );
		suite->addTest( new CppUnit::TestCaller<fixture>( "single_value", 
							      &fixture::single_value ) );
		suite->addTest( new CppUnit::TestCaller<fixture>( "streams",
							      &fixture::streams ) );
		suite->addTest( new CppUnit::TestCaller<fixture>( "transactions", 
							      &fixture::transactions ) );
		suite->addTest( new CppUnit::TestCaller<fixture>( "quote", 
							      &fixture::quote ) );
		suite->addTest( new CppUnit::TestCaller<fixture>( "epoch_date", 
							      &fixture::epoch_date ) );
		suite->addTest( new CppUnit::TestCaller<fixture>( "result_sets", 
							      &fixture::result_sets ) );

		return suite;
	}

}
#endif // _TESTS_RDMS_H_


