/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2015 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: krakjoe                                                      |
  +----------------------------------------------------------------------+
*/

/* $Id$ */
#ifndef HAVE_INDEXED_OBJECT_H
#define HAVE_INDEXED_OBJECT_H
/* {{{ */
typedef struct _php_indexed_t {
	zval        *data;
	zend_long    size;
	zend_object  std;
} php_indexed_t; 

#define PHP_INDEXED_FETCH_FROM(o)	((php_indexed_t*) (((char*)o) - XtOffsetOf(php_indexed_t, std)))
#define PHP_INDEXED_FETCH(z)		PHP_INDEXED_FETCH_FROM(Z_OBJ_P(z)) /* }}} */

/* {{{ */
zend_class_entry *Indexed_ce; /* }}} */

/* {{{ */
void php_indexed_init(void); /*}}} */

/* {{{ */
void php_indexed_construct(zval *indexed, zend_long size, HashTable *data);
zend_long php_indexed_count(zval *indexed);
void php_indexed_set(zval *indexed, zend_long index, zval *value);
void php_indexed_get(zval *indexed, zend_long index, zval *value);
void php_indexed_unset(zval *indexed, zend_long index);
void php_indexed_exists(zval *indexed, zend_long index, zval *return_value);
void php_indexed_resize(zval *indexed, zend_long resize);
void php_indexed_flip(zval *indexed, zval *retval); /* }}} */
#endif

