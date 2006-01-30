
// 
// Copyright Emin Martinian, emin@allegro.mit.edu, 1998
// Permission granted to copy and distribute provided this
// comment is retained at the top.
//
// $Id: hash_map.h 1353 2005-11-17 19:42:15Z nas $
//
//
// *** IMPORTANT: When you want to compile with debugging assertions
//                turned off, #define NDEBUG .If the NDEBUG preprocessor
//		  token is not defined then debugging assertion
//		  checks are turned on by default.
//

#ifndef EMIN_HASH_TABLE
#define EMIN_HASH_TABLE

// --------------------------------------------------------------
//
// Introduction:
//
// This file is desigend to implement a generic hash table class
// called "my_hash_map".  The my_hash_map class is based upon the
// hash_map class in the UNIX implementation of the
// Standard Template Library.  For some incomprehensible reason
// the hash_map class is not part of the STL standard so some 
// compilers (such as the Microsoft compiler) don't implement
// hash_map.  Basically, the my_hash_map class was designed to 
// have the same interface as the hash_map class from STL, but
// there are currently a few slight differences which I will
// eventually fix.
//
// How To Use: Setting things up
//
// Using the my_hash_map class is pretty simple.  First decide 
// the key type and the value type.  For example, assume you want
// a hash table indexed with integer keys with characters as the
// value.  The first thing you need to do is create a hashing
// object.  The hashing object will implement the hash function
// for your keys.  The hashing object has to be an object which 
// implements hash functions.  For most efficient preformance
// your functor should implement operator() and SecondHashValue,
// but if you want you can just implement operator().
//
// The operator() member function should take the key as input and
// return an integer as output.  Ideally the hash function will
// have the property that different keys are mapped to different
// hash results, but this is not absolutely essential.  The
// SecondHashValue member function is another hash function which
// takes the key as input, but it MUST return an ODD integer as
// the output.  The reason SecondHashValue must return and ODD
// integer is described in the Design Overview sectoion.
//
// The GENERIC_HASH function is a generic hash function which
// hashes on integers.  You can use this function if your
// keys are integers, or you can use it to build more
// complicated hash functions.
//
// How To Use: constructor
//
// Assume we have defined a hashing object called IntHasher, then
// we can create a hash table as follows:
//
//		IntHasher hashObject;
//		my_hash_map<int, char, IntHasher> myTable(10,hashObject);
//
// The first argument in the template specification is "int" 
// which means that the keys will be integers.  The second
// argument in the template specifcation is "char" meaning that
// the values will be characters.  Finally, the third argument
// is "IntHasher" meaning an object of the hashObject class will
// provide the necessary hashing.  
//
//
// How To Use: inserting
//
// To insert a key and value you can either use the insert
// member function or the [] operator.  For example, 
// the following statements would insert a value of 'c' or 'd' 
// into the table for a key of 3:
//
//		myTable.insert(3,'c');
//		myTable[3] = 'd';
//
// Note that the hash table will only store 1 value for each key.
// Thus the second statement above will replace the value of 'c'
// with 'd' for the key 3.
//
// How To Use: searching
//
// The find(...) function can be used to look for a key and value
// pair in the hash table.  Calling myTable.find(someKey) returns
// a HashTableIterator.  Iterator are basically like generalized
// pointers.  The iterator idea is from the Standard Template
// Library.  If someKey exists in myTable, then the iterator 
// returned will point to the corresponding key-value pair.  If
// someKey is not in myTable the the iterator returned will match
// the iterator returned by myTable.end().  The following code
// fragment gives an example of how search for an item using
// iterators:
//
// HashTableIterator<int,char,IntHasher> i = myTable.find(3);
// if (i < myTable.end()) {
//		printf("found item with key = %i ",i->first);
//		printf("and value %c\n",i->second);
// } else {
//		printf("no match in table\n");
// }
//
// You can also say my_hash_map<int,char,IntHasher>::iterator
// instead of HashTableIterator<int,char,IntHasher>.  The first
// syntax corresponds to the STL convention.  Alternativly
// you can use a typedef statement.
//
// How To Use: Deleting
//
// To erase an item from the table you can use the erase
// member function.  Calling myTable.erase(someKey) will attempt 
// to erase the pair with key someKey.  If someKey is found in
// the table then it is deleted and true is returned, otherwise
// false is returned and the table is not modified.
//
// How To Use: for_each
//
// One of the main uses of iterators is that you can do an
// operation for each item in the hash table.  If you are
// using the Standard Template Library you can simply use
// the for_each function.  Otherwise you can use a code
// fragment like the one below:
//
// for(HashTableIterator<int,char,IntHasher> i = myTable.begin();
//      i < myTable.end(); i++) {
//		printf("key: %i, value %c\n",i->first,i->second);
// }
//
// Design Overview:
//
//
// The my_hash_map is implemented as a quadratic probing hash
// table.  In a quadratic probing table we use two hash functions
// for each key.  The first hash function tells us a starting
// point in the table to insert the new record.  If there is
// already another record in that spot (e.g. because another
// key hashed to the same spot), then we use the second hash
// function to check other spots in the table.  To make sure
// that the entire hash table is searched when necessary the
// value returned by the second hash function must be 
// relatively prime to the size of the hash table.  This is
// accomplished by making the hash table size a power of 2 and
// requiring the second hash function return an odd value.
//
// 
//
// For an example of how to use the my_hash_map class see
// the hash_test.cpp file.

