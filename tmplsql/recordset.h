/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * $Id: recordset.h 1353 2005-11-17 19:42:15Z nas $ 
 * Copyright (C) 2002 Nathan Stitt  
 * See file COPYING for use and distribution permission.
 */


#ifndef _TMPLSQL_RECORDSET_H_
#define _TMPLSQL_RECORDSET_H_

#include "tmplsql/handle.h"
#include <boost/tuple/tuple.hpp>
#include "tmplsql/lexical_cast.h"
#include <string>



namespace tmplsql {
	//! recordset class.  This performs the 'magic' of taking values from the rdms class and transforming
	//! them into thier proper datatypes.
	template <  class T0,                           
		    class T1 = boost::tuples::null_type, class T2 = boost::tuples::null_type, class T3 = boost::tuples::null_type,
		    class T4 = boost::tuples::null_type, class T5 = boost::tuples::null_type, class T6 = boost::tuples::null_type,
		    class T7 = boost::tuples::null_type, class T8 = boost::tuples::null_type, class T9 = boost::tuples::null_type >
	class recordset {
		// private typedefs
		// note that tuple is merly a typedef, and is never instanciated.  All we really use the boost tuple stuff
		// for here is to remember what types to convert to.
		typedef boost::tuple<T0,T1, T2, T3, T4, T5, T6, T7, T8, T9 > tuple;

		typedef recordset<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> self;
	public:

		//! number of types we were passed. Used to test that the number of columns returned by the rdms matches the number
		//! we should have.  If not an assert is triggered.
		static const int length =  boost::tuples::length< tuple >::value;

		//! ctor.  Is passed an sql handle and executes the query stored in it. 

		recordset( const handle& h ) :
			need_exec_( true ),
			handle_(h)
		{ }

		//! iterator class.  Note that there is no corresponding const_iterator class provided, as modifications of values are
		//! not permitted.
		class iterator : public rdms::result_set::rows_iterator {
			//! only recordset may create a valid iterator
			friend class recordset;
		public:
			//! ctor.  a iterator created by this method is in an invalid state.
			iterator() { }

			//! returns converted value of field at Index
			template<int Index>
			typename boost::tuples::element<Index,tuple >::type
			get(){
				return lexical_cast< typename boost::tuples::element<Index,tuple >::type >( (*this)[Index] );
			}
		private:
			iterator( const rdms::result_set::rows_iterator& ri ) :
				rdms::result_set::rows_iterator( ri )
			{ }
		};

		//! this is a potentially dangerous method to use, as it requires you to have independantly have submitted a new query to the sql handle
		//! to test for success, test for begin() == end()
		void refresh(){
			need_exec_ = true;
		};

		//! begin of results
		iterator begin() {
			if ( need_exec_ ){
				this->exec();
			}
			return iterator( rs_.begin() );
		}

		//! end of results
		iterator end(){
			return iterator( rs_.end() );
		}

		//! return the tmplsql::handle that record_set is using
		handle&
		get_handle(){
			return handle_;
		}
	private:
		bool need_exec_;
		void exec(){
			if ( handle_.valid() ){
				rs_ = handle_->select();
				need_exec_ = false;
			}
		}
		rdms::result_set rs_;
		handle handle_;
	};

}

#endif // _TMPLSQL_RECORDSET_H_
