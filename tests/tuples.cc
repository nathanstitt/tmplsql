/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * $Id: tuples.cc 78 2003-09-15 20:49:23Z nas $ 
 * Copyright (C) 2002 Nathan Stitt  
 * See file COPYING for use and distribution permission.
 */

#include "tests/tuples.h"
//#include "tmplsql/tuples.h"

using namespace tuples_test;



struct test1 {
	int test() { return 1; }
	int flag;
};

struct test2{
	int test() { return 2; }
	int flag;
};

void
fixture::get() {

// 	tmplsql::tuples::tuple<test1,test2> t;

// 	{
// 		test1 &t1 = tmplsql::tuples::get<0>(t);
// 		t1.flag = 1;
// 		test2 &t2 = tmplsql::tuples::get<1>(t);
// 		t2.flag = 2;		
// 		CPPUNIT_ASSERT( t1.test() == 1 );
// 		CPPUNIT_ASSERT( t2.test() == 2 );
// 	}

// 	test1 &t1 = tmplsql::tuples::get<0>(t);
// 	CPPUNIT_ASSERT( t1.flag == 1 );

// 	test2 &t2 = tmplsql::tuples::get<1>(t);
// 	CPPUNIT_ASSERT( t2.flag == 2 );

}

void
fixture::length(){
// 	int length = tmplsql::tuples::length< tmplsql::tuples::tuple<test1,test2> >::value;
// 	CPPUNIT_ASSERT( length  == 2 );
}
