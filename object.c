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
#ifndef HAVE_INDEXED_OBJECT
#define HAVE_INDEXED_OBJECT

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "zend_interfaces.h"
#include "ext/spl/spl_iterators.h"

#include "php_indexed_object.h"
#include "php_indexed_iterator.h"

static zend_object_handlers php_indexed_handlers;
extern zend_function_entry Indexed_methods[];

#define PHP_INDEXED_CHECK(pi, i)	do {\
	if ((i) >= (pi)->size) {\
		zend_throw_exception_ex(NULL, 0, "index %ld is OOB", (i)); \
		return; \
	} \
	if ((i) < 0) { \
		if ((pi)->size + (i) < 0) { \
			zend_throw_exception_ex(NULL, 0, "index %ld (%d) is OOB", (i), (pi)->size + (i)); \
			return; \
		} \
		(i) = (pi)->size + (i);\
	} \
} while(0)

/* {{{ */
zend_object* php_indexed_create(zend_class_entry *ce) {
	php_indexed_t *pi = 
		(php_indexed_t*) ecalloc(1, sizeof(php_indexed_t));

	zend_object_std_init(&pi->std, ce);

	pi->std.handlers = &php_indexed_handlers;	

	return &pi->std;
} /* }}} */

/* {{{ */
static void php_indexed_set_data(zval *indexed, HashTable *data) {
	php_indexed_t *pi = PHP_INDEXED_FETCH(indexed);
	zval      *item;
	zval      *items;

	if (!data)
		return;

	if (pi->size < zend_hash_num_elements(data))
		php_indexed_resize(indexed, zend_hash_num_elements(data));

	items = pi->data;

	ZEND_HASH_FOREACH_VAL(data, item) {
		ZVAL_COPY(items++, item);
	} ZEND_HASH_FOREACH_END();
} /* }}} */

/* {{{ */
void php_indexed_construct(zval *indexed, zend_long size, HashTable *data) {
	php_indexed_t *pi = PHP_INDEXED_FETCH(indexed);

	pi->size = size;
	pi->data = (zval*) ecalloc(sizeof(zval), pi->size);

	if (data)
		php_indexed_set_data(indexed, data);	
} /* }}} */

/* {{{ */
zend_long php_indexed_count(zval *indexed) {
	return PHP_INDEXED_FETCH(indexed)->size;
} /* }}} */

/* {{{ */
void php_indexed_set(zval *indexed, zend_long index, zval *value) {
	php_indexed_t *pi = PHP_INDEXED_FETCH(indexed);

	PHP_INDEXED_CHECK(pi, index);

	if (Z_TYPE(pi->data[index]) != IS_UNDEF) {
		zval_ptr_dtor(&pi->data[index]);
	}

	ZVAL_COPY(&pi->data[index], value);
} /* }}} */

/* {{{ */
void php_indexed_get(zval *indexed, zend_long index, zval *return_value) {
	php_indexed_t *pi = PHP_INDEXED_FETCH(indexed);

	PHP_INDEXED_CHECK(pi, index);

	if (Z_TYPE(pi->data[index]) != IS_UNDEF)
		ZVAL_COPY(return_value, &pi->data[index]);
} /* }}} */

/* {{{ */
void php_indexed_unset(zval *indexed, zend_long index) {
	php_indexed_t *pi = PHP_INDEXED_FETCH(indexed);

	PHP_INDEXED_CHECK(pi, index);
	
	if (Z_TYPE(pi->data[index]) != IS_UNDEF) {
		zval_ptr_dtor(&pi->data[index]);
		ZVAL_UNDEF(&pi->data[index]);
	}
} /* }}} */

/* {{{ */
void php_indexed_exists(zval *indexed, zend_long index, zval *return_value) {
	php_indexed_t *pi = PHP_INDEXED_FETCH(indexed);

	PHP_INDEXED_CHECK(pi, index);

	RETURN_BOOL(Z_TYPE(pi->data[index]) != IS_UNDEF);
} /* }}} */

/* {{{ */
static void php_indexed_free(zend_object *o) {
	php_indexed_t *pi = PHP_INDEXED_FETCH_FROM(o);
	
	if (pi->data) {
		zend_long it = 0, end = pi->size;

		for (; it < end; it++)
			zval_ptr_dtor(&pi->data[it]);
		efree(pi->data);
	}

	zend_object_std_dtor(o);
} /* }}} */

/* {{{ */
static HashTable* php_indexed_gc(zval *indexed, zval **table, int *n) {
	php_indexed_t *pi = PHP_INDEXED_FETCH(indexed);

	*table = pi->data;
	*n = pi->size;

	return NULL;
} /* }}} */

/* {{{ */
static HashTable* php_indexed_dump(zval *indexed, int *is_temp) {
	php_indexed_t *pi = PHP_INDEXED_FETCH(indexed);
	HashTable *ht;
	zend_long it;

	ALLOC_HASHTABLE(ht);
	zend_hash_init(ht, pi->size, NULL, ZVAL_PTR_DTOR, 0);
	*is_temp = 1;

	for (it = 0; it < pi->size; it++) {
		if (Z_TYPE(pi->data[it]) != IS_UNDEF && 
		    zend_hash_index_update(ht, it, &pi->data[it])) {
			Z_TRY_ADDREF(pi->data[it]);
		}
	}

	return ht;
} /* }}} */

