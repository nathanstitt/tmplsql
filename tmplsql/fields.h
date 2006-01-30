/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * $Id: fields.h 1274 2005-10-07 21:56:07Z nas $ 
 * Copyright (C) 2002 Nathan Stitt  
 * See file COPYING for use and distribution permission.
 */




#ifndef _TMPLSQL_FIELDS_H_
#define _TMPLSQL_FIELDS_H_

#include <string>
#include "tmplsql/lexical_cast.h"
#include "tmplsql/row_saver_base.h"
#include "tmplsql/quote.h"

namespace tmplsql {
	class row_saver_base;

	namespace fields {
		typedef const char* table_name_t;
		typedef const char* field_name_t;
		//! primary key type.  A class with primary as a typedef to field_type
		//! is considered to be a primary key
		struct primary {};
		//! non primary type.  base_field's field_type is typedef to non_primary, 
		//! therefore all types that inherit from base field are assumed to not
		//! be a primary field, unless they set field_type to be fields::primary
		struct non_primary {};
	};

	//! base_field class.  Holds basic info on a field, such as it's name and the table it belongs to.
	//! note that the base_field class does not have any way to hold a value.
	class base_field {
	public:
		//! the type of the field.  base_field is defined as being non_primary courtesy of this typedef
		typedef fields::non_primary field_type;

		//! ctor
		base_field( fields::field_name_t name, fields::table_name_t table );

		//! copy ctor
		base_field( const base_field& bf );

		//! name of the field
		fields::field_name_t name() const;

		//! name of table the field belongs to
		fields::table_name_t table() const;
		//! has the field been modified.  base_field will always return false, however is_modified
		//! has been declared virtual so this behaviour will be redefiened later.
		virtual bool is_modified() const;
		//! Set the fields row_saver.
		/*!
		    Note that base_field simply turns around and calls row_saver_base::remove_ref to unregister itself
		    as soon as this method is called.  Declared virtual so this behaviour can be redefiened later. 
		 * @param rs row_saver to unregister with before destructing
		 * @param key a unique value that enables row_saver to identify the object.
		 */
		virtual void set_row_saver( row_saver_base *rs, void *key );
		//! returns true if it is safe to delete object.
		/*! base_field always returns true here.
		Declared virtual so this behaviour can be redefiened later.
		*/
		virtual bool delete_ok();
		//! virtual destructor
		virtual ~base_field();
	private:
	
		fields::field_name_t name_;
		fields::table_name_t table_;
	};

	//! a field that holds a value of type T.
	/*  field<T> is reference counted so that for better or worse things such as the below will work:
	   <pre><code>
	   field<int> f;
	   f.set( 1234 );
	   field<int> f2(f);
	   assert( f2.get() == 1234 );
	   f2 = 42;
	   assert( f.get() == 42 );
	   <pre><code>
	   the goal of the field<T> class is to provide a small wrapper around the field information and a value of type<T>.
	*/
	template < class T >
	class field : public base_field {
	public:
		//! the type of the field
		typedef T value_type;
	private:
		friend class row_saver_base;

		value_type *value_;

		void inc_ref_count(){
			++*count_;
		}

		void dec_ref_count(){
			if (  0 == --*count_ ) {
				delete value_;
				delete count_;
			} else  if ( rs_ && (*count_) == 1 ) {
				rs_->remove_ref( key_ );
				rs_=0;
			}
		}
	protected:
		//! count of how many instances are sharing value_
		size_t *count_;
		//! the row_saver to remove our reference to.
		row_saver_base *rs_;
		//! a unique value to send to row_saver_base::unregister
		void *key_;
	public:
		//! the only constructor.
		/*!
		 * @param name the name of the field
		 * @param table the name of the table
		 */
		field( fields::field_name_t name, fields::table_name_t table ) : 
			base_field ( name,table ),
			value_( new value_type() ),
			count_( new unsigned int(1) ),
			rs_( 0 ),
			key_(0)
		{ 


		}
		//! returns true if it is safe to delete object
		virtual bool delete_ok(){
			return ( *count_ == 1 );
		}

		//! copy constructor.
		field( const field& f ) :
			base_field ( f ),
			value_( f.value_ ),
			count_( f.count_ ),
			rs_( f.rs_ ),
			key_( f.key_ )
			
		{
			inc_ref_count();
		}

		//! initialize the fields value.
		/*! settting the value in this manner does not set the
		  field to be modifield 
		  @param value value to set field to */
		virtual bool initialize( const T& value ) {
			*value_ = value;
			return true;
		}
		//! retrieve the value of the field
		/*! @return field's value */
		T get() const {
			return *value_;
		}
		//! conversion operator
		/*!  this is somewhat controversial, however I feel it makes sense to be able to do:
		  <code><pre>
		  field<int> f;
		  f.initialize( 49 );
		  int i = f;
		  </pre></code>
		*/
		operator T() const {
			return *value_;
		}

		//! return true if the field's value is equal to value
		bool operator== ( const T& value ) const {
			return (*value_) == value;
		}
		//! return true if the field's value is not equal to value
		bool operator!= ( const T& value ) const {
			return *value_ != value;
		}
		//! destructor
		virtual ~field() {
			this->dec_ref_count();
		}
		//! Set the fields row_saver.
		/*!
		    Unlike  base_field,  The field<T> class does not immediatly unregister itself.  Rather, it waits until it's reference
		    count has reached one, then it calls row_saver_base::remove_ref to give row_saver a chance to save the value back to the
		    tmplsql::handle before it destructs.
		 * @param rs row_saver to unregister with before destructing
		 * @param key a unique value that enables row_saver to identify the object.
		 */
		virtual void set_row_saver( row_saver_base *rs, void *key ){
			rs_ = rs;
			key_ = key;
		}
	};


	//! a field athat allows it's value to be updated.  However, this will only work if the field has a record_saver_base
	//! class that has been set.
	template < class T >
	class updateable_field : public field<T> {
		bool *is_modified_;
	protected:

	public:
		//! constructor
		/*!  @param name the name of the field
		 * @param table the name of the table */
		updateable_field(  fields::field_name_t name, fields::table_name_t table ) :
			field<T>( name,table ),
			is_modified_ ( new bool(false) )
		{ }

		//! copy constructor
		updateable_field( const updateable_field& f ) :
			field<T>( f ),
			is_modified_ ( f.is_modified_ )
		{

		}

		//! set the value of the field to <i>value</i>
		/*! Note that calling this causes the field to be marked as modified.
		  @param value the value to set the field to. */
		bool set( const T& value ){
			if ( field<T>::rs_ ) {
				*is_modified_ = true;
				return this->initialize( value );
			} else {
				return false;
			}
		}
		//! @return true if the field has been modified, false otherwise
		bool is_modified() const {
			return *is_modified_;
		}
		//! attempt to save the field immediatly, rather than waiting until all instances of it go out of scope
		bool sync() {
			*is_modified_ = false;
			if ( field<T>::rs_ ) {
				return field<T>::rs_->sync();
			} else {
				return false;
			}
		}
		//! dtor
		virtual ~updateable_field(){
			if ( 1 == *(field<T>::count_) ) {
				delete is_modified_;
			}
		}
	};

} // namespace tmplsql



#endif // _TMPLSQL_FIELDS_H_
