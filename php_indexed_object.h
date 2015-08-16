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
static inline zend_object* php_indexed_create(zend_class_entry *ce) {
	php_indexed_t *pi = 
		(php_indexed_t*) ecalloc(1, sizeof(php_indexed_t));

	zend_object_std_init(&pi->std, ce);

	pi->std.handlers = &php_indexed_handlers;	

	return &pi->std;
} /* }}} */

/* {{{ */
static inline void php_indexed_free(zend_object *o) {
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
static inline HashTable* php_indexed_gc(zval *indexed, zval **table, int *n) {
	php_indexed_t *pi = PHP_INDEXED_FETCH(indexed);

	*table = pi->data;
	*n = pi->size;

	return pi->std.properties;
} /* }}} */

/* {{{ */
static inline HashTable* php_indexed_dump(zval *indexed, int *is_temp) {
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
static inline zend_object* php_indexed_clone(zval *object) {
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
static inline int php_indexed_cast(zval *indexed, zval *retval, int type) {
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
#define PHP_INDEXED_NO_PROPERTIES() do { \
	zend_throw_exception_ex(NULL, 0, \
		"properties on Indexed objects are not allowed"); \
} while(0)

static inline zval* php_indexed_read(zval *object, zval *member, int type, void **cache_slot, zval *rv) {
	PHP_INDEXED_NO_PROPERTIES();
	return &EG(uninitialized_zval);
}

static inline void php_indexed_write(zval *object, zval *member, zval *value, void **cache_slot) {
	PHP_INDEXED_NO_PROPERTIES();
}

static inline int php_indexed_exists(zval *object, zval *member, int has_set_exists, void **cache_slot) {
	PHP_INDEXED_NO_PROPERTIES();
	return 0;
}

static inline void php_indexed_unset(zval *object, zval *member, void **cache_slot) {
	PHP_INDEXED_NO_PROPERTIES();
} /* }}} */

/* {{{ */
static inline void php_indexed_resize(php_indexed_t *pi, zend_long resize) {
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
static inline void php_indexed_flip(zval *indexed, zval *retval) {
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
static inline void php_indexed_set_data(php_indexed_t *pi, HashTable *data) {
	zval      *item;
	zval      *items;

	if (!data)
		return;	

	if (pi->size < zend_hash_num_elements(data))
		php_indexed_resize(pi, zend_hash_num_elements(data));

	items = pi->data;

	ZEND_HASH_FOREACH_VAL(data, item) {
		ZVAL_COPY(items++, item);
	} ZEND_HASH_FOREACH_END();
} /* }}} */
#endif
