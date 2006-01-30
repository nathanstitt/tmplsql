/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * $Id: select.cc 38 2003-03-14 02:16:49Z nas $ 
 * Copyright (C) 2002 Nathan Stitt  
 * See file COPYING for use and distribution permission.
 */



#include "select.h"
#include <iostream>

#include <stdlib.h>
#include <time.h>


#include "tmplsql/fields.h"

using namespace select_test;

struct field1 : public tmplsql::updateable_field<std::string> {
	field1() : tmplsql::updateable_field<std::string>("field1","tmplsql_tester") { }
};

struct field2 : public tmplsql::field<int> {
	typedef tmplsql::fields::primary field_type;
	field2() : tmplsql::field<int>("field2","tmplsql_tester" ) {  }
};

struct field3 : public tmplsql::field<double> {
	field3() : tmplsql::field<double>("field3","tmplsql_tester") { }
};

struct j_id : public tmplsql::field<int> {
	j_id() : tmplsql::field<int>("id","tmplsql_tester1" ) { }
};

struct j_field1 : public tmplsql::field<std::string> {
	j_field1() : tmplsql::field<std::string>("field1","tmplsql_tester1") { }
};

typedef tmplsql::query<field1,field2,field3> myquery;
typedef tmplsql::query<j_id> subquery;
typedef tmplsql::query<field1,field2,field3,j_id,j_field1> myjoin;
void
fixture::init(){
	tmplsql::handle sql;
	sql.valid();
	*sql << "create table tmplsql_tester (field1 text, field2 int, field3 numeric )";
	sql->exec();
	*sql << "create table tmplsql_tester1 (id int,field1 text )";
	sql->exec();
	*sql << "delete from tmplsql_tester";
	sql->exec();
	*sql << "insert into tmplsql_tester (field1, field2,field3) values ('test1',1,2.0234)";
	sql->exec();
	*sql << "insert into tmplsql_tester1 (id,field1) values (1,'join1')";
	sql->exec();
	*sql << "insert into tmplsql_tester (field1, field2,field3) values ('test2',2,332.54)";
	sql->exec();
	*sql << "insert into tmplsql_tester1 (id,field1) values (2,'join2')";
	sql->exec();
}

void
fixture::boom(){
	tmplsql::handle sql;
	*sql << "drop table tmplsql_tester";
	sql->exec();
	*sql << "drop table tmplsql_tester1";
	sql->exec();
}

void
fixture::straight_select(){
	this->init();
	myquery q;
 	myquery::iterator it = q.begin();
 	CPPUNIT_ASSERT( q.end()  != it );

 	CPPUNIT_ASSERT( it.get<0>() == "test1" );
	CPPUNIT_ASSERT( it.get<1>() == 1 );
	CPPUNIT_ASSERT( it.get<2>() == 2.0234 );
	++it;
	field1 f1 = it.get<0>();
	field2 f2 = it.get<1>();
	CPPUNIT_ASSERT( q.end()  != it );
	CPPUNIT_ASSERT( it.get<0>() == "test2" );
	CPPUNIT_ASSERT( it.get<1>() == 2 );
	CPPUNIT_ASSERT( it.get<2>() == 332.54 );
	++it;
	CPPUNIT_ASSERT( q.end()  == it );
	this->boom();
}

void
fixture::limited_select(){

	this->init();
	myquery q;
	q.set_limit( 1 );
	myquery::iterator it = q.begin();
	CPPUNIT_ASSERT( q.end()  != it );
	++it;
	CPPUNIT_ASSERT( q.end()  == it );
	this->boom();
}

void
fixture::field_filter(){
	this->init();
	myquery q;
	field2 f;
	f.initialize( 2 );
	q.set_filter( f );
	myquery::iterator it = q.begin();
	CPPUNIT_ASSERT( q.end()  != it );
	CPPUNIT_ASSERT( it.get<0>() == "test2" );
	CPPUNIT_ASSERT( it.get<1>() == 2 );
	CPPUNIT_ASSERT(  it.get<2>() == 332.54 );
	++it;
	CPPUNIT_ASSERT( q.end()  == it );
	this->boom();
}

void
fixture::query_filter(){
	this->init();
	myquery q;
	subquery sq;
	field2 f2;
	j_field1 sqf;
	sqf.initialize( "join2" );

	sq.set_filter( sqf );

	q.set_filter(f2,sq);

	myquery::iterator it = q.begin();
	CPPUNIT_ASSERT( q.end()  != it );

	CPPUNIT_ASSERT( it.get<0>() == "test2" );
	CPPUNIT_ASSERT( it.get<1>() == 2 );
	CPPUNIT_ASSERT( it.get<2>() == 332.54 );
 	++it;
 	CPPUNIT_ASSERT( q.end()  == it );

	this->boom();
}

