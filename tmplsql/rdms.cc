/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * $Id: rdms.cc 1353 2005-11-17 19:42:15Z nas $
 * Copyright (C) 2001 Nathan Stitt  
 * See file COPYING for use and distribution permission.
 */


#include "tmplsql/handle.h"
#include "tmplsql/rdms.h"
#include "tmplsql/quote.h"

#include "config.h"

#include <sstream>
#include <queue>
#include <map>
#include <string.h>
#include <iostream>

using namespace tmplsql;

typedef std::queue<class tmplsql::rdms*> queue_type;
static queue_type bench_warmers;
static queue_type::size_type max_spare_handles = 0;


#include <IceUtil/RecMutex.h>
static IceUtil::RecMutex sql_queue_mutex;


static std::string conn_string;


bool
rdms::initialize( const connection_string& con, int num_spare_connections ) {
	conn_string.clear();
	max_spare_handles = num_spare_connections;
	if ( ! con.login.empty() ) {
		conn_string += " user=";
		conn_string += con.login;
	}
	if ( ! con.password.empty() ) {
		conn_string += " password=";
		conn_string += con.password;
	}
	if ( ! con.host.empty() ) {
		conn_string += " host=";
		conn_string += con.host;
	}
	if ( ! con.port.empty() ) {
		conn_string += " port=";
		conn_string += con.port;
	}
	if ( ! con.dbname.empty() ) {
		conn_string += " dbname=";
		conn_string += con.dbname;
	}
	return true;
}

bool
rdms::clear_cache(){
	sql_queue_mutex.lock();
	while (  bench_warmers.size() ) {
		delete bench_warmers.front();
		bench_warmers.pop();
	}

	sql_queue_mutex.unlock();
	return true;
}

bool
rdms::exec() {
	bool ret_val=false;
	//	std::cout << buffer_.curval() << std::endl;

	if ( ( ! in_trans_ ) || ( ! trans_error_ ) ) {
		PGresult *res = PQexec( conn,  buffer_.curval().c_str() ) ; //buffer_.curval().c_str() );
		if ( PQresultStatus(res) == PGRES_COMMAND_OK ) {
			ret_val=true;
		} else {
			this->log_error(buffer_.curval(),res);
			if ( in_trans_ ) {
				trans_error_ = true;
			}
		}
		PQclear(res);
	}
	buffer_.abandon();
	return ret_val;
}



bool
rdms::begin_trans() {
	bool ret_val=false;
	if ( ! in_trans_ ){
		PGresult *res = PQexec( conn, "BEGIN TRANSACTION" );
		if (PQresultStatus(res) == PGRES_COMMAND_OK) {
			ret_val = true;
			in_trans_=true;
 		} else {
			this->log_error("BEGIN TRANSACTION",res);
		}
		PQclear(res);
		trans_error_ = false;
	} else {
		ret_val=false;
	}
	return ret_val;
}


bool
rdms::commit_trans() {
	bool ret_val=false;
	if ( in_trans_ && ( ! trans_error_ ) ) {
		PGresult *res = PQexec( conn, "COMMIT TRANSACTION");
		if (PQresultStatus(res) == PGRES_COMMAND_OK) {
			ret_val=true;
		} else {
			this->log_error("COMMIT TRANSACTION",res);
		}
		PQclear(res);
		in_trans_ = trans_error_ = false;
	} else {
		this->abort_trans();
	}
	return ret_val;
}


bool
rdms::abort_trans() {
	bool ret_val=false;
	if ( in_trans_ ){
		PGresult *res = PQexec( conn, "ABORT TRANSACTION");
		if (PQresultStatus(res) == PGRES_COMMAND_OK) {
			ret_val      = true;
		} else {
			this->log_error("ABORT TRANSACTION",res);
		}
		PQclear(res);
	} else {
		ret_val=false;
	}
	in_trans_ = trans_error_ = false;
	return ret_val;
}


bool 
rdms::in_trans() {
	return in_trans_;
}

bool 
rdms::trans_error() {
	return trans_error_;
}


std::string
rdms::current_statement(){
	return buffer_.curval();
}


bool
rdms::ping(){
	if ( PQstatus(conn) == CONNECTION_OK) {
		return true;
	} else {
		return this->connect();
	}
}

std::string
rdms::epoch_date(const std::string& field){
	return "date_part('epoch',"+field+")";
}

void
rdms::abandon_statement(){
	buffer_.abandon();
}

std::string
rdms::single_value() {
	std::string ret_val;

	PGresult *res = PQexec( conn,  buffer_.curval().c_str() );
	if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) &&  PQnfields(res)  ) {
		ret_val = PQgetvalue(res, 0, 0);
	} else {
		this->log_error(buffer_.curval(),res);
		if ( in_trans_ ){
			trans_error_ = true;
		}
	}
	buffer_.abandon();
	PQclear(res);
	return ret_val;
}

