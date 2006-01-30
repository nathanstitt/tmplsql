/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * $Id: handle.h 25 2003-02-18 05:39:38Z nas $ 
 * Copyright (C) 2002 Nathan Stitt  
 * See file COPYING for use and distribution permission.
 */


#ifndef _TMPLSQL_HANDLE_H_
#define _TMPLSQL_HANDLE_H_

#include "tmplsql/rdms.h"


namespace tmplsql {
	class rdms;
	//! a simple class using the shared_ptr idiom, however
	//! instead of deleting the underlying pointer to a
	//! rdms class this calles ->release() on it,
	/*!
	  Thread safety:  If the library is configured with --enable-threads, all that does is insert locking around the 
	  handle creation and release.  It DOES NOT make the connection handle itself be thread safe.  To do that would be outside
	  the scope of this library, as the library only maps onto the lowlevel c api of the database in question, which in most
	  cases is not thread safe.  Therefore do not share a handle among muliple threads and expect things to work properly.
	*/
	class handle {
	public:
		//! ctor
		handle();
		//! copy ctor
		handle( const handle &h );
		//! assignment operator
		handle& operator=(const handle& h);
		//! dref operator
		rdms& operator*() const;
		//! allow use as if was a pointer to
		//! the sql class
		rdms* operator->() const;
		//! release the sql class,
		//! DO NOT call this and then attempt 
		//! further usage, as such use is,
		//! as they say <i>undefined</i>, meaning
		//! bad things will almost certainly happen.
		void release();
		//! test if our connection is still valid
		bool valid();
		//! dtor.  Releases connection back to connection pool, possibly disconnecting it, if
		//! there are more connections open than were set by rdms::initilize
		~handle();
	private:
		rdms *handle_;
		unsigned int *count_;
	};
}

#endif // _TMPLSQL_HANDLE_H_
