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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_indexed.h"

#include "zend_interfaces.h"
#include "ext/spl/spl_iterators.h"

zend_class_entry *Indexed_ce;

typedef struct _php_indexed_t {
	zval        *data;
	zend_long    size;
	zend_object  std;
} php_indexed_t;

zend_object_handlers php_indexed_handlers;

#define PHP_INDEXED_FETCH_FROM(o)	((php_indexed_t*) (((char*)o) - XtOffsetOf(php_indexed_t, std)))
#define PHP_INDEXED_FETCH(z)		PHP_INDEXED_FETCH_FROM(Z_OBJ_P(z))

#include "php_indexed_iterator.h"

#define PHP_INDEXED_CHECK(pl, i)	do {\
	if ((i) >= (pl)->size) {\
		zend_throw_exception_ex(NULL, 0, "index %ld is OOB", (i)); \
		return; \
	} \
	if ((i) < 0) { \
		if ((pl)->size + (i) < 0) { \
			zend_throw_exception_ex(NULL, 0, "index %ld (%d) is OOB", (i), (pl)->size + (i)); \
			return; \
		} \
		(i) = (pl)->size + (i);\
	} \
} while(0)

/* {{{ */
static inline zend_object* php_indexed_create(zend_class_entry *ce) {
	php_indexed_t *pl = 
		(php_indexed_t*) ecalloc(1, sizeof(php_indexed_t));

	zend_object_std_init(&pl->std, ce);

	pl->std.handlers = &php_indexed_handlers;	

	return &pl->std;
} /* }}} */

/* {{{ */
static inline void php_indexed_free(zend_object *o) {
	php_indexed_t *pl = PHP_INDEXED_FETCH_FROM(o);
	
	if (pl->data) {
		zend_long it = 0, end = pl->size;

		for (; it < end; it++)
			zval_ptr_dtor(&pl->data[it]);
		efree(pl->data);
	}

	zend_object_std_dtor(o);
} /* }}} */

/* {{{ */
static inline HashTable* php_indexed_gc(zval *indexed, zval **table, int *n) {
	php_indexed_t *pl = PHP_INDEXED_FETCH(indexed);

	*table = pl->data;
	*n = pl->size;

	return NULL;
} /* }}} */

/* {{{ */
static inline HashTable* php_indexed_dump(zval *indexed, int *is_temp) {
	php_indexed_t *pl = PHP_INDEXED_FETCH(indexed);
	HashTable *ht;
	zend_long it = 0;
	
	ALLOC_HASHTABLE(ht);
	zend_hash_init(ht, pl->size, NULL, ZVAL_PTR_DTOR, 0);
	*is_temp = 1;

	for (; it < pl->size; it++) {
		zend_hash_next_index_insert(ht, &pl->data[it]);
		Z_TRY_ADDREF(pl->data[it]);
	}
	
	return ht;
} /* }}} */

/* {{{ */
static inline zend_object* php_indexed_clone(zval *object) {
	php_indexed_t *pl = PHP_INDEXED_FETCH(object);
	php_indexed_t *cl = (php_indexed_t*) ecalloc(1, sizeof(php_indexed_t));
	zend_long it;

	zend_object_std_init(&cl->std, pl->std.ce);

	cl->std.handlers = &php_indexed_handlers;	

	cl->data = (zval*) ecalloc(pl->size, sizeof(zval));
	for (it = 0; it < pl->size; it++)
		ZVAL_COPY(&cl->data[it], &pl->data[it]);
	cl->size = pl->size;

	return &cl->std;
} /* }}} */

/* {{{ */
static inline int php_indexed_cast(zval *indexed, zval *retval, int type) {
	php_indexed_t *pl = PHP_INDEXED_FETCH(indexed);
	zend_long it;

	if (type != IS_ARRAY) {
		return FAILURE;
	}

	array_init(retval);

	for (it = 0; it < pl->size; it++) {
		if (add_next_index_zval(retval, &pl->data[it])) 
			Z_TRY_ADDREF(pl->data[it]);
	}

	return SUCCESS;
} /* }}} */

/* {{{ */
static inline void php_indexed_resize(php_indexed_t *pl, zend_long resize) {
	while (resize < pl->size) {
		if (Z_TYPE(pl->data[pl->size-1]) != IS_UNDEF)
			zval_ptr_dtor(&pl->data[pl->size-1]);
		pl->size--;
	}

	pl->data = erealloc(pl->data, sizeof(zval) * resize);

	while (pl->size < resize)
		ZVAL_UNDEF(&pl->data[pl->size++]);
} /* }}} */

/* {{{ */
static inline void php_indexed_set_data(php_indexed_t *pl, HashTable *data) {
	zval      *item;
	zval      *items;
	
	if (pl->size < zend_hash_num_elements(data))
		php_indexed_resize(pl, zend_hash_num_elements(data));

	items = pl->data;	

	ZEND_HASH_FOREACH_VAL(data, item) {
		ZVAL_COPY(items++, item);
	} ZEND_HASH_FOREACH_END();
} /* }}} */

/* {{{ */
ZEND_BEGIN_ARG_INFO_EX(Indexed_construct_arginfo, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, size, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, data, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

PHP_METHOD(Indexed, __construct)
{
	php_indexed_t *pl = PHP_INDEXED_FETCH(getThis());
	HashTable     *data = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l|H", &pl->size, &data) != SUCCESS) {
		return;
	}

	pl->data = (zval*) ecalloc(sizeof(zval), pl->size);

	if (data) {
		php_indexed_set_data(pl, data);
	}		
}
/* }}} */

