/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * $Id: rdms.h 1353 2005-11-17 19:42:15Z nas $ 
 * Copyright (C) 2001 Nathan Stitt  
 * See file COPYING for use and distribution permission.
 */

#ifndef _TMPLSQL_H_FLAG_
#define _TMPLSQL_H_FLAG_

#include <string>
#include "tmplsql/handle.h"
extern "C" { 
#include "postgresql/libpq-fe.h"
}


#include <iostream>

//!  The tmplsql namespace
/*!
  Holds all resources associated with accessing a RDMS
*/
namespace tmplsql {
	class handle;

	//!  The base rdms class.
	/*!
	  the rdms class is a singleton, meaning that only
	  a limited number of instances of it is created.  It is accesed by createing an instance of the lightweight
	  handle class, which handles aquiring on ctor and releaseing the sql connection in dtor.
	  As a general rule the handle class should only be created on the stack, not heap, as a sql connection will be
	  leaked if the handle is not destroyed.
	  <pre><code>
	  tmplsql::handle sql;
	  sql->exec("DELETE from very_important_table");
	  </code></pre>
	  This class is meant to be written in a database generic method, so that it is possible to easily switch 
	  to different RDMS if nessecery, by only modifing this class.
	  
	  Thread safety:  If the library is configured with --enable-threads, all that does is insert locking around the 
	  handle creation and release.  It DOES NOT make the connection handle itself be thread safe.  To do that would be outside
	  the scope of this library, as the library only maps onto the lowlevel c api of the database in question, which in most
	  cases is not thread safe.  Therefore do not share a handle among muliple threads and expect things to work properly.

	  TODO:  use a private magic cookie value for the connection parameters so each time they are changed,
	  so it's possible to reconnect our handle if needed before it's handed out.
	*/
	class rdms : public std::ostream {
		//! Using the handle class is the only way to get an instance of this class.
		friend class handle;
	public:
		struct connection_string;

		//! initialize our rdms connections.
		/*!
		  @param con connection information to use to connect to the rdms.
		  @param num_spare_connections Number of connections to leave open before pruning them, defaults to 0, which will cause
		  and immediate disconnction once the handle goes out of scope.
		*/
		static bool initialize( const connection_string& con,int num_spare_connections = 0 );

		//! closes all rmds connections that are open but not in use.
		/*!
		  This method only closes connections that are in use.  To close all connections all handle methods must go out of scope.
		 */
		static bool clear_cache();


		//! For all sql operations that do not return rows.
		/*! This is for inserts where the value of the sequence column is not needed to be known,
		  updates, deletes, anything that does not return rows.
		  The return value is undertermined when a select statement is set and exec is called.  Not sure
		  why you would want to do that, just letting you know.
		  However, DO NOT start or end a transaction using this method, 
		  rather use the begin or end trans methods so that overlapping transactions can
		  be advoided.
		*/
		bool exec();

		//! Begin a transaction. 
		/*!
		  @return true if transaction began successfully, false if it failed
		  ( most likely becouse the handle was already in a transaction )
		*/
	        bool begin_trans();

		//! commit a transaction. 
		/*! 
		  @return true if transaction commited successfully, false if it failed
		  ( most likely becouse the one of the operations in the transaction failed )
		*/
		bool commit_trans();

		//! Aborts a transaction. 
		/*!
		  @return true if transaction aborted successfully, false if it failed
		  abort_trans() should never fail
		*/
		bool abort_trans();

		//! Is a transaction still open?
		/*! 
		  Convienience function to tell whether a transaction is currently open.
		  @return true if in transaction, false if not
		*/
		bool in_trans();
		
		//! has an error occured in the transaction
		/*! 
		  Convienience function to tell whether an error has occured in the transaction
		  @return true if in an error has occured, false if not
		*/
		bool trans_error();

		
		//! return the current contents of the buffer awaiting execution
		std::string current_statement();


		//! ping
		//! check connection to determine if it is valid
		bool ping();

