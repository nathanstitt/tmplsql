/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * $Id: rdms.cc 1178 2005-06-29 14:24:27Z  $ 
 * Copyright (C) 2002 Nathan Stitt  
 * See file COPYING for use and distribution permission.
 */


#include "rdms.h"
#include <iostream>
#include "tmplsql/quote.h"
#include <stdlib.h>
#include <time.h>

using namespace rdms_test;

tmplsql::handle
fixture::get_handle(){
	tmplsql::handle sql;
	if ( ! sql.valid() ) {
		CPPUNIT_FAIL  ( "Sorry But I'm unable to connect to rdms, therefore there's really no point in continuing.  Good-Bye." );
	}
	return sql;
}


void
fixture::test_connection(){
	std::cerr << __PRETTY_FUNCTION__ << std::endl;
	tmplsql::handle sql = this->get_handle();
	CPPUNIT_ASSERT( sql->ping() );
}


void
fixture::handle_release(){
	std::cerr << __PRETTY_FUNCTION__ << std::endl;
	tmplsql::handle sql = this->get_handle();
	sql.release();
	CPPUNIT_ASSERT( ! sql.valid() );
}

void
fixture::stream(){
	tmplsql::handle sql = this->get_handle();
	CPPUNIT_ASSERT( sql.valid() );
	*sql << "resasfdafd sdfvas fdas";
	CPPUNIT_ASSERT( sql->current_statement() == "resasfdafd sdfvas fdas" );
	sql->abandon_statement();
	*sql << "testing one two three";
	CPPUNIT_ASSERT( sql->current_statement() == "testing one two three" );
	sql->abandon_statement();
	CPPUNIT_ASSERT( sql->current_statement().empty() );
}

void
fixture::errors(){
	tmplsql::handle sql = this->get_handle();
	*sql << "sdfsd fasdf adsfsda ff";
	CPPUNIT_ASSERT( ! sql->exec() );
	CPPUNIT_ASSERT( ! sql->error_msg().empty() );
	// now that we've retrived our error message, it should be empty on next call
	CPPUNIT_ASSERT( sql->error_msg().empty() );
}



void
fixture::exec(){
	tmplsql::handle sql = this->get_handle();
	// gibberish - should cause statement to fail
	*sql << "sdfsd fasdf adsfsda ff";
	CPPUNIT_ASSERT(! sql->exec() );

	// these may very well fail if it's the first time we've run the tests, so no assert for it
 	*sql << "drop table tmplsql_tester";
 	sql->exec();
 	*sql << "drop sequence tmplsql_seq";
 	sql->exec();

	*sql << "create sequence tmplsql_seq";
	CPPUNIT_ASSERT( sql->exec() );

	*sql << "create table tmplsql_tester( id integer default nextval('tmplsql_seq') primary key, "
	     << "test1 text, test2 timestamp, test3 int)";
	CPPUNIT_ASSERT( sql->exec() );

}

void
fixture::insert() {
	tmplsql::handle sql = this->get_handle();
	*sql << "drop table tmplsqla_tester";
		sql->exec() ;
	*sql << "create table tmplsqla_tester( id serial primary key, "
	     << "test1 text)";
	 sql->exec() ;

	*sql << "insert into tmplsqla_tester (test1) values ('foo')";

	CPPUNIT_ASSERT( sql->insert("tmplsqla_tester_id_seq").size()  );
	*sql << "drop table tmplsqla_tester";
	CPPUNIT_ASSERT( sql->exec() );	
}


void
fixture::single_value(){
	tmplsql::handle sql = this->get_handle();
	*sql << "insert into tmplsql_tester (test1, test2, test3) values ('foo',now()," << 1 << ")";
	CPPUNIT_ASSERT( sql->exec() );


	*sql << "select test1 from tmplsql_tester where test3 = " << 1;
	CPPUNIT_ASSERT(  sql->single_value() == "foo" );

	*sql << "delete from tmplsql_tester";
	CPPUNIT_ASSERT( sql->exec() );	
}


void
fixture::streams(){
	tmplsql::handle sql = this->get_handle();
	// gibberish - should cause next statement to fail, if it is not abandonded.
	*sql << "sdfsd fasdf adsfsda ff";
	sql->abandon_statement();

	*sql << "select now()";
	CPPUNIT_ASSERT( ! sql->single_value().empty() );
}