/* {{{ */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Indexed_count_arginfo, 0, 0, IS_LONG, NULL, 1)
ZEND_END_ARG_INFO()

PHP_METHOD(Indexed, count) {
	php_indexed_t *pl = PHP_INDEXED_FETCH(getThis());

	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	RETURN_LONG(pl->size);
} /* }}} */

/* {{{ */
ZEND_BEGIN_ARG_INFO_EX(Indexed_set_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, index)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

PHP_METHOD(Indexed, offsetSet) {
	php_indexed_t *pl = PHP_INDEXED_FETCH(getThis());
	zend_long index;
	zval *value;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "lz", &index, &value) != SUCCESS) {
		return;
	}

	PHP_INDEXED_CHECK(pl, index);

	if (Z_TYPE(pl->data[index]) != IS_UNDEF) {
		zval_ptr_dtor(&pl->data[index]);
	}

	ZVAL_COPY(&pl->data[index], value);
} /* }}} */

/* {{{ */
ZEND_BEGIN_ARG_INFO_EX(Indexed_get_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()

PHP_METHOD(Indexed, offsetGet) 
{
	php_indexed_t *pl = PHP_INDEXED_FETCH(getThis());
	zend_long index;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &index) != SUCCESS) {
		return;
	}
	
	PHP_INDEXED_CHECK(pl, index);

	if (Z_TYPE(pl->data[index]) != IS_UNDEF)
		ZVAL_COPY(return_value, &pl->data[index]);
} /* }}} */

/* {{{ */
ZEND_BEGIN_ARG_INFO_EX(Indexed_unset_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()

PHP_METHOD(Indexed, offsetUnset) 
{
	php_indexed_t *pl = PHP_INDEXED_FETCH(getThis());
	zend_long index;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &index) != SUCCESS) {
		return;
	}
	
	PHP_INDEXED_CHECK(pl, index);
	
	if (Z_TYPE(pl->data[index]) != IS_UNDEF) {
		zval_ptr_dtor(&pl->data[index]);
		ZVAL_UNDEF(&pl->data[index]);
	}
} /* }}} */

/* {{{ */
ZEND_BEGIN_ARG_INFO_EX(Indexed_exists_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()

PHP_METHOD(Indexed, offsetExists) 
{
	php_indexed_t *pl = PHP_INDEXED_FETCH(getThis());
	zend_long index;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &index) != SUCCESS) {
		return;
	}
	
	PHP_INDEXED_CHECK(pl, index);

	RETURN_BOOL(Z_TYPE(pl->data[index]) != IS_UNDEF);
} /* }}} */

/* {{{ */
ZEND_BEGIN_ARG_INFO_EX(Indexed_resize_arginfo, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, size, IS_LONG, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(Indexed, resize)
{
	php_indexed_t *pl = PHP_INDEXED_FETCH(getThis());
	zend_long resize;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &resize) != SUCCESS) {
		return;
	}

	php_indexed_resize(pl, resize);
} /* }}} */

/* {{{ */
static zend_function_entry Indexed_methods[] = {
	PHP_ME(Indexed, __construct,  Indexed_construct_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Indexed, count,        Indexed_count_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Indexed, offsetSet,    Indexed_set_arginfo,	 ZEND_ACC_PUBLIC)
	PHP_ME(Indexed, offsetGet,    Indexed_get_arginfo,	 ZEND_ACC_PUBLIC)
	PHP_ME(Indexed, offsetUnset,  Indexed_unset_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Indexed, offsetExists, Indexed_exists_arginfo,    ZEND_ACC_PUBLIC)
	PHP_ME(Indexed, resize,	      Indexed_resize_arginfo,    ZEND_ACC_PUBLIC)
	PHP_FE_END
}; /* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(indexed)
{
	zend_class_entry ce;
	zend_object_handlers *zh;
	
	INIT_CLASS_ENTRY(ce, "Indexed", Indexed_methods);
	Indexed_ce = zend_register_internal_class(&ce);
	Indexed_ce->create_object = php_indexed_create;
	zend_class_implements(
		Indexed_ce, 2, 
		spl_ce_ArrayAccess, 
		spl_ce_Countable);
	Indexed_ce->get_iterator = php_indexed_iterator;

	zh = zend_get_std_object_handlers();

	memcpy(&php_indexed_handlers, zh, sizeof(zend_object_handlers));

	php_indexed_handlers.free_obj = php_indexed_free;
	php_indexed_handlers.get_gc   = php_indexed_gc;
	php_indexed_handlers.get_debug_info = php_indexed_dump;
	php_indexed_handlers.clone_obj = php_indexed_clone;
	php_indexed_handlers.cast_object = php_indexed_cast;
	php_indexed_handlers.get_properties = NULL;

	php_indexed_handlers.offset = XtOffsetOf(php_indexed_t, std);
	
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(indexed)
{
#if defined(COMPILE_DL_INDEXED) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(indexed)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "indexed support", "enabled");
	php_info_print_table_end();
}
/* }}} */

/* {{{ indexed_module_entry
 */
zend_module_entry indexed_module_entry = {
	STANDARD_MODULE_HEADER,
	"indexed",
	NULL,
	PHP_MINIT(indexed),
	NULL,
	PHP_RINIT(indexed),
	NULL,
	PHP_MINFO(indexed),
	PHP_INDEXED_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_INDEXED
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE();
#endif
ZEND_GET_MODULE(indexed)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