		//! return a field in the proper format to get a unix epoch date in response to a select.
		/*! 
		  @param field field to use
		  @return string formated for proper date format
		*/
		static std::string epoch_date(const std::string& field);

		//! abandon stored sql statement set with streams operators
		void abandon_statement();

		//! For sql inserts where the value of the sequence column from the new row is needed
		//! I may have to remove this method eventually, as I'm not sure how the concept of sequences
		//! maps between other rdms's.  I do know that Oracle, Sybase, and Postgresql support it.
		/*!
		 this has a slightly higher overhead than just calling exec(), so if you don't need the sequence value,
		 just use exec()
		 @param sequence name of the sequence to be checked
		 @return value of sequence
		*/
		std::string insert( const std::string& sequence );

		//! return only the first field from the first row of a sql select statement
		//! here for convience for those times when you really don't want to
		//! use the lengthier iterator based result set inteface
		std::string single_value();

		//! return description of last error that occured
		std::string error_msg();

		//! destructor
		~rdms();

		//! collection of iterators to allow traversing
		//! through the results of a select statement.
		/*!
		  this class owns a pointer to the rdms results. 
		  It is reference counted so as to only free the results once all
		  copies are destroyed, however no such safeguards are done if the rdms handle
		  is destroyed before the result_set is.  Therefore the handle that created
		  this should not go out of scope while the result_set is still in use.
		*/ 
		class result_set {
			//! only the rdms class is allowed to create a valid instance of this class
			friend class rdms;
		public:
			//! copy constructor.  The main constructor is private
			//! as a result_set can only be created by the rdms class
			result_set( const result_set& rs);

			//! default constructor.  Objects created in this manner are in as they say,
			//! an <i>undefined state</i>.  Meaning bad shit will happen sooner or later if
			//! you attempt to use it.  It is safe howerver to create a result_set using this, 
			//! and then later assign it to a valid one returned by begin()
			result_set();

			//! number of rows returned
			const unsigned int size();

			//! assignment operator.
			result_set operator=( const result_set& rs );

			// forward declaration
			class rows_iterator;

			//! return number of fields.  Note this is NOT a 0 based count.
			//! A return value of 0 or -1 means that something went wrong
			//! with your query.
			int num_fields();
			
			//! number of rows found.  A return value of -1 means an error has occured.
			int num_rows();

			//! although most rdms's will allow you to determine the number of rows
			//! returned by a select statement, not all do,
			//! therefore we deliberatly advoid having operator-- for our iterators, or allowing
			//! direct access to an arbitrary row.
			class fields_iterator {
				//! only row_iterator is allowed to create a valid instance of this class
				friend class rows_iterator;
			public:
				//! ctor.  a fields_iterator created by this method is in an invalid state.
				fields_iterator();
				//! copy ctor.
				fields_iterator( const fields_iterator& ri );
				//! assignment operator
				fields_iterator operator=( const fields_iterator& it );
				//! test for inequality against another fields_iterator
				bool operator!=( const fields_iterator& it ) const;
				//! test for equality against another fields_iterator
				bool operator==( const fields_iterator& it ) const;
				//! move to next field
				fields_iterator operator++(int);
				//! move to next field
				fields_iterator operator++();
				//! I've decided to make the deref operator return
				//! const char* although I normally much prefer std::string,
				//! as thats how the c interface to most (all?) rmds's 
				//! handle returning values, and efficiency is paramont
				const char* operator*() const;
			private:
				// ctor, may only be called by our friend rdms
				fields_iterator( PGresult *res ,int x_index,int y_index );
				PGresult *res_;
				// field
				int x_index_;
				// rows
				int y_index_;
			};
			//! iterator class for rows returned.
			//! Note that there is no corresponding const_iterator class provided, as modifications of values are
			//! not permitted.
			class rows_iterator {
				//! a valid rows_iterator may only be created by result_set
				friend class result_set;
			public:
				//! ctor.  a rows_iterator created by this method is in an invalid state.
				rows_iterator();
				//! copy ctor.
				rows_iterator( const rows_iterator& ri );
				//! assignment operator
				rows_iterator operator=( const rows_iterator& ri );
				//! test for inequality against another rows_iterator
				bool operator!=( const rows_iterator& ri ) const;
				//! test for equality against another rows_iterator
				bool operator==( const rows_iterator& ri ) const;
				//! move to next row of results
				rows_iterator operator++(int);
				//! move to next row of results
				rows_iterator operator++();
				//! I've decided to make the subscript operator return
				//! const char* although I normally much prefer std::string,
				//! as thats how the c interface to most (all?) rmds's 
				//! handle returning values, and efficiency is paramont
				const char* operator[] ( int index ) const;
				//! return fields_iterator pointing to first column of current row
				fields_iterator begin();
				//! return fields_iterator pointing to last colum of current row
				fields_iterator end();
			protected:
				//! our x index is protected so it can be used by other iterators that inherit from rows_iterator
				int x_index_;
			private:
				rows_iterator( PGresult *res , int x_index, int y_index );
				PGresult *res_;