//
// Efficiency Comments: If you do a lot of insertion and deletion
// without resizing the table, then it is possible for the hash table
// to get into an inefficient state.  This is because when stuff is
// deleted from the hash table we mark the location as deleted instead
// of empty due to the requirements of the quadratic probing
// algorithm.  Consequently if lots of cells get marked as deleted
// instead of empty then inserting and searching will become slow (as
// slow as O(n)).  Therefore if you are using the hash table in an
// application where you do lots of inserting and deleting it might be
// good to periodically resize or otherwise clear the table.  When I
// get some time I will put in stuff to automatically keep track of
// the fraction of deleted cells and automatically clean up the table
// when there are too many deleted cells.

//
// 
// TODO:
//
// *   Make the hash table keep track of how many deleted cells exist
//     and automatically clean up if there are too many.  See the
//     efficiency comment above.
//
// *	Right now the const_iterator is the same as an iterator
//	we should fix this so that const_iterator implements const-ness.
//


#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <utility>



namespace detail {

	/* The following two constants are used to build a hash function based
	 * on the multiplication method described in the book
	 * _Introduction_To_Algorithms_ by Cormen, Leisserson, and Rivest. */

	static const double g_HASH_CONSTANT =  0.6180339887; /* (sqrt(5) -1)/2 */
	static const double g_HASH_SEED  = 1234567;  /* could also make this
							randomly generated */

	inline int GENERIC_HASH(const int dataToHash) {
		return ( (int) ( g_HASH_SEED * ( g_HASH_CONSTANT * ( dataToHash) - 
						 (int) (g_HASH_CONSTANT*( dataToHash) ))));
	}

	// template <class TF, class TS> 
	// struct my_pair {
	// public:
	//   my_pair(TF const & the_first, TS const & the_second)
	//     :first(the_first), second(the_second)
	//     {}
  
	//   TF first;
	//   TS second;
	// };


#define EMPTY_CELL 0
#define VALID_CELL 1
#define DELETED_CELL 2

