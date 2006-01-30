/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * $Id: handle.cc 1178 2005-06-29 14:24:27Z  $ 
 * Copyright (C) 2002 Nathan Stitt  
 * See file COPYING for use and distribution permission.
 */

#include "tmplsql/handle.h"
#include "tmplsql/rdms.h"
#include <cassert>

using namespace tmplsql;

handle::handle() :
	handle_( rdms::handle() ),
	count_( new unsigned int(1) )
{
	assert ( handle_ );
}

handle::handle(const handle& h) :
	handle_ ( h.handle_ ),
	count_( h.count_ )
{
	++*count_;
}

handle&
handle::operator=( const handle& h ) {
	if ( this != &h ) {
		if ( handle_ && 0 == --*count_ ) {
			handle_->release();
			delete count_;
		}
		handle_ = h.handle_;
		count_ = h.count_;
		++*count_;
        }
        return *this;
}


rdms&
handle::operator*() const {
	return *handle_;
}

rdms*
handle::operator->() const {
	return handle_;
}



void
handle::release(){
	if ( handle_ && 0 == --(*count_) ) {
		delete count_;
		handle_->release();
	}
	handle_=0;
}

handle::~handle() {
	if ( handle_ && 0 == --(*count_) ) {
		delete count_;
		handle_->release();
	}
}

bool
handle::valid(){
	bool ret_val = false;
	if (  handle_ ){
		ret_val = handle_->ping();
	}
	return ret_val;
}
