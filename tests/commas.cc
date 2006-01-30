/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * $Id: commas.cc 2 2002-08-24 19:02:47Z nas $ 
 * Copyright (C) 2002 Nathan Stitt  
 * See file COPYING for use and distribution permission.
 */

#include "tests/commas.h"
#include "tmplsql/commas.h"

using namespace commas_test;


void
fixture::get() {
	tmplsql::commas comma;
	CPPUNIT_ASSERT( comma.get() == ' ' );
	CPPUNIT_ASSERT( comma.get() == ',' );
}

void
fixture::reset() {
	tmplsql::commas comma;
	CPPUNIT_ASSERT( comma.get() == ' ' );
	CPPUNIT_ASSERT( comma.get() == ',' );
	comma.reset();
	CPPUNIT_ASSERT( comma.get() == ' ' );
}



void
fixture::times_called() {
	tmplsql::commas comma;
	CPPUNIT_ASSERT( comma.times_called() == 0 );
	CPPUNIT_ASSERT( comma.get() == ' ' );
	CPPUNIT_ASSERT( comma.times_called() == 1 );
	CPPUNIT_ASSERT( comma.get() == ',' );
	CPPUNIT_ASSERT( comma.times_called() == 2 );
	comma.reset();
	CPPUNIT_ASSERT( comma.times_called() == 0 );
	CPPUNIT_ASSERT( comma.get() == ' ' );
	CPPUNIT_ASSERT( comma.times_called() == 1 );
}

