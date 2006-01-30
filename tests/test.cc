/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * $Id: test.cc 1178 2005-06-29 14:24:27Z  $ 
 * Copyright (C) 2002 Nathan Stitt  
 * See file COPYING for use and distribution permission.
 */

#include <iostream>

#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestResult.h> 
#include <cppunit/TestResultCollector.h>

#include "tests/rdms.h"
#include "tests/commas.h"
#include "tests/tuples.h"
#include "tests/recordset.h"
#include "tests/fields.h"
#include "tests/select.h"
#include <queue>

static std::queue<tmplsql::rdms*> sq;


int
main (int argc, char **argv) 
{

	tmplsql::rdms::connection_string cs("test");
 	tmplsql::rdms::initialize( cs ,0);

	std::cout << "--------------------------------------------------------------------------------\n";
	std::cout << "  During these test we will occasionally intentionally cause errors to occur,\n"
		  << "  and then deal with them. In order to test erorr handling, you have to\n"
		  << "  actually cause errors :) However at times the low level interface to sql\n"
		  << "  library may spew notices about the errors.\n\n"
		  << "  Please do not worry about them, and only pay attention to the results\n"
		  << "  printed at the end of the run.\n\n"
		  << "  Thanks, we will now start the tests.....\n" 
		  << std::endl;
	std::cout << "--------------------------------------------------------------------------------\n";
	CppUnit::TextUi::TestRunner runner;

	runner.addTest( rdms_test::suite() );
  	runner.addTest( commas_test::suite() );
  	runner.addTest( tuples_test::suite() );
  	runner.addTest( recordset_test::suite() );
  	runner.addTest( fields_test::suite() );
 	runner.addTest( select_test::suite() );

	runner.run();
	std::cout << "--------------------------------------------------------------------------------\n";
	CppUnit::TestResultCollector &result = runner.result();
	if ( result.testFailuresTotal() ){
		std::cout << "Ooops, we have errors\nPlease fix 'em!" << std::endl;
	} else {
		std::cout << "  A clean run. Congratulations....\n"
			  << "  You may now go forth and add those cool new features." << std::endl;
	}
	std::cout << "--------------------------------------------------------------------------------\n";
	return 0;
}	
