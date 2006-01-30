/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * $Id: query.h 1178 2005-06-29 14:24:27Z  $ 
 * Copyright (C) 2002 Nathan Stitt  
 * See file COPYING for use and distribution permission.
 */



#ifndef _TMPLSQL_SELECT_H_
#define _TMPLSQL_SELECT_H_

#include "tmplsql/recordset.h"
#include "tmplsql/fields.h"
#include "tmplsql/functors.h"
#include "tmplsql/hash_map.h"
#include "tmplsql/operators.h"
#include "tmplsql/commas.h"
#include "tmplsql/row_saver.h"
#include <boost/tuple/tuple.hpp>
#include <bitset>



namespace tmplsql {

// 	// forward declare
	template< class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9 > class query;

	namespace detail {
		template< class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9 > class rs_holder;
	} // namespace detail


	//! base class for a query.  If you want create your own methods of permforming a subselect, derive from base_query, implementing the select_query method.
	struct base_query {
		//! this returns a sql statment that is appropriate to be used in a subselect.
		virtual std::string sub_select_query() const=0;

		//! destructor is of course virtual, as this is meant to be a base class
		virtual ~base_query() { };
	};

	/*! peforms a query using a tmplsql::handle.  Types of queries supported are:
	 -inner and outer joins
	 -subselects using a tmplsql::comp_operator
	 -filters on fields using a tmplsql::comp_operator
	 -limit clause
	*/
	template <  class T0,                           
		    class T1 = boost::tuples::null_type, class T2 = boost::tuples::null_type, class T3 = boost::tuples::null_type,
		    class T4 = boost::tuples::null_type, class T5 = boost::tuples::null_type, class T6 = boost::tuples::null_type,
		    class T7 = boost::tuples::null_type, class T8 = boost::tuples::null_type, class T9 = boost::tuples::null_type >
	class query :
		public base_query

	
	{
		//! detail::rs_holder<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9> is a friend
		/*! this is so that it can find wich row_saver is responsible for which key by peeking at
		 the primary_key linked list */
		friend class detail::rs_holder<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9>;
		
		typedef typename boost::tuple< T0,T1,T2,T3,T4,T5,T6,T7,T8,T9 > tuple_type;
		typedef rs_holder<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9> rs_holder_t;
		typedef ::detail::hash_map<size_t,::tmplsql::detail::rs_holder<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9>* > rsh_map_t;
		rsh_map_t rsh_map;


		// typedef for our recordset, which handles the actual dirty work of permforming the query, and convterting to base types.
		typedef recordset< typename detail::field_value_type<T0>::type,typename detail::field_value_type<T1>::type,
				   typename detail::field_value_type<T2>::type,typename detail::field_value_type<T3>::type,
				   typename detail::field_value_type<T4>::type,typename detail::field_value_type<T5>::type,
				   typename detail::field_value_type<T6>::type,typename detail::field_value_type<T7>::type,
				   typename detail::field_value_type<T8>::type,typename detail::field_value_type<T9>::type > recordset_t;


		//! the primary_key struct holds information about the fields that are primary_keys, 
		//! and may therefore be used to update other fields in the query.  
		struct primary_key {
			std::bitset< 9 > fields;
			char index;
			fields::field_name_t field;
			fields::table_name_t table;
			primary_key *next;
			primary_key() : index(0),next(0) { }
			~primary_key(){
				delete next;
			}
		};

		// functors to create primary keys linked list.

		// return 0 if the field isn't a primary key
		template< class T >
		primary_key* pk_create( const T& f, const fields::non_primary&, char Index = 0 ){ return 0; }

		// create a new key, set it's data members, and return it if the type is primary
		template< class T >
		primary_key* pk_create( const T& f,const fields::primary&, char Index ){
			primary_key *ret_val = new primary_key;
			ret_val->field = f.name();
			ret_val->table = f.table();
			ret_val->index = Index;
			return ret_val;
		}

		// we've reached the last field, therefore return 0, and don't recurse any further.
		inline primary_key* create_primary_keys( const boost::tuples::null_type&, primary_key *pk, char ) { return pk; };

		// the main functor, calls itself recursivly, calling pk_create to determine if the field is primary or no.
		template < class H, class T >
		inline primary_key*
		create_primary_keys( const boost::tuples::cons<H, T>& x, primary_key *pk=0, char Index = 0 ) {

			primary_key *our_key = pk_create<H>( x.get_head(), typename H::field_type(), Index );
			if ( our_key ){
				if ( pk ){
					primary_key *tmp=pk;
					while (  tmp->next ) {
						tmp = tmp->next;
					}
					tmp->next = our_key;
				} else {
					pk = our_key;
				}
			} 
			return create_primary_keys( x.get_tail(), pk, ++Index );
		}

		void reset_query(){
			for ( typename rsh_map_t::iterator it = rsh_map.begin(); rsh_map.end() != it; ++it ){
				delete it->second;
				rsh_map.erase( it );
			}
			needs_select_=true;
		}
	public:
		query() :
			pk_( 0 ),
			needs_select_(true),
			limit_(0),
			rs_( handle() )
		{
			// set our primary_key linked list.  If there are no primary fields, then pk_ will be 0, and the query will
			// not be considered updatable.
			pk_=create_primary_keys<typename tuple_type::head_type, typename tuple_type::tail_type >( tup );
		}

		//! an iterator that is used to return the initialized fields from a query.
		struct iterator : public recordset_t::iterator {
			//! query<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9> is a friend
			/* this is so that it can create an iterator with the proper constructor */
			friend class query<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9>;
			//! detail::rs_holder<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9> is a fried
 			/* this is so that it access our pointer to the main query class */
 			friend class detail::rs_holder<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9>;
		public:
			//! constructor.  An iterator created by this method is in an invalid state and should not be used.
			iterator() :
				query_(0) 
			{ }
			/*!
			  get the field referenced by position Index
			  @return an initialized instance of the class stored at position Index.
			*/
			template<int Index>
			typename boost::add_reference<
			typename boost::tuples::element<Index, tuple_type >::type
			>::type
			get();
		private:
			iterator( const typename recordset_t::iterator& it,query<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9> *q ) :
				recordset_t::iterator( it ),
				query_(q)
			{ }
			query<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9> *query_;

		};
		//! @return an iterator pointing to the first row returned by the query
		iterator begin(){
			if ( needs_select_ ) {
				this->select();
			}
			return iterator( rs_.begin(),this );
		}
		//! @return an iterator marking the end of the rows.
		iterator end(){
			return iterator( rs_.end(),this );
		}

		//! filter the rows selected by applying operator op to the value held by T
		/*! return true if successfull, false otherwise */
		template<typename T>
		bool set_filter( const T& f, const comp_operator &op = eq_operator() ) {
			this->reset_query();
			std::stringstream str;
			str << f.table() << "." << f.name()
			    << op.before_value() 
			    << quote( f.get() )
			    << op.after_value();
			where_ = str.str();
			return true;
		}

		//! filter the rows selected using the field located at Index in query.
		/*! operator op is applied to the value val.
		  return true if successfull, false otherwise */		
		template<int Index>
		bool
		set_filter( const typename boost::tuples::element<Index, tuple_type>::type::value_type& val,  const comp_operator& op = eq_operator() ) {
			this->reset_query();
			std::stringstream str;
			str <<  boost::tuples::get<Index>(tup).table() << "." <<  boost::tuples::get<Index>(tup).name()
			    << op.before_value() 
			    << val
			    << op.after_value();
			where_ = str.str();
			return true;
		}
		//! filter the rows selected by performing a subselect.
		/*! operator op is applied to the select statement returned by  base_field::sub_select_query */
		bool set_filter( const base_field& f, const base_query& q, const comp_operator &op = eq_operator() ) {
			this->reset_query();
			std::stringstream str;
			str << f.table() << "." << f.name()
			    << op.before_value() 
			    << q.sub_select_query()
			    << op.after_value();
			where_ = str.str();
			return true;
		}

		//! limit the number of rows returned
		/*! @param limit the maximum number of rows to return
		  @return true on success, false otherwise */
		bool set_limit( unsigned int limit ){
			this->reset_query();
			limit_ = limit;
			return true;
		}

		//! set field to use an inner join
		/*! fields in query located at Field1 and Field2 are compared using operator op */
		template<int Field1,int Field2>
		bool set_inner_join( const comp_operator& op = eq_operator() ) { 
			this->reset_query();
			std::stringstream str;
			str << boost::tuples::get<Field1>(tup).table() << " inner join " 
			    << boost::tuples::get<Field2>(tup).table() << " on " 
			    << boost::tuples::get<Field1>(tup).table() << "." << boost::tuples::get<Field1>(tup).name()
			    << op.before_value()
			    << boost::tuples::get<Field2>(tup).table() << "." << boost::tuples::get<Field2>(tup).name()
			    << op.after_value();
			join_condition_ = str.str();
			return true;
		}

		//! set field to use an outer join
		/*! fields in query located at Field1 and Field2 are compared using operator op */
		template<int Field1,int Field2>
		bool set_outer_join( const comp_operator& op = eq_operator() ) { 
			this->reset_query();
			std::stringstream str;
			str << boost::tuples::get<Field1>(tup).table() << " left outer join " 
			    << boost::tuples::get<Field2>(tup).table() << " on " 
			    << boost::tuples::get<Field1>(tup).table() << "." << boost::tuples::get<Field1>(tup).name()
			    << op.before_value()
			    << boost::tuples::get<Field2>(tup).table() << "." << boost::tuples::get<Field2>(tup).name()
			    << op.after_value();
			join_condition_ = str.str();
			return true;
		}
		

		//! destructor
		~query(){
			delete pk_;
			for ( typename rsh_map_t::iterator it = rsh_map.begin(); rsh_map.end() != it; ++it ){
				delete it->second;
				rsh_map.erase( it );
			}
		}
	private:
		std::string
		sub_select_query() const {
			std::stringstream str;
			str << "( ";
			unsigned int tmp_lim = limit_;
			unsigned int& lim_ref = const_cast<unsigned int&>(limit_);
			lim_ref = 1;
			this->stream_query( str );
			lim_ref=tmp_lim;
			str << " )";
			return str.str();
		}

	 	bool stream_query( std::ostream& str ) const {
			commas comma;
			str << "select";
			detail::set_field_spec< typename tuple_type::head_type, typename tuple_type::tail_type >( tup,str,comma );
			str << " from ";
			if ( ! join_condition_.empty() ){
				str << join_condition_;
			} else {
				if ( pk_ ){
					comma.reset();
					primary_key *pk = pk_;
					while ( pk->next ) { pk = pk->next; }
					str << comma << pk->table;
					
				} else {
					str << boost::tuples::get<0>(tup).table();
				}
			}

			if ( ! where_.empty() ){ 
				str <<  " where " << where_;
			}
 			
 			if ( limit_ ){
 				str << " limit " << limit_;
 			}
			return true;
 		}

		void select(){
			if ( this->stream_query( *(rs_.get_handle()) ) ){
				rs_.refresh();
				needs_select_ =	true;
			}
		}

		// the first in our linked list of primary keys.  Is set to 0 initialy.
		primary_key *pk_;
		
		bool needs_select_;
		unsigned int limit_;
		recordset_t rs_;
		std::string where_;
		std::string join_condition_;
		tuple_type tup;
	};




