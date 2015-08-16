Indexed
------
*for educational purposes ONLY ...*

[![Build Status](https://travis-ci.org/krakjoe/indexed.svg)](https://travis-ci.org/krakjoe/indexed)

This extension implements the following abstract:

	class Indexed implements ArrayAccess, Countable {
		public function __construct(int $size, array $data = []);

		public function offsetGet(int $index);
		public function offsetSet(int $index, $value);
		public function offsetExists(int $index) : bool;
		public function offsetUnset(int $index);

		public function count() : int;

		public function resize(int $size);
	}

This touches upon all of the following Zend subjects:

 - custom zend objects
 - implementing internal interfaces
 - paramters, optional parameters, return types
 - casting objects
 - interfacing with garbage collector
 - dealing with reference counting/copying
 - implementing cloning
 - testing
