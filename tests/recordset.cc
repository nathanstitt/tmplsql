/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * $Id: recordset.cc 23 2003-01-31 05:21:47Z nas $ 
 * Copyright (C) 2002 Nathan Stitt  
 * See file COPYING for use and distribution permission.
 */

#include "tests/recordset.h"
#include "tmplsql/recordset.h"
#include "tmplsql/rdms.h"
#include <iostream>
using namespace recordset_test;


void
fixture::select() {
	tmplsql::handle sql;

	CPPUNIT_ASSERT( sql.valid() );

	*sql << "create table tmplsql_tester (field1 text, field2 int, field3 numeric )";
	sql->exec();

	*sql << "delete from tmplsql_tester";
	CPPUNIT_ASSERT( sql->exec() );

	*sql << "insert into tmplsql_tester (field1, field2,field3) values ('test1',1,2.0234)";
	CPPUNIT_ASSERT( sql->exec() );

	*sql << "select field1,field2,field3 from tmplsql_tester limit 1";
	tmplsql::recordset<std::string,int,double> rs(sql);

	tmplsql::recordset<std::string,int,double>::iterator it = rs.begin();
	CPPUNIT_ASSERT( it != rs.end() );
	it++;
 	CPPUNIT_ASSERT( it == rs.end() );
}


void
fixture::get_values() {
	tmplsql::handle sql;
	*sql << "select field1,field2,field3 from tmplsql_tester limit 1";
 	tmplsql::recordset< std::string,int,double> rs(sql);
	
	tmplsql::recordset< std::string,int,double>::iterator it = rs.begin();

	CPPUNIT_ASSERT( it.get<0>() == "test1" );
 	CPPUNIT_ASSERT( it.get<1>() == 1 );
 	CPPUNIT_ASSERT( it.get<2>() == 2.0234 );
 	it++;
 	CPPUNIT_ASSERT( it == rs.end() );

	*sql << "drop table tmplsql_tester";
	sql->exec();
}

void
fixture::length(){
	int length = tmplsql::recordset<std::string,int,double>::length;
	CPPUNIT_ASSERT( 3 == length );
}