void
fixture::transactions() {

	tmplsql::handle sql1 = this->get_handle();
	CPPUNIT_ASSERT( sql1->ping() );

	tmplsql::handle sql2 = this->get_handle();
	CPPUNIT_ASSERT( sql2->ping() );

	*sql1 << "delete from tmplsql_tester";
	CPPUNIT_ASSERT( sql1->exec() );

	CPPUNIT_ASSERT( sql1->begin_trans() );
	CPPUNIT_ASSERT( sql1->in_trans() );
	*sql1 << "insert into tmplsql_tester (test1, test2, test3) values ('foo',now()," << 1 << ")";
	CPPUNIT_ASSERT( sql1->exec() );

	*sql2 << "select test1 from tmplsql_tester where test3 = " << 1;
	CPPUNIT_ASSERT( sql2->single_value().empty() );

	CPPUNIT_ASSERT( sql1->commit_trans() );
	CPPUNIT_ASSERT( ! sql1->in_trans() );

	*sql2 << "select test1 from tmplsql_tester where test3 = " << 1;
	CPPUNIT_ASSERT( ! sql2->single_value().empty() );

	*sql1 << "delete from tmplsql_tester";
	CPPUNIT_ASSERT( sql1->exec() );

	CPPUNIT_ASSERT( sql1->begin_trans() );
	*sql1 << "insert into tmplsql_tester (test1, test2, test3) values ('foo',now()," << 1 << ")";
	CPPUNIT_ASSERT( sql1->exec() );

	*sql2 << "select test1 from tmplsql_tester where test3 = " << 1;
	CPPUNIT_ASSERT( sql2->single_value().empty() );

	CPPUNIT_ASSERT( sql1->abort_trans() );

	*sql2 << "select test1 from tmplsql_tester where test3 = " << 1;
	CPPUNIT_ASSERT( sql2->single_value().empty() );

	// now test errors are correctly logged
	CPPUNIT_ASSERT( sql1->begin_trans() );
	*sql1 << "select fadfa jsldfasd ";
	CPPUNIT_ASSERT( ! sql1->exec() );
	CPPUNIT_ASSERT( sql1->trans_error() );


	*sql1 << "select test1 from tmplsql_tester";
	CPPUNIT_ASSERT( ! sql1->exec() );
	CPPUNIT_ASSERT( sql1->trans_error() );

	CPPUNIT_ASSERT( ! sql1->commit_trans() ); 
}

void
fixture::quote(){
	tmplsql::handle sql = this->get_handle();

	*sql << "insert into tmplsql_tester (test1, test2, test3) values (" 
	     <<	tmplsql::quote("fu'ed") << ",now()," << 1 << ")";
	CPPUNIT_ASSERT( sql->exec() );

	*sql << "delete from tmplsql_tester";
	CPPUNIT_ASSERT( sql->exec() );
}

void
fixture::epoch_date(){
	tmplsql::handle sql = this->get_handle();

	*sql << "insert into tmplsql_tester (test1, test2, test3) values (" 
	     <<	tmplsql::quote("fu'ed") << ",now()," << 1 << ")";
	CPPUNIT_ASSERT( sql->exec() );

	*sql << "select " << tmplsql::rdms::epoch_date("test2") << " from tmplsql_tester where test3 = " << 1;
	std::string result = sql->single_value();
	CPPUNIT_ASSERT( ! result.empty() );

	long ret_time = atol( result.c_str() );

	// this is just a guess here, but if definatly shouldn't take more than 30 seconds to insert and select our value from an
	// empty table
	CPPUNIT_ASSERT( ret_time > ( time(NULL) - 30  ) );

	*sql << "delete from tmplsql_tester";
	CPPUNIT_ASSERT( sql->exec() );
}


void
fixture::result_sets(){
	tmplsql::handle sql = this->get_handle();

	*sql << "delete from tmplsql_tester";
	CPPUNIT_ASSERT( sql->exec() );

	*sql << "insert into tmplsql_tester (test1, test2, test3) values (" 
		     << "'foo',now()," << 1 << ")";
	CPPUNIT_ASSERT( sql->exec() );

	*sql << "select test1,test3 from tmplsql_tester";
	tmplsql::rdms::result_set rs = sql->select();


	tmplsql::rdms::result_set::rows_iterator it;
	it = rs.begin();
	CPPUNIT_ASSERT( it != rs.end() );
	it++;
	CPPUNIT_ASSERT( it == rs.end() );
	
	*sql << "delete from tmplsql_tester";
	CPPUNIT_ASSERT( sql->exec() );


	for ( int i = 0; i < 100; i++ ){
		*sql << "insert into tmplsql_tester (test1, test2, test3) values (" 
		     << "'foo',now()," << i << ")";
		CPPUNIT_ASSERT( sql->exec() );
	}

	*sql << "select test1,test3 from tmplsql_tester order by test3";
	rs = sql->select();
	CPPUNIT_ASSERT( sql->error_msg().empty() );
	int x = 0;
	int y = 0;
	tmplsql::rdms::result_set::rows_iterator row;
	
	row = rs.begin();
	CPPUNIT_ASSERT( row != rs.end() );

	CPPUNIT_ASSERT( 2 == rs.num_fields() );

	// check that the results were returned in correct order
	CPPUNIT_ASSERT( ! strcmp( row[1] , "0" ) );
	for ( ; row != rs.end(); row++ ){
		x++;
		y=0;
		for ( tmplsql::rdms::result_set::fields_iterator field = row.begin(); field != row.end(); field++ ){
			*field;
			y++;
		}
	}
	CPPUNIT_ASSERT( x == 100 );
	CPPUNIT_ASSERT( y == 2 );
	*sql << "select fasdfasdf ";
	tmplsql::rdms::result_set bad_rs = sql->select();
	row = bad_rs.begin();
	CPPUNIT_ASSERT( row == bad_rs.end() );

	*sql << "drop table tmplsql_tester";
	sql->exec();
}