				// we calc number only once so we can can
				// avoid doing it each time
				// we create do a new row
				int y_index_;
			};
			//! return rows_iterator pointing to begining of results
			rows_iterator begin();
			//! return rows_iterator pointing to end of results
			rows_iterator end();
			//! destructor.  deincrements reference count and free's pointer
			//! to result set once reference count reaches zero
			~result_set();
		private:
			result_set( PGresult *res );
			PGresult *res_;
			unsigned int *count_;
			int num_rows_;
			int num_fields_;
		};
		//! execute whatever sql statements have been pushed in to our streambuffer
		//! and return the results as a result_set.  Gracefully handles situations
		//! where the statement(s) were not selects, by not the result_set not containing
		//! any rows.
		result_set select();

		//! struct to hold connection details for the rdms.  Before anything may be done,
		//! this must be passed to 
		struct connection_string {
			//! constructor.  Note that all methods default to empty strings,
			//! usefull if you want to use whatever the default connection your database system offers.
			/** 
			 * @param dbname database to connect to
			 * @param login user login to connect as
			 * @param password user password to use
			 * @param host host to connect to.  May be either dns name or ip address. Some rdms systems also allow Unix-domain socket here.
			 * @param port port to connect to on host.  If Unix domain sockets are used, this should be left blank
			 */
			connection_string(
					  std::string dbname="",
					  std::string login="",
					  std::string password="",
					  std::string host="",
					  std::string port=""
					  );
			//! database to connect to
			std::string dbname;
			//! user login to connect as
			std::string login;
			//! user password to use
			std::string password;
			//! host to connect to.  May be either dns name or ip address. Some rdms systems also allow Unix-domain socket here.
			std::string host;
			//! port to connect to on host.  If Unix domain sockets are used, this should be left blank
			std::string port;
			
		};

		//! obtain a handle to the rdms
		static rdms* handle();

		//! release the handle.
		void release();
	private:
		rdms();

		std::string last_error_;
		//! log error message
		void log_error( const std::string& sqlstmt,const PGresult *res );

		bool connect();

		//! The postgres connection.
		PGconn     *conn;

		//! are we in a transaction or no?
		bool in_trans_;

		//! has an error occured during transaction
		bool trans_error_;

		//! Constructor
		/*! The sql::sql constructor, notice that this is the only one, 
		  and it takes no arguments, as all configuration comes from our
		  xml encoded config file */

		//! are we connected to the RDMS?
		bool connected_;

		//! class to buffer our sql statements that have been inserted by rdms's inherited ostream
		//! TODO support xsputn
		class sql_stmt_buffer : public std::streambuf {
		public:
			sql_stmt_buffer();
			//! flush buffer
			std::string flush();
			//! return contents of buffer
			std::string curval();
			//! abandon contents of buffer
			void abandon();
		private:
			int overflow(int c);
			std::string buffer_;
		};

		sql_stmt_buffer buffer_;
	};

}


  
#endif // _SQL_H_FLAG_
