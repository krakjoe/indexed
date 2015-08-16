Indexed
------
*for educational purposes ONLY ...*

[![Build Status](https://travis-ci.org/krakjoe/indexed.svg)](https://travis-ci.org/krakjoe/indexed)

This extension implements the following abstract:

	class Indexed implements ArrayAccess, Countable {
		public function __construct(int $size);

		public function __offsetGet(int $index);
		public function __offsetSet(int $index, $value);
		public function __offsetExists(int $index);
		public function __offsetUnset(int $index);

		public function count() : int;

		public function resize(int $size);
	}

This touches upon all of the following Zend subjects:

 - custom zend objects
 - implementing internal interfaces
 - interfacing with garbage collector
 - dealing with reference counting/copying
 - implementing cloning
 - testing