void
fixture::join(){

	this->init();
	myjoin j;
	j.set_inner_join<1,3>();

	myjoin::iterator it = j.begin();
	CPPUNIT_ASSERT( j.end()  != it );
 
	CPPUNIT_ASSERT( it.get<0>() == "test1" );
	CPPUNIT_ASSERT( it.get<1>() == 1 );
	CPPUNIT_ASSERT( it.get<2>() == 2.0234 );
 	CPPUNIT_ASSERT( it.get<3>() == 1);
 	CPPUNIT_ASSERT( it.get<4>() == "join1" );

	++it;
	CPPUNIT_ASSERT( j.end()  != it );

	CPPUNIT_ASSERT( it.get<0>() == "test2" );
	CPPUNIT_ASSERT( it.get<1>() == 2 );
	CPPUNIT_ASSERT( it.get<2>() == 332.54 );
 	CPPUNIT_ASSERT( it.get<3>() == 2);
 	CPPUNIT_ASSERT( it.get<4>() == "join2" );
	++it;
	CPPUNIT_ASSERT( j.end()  == it );


	this->boom();
}

void
fixture::reopen(){
	this->init();
	myjoin j;
	j.set_inner_join<1,3>();
	myjoin::iterator it = j.begin();
	CPPUNIT_ASSERT( j.end()  != it );
 
	CPPUNIT_ASSERT( it.get<0>() == "test1" );
	CPPUNIT_ASSERT( it.get<1>() == 1 );
	CPPUNIT_ASSERT( it.get<2>() == 2.0234 );
 	CPPUNIT_ASSERT( it.get<3>() == 1);
 	CPPUNIT_ASSERT( it.get<4>() == "join1" );

	++it;
	CPPUNIT_ASSERT( j.end()  != it );

	CPPUNIT_ASSERT( it.get<0>() == "test2" );
	CPPUNIT_ASSERT( it.get<1>() == 2 );
	CPPUNIT_ASSERT( it.get<2>() == 332.54 );
 	CPPUNIT_ASSERT( it.get<3>() == 2);
 	CPPUNIT_ASSERT( it.get<4>() == "join2" );
	++it;


	j.set_limit( 1 );
	it = j.begin();
	CPPUNIT_ASSERT( j.end()  != it );
	++it;
	CPPUNIT_ASSERT( j.end()  == it );


	field2 f;
	f.initialize( 2 );
	j.set_filter( f );
	it = j.begin();
 	CPPUNIT_ASSERT( it.get<3>() == 2);

	subquery sq;
	field2 f2;
	j_field1 sqf;
	sqf.initialize( "join2" );

	sq.set_filter( sqf );

	j.set_filter(f2,sq);

	it = j.begin();
 	CPPUNIT_ASSERT( j.end()  != it );

	CPPUNIT_ASSERT( it.get<0>() == "test2" );
	CPPUNIT_ASSERT( it.get<1>() == 2 );
	CPPUNIT_ASSERT( it.get<2>() == 332.54 );
 	CPPUNIT_ASSERT( it.get<3>() == 2);
 	CPPUNIT_ASSERT( it.get<4>() == "join2" );

 	++it;
 	CPPUNIT_ASSERT( j.end()  == it );

	this->boom();
};

void
fixture::update(){
	


	this->init();
	int id;
	{
		myquery q;
		myquery::iterator it = q.begin();
		CPPUNIT_ASSERT( q.end()  != it );
		CPPUNIT_ASSERT( it.get<0>() == "test1" );
		id = it.get<1>().get();

		it.get<0>().set( "be afraid.. be very afraid" );
		CPPUNIT_ASSERT( it.get<0>() == "be afraid.. be very afraid" );
	}



 	{
 		myquery q;
		field2 f;
		f.initialize( id );
		q.set_filter(f);

 		myquery::iterator it = q.begin();
		
 		CPPUNIT_ASSERT( q.end()  != it );
 		CPPUNIT_ASSERT( it.get<0>() == "be afraid.. be very afraid" );
 		it.get<0>().set( "test1" );
 		CPPUNIT_ASSERT( it.get<0>() == "test1" );
 	}


	this->boom();

}