/* {{{ */
static zend_object* php_indexed_clone(zval *object) {
	php_indexed_t *pi = PHP_INDEXED_FETCH(object);
	php_indexed_t *cl = (php_indexed_t*) ecalloc(1, sizeof(php_indexed_t));
	zend_long it;

	zend_object_std_init(&cl->std, pi->std.ce);

	cl->std.handlers = &php_indexed_handlers;	

	cl->data = (zval*) ecalloc(pi->size, sizeof(zval));

	for (it = 0; it < pi->size; it++)
		ZVAL_COPY(&cl->data[it], &pi->data[it]);

	cl->size = pi->size;

	return &cl->std;
} /* }}} */

/* {{{ */
static int php_indexed_cast(zval *indexed, zval *retval, int type) {
	php_indexed_t *pi = PHP_INDEXED_FETCH(indexed);
	zend_long it;

	if (type != IS_ARRAY) {
		return FAILURE;
	}

	array_init(retval);

	for (it = 0; it < pi->size; it++) {
		if (Z_TYPE(pi->data[it]) != IS_UNDEF && 
		    add_next_index_zval(retval, &pi->data[it])) {
			Z_TRY_ADDREF(pi->data[it]);
		}
	}

	return SUCCESS;
} /* }}} */

/* {{{ */
void php_indexed_resize(zval *indexed, zend_long resize) {
	php_indexed_t *pi = PHP_INDEXED_FETCH(indexed);

	while (resize < pi->size) {
		if (Z_TYPE(pi->data[pi->size-1]) != IS_UNDEF)
			zval_ptr_dtor(&pi->data[pi->size-1]);
		pi->size--;
	}

	pi->data = erealloc(pi->data, sizeof(zval) * resize);

	while (pi->size < resize)
		ZVAL_UNDEF(&pi->data[pi->size++]);
} /* }}} */

/* {{{ */
void php_indexed_flip(zval *indexed, zval *retval) {
	php_indexed_t *pi = PHP_INDEXED_FETCH(indexed), 
		      *pf;
	zend_long it;

	object_init_ex(retval, pi->std.ce);

	pf = PHP_INDEXED_FETCH(retval);
	pf->size = pi->size;
	pf->data = (zval*) ecalloc(pf->size, sizeof(zval));

	for (it = pf->size; it > 0; it--)
		ZVAL_COPY(&pf->data[(pf->size) - it], &pi->data[it - 1]);
} /* }}} */

/* {{{ */
#define PHP_INDEXED_NO_PROPERTIES() do { \
	zend_throw_exception_ex(NULL, 0, \
		"properties on Indexed objects are not allowed"); \
} while(0)

static zval* php_indexed_property_read(zval *object, zval *member, int type, void **cache_slot, zval *rv) {
	PHP_INDEXED_NO_PROPERTIES();
	return &EG(uninitialized_zval);
}

static void php_indexed_property_write(zval *object, zval *member, zval *value, void **cache_slot) {
	PHP_INDEXED_NO_PROPERTIES();
}

static int php_indexed_property_exists(zval *object, zval *member, int has_set_exists, void **cache_slot) {
	PHP_INDEXED_NO_PROPERTIES();
	return 0;
}

static void php_indexed_property_unset(zval *object, zval *member, void **cache_slot) {
	PHP_INDEXED_NO_PROPERTIES();
} /* }}} */

/* {{{ */
void php_indexed_init(void) {
	zend_class_entry ce;
	zend_object_handlers *zh;
	
	INIT_CLASS_ENTRY(ce, "Indexed", Indexed_methods);
	Indexed_ce = zend_register_internal_class(&ce);
	Indexed_ce->create_object = php_indexed_create;
	Indexed_ce->get_iterator = php_indexed_iterator;
	Indexed_ce->ce_flags |= ZEND_ACC_FINAL;

	zend_class_implements(
		Indexed_ce, 2,
		spl_ce_ArrayAccess, spl_ce_Countable);

	zh = zend_get_std_object_handlers();	

	memcpy(&php_indexed_handlers, zh, sizeof(zend_object_handlers));
	
	php_indexed_handlers.free_obj = php_indexed_free;
	php_indexed_handlers.get_gc   = php_indexed_gc;
	php_indexed_handlers.get_debug_info = php_indexed_dump;
	php_indexed_handlers.clone_obj = php_indexed_clone;
	php_indexed_handlers.cast_object = php_indexed_cast;

	php_indexed_handlers.read_property = php_indexed_property_read;
	php_indexed_handlers.write_property = php_indexed_property_write;
	php_indexed_handlers.has_property = php_indexed_property_exists;
	php_indexed_handlers.unset_property = php_indexed_property_unset;

	php_indexed_handlers.get_properties = NULL;

	php_indexed_handlers.offset = XtOffsetOf(php_indexed_t, std);
} /* }}} */
#endif

