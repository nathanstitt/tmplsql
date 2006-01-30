/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * $Id: functors.h 1353 2005-11-17 19:42:15Z nas $ 
 * Copyright (C) 2002 Nathan Stitt  
 * See file COPYING for use and distribution permission.
 */



#ifndef _TMPLSQL_DETAIL_H_
#define _TMPLSQL_DETAIL_H_

#include <boost/tuple/tuple.hpp>
#include <boost/type_traits.hpp>
#include <list>
#include "tmplsql/row_saver.h"

namespace tmplsql {
	class base_field;

	namespace detail {

// 		template <int Len=50>
// 		struct char_ptr_cmp {
// 			bool operator()( const char* s1, const char* s2) const  {
// 				return strncmp(s1, s2,Len) < 0;
// 			}
// 		};

		//! converts a type to a pointer
		template<class T> 
		struct make_ptr {
			//! convert the type to a pointer
			typedef T* type;
		};

		//! specialization of make_pter to leave null_type as is.
		template <>
		struct make_ptr< class boost::tuples::null_type > {
			//! don't muck with null_type
			typedef boost::tuples::null_type type;
		};


		//! simple functor that provides appropriate typedefs for the value_type
		//! of a field used in row_saver, makes 'real' values become a pointer, but leaves
		//! null_type as is.
		template< typename T >
		struct field_value_type {
			//! typedef to get the field's value type
			typedef typename T::value_type type;
		};

		//! simple functor that provides appropriate typedefs for the value_type
		//! of a field used in row_saver, makes 'real' values become a pointer, but leaves
		//! null_type as is.
		template <>
		struct field_value_type< boost::tuples::null_type > {
			//! of course, null_type lacks the value_type typedef, therefore we just use it
			typedef boost::tuples::null_type type;
		};

		//! terminator for our recursive funtor
		inline bool
		remove_field_ref( const boost::tuples::null_type&, void *key ) {
			return false;
		};

		//! tests if the field's memory address matches key, if it does,
		//! the field is deleted.
		template <class H, class T>
		inline bool
		remove_field_ref( boost::tuples::cons<H, T>& x, void *key ) { 
			if ( x.get_head() && x.get_head() == key ){
				delete x.get_head();
				x.get_head()=0;
				return true;
			} else {
				return remove_field_ref( x.get_tail(),key );
			}
		}

                inline void
                set_field_spec( const boost::tuples::null_type&, std::ostream &stmt,commas &comma ) { };

                template <class H, class T>
                inline void
                set_field_spec( const boost::tuples::cons<H, T>& x, std::ostream &stmt, commas& comma ) {
                        stmt << comma << x.get_head().table() << "." << x.get_head().name();
                        set_field_spec( x.get_tail(),stmt,comma );
                }

		inline size_t
		stream_modified_field_updates( const boost::tuples::null_type&, std::ostream &stmt,commas &comma ) { return 0; };

		template <class H, class T>
		inline size_t
		stream_modified_field_updates( const boost::tuples::cons<H, T>& x, std::ostream &stmt, commas& comma ) {
			if ( x.get_head() && x.get_head()->is_modified() ){
 				stmt << comma  << x.get_head()->name() << "=" << quote( x.get_head()->get() );
				return  1+stream_modified_field_updates( x.get_tail(),stmt,comma );				
 			} else {
				return stream_modified_field_updates( x.get_tail(),stmt,comma );
			}

		}

		inline int
		num_active( const boost::tuples::null_type& ) {
			return 0;
		};

		template <class H, class T>
		inline int
		num_active( boost::tuples::cons<H, T>& x ) { 
			if ( x.get_head() ){
				return 1 + num_active( x.get_tail() );
			} else {
				return num_active( x.get_tail() );
			}
		}

		inline void maybe_delete( const boost::tuples::null_type& ) {};

		template < class H, class T >
		inline void maybe_delete( boost::tuples::cons<H, T>& x) {
			if ( x.get_head() && x.get_head()->delete_ok() ) {
				delete x.get_head();
				x.get_head() = 0;
			}
			maybe_delete( x.get_tail() );
		}

//  		struct pk {
//  			pk() : f(0) { }
//  			base_field* f;
//  			unsigned int indx;
// 			//char [9] fields;
//  		};

// 		inline void
// 		get_field_refs( const boost::tuples::null_type&, std::list<base_field*>& ) { };

// 		template <class H, class T>
// 		inline void
// 		get_field_refs( const boost::tuples::cons<H, T>& x, std::list<base_field*>& bf ) {
// 			bf.push_back( &( x.get_head() ) );
// 			get_pks( x.get_tail(),bf );
// 		}
// 			if ( x.get_head().is_primary_key() ){
// 				bool found = false;
// 				for ( std::list<detail::pk>::iterator field = pk.begin(); pk.end() != field; ++field ){
// 					if ( ! strncmp( field->f->table(), x.get_head().table(),50 ) ){
// 						found = true;
// 						break;
// 					}
// 				}
// 				if ( ! found ) {
// 					const base_field *f = &( x.get_head() );  // dynamic_cast<const base_field*> ( &(x.get_head() ) )
// 					detail::pk tmp;
// 					for ( int i=0;i<=9; ++i ){
// 						tmp.fields[i] = 0;
// 					}
// 					tmp.f =  const_cast<tmplsql::base_field*>( f );
// 					tmp.indx = index;
// 					pk.push_back( tmp );
// 				}
// 			} else {
// 				for ( std::list<detail::pk>::iterator field = pk.begin(); pk.end() != field; ++field ){
// 					if ( ! strncmp( field->f->table(), x.get_head().table(),50 ) ){
// 						found = true;
// 						break;
// 					}
// 				}
// 			}
		
// 		}




	}
}

#endif // _TMPLSQL_DETAIL_H_