	// Define Hash_Assert macro to check assumptions during debugging.
#ifdef NDEBUG
#define Hash_Assert(a , b) ;
#else
#define Hash_Assert(a , b) if (!a) \
{ fprintf(stderr,"Error: Hash_Assertion '%s' failed.\n%s\n",#a,b); abort(); }
#endif

	// Define ExitProgramMacro to abort if something wierd happens
#define ExitProgramMacro( a ) {fprintf(stderr, a ); abort();}

#define MHM_TEMPLATE <class HashType, class HashValue, class Hasher, class EqualityComparer>  
#define MHM_TEMP_SPEC <HashType,HashValue,Hasher,EqualityComparer> 
#define MHM_TYPE_SPEC hash_map MHM_TEMP_SPEC


	inline size_t hash_char_ptr(const char* key) {
		unsigned long val = 0; 
		for ( ; *key; ++key)
			val = 5*val + *key;
  		return size_t(val);
	}
	//! our default hash function
	template <class _Key> struct hash  { };
	//! specialization hash function
	template<> struct hash<std::string> {
		//! return size_t hash
		inline size_t operator()(const std::string& key) const { return  hash_char_ptr( key.c_str() ); }
		//! if the hash returned by operator() is not unique, then SecondHashValue() will be called
		inline size_t SecondHashValue(const std::string& key) const { return (operator()(key) << 1) + 1; }
	};
	//! specialization hash function
	template<> struct hash<char*> {
		//! return size_t hash
		inline size_t operator()(const char* key) const { return hash_char_ptr(key); }
		//! if the hash returned by operator() is not unique, then SecondHashValue() will be called
		inline size_t SecondHashValue(const char* key) const { return (operator()(key) << 1) + 1; }
	};
	//! specialization hash function
	template<> struct hash<const char*> {
		//! return size_t hash
		size_t operator()(const char* key ) const { return hash_char_ptr(key); }
		//! if the hash returned by operator() is not unique, then SecondHashValue() will be called
		inline size_t SecondHashValue( const char* key) const { return (operator()(key) << 1) + 1; }
	};
	//! specialization hash function
	template<> struct hash<char> {
		//! return size_t hash
		size_t operator()(char key) const { return key; }
		//! if the hash returned by operator() is not unique, then SecondHashValue() will be called
		inline size_t SecondHashValue( char key) const { return (operator()(key) << 1) + 1; }
	};
	//! specialization hash function
	template<> struct hash<unsigned char> {
		//! return size_t hash
		size_t operator()(unsigned char key) const { return key; }
		//! if the hash returned by operator() is not unique, then SecondHashValue() will be called
		inline size_t SecondHashValue(unsigned char key) const { return (operator()(key) << 1) + 1; }
	};
	//! specialization hash function
 	template<> struct hash<signed char> {
		//! return size_t hash
 		size_t operator()( signed char key) const { return key; }
		//! if the hash returned by operator() is not unique, then SecondHashValue() will be called
 		inline size_t SecondHashValue( signed key) const { return (operator()(key) << 1) + 1; }
 	};
	//! specialization hash function
	template<> struct hash<short> {
		//! return size_t hash
		size_t operator()(short key) const { return key; }
		//! if the hash returned by operator() is not unique, then SecondHashValue() will be called
		inline size_t SecondHashValue( short key) const { return (operator()(key) << 1) + 1; }
	};
	//! specialization hash function
	template<> struct hash<unsigned short> {
		//! return size_t hash
		size_t operator()(unsigned short key) const { return key; }
		//! if the hash returned by operator() is not unique, then SecondHashValue() will be called
		inline size_t SecondHashValue( unsigned short key) const { return (operator()(key) << 1) + 1; }
	};
	//! specialization hash function
	template<> struct hash<int> {
		//! return size_t hash
		size_t operator()(int key) const { return key; }
		//! if the hash returned by operator() is not unique, then SecondHashValue() will be called
		inline size_t SecondHashValue( int key) const { return (operator()(key) << 1) + 1; }
	};
	//! specialization hash function
	template<> struct hash<unsigned int> {
		//! return size_t hash
		size_t operator()(unsigned int key) const { return key; }
		//! if the hash returned by operator() is not unique, then SecondHashValue() will be called
		inline size_t SecondHashValue( unsigned int key) const { return (operator()(key) << 1) + 1; }
	};
	//! specialization hash function
	template<> struct hash<long> {
		//! return size_t hash
		size_t operator()(long key) const { return key; }
		//! if the hash returned by operator() is not unique, then SecondHashValue() will be called
		inline size_t SecondHashValue( long key) const { return (operator()(key) << 1) + 1; }
	};
	//! specialization hash function
	template<> struct hash<unsigned long> {
		//! return size_t hash
		size_t operator()(unsigned long key) const { return key; }
		//! if the hash returned by operator() is not unique, then SecondHashValue() will be called
		inline size_t SecondHashValue( unsigned long key) const { return (operator()(key) << 1) + 1; }
	};

	template <class HashType, class HashValue, class Hasher=hash<HashType>, class EqualityComparer=std::equal_to<HashType> > class hash_map;

	//! an iterator for the hash_table
	template <class HashType, class HashValue, class Hasher, class EqualityComparer> 
	class HashTableIterator {
		friend class hash_map<HashType,HashValue,Hasher,EqualityComparer>;
	    
		inline std::pair<const HashType, HashValue>* 
		GetCurrentItem() 
		{
			while(_currentIndex < _table->maxLength)
				if (VALID_CELL == _table->tableStatus[_currentIndex])
					return _table->tableData[_currentIndex];
				else
					_currentIndex++;
			return NULL;
		}
	public:
		//! create an iterator pointing to the first element of the hash map
		/*! @param hm the hash map to traverse */
		HashTableIterator( hash_map<HashType,HashValue,Hasher,EqualityComparer> const & hm )
			:_table(&hm), _currentIndex(0)
		{ }

		//! create an iterator pointing to element located at index
		/*! @param hm the hash map to traverse
		   @param index the index location to start at */
		HashTableIterator( hash_map<HashType,HashValue,Hasher,EqualityComparer> const & hm, const int index )
			:_table(&hm), _currentIndex(index)
		{ }

		//! default constructor for iterator.  note that an iterator created in this manner, points to hash_map::end()
		HashTableIterator()
			:_table(NULL), _currentIndex(0)
		{ }
		


		//! postfix operator. moves iterator to next element in hash_map
		inline std::pair<const HashType, HashValue>* operator++(int) 
		{
			std::pair<const HashType, HashValue> * result = NULL;
			for( ; _currentIndex < _table->maxLength; _currentIndex++)
				if (VALID_CELL == _table->tableStatus[_currentIndex]) {
					result = _table->tableData[_currentIndex++];
					break;
				}
			for(; _currentIndex < _table->maxLength; _currentIndex++)
				if (VALID_CELL == _table->tableStatus[_currentIndex]) 
					break;
			return result;
		}

		//! dereference value held by iterator
		inline std::pair<const HashType, HashValue> & 
		operator*() {
			return *(GetCurrentItem());
		}

		//! dereference value held by iterator
		inline std::pair<const HashType, HashValue> * 
		operator->() {
			return GetCurrentItem();
		}

	
		//! prefix operator. moves iterator to next element in hash_map
		inline std::pair<const HashType, HashValue> * operator++() 

		{
			++_currentIndex;
			while(_currentIndex < _table->maxLength)
				if (VALID_CELL == _table->tableStatus[_currentIndex]) 
					return _table->tableData[_currentIndex];
				else 
					++_currentIndex;
			return NULL;
		}
		//! return true if iterator comes before other
		inline bool operator<(HashTableIterator <HashType,HashValue,Hasher,EqualityComparer>
				      const& other) const {
			return _currentIndex < other._currentIndex;
		}
		//! return true if iterator != other
		inline bool operator!=(HashTableIterator <HashType,HashValue,Hasher,EqualityComparer> const& other) const {
			return _currentIndex != other._currentIndex;
		}
		//! return true if iterator comes after other
		inline bool operator>(HashTableIterator <HashType,HashValue,Hasher,EqualityComparer> const& other) const{
			return _currentIndex > other._currentIndex;
		}
		//! return true if iterator == other
		inline bool operator==(HashTableIterator <HashType,HashValue,Hasher,EqualityComparer> const& other) const{
			return _currentIndex == other._currentIndex;
		}

	private:
		const hash_map<HashType,HashValue,Hasher,EqualityComparer> * _table;
		unsigned int _currentIndex;
	};

	/*! hash_map implements a generic hash table class called
	  "hash_map".  The hash_map class is based upon the hash_map
	  class found in the SGI implementation of the Standard
	  Template Library.  For some incomprehensible reason the
	  hash_map class is not part of the STL standard so some
	  compilers (such as the Microsoft compiler) don't implement
	  hash_map, additionally the g++ team moved it to a very
	  illegitimate namespace starting with g++ version 3.0.
	  Basically, the hash_map class was designed to have the same
	  interface as the hash_map class from SGI STL, but there are
	  currently a few slight differences which I eventually be
	  fixed.
	*/
	template <class HashType, class HashValue, class Hasher, class EqualityComparer >
	class hash_map {
		friend class HashTableIterator <HashType,HashValue,Hasher,EqualityComparer>;
	public:
		//! how the key->value associations are held in our hash.
		typedef std::pair<const HashType, HashValue> pair_type;
                //! our iterator
		typedef HashTableIterator <HashType,HashValue,Hasher,EqualityComparer> iterator;
                //! our const_iterator
		typedef HashTableIterator <HashType,HashValue,Hasher,EqualityComparer> const_iterator;
                
		//! our constructor
		/*!
		  @param table_size the initial size of the
		  hash table.  The second argument to the constructor is an instance
		  of the IntHasher class which will provide the necessary hashing
		  services.  The initial size will be rounded up to the nearest power
		  of 2.  Therefore we could have equivalently put 11, 13, or 16 and
		  the size would still be 16.  You don't really have to worry about
		  the size because the my_hash_map classes will automatically grow
		  itself when necessary.  Thus even if you start with size 16, and
		  then put in 50 items everything will be fine.
                 @param resize_ratio  Whenver the ratio of items in
		 the table to the table size exceeds the resize ratio, the hash
		 table expands.  The default resize ratio is .5 which means that the
		 table expands once it becomes over half full.  You probably
		 shouldn't mess with the resize ratio unless you understand how this
		 affects hash table and search times.  See the
		 _Introduction_To_Algorithms_ book by Cormen, Leisserson, and Rivest
		 for such a discussion.
		 */
		hash_map(unsigned int table_size=16,float resize_ratio = .5 );

		~hash_map();
		//! Returns a reference to the object that is associated with a particular key.
		//! Be warned that if the hash_map does not already contain such an object,
		//! operator[] inserts the default object data_type().
		HashValue & operator[](HashType const &);
                //! insert a key->value pair into the hash
		pair_type * insert(HashType const &,HashValue const &);
                //! erase the element who's key is k
		bool erase(HashType const &k);
		//! erase the element pointed to by it
		bool erase( iterator& it );
		//! erase the range between begin and end
		bool erase( iterator& begin,const iterator& end );
		//! attempt to find an element who's key is k.  On failure, the iterator returned will equal hash_map::end()
		iterator find(HashType const &k) const;
		
		iterator InsertWithoutDuplication(HashType const &, HashValue const &,int*);
		//! increase the number of buckets to at least n
		void resize(unsigned int n);
		//! stream hash out in a 'hopefully' neat layout
		std::ostream & operator<<(std::ostream &) const;
		//! erase all elements from the hash
		void clear();
		//! return an iterator to the begining of the hash_map
		inline const iterator begin() const { 
			// Note that we can't just return an iterator pointing at tableData[0]
			// because tableData[0] might not be a VALID_CELL.  So if the table
			// is empty we want begin() == end().
			if (currentLength == 0)
				return _end;
			else
				return _begin; 
		}
		//! return an iterator pointing to the end of the hash_map
		inline const iterator end() const { 
			return _end; 
		}
		//! true if the size of the hash_map == 0
		inline int empty() const {return currentLength == 0; }
		//! return largest possible size of the hash_map
		inline unsigned int max_size() const { return maxLength; }
		//! return the size of the hash_map
		inline unsigned int size() const { return currentLength; }
		//! return resize_ratio
		inline float get_resize_ratio() const { return resizeRatio; }
		//! set resize_ratio
		inline void set_resize_ratio(float a) { resizeRatio=a; maxLengthTIMESresizeRatio = (int)(maxLength*a);}
	private:
		void AssignToCell(const int ,pair_type* );
		inline void IncrementNumDeletedCells() {
			if (++numDeletedCells >= maxLengthTIMESresizeRatio) resize(maxLength);
		}

		inline void DecrementNumDeletedCells() {
			numDeletedCells--;
			Hash_Assert( (numDeletedCells >= 0), "numDeletedCells went below 0" );
		}

		float resizeRatio;
		unsigned int maxLength;
		unsigned int currentLength;
		unsigned int numDeletedCells;
		unsigned int maxLengthTIMESresizeRatio;
		pair_type** tableData;
		char * tableStatus;
		iterator _begin;
		iterator _end;
		Hasher	_hashFunctor;
		EqualityComparer _equalityFunctor;
	};


	// 
	// This stuff would normally go in a .cc file but since it is a
	// template and everything needs to see the definitions we include
	// it in the .H file
	//
	// ***************************************************************

	static inline unsigned int RoundUpToPowerOfTwo(unsigned int x) {
		unsigned int returnValue = 1;
		for (; x > returnValue; returnValue*=2) ; // ';' is body of for loop
		return returnValue;
	}

	/****** start hash_map functions ********/

	// tableSize must be a power of 2 and SecondHashValue() must return an
	// odd positve number.  This is to insure that the tableSize and the
	// SecondHashValue() are relatively prime.  Otherwise the entire hash
	// table might not be searched in Insert, Delete or find (unless you
	// know that SecondHashValue() will always return something relatively
	// prime to tableSize.
	template <class HashType, class HashValue, class Hasher, class EqualityComparer>	
	hash_map<HashType,HashValue,Hasher,EqualityComparer>::hash_map
	(unsigned int table_size, float resize_ratio)
		: resizeRatio( resize_ratio ),
		  maxLength( RoundUpToPowerOfTwo( table_size) ),
		  currentLength(0),
		  numDeletedCells(0),
		  maxLengthTIMESresizeRatio( (int)(maxLength*resizeRatio) ),
		  tableData( new pair_type*[maxLength] ),
		  tableStatus( new char[maxLength] ),
		  _begin( HashTableIterator <HashType,HashValue,Hasher,EqualityComparer> (*this,0)),
		  _end(  HashTableIterator <HashType,HashValue,Hasher,EqualityComparer> (*this,maxLength))
	{
		for (unsigned int i = 0; i < maxLength; i++) 
			tableStatus[i] = EMPTY_CELL;
	}

	template <class HashType, class HashValue, class Hasher, class EqualityComparer>
	void hash_map <HashType,HashValue,Hasher,EqualityComparer>::AssignToCell
	(const int hashLocation, pair_type* newPair) {
		tableStatus[hashLocation] = VALID_CELL;
		tableData[hashLocation] = newPair;
		currentLength++;
	}

	template <class HashType, class HashValue, class Hasher, class EqualityComparer>
	HashValue & hash_map <HashType,HashValue,Hasher,EqualityComparer>::operator[]
	(HashType const & key) {
		int didInsert;
		iterator i=InsertWithoutDuplication(key,HashValue(),&didInsert);
		return i->second;
	}

	//  If currentLength * resizeRatio >= maxLength do resize(maxLength *
	//  resizeRatio).  Otherwise key,value is inserted in the hash table
	//  even if it already exists in the table.  Also a pointer to the
	//  pair_type that is inserted into the hash table is returned.  This
	//  feature is explicitly used in the implementation of
	//  operator[]. For inserting only unique elements use
	//  InsertWithoutDuplication.  

	template <class HashType, class HashValue, class Hasher, class EqualityComparer>
	typename hash_map<HashType,HashValue,Hasher,EqualityComparer>::pair_type *
	hash_map<HashType,HashValue,Hasher,EqualityComparer>::
	insert(HashType const & key, HashValue const & value) 
	{
		int didInsert;
		iterator i = InsertWithoutDuplication(key,value,&didInsert);
		if (didInsert) {
		    return i.GetCurrentItem();
		} else {
			delete tableData[i._currentIndex];
			pair_type*const newPair = new pair_type(key,value);
			tableData[i._currentIndex] = newPair;
			return newPair;
		}
	}

	/*!
	  If an item matching key is already in the table, then an iterator
	  point to that item is returned and the table is not modified and
	  *didInsert is set to 0.  Otherwise an iterator pointing to the
	  newly inserted item is returned and *didInsert is set to 1.
	*/
	template <class HashType, class HashValue, class Hasher, class EqualityComparer> typename hash_map  <HashType,HashValue,Hasher,EqualityComparer>::iterator 
	hash_map <HashType,HashValue,Hasher,EqualityComparer>::InsertWithoutDuplication
	(HashType const & key, HashValue const & value, int* didInsert) 
	{
		int firstDeletedLocation = -1;
		*didInsert = 0;
		if (currentLength >= maxLengthTIMESresizeRatio)
			resize((int)(maxLength/resizeRatio));
		int hashIncrement;
		int hashLocation = _hashFunctor.operator()(key)%maxLength;
		unsigned int timesInLoop = 0; 
		Hash_Assert( (hashLocation >= 0) ,
			     "An instance of HashType returned a negative value");
		switch(tableStatus[hashLocation]) {
		case EMPTY_CELL:
			{
				AssignToCell(hashLocation,new pair_type(key,value));
				*didInsert = 1;
				return iterator(*this,hashLocation);
			}
			break;
		case DELETED_CELL:
			{
				firstDeletedLocation = hashLocation;
			}
			break;
		case VALID_CELL:
			{
				if (_equalityFunctor(tableData[hashLocation]->first,key))
					return iterator(*this,hashLocation);
			}
			break;
		default:
			{
				ExitProgramMacro("Wrong Status in hash_map::insert(...)");
			}
			break;
		}

		hashIncrement = _hashFunctor.SecondHashValue(key) ;
		Hash_Assert( ( hashIncrement % 2) != 0, 
			     "Even value returned by SecondHashValue()");

		for(;;) {
			hashLocation = (hashLocation + hashIncrement)%maxLength;
   
			switch(tableStatus[hashLocation]) {
			case EMPTY_CELL:
				{
					if (firstDeletedLocation != -1)  
						hashLocation = firstDeletedLocation;
					AssignToCell(hashLocation,new pair_type(key,value));
					*didInsert = 1;
					return iterator(*this,hashLocation);
				}
				break;
			case DELETED_CELL:
				{
					if (firstDeletedLocation == -1) 
						firstDeletedLocation = hashLocation;
				}
				break;
			case VALID_CELL:
				{
					if (_equalityFunctor(tableData[hashLocation]->first, key))
						return iterator(*this,hashLocation);
				}
				break;
			default:
				{
					ExitProgramMacro("Wrong Status in hash_map::Insert");
				}
				break;
			}
			if (++timesInLoop > maxLength) {
				// We searched the entire table and didn't find a matching key or
				// an empty cell.  This means that we are indeed inserting without
				// duplication.  It just happened that lots of stuff was already
				// in the hash table but got deleted.
				Hash_Assert( (firstDeletedLocation != -1),
					     "insert: searched entire table without good reason");
				DecrementNumDeletedCells();
				AssignToCell(hashLocation,new pair_type(key,value));
				*didInsert = 1;
				return iterator(*this,hashLocation);
			}
		}
	}

	//  searches for searchInfo in the table and returns an iterator
	//  pointing to the match if possible and end() otherwise.
	template <class HashType, class HashValue, class Hasher, class EqualityComparer>
	typename hash_map<HashType,HashValue,Hasher,EqualityComparer>::iterator 
	hash_map<HashType,HashValue,Hasher,EqualityComparer>::
	find(HashType const & key) const
	{
		int hashIncrement;
		int hashLocation = _hashFunctor.operator()(key)%maxLength;
#ifndef NDEBUG
		unsigned int timesInLoop = 0; // used for checking assertions
#endif
		switch(tableStatus[hashLocation]) {
		case EMPTY_CELL:
			{
				return end();
			}
			break;
		case VALID_CELL:
			{
				if (_equalityFunctor(tableData[hashLocation]->first,key))
					return iterator(*this,hashLocation);
			}
			break;
		}
		hashIncrement = _hashFunctor.SecondHashValue(key);
		Hash_Assert( (hashIncrement % 2) != 0 ,
			     "even value returned by SecondHashValue()");
		for(;;) {
			hashLocation = ( hashLocation + hashIncrement ) % maxLength;
			switch(tableStatus[hashLocation]) {
			case EMPTY_CELL:
				{
					return end();
				}
				break;
			case VALID_CELL:
				{
					if (_equalityFunctor(tableData[hashLocation]->first,key))
						return iterator(*this,hashLocation);
				}
				break;
			}
			Hash_Assert((++timesInLoop < maxLength) ,
				    "searched entire hash table and still going in find(...)");
		}
	}

	template <class HashType, class HashValue, class Hasher, class EqualityComparer>
	bool hash_map<HashType,HashValue,Hasher,EqualityComparer>::
	erase(  iterator& it ) {
		HashType ht = (*it).first;
		return this->erase( ht );
	}

	template <class HashType, class HashValue, class Hasher, class EqualityComparer>
	bool hash_map<HashType,HashValue,Hasher,EqualityComparer>::
	erase(  iterator& begin,const iterator& end ){
		bool ret_val = true;
		for ( iterator it=begin; it != end; ++it ){
			if ( ! this->erase( it->first ) ) {
				ret_val = false;
			}
		}
		return ret_val;
	}

	//  Removes deleteInfo from the hash table if it exists and does
	//  nothing if the item is not in the hash table.  Returns true if the
	//  item to be deleted was found in the table.

	template <class HashType, class HashValue, class Hasher, class EqualityComparer>
	bool hash_map<HashType,HashValue,Hasher,EqualityComparer>::
	erase(HashType const & key) 
	{
		int hashIncrement;
		int hashLocation = _hashFunctor.operator()(key)%maxLength;
#ifndef NDEBUG
		unsigned int timesInLoop = 0; // used for checking assertions
#endif
		switch(tableStatus[hashLocation]) {
		case EMPTY_CELL:
			{
				return false;
			}
			break;
		case VALID_CELL:
			{
				if (_equalityFunctor(tableData[hashLocation]->first,key)) {
					tableStatus[hashLocation] = DELETED_CELL;
					delete tableData[hashLocation];
					currentLength--;
					IncrementNumDeletedCells();
					return true;
				}
			}
			break;
		}
		hashIncrement = _hashFunctor.SecondHashValue(key);
		Hash_Assert( (hashIncrement > 0),
			     "negative value returned by SecondHashValue()");
		Hash_Assert( (hashIncrement % 2) != 0 ,
			     "even value returned by SecondHashValue()");
		for(;;) {
			hashLocation = ( hashLocation + hashIncrement ) % maxLength;
			switch(tableStatus[hashLocation]) {
			case EMPTY_CELL :
				{
					return false;
				}
				break;
			case VALID_CELL :
				{
					if (_equalityFunctor(tableData[hashLocation]->first, key)) {
						tableStatus[hashLocation] = DELETED_CELL;
						delete tableData[hashLocation];
						currentLength--;
						IncrementNumDeletedCells();
						return true;
					}
				}
				break;
			case DELETED_CELL:
				{
				}
				break;
			default :
				{
					ExitProgramMacro("Wrong type in hash_map::erase");
				}
				break;
			}
			Hash_Assert((++timesInLoop < maxLength) ,
				    "searched entire hash table and still going in erase(...)");
		}
	}

	template <class HashType, class HashValue, class Hasher, class EqualityComparer>
	std::ostream & hash_map <HashType,HashValue,Hasher,EqualityComparer>::operator<<
		(std::ostream &s) const
	{
		int k = 0;
		while ( k < maxLength) {
			s << "Location " << k << ": ";
			switch(tableStatus[k]) {
			case EMPTY_CELL: 
				{
				    s << "EMPTY_CELL" << std::endl;
				} 
				break;
			case DELETED_CELL : 
				{
				    s << "DELETED_CELL" << std::endl;
				}
				break;
			case VALID_CELL :
				{
				    s << "VALID_CELL : " << std::endl;
				    s << (*tableData[k]);
				}
				break;
			default :
				{
				    s << "unknown type of cell : " << tableStatus[k] << std::endl;
				}
				break;
			}
			k++;
		}
		return s;
	}

	template <class HashType, class HashValue, class Hasher, class EqualityComparer>
	hash_map <HashType,HashValue,Hasher,EqualityComparer>::~hash_map() 
	{
		for (unsigned int i = 0; i < maxLength; i++)
			if (tableStatus[i] == VALID_CELL)
				delete tableData[i];
  
		delete [] tableData;
		delete [] tableStatus;
	}

	template <class HashType, class HashValue, class Hasher, class EqualityComparer>
	void hash_map <HashType,HashValue,Hasher,EqualityComparer>::
	resize(unsigned int newMaxSize) 
	{
		newMaxSize = RoundUpToPowerOfTwo(newMaxSize);
		Hash_Assert((newMaxSize >= currentLength),
			    "resize called with newMaxSize < currentLength !");
#ifdef WARN_WHEN_RESIZING
		cerr << "Warning: resize(" << newMaxSize <<") called when "<<endl;
		cerr << "resizeRatio = " << resizeRatio << endl;
		cerr << "currentLength = " << currentLength <<endl;
		cerr << "maxLength = " <<  maxLength << endl;
		cerr << "maxLengthTIMESresizeRatio = " << maxLengthTIMESresizeRatio << endl;
		if (newMaxSize == maxLength) {
			cerr << "This resize is really doing defragmentation not resizing." <<endl;
		}
#endif
		pair_type** oldTableData = tableData;
		char * oldTableStatus = tableStatus;
		int oldMaxSize = maxLength;
		maxLength = newMaxSize;
		_end._currentIndex = maxLength;
		tableData = new pair_type*[maxLength];
		tableStatus = new char[maxLength];
		numDeletedCells = 0;
		for (unsigned int i = 0; i < maxLength; i++) 
			tableStatus[i] = EMPTY_CELL;

		currentLength = 0;
		for (int k = 0; k < oldMaxSize ; k++) 
			if (VALID_CELL == oldTableStatus[k]) {
				insert(oldTableData[k]->first,oldTableData[k]->second);
				delete oldTableData[k];
			}
		delete [] oldTableData;
		delete [] oldTableStatus;
		maxLengthTIMESresizeRatio = ((unsigned int) (maxLength*resizeRatio));
	}

	template <class HashType, class HashValue, class Hasher, class EqualityComparer>
	void hash_map <HashType,HashValue,Hasher,EqualityComparer>::clear()
	{
		for (unsigned int i = 0; i < maxLength; i++) {
			if (tableStatus[i] == VALID_CELL)
				delete tableData[i];
			tableStatus[i] = EMPTY_CELL;
		}
		currentLength = 0;
		numDeletedCells = 0;
	}


} // namespace detail
#endif