std::string
rdms::insert( const std::string& sequence ){
		
	if ( this->exec() && ( ! sequence.empty() ) ) {
		(*this) << "select currval(" << quote( sequence ) << ")";
		return this->single_value();
	} else {
		return std::string();
	}
}

rdms::~rdms() {
	PQfinish(conn);
}

rdms::rdms() :
	// seems silly/dangerous to to do this, as output_buffer 
	// isn't defined yet, but is needed to keep gcc 3.x happy
	std::ostream( &buffer_ ),
	conn(0),
	in_trans_(false),
	trans_error_( false )
{
	this->rdbuf( &buffer_ );
	connected_=connect();
}

rdms*
rdms::handle() {
	rdms* ret_val = 0;
	sql_queue_mutex.lock();
	if ( bench_warmers.empty() ) {
		ret_val = new rdms;
	} else {
		ret_val = bench_warmers.front();
		bench_warmers.pop();
	}
	sql_queue_mutex.unlock();
	// do a ping to make sure the handle is still good
	ret_val->ping();

	return ret_val;
}

void
rdms::release(){
	if( in_trans_ ) {
		this->abort_trans();
	}
	this->abandon_statement();
	sql_queue_mutex.lock();

	if ( bench_warmers.size() && bench_warmers.size() >= max_spare_handles ) {
		delete bench_warmers.front();
		bench_warmers.pop();
	}

	if ( max_spare_handles ) {
		bench_warmers.push( this );
	} else {
		delete this;
	}

	sql_queue_mutex.unlock();
}


bool
rdms::connect() {
	in_trans_ = false; 
	conn = PQconnectdb( conn_string.c_str() );
	if ( PQstatus(conn) == CONNECTION_OK) {
		connected_=true;
	} else {
#ifdef DEBUG
		std::cerr << "Unable to connect to sql database using conection string: " 
					  << "    '" << conn_string << "'\n"
					  << "Error msg: \n"
					  << "    " << PQerrorMessage( conn ) 
					  << std::endl;
#endif // DEBUG
       	}
	return connected_;
}



rdms::sql_stmt_buffer::sql_stmt_buffer(){

}

std::string
rdms::sql_stmt_buffer::flush(){
	std::string ret_val = buffer_;
	buffer_="";
	return ret_val;
}

std::string
rdms::sql_stmt_buffer::curval(){
	return buffer_;
}

void
rdms::sql_stmt_buffer::abandon(){
	buffer_="";
}


int
rdms::sql_stmt_buffer::overflow(int c) {
	buffer_ += static_cast<char>(c);
	return c;
}

std::string
rdms::error_msg(){
	std::string ret_val = last_error_;
	last_error_ = "";
	return ret_val;	
}

void
rdms::log_error( const std::string& sqlstmt,const PGresult *res ){
	last_error_ = "Statement: \n";
	last_error_ += sqlstmt + "\n";
	last_error_ += PQresultErrorMessage(res);
#ifdef DEBUG
	std::cerr << "sql statement\n" 
		  << sqlstmt 
		  << "\nfailed with error message:\n" 
		  << last_error_ << "\n";
#endif
}


////////////////////////////////// result set //////////////////////////////////

rdms::result_set::result_set( PGresult *res ) :
	res_(res),
	count_( new unsigned int(1) ),
	num_rows_( PQntuples(res_) ),
	num_fields_( PQnfields(res_) )
{

}


rdms::result_set::result_set( const result_set& rs ) :
	res_(rs.res_),
	count_( rs.count_ ),
	num_rows_( rs.num_rows_ ),
	num_fields_( rs.num_fields_ )
{
	++*count_;
}


rdms::result_set::result_set() :
	res_( 0 ),
	count_( new unsigned int(1) ),
	num_rows_(-1),
	num_fields_(-1)
{

}

int
rdms::result_set::num_fields(){
	return num_fields_;
}

int
rdms::result_set::num_rows(){
	return num_rows_;
}

rdms::result_set
rdms::result_set::operator=( const result_set& rs ){
	if ( this != &rs ) {
		if ( 0 == --*count_ ) {
			PQclear(res_);	
			delete count_;
		}
		res_ = rs.res_;
		count_ = rs.count_;
		++*count_;
		num_rows_= rs.num_rows_;
		num_fields_ = rs.num_fields_;
        }
        return *this;
}

rdms::result_set::~result_set(){
	if (count_ && 0 == --(*count_) ) {
		delete count_;
		PQclear(res_);
	}
}

