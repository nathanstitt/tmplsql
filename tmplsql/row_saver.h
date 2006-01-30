/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * $Id: row_saver.h 37 2003-03-12 22:12:43Z nas $ 
 * Copyright (C) 2002 Nathan Stitt  
 * See file COPYING for use and distribution permission.
 */


#ifndef _TMPLSQL_ROW_SAVER_H_
#define _TMPLSQL_ROW_SAVER_H_

#include "tmplsql/commas.h"
#include "tmplsql/functors.h"
#include "tmplsql/row_saver_base.h"
#include <boost/tuple/tuple.hpp>
#include <boost/type_traits/transform_traits.hpp>
#include <sstream>
#include "tmplsql/row_saver_base.h"
#include "tmplsql/quote.h"


namespace tmplsql {


	// forward declare
	template< class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9 > class rs_holder;


	//! the row_saver is responsible for creating the fields on the stack and keeping a pointer to them until delete_ok returns true
	template <  class T0,                     class T1 = boost::tuples::null_type, class T2 = boost::tuples::null_type, 
		    class T3 = boost::tuples::null_type, class T4 = boost::tuples::null_type, class T5 = boost::tuples::null_type, 
		    class T6 = boost::tuples::null_type, class T7 = boost::tuples::null_type, class T8 = boost::tuples::null_type, 
		    class T9 = boost::tuples::null_type >
	class row_saver : public row_saver_base
	{
		typedef typename boost::tuple<
			typename detail::make_ptr<T0>::type,
			typename detail::make_ptr<T1>::type,
			typename detail::make_ptr<T2>::type,
			typename detail::make_ptr<T3>::type,
			typename detail::make_ptr<T4>::type,
			typename detail::make_ptr<T5>::type,
			typename detail::make_ptr<T6>::type,
			typename detail::make_ptr<T7>::type,
			typename detail::make_ptr<T8>::type,
			typename detail::make_ptr<T9>::type
		> storage_type;

	public:
		/** 
		 * @param field the name of the primary key
		 * @param table the table the primary key belongs to
		 * @param pk_value the value of the primary key
		 */
		row_saver( fields::field_name_t field, fields::table_name_t table, const std::string& pk_value ) :
			field_( field ),
			table_( table ),
			pk_value_( pk_value ),
			released_(false)
		{
			//			detail::initialize< typename storage_type::head_type, typename storage_type::tail_type >( tup );
		}


		row_saver() :
			field_( 0 ),
			table_( 0 ),
			released_(false)
			{ 
			}
		

		//! retrieve the field located at Index
		template<int Index>
		typename boost::add_reference<
		typename boost::remove_pointer< typename boost::tuples::element<Index, storage_type>::type >::type
		>::type
		get( rdms::result_set::rows_iterator *it ) {

			typename boost::tuples::element<Index, storage_type>::type ret =  tup.template get<Index>();
			if ( ! ret ){
				ret = new typename boost::remove_pointer< typename boost::tuples::element<Index, storage_type>::type >::type;
				ret->initialize( lexical_cast< 
						 typename boost::remove_pointer< 
						 typename boost::tuples::element< Index,storage_type >::type
						 >::type::value_type
						 >( (*it)[Index] ) 
						 );
				tup.template get<Index>() = ret;
			}
			ret->set_row_saver(this,ret);
			return *ret;
		}
		//! allow the row_saver to delete itself once the last field calls remove_ref( void *key )
		void release(){
			this->sync();
			detail::maybe_delete< typename storage_type::head_type, typename storage_type::tail_type >( tup );

			if ( ! detail::num_active< typename storage_type::head_type, typename storage_type::tail_type >(tup) ) {
				delete this;
			} else {
				released_ = true;
			}

		}
		//! this method should be called once for each field when the last instance of the field is ready to destruct.
		void remove_ref( void *key ){
			this->sync();
			detail::remove_field_ref< typename storage_type::head_type, typename storage_type::tail_type >( tup,key );
			if ( released_ && ! detail::num_active< typename storage_type::head_type, typename storage_type::tail_type >(tup) ) {
				delete this;
			}
		}
		//! check fields for modification, and make an update statement with them.
		/*! @return true if update occurs without errors, or if no update statement was needed.
		  false if the update fails, or an update is not possible due to the row_saver not haveing a primary_key  */
		bool sync(){
			
			if ( field_  ){
				handle h;
				commas comma;
				*h << "update " << table_ << " set ";
				if ( detail::stream_modified_field_updates< typename storage_type::head_type, typename storage_type::tail_type >( tup,*h,comma ) ){
					*h << " where " << table_ << "." << field_ << "=" << pk_value_;
					return h->exec();
				} else {
					h->abandon_statement();
					return true;
				}
			} else {
				return false;
			}
		}

	private:
		fields::table_name_t field_;
		fields::table_name_t table_;
		std::string pk_value_;
		bool released_;
		storage_type tup;

	};
}

/*
row_saver holds pointer to each field, which is initially set to null.
On query->get() the field is created, and returned.  
Once ref count of field reaches one, row_saver->remove_ref() is called.
On row_saver->remove_ref(), row_saver makes sweep of fields, updating as needed, then removes its reference to field.
Row_Saver also checks how many fields it has active, and if none are, it calls delete this;
If during a fields lifecycle, field may call row_saver->sync() to update itself, and any other fields that have been modified.



*/

#endif // _TMPLSQL_ROW_SAVER_H_