	namespace detail {

		
		/** 
		    The rs_holder class holds an instance of a row_saver.  It's only purpose is to:
		    - Only create a row_saver if the row is updateable ( a primary key was given for the table )
		    - Delay creation of a row_saver untill the row is actually needed, 
		    then keep track of it, so it may be reused if the query is iterated over more than once.
		*/
		template< class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9 >
		class rs_holder {
			// typedef to save my fingers
			typedef rs_holder<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9> self;
		public:
			//! get the row saver responsible for field_index
			/*! @param field_index index of the field we need a row_saver for.
			  @param it iterator to get the value from in case we have to create and initialize a new field
			*/
			row_saver<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9>* 
			get_rs( int field_index, typename query<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9>::iterator* it ){
				// first we need to find the rs_holder that is resposible for that
				// field's table.  Start with ourself
				self *row_saver_hld = this;

				// now we loop through each of query's primary_keys, attempting to find
				// the one that has it's field set for this index.

				// there is a possible bug here:  If we don't find the field, row_saver_hld will
				// still be set to this, however since we have to have something to return,
				// as if we are using rs_holder's then we are under the impression that we are
				// updateable.  So it might as well work this way, and maybe have a chance of working
				typename query<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9>::primary_key *pk=it->query_->pk_;

				if ( pk ) {
					for ( 	typename query<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9>::primary_key *pk = it->query_->pk_; pk->next; pk = pk->next ){
						// did we find it?
						if ( ! pk->fields.test( field_index ) ) {
							// no, so increment row_saver_hld to the next field
							row_saver_hld = row_saver_hld->next;
						}
					}
				}

				// if our row_saver_holder does not already have a row_saver, then create it.
				if ( ! row_saver_hld->rs ){
					// no, so create one, passing it the relevent primary key information.

					// did we find any the right key?
					if ( pk ){
						// yes, so create a row_saver with that primary_key info
						row_saver_hld->rs = new row_saver<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9>( pk->field,pk->table,quote( (*it)[ pk->index ] ) );
					} else {
						// no so create a row_saver that won't actually save anything
						row_saver_hld->rs = new row_saver<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9>();
					}

				}

				return row_saver_hld->rs;
			}