const
unsigned int
rdms::result_set::size() {
	if ( num_rows_ != -1 ) {
		return num_rows_;
	} else {
		return 0;
	}
}

rdms::result_set::rows_iterator
rdms::result_set::begin(){
	if  ( num_rows_ != -1 ) {
		return rows_iterator( res_,0,num_fields_ );
	} else {
		return rows_iterator( res_,num_rows_,num_fields_ );
	}
}

rdms::result_set::rows_iterator
rdms::result_set::end(){
	return rows_iterator( res_,num_rows_,num_fields_ );
}



///////////////////////////// rows iterator /////////////////////////////////////////


rdms::result_set::rows_iterator::rows_iterator() :
	x_index_( -1 ),
	y_index_( -1 )
{

}

rdms::result_set::rows_iterator::rows_iterator( PGresult *res, int x_index, int y_index) :
	x_index_( x_index ),
	res_(res),
	y_index_( y_index )
{

}

rdms::result_set::rows_iterator::rows_iterator( const rows_iterator& ri )  :
	x_index_( ri.x_index_ ),
	res_( ri.res_ ),
	y_index_( ri.y_index_ ) 
{


}

rdms::result_set::rows_iterator
rdms::result_set::rows_iterator::operator=( const rows_iterator& ri ){
	x_index_ = ri.x_index_;
	y_index_ = ri.y_index_;
	res_ = ri.res_;
	return ri;
}



bool
rdms::result_set::rows_iterator::operator!=( const rows_iterator& ri ) const {
	return   ri.x_index_ != x_index_;
}

bool
rdms::result_set::rows_iterator::operator==( const rows_iterator& ri ) const {
	return  ri.x_index_ == x_index_ ;
}

rdms::result_set::rows_iterator
rdms::result_set::rows_iterator::operator++(int) {
	rows_iterator tmp(*this);
	++x_index_;
	return tmp;
}

rdms::result_set::rows_iterator
rdms::result_set::rows_iterator::operator++() {
	++x_index_;
	return *this;
}

const char*
rdms::result_set::rows_iterator::operator[] ( int index ) const {
	return PQgetvalue(res_, x_index_, index );
}


rdms::result_set::fields_iterator
rdms::result_set::rows_iterator::begin(){
	return fields_iterator( res_, x_index_,0 );
}


rdms::result_set::fields_iterator
rdms::result_set::rows_iterator::end(){
	return fields_iterator( res_, x_index_,y_index_ );
}

///////////////////////////// fields iterator /////////////////////////////////////////

rdms::result_set::fields_iterator::fields_iterator() :
	res_( 0 ),
	x_index_( -1 ),
	y_index_( -1 )
{

}

rdms::result_set::fields_iterator::fields_iterator( PGresult *res, int x_index,int y_index) :
	res_(res),
	x_index_( x_index ),
	y_index_( y_index )
{

}


rdms::result_set::fields_iterator::fields_iterator( const fields_iterator& fi ) :
	res_(fi.res_),
	x_index_( fi.x_index_ ),
	y_index_( fi.y_index_ )

{

}



rdms::result_set::fields_iterator
rdms::result_set::fields_iterator::operator=( const fields_iterator& fi ){
	y_index_ = fi.y_index_;
	x_index_ = fi.x_index_;
	res_ = fi.res_;
	return fi;
}

bool
rdms::result_set::fields_iterator::operator!=( const fields_iterator& fi ) const {
	return  fi.y_index_ != y_index_;
}

bool
rdms::result_set::fields_iterator::operator==( const fields_iterator& fi ) const {
	return  fi.y_index_ == y_index_;
}

rdms::result_set::fields_iterator
rdms::result_set::fields_iterator::operator++() {
	++y_index_;
	return *this;
}

rdms::result_set::fields_iterator
rdms::result_set::fields_iterator::operator++(int) {
	fields_iterator tmp( *this );
	++y_index_;
	return tmp;
}

const char*
rdms::result_set::fields_iterator::operator*() const{
	return PQgetvalue(res_, x_index_,y_index_ );
}

rdms::result_set
rdms::select(){
	PGresult *res = PQexec( conn,  buffer_.curval().c_str() );
	if ( PQresultStatus(res) != PGRES_TUPLES_OK ) {
		this->log_error(buffer_.curval(),res);
		if ( in_trans_ ) {
			trans_error_ = true;
		}
	}
	buffer_.abandon();
	return result_set( res );
}


rdms::connection_string::connection_string(
					   std::string dbname,
					   std::string login,
					   std::string password,
					   std::string host,
					   std::string port
					   ):
	dbname(dbname),
	login(login),
	password(password),
	host(host),
	port(port)
{ }
	
