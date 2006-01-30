/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * $Id: commas.h 36 2003-03-11 21:36:59Z nas $ 
 * Copyright (C) 2001 Nathan Stitt  
 * See file COPYING for use and distribution permission.
 */

#ifndef _TMPLSQL_COMMAS_H_
#define _TMPLSQL_COMMAS_H_

#include <string>
#include <iostream>

namespace tmplsql {
	//! A simple class to return commas every time except the first time it is used.
	/*! Not strictly related to the use of this library, but included as I find it usefull.
	  Used like so:
	 <pre><code>
	  tmplsql::commas comma;
	  tmplsql::handle sql;
	  *sql << "select " 
	  for ( fields::iterator it = fields.begin(); it != fields.end(); it++ ){
                 *sql << comma << (*it).name();
	  }
	  </code></pre>
	*/
	class commas {
	public:
		//! ctor
		commas();
		//! return a comma unless it is the first time used
		char get();
		//! return number of times get was called
		int times_called() const;
		//!  reset times_called to 0
		void reset();
	private:
		int times_called_;
	};
}

std::ostream & operator << ( std::ostream& out_stream, tmplsql::commas& comma );

#endif // _TMPLSQL_COMMAS_H_

