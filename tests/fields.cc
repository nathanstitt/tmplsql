/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * $Id: fields.cc 36 2003-03-11 21:36:59Z nas $ 
 * Copyright (C) 2002 Nathan Stitt  
 * See file COPYING for use and distribution permission.
 */



#include "fields.h"
#include <iostream>

#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "tmplsql/fields.h"

using namespace fields_test;

struct field : public tmplsql::field<std::string> {
	field() : tmplsql::field<std::string>("field","table" ) { }
};


struct u_field : public tmplsql::updateable_field<std::string> {
	u_field() : tmplsql::updateable_field<std::string>("field","table" ) { }
	std::string operator =( const std::string& value ) {
		this->set( value );
		return  value;
	}
};

struct field_p : public tmplsql::field<std::string> {
	field_p() : tmplsql::field<std::string>("field","table" ) { }

};

void
fixture::name(){
	field f;
	CPPUNIT_ASSERT( ! strcmp( f.name(),"field" ) );
}

void
fixture::table(){
	field f;
	CPPUNIT_ASSERT( ! strcmp( f.table(),"table" ) );
}

void
fixture::primary_key(){
	
	tmplsql::fields::non_primary np = field::field_type();
}

void
fixture::modified() {
	u_field f;
	CPPUNIT_ASSERT( ! f.is_modified() );
	f.initialize( "123" );
	CPPUNIT_ASSERT( ! f.is_modified() );

	// these will fail as there is no recordsaver
	// to save the value
	CPPUNIT_ASSERT( ! f.set( "12345" ) );
	CPPUNIT_ASSERT( ! f.is_modified() );
	CPPUNIT_ASSERT( ! f.sync() );

}

void
fixture::assign_comp() {
	tmplsql::updateable_field<int> f("field","table");
	f.initialize( 123 );
	CPPUNIT_ASSERT( f == 123 );
	CPPUNIT_ASSERT( f != 1 );
	f.initialize( 345 );
	CPPUNIT_ASSERT( f == 345 );
	CPPUNIT_ASSERT( f != 1 );

}