			//! create a new holder.
			/*! rs_holders created with this method will use the iterator to create one of themselves for each query::primary_key that exists */
			rs_holder( typename query<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9>::iterator *it ) :
				next ( 0 ),
				rs ( 0 )
			{
				self *row_saver_hld = this;
				for ( typename query<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9>::primary_key *pk = it->query_->pk_; pk->next; pk = pk->next ){
					row_saver_hld->next = new self();
					row_saver_hld = row_saver_hld->next;
				}
			}
			//! dtor
			~rs_holder(){
				if ( rs ) {
					rs->release();
				}
				if ( next ){
					delete next;
				}
			}
		private:
			rs_holder() : 
				next ( 0 ),
				rs( 0 )
			{ }
			rs_holder ( const self &rsc );
			self *next;
			row_saver<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9> *rs;
		
		};
	} // namespace detail


	template< class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9 >
	template<int Index>
	typename boost::add_reference<
	typename boost::tuples::element<Index, typename query<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9>::tuple_type >::type
	 >::type
	query<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9>::iterator::
	get(){

// 		// first check and see if our query is updatable or not.
// 		// we do this by seeing if we have any primary keys or not.				
// 		if ( query_->pk_ ){
		// our row_saver holder
		::tmplsql::detail::rs_holder<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9> *row_saver_hld = 0;
			
			// is our holder in our queries hash map for this row, ie: has it been created before.
		typename query<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9>::rsh_map_t::iterator it 
			= query_->rsh_map.find( this->x_index_ );

		// did we find it?
		if ( query_->rsh_map.end() == it ) {
			// nope, so we create one and insert it into the hash_map
			row_saver_hld = new ::tmplsql::detail::rs_holder<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9>( this );
			query_->rsh_map.insert( this->x_index_,row_saver_hld );
		} else {
			// yep, so use it
			row_saver_hld = it->second;
		}

		// now that we have our row_saver holder, retrieve the field from it
		//::tmplsql::row_saver<T0,T1,T2,T3,T4,T5,T6,T7,T8,T9> *row_svr = row_saver_hld->get_rs( Index,this );
		// make sure we actually got a row_saver, then set our return value to the field it gives us.
		// note that we pass a pointer to the iterator to the row_saver, which handles
		// initialization of the field if it needs to.
		return 	row_saver_hld->get_rs( Index,this )->template get<Index>( this );
		// } else {
// 			typename boost::tuples::element<Index, tuple_type >::type ret_val;
// 			// we are not updateable, so just initialize the field with the value
// 			ret_val.initialize
// 				( lexical_cast< typename boost::tuples::element<Index,tuple_type >::type::value_type >( (*this)[Index] ) );
// 			return ret_val;	
// 		}
		

	}
	

} // namespace tmplsql






/*
select holds list of primary key pointers

pointer to select is passed to iterator, and iterator is declared a friend of select

when iterator.get<Index>() is called, iterator checks select to find fields's table. 

If a row_saver has been created for that table, 
    then the row_saver creates and returns field
if not
    iterator creates row_saver, associates row_saver with it's table, and instructs row_saver on what it's primary_key is
    it then gets field from newly created row_saver
*/

#endif // _TMPLSQL_SELECT_H_
