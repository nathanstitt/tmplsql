/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * $Id: operators.h 37 2003-03-12 22:12:43Z nas $ 
 * Copyright (C) 2002 Nathan Stitt  
 * See file COPYING for use and distribution permission.
 */




#ifndef _TMPLSQL_OPERATORS_H_
#define _TMPLSQL_OPERATORS_H_



namespace tmplsql {

	//! the pure virtual comparison operator
	struct comp_operator {
		/*! the value that should go before the field being compared.  For instance:
		<pre><code>
		select foo from bar where id=i
 		                            ^--- the equals sign is the before_value
                                            
		</code></pre>
		*/
		virtual std::string before_value() const = 0;

		/*! the value that should go after the field being compared.  For instance:
		<pre><code>
		select foo from bar where id in ( select id from template )
		                             ^--- the 'in ( '             ^--- the ) is
                                              is the before_value         the after value
                                            
		</code></pre>
		*/
		virtual std::string after_value() const = 0;
	};

	//! the most used operator, tests for equality.
	struct eq_operator : public comp_operator {
		//! return an '='
		std::string before_value() const {
			return "=";
		};
		//! return 0 length string
		std::string after_value() const {
			return "";
		};
	};

} // namespace tmplsql

#endif // _TMPLSQL_OPERATORS_H_
