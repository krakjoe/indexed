config.m4
---------

 - check config options and declare extension with multiple source files

php_indexed.h
-------------

 - extern the module entry and define pointer
 - include TSRM
 - define TSRMLS cache extern

indexed.c
---------

 - module code: arginfo and method pairs
 - module entry
 - module functions

php_indexed_object.h
--------------------

 - typedef custom object
 - define fetching macros
 - declare zend class entry
 - initialization routine
 - api routines

object.c
--------

 - declare static handlers, extern methods (indexed.c), extern get_iterator (iterator.c)
 - define bounds checking/correcting macro
 - implement API routines
 - implement static handlers
   - get_gc
   - get_debug_info
   - clone_object
   - cast_object
   - read/write/isset/unset properties (dummies)
 - implement initialization routine, uses static handlers, wires everything together in register class entry

iterator.c
----------

 - include object header for typedef/fetch
 - static iterator funcs
 - declare get_iterator externed by object.c
