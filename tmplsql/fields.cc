/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * $Id: fields.cc 36 2003-03-11 21:36:59Z nas $ 
 * Copyright (C) 2002 Nathan Stitt  
 * See file COPYING for use and distribution permission.
 */



#include "tmplsql/fields.h"


using namespace tmplsql;


base_field::base_field( fields::field_name_t name, fields::table_name_t table ) :
	name_( name ),
	table_( table )
{ }

base_field::base_field( const base_field& bf ) :
	name_(bf.name_),
	table_(bf.table_)
{

}

fields::field_name_t
base_field::name() const {
	return name_;
}


fields::table_name_t
base_field::table() const {
	return table_;
}


// const char*
// base_field::get() const{
// 	return "";
// }

// std::string
// base_field::quoted_value() const {
// 	return this->value();
// }



bool
base_field::is_modified() const {
	return false;
}

void
base_field::set_row_saver( row_saver_base *rs, void *key ){
	rs->remove_ref( key );
}

bool
base_field::delete_ok(){
	return true;
}

base_field::~base_field(){

}

// std::string
// base_field::value() const {
// 	return std::string();
// }
