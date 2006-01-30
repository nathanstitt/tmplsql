/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * $Id: row_saver_base.h 37 2003-03-12 22:12:43Z nas $ 
 * Copyright (C) 2002 Nathan Stitt  
 * See file COPYING for use and distribution permission.
 */


#ifndef _TMPLSQL_ROW_SAVER_BASE_
#define _TMPLSQL_ROW_SAVER_BASE_


namespace tmplsql {
	//! the base class for a row_saver.
       	class row_saver_base {
	public:
		//! check fields for modification, and make an update statement with them.
		/*! @return true if update occurs without errors, or if no update statement was needed.
		  false if the update fails, or an update is not possible due to the row_saver not haveing a primary_key  */
		virtual bool sync()=0;

		//! this method should be called once for each field when the last instance of the field is ready to destruct.
		/*! @param key the unique value that was passed to the field by calling field<T>::set_row_saver(). It enables row_saver to identify the object. */
		virtual void remove_ref( void *key )=0;
		//! dtor
		virtual	~row_saver_base(){ }
	private:

	};


}


#endif // _TMPLSQL_ROW_SAVER_BASE_
