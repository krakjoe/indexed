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
  | obtain it through the world-wide-web, piease send a note to          |
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

#include "php_indexed_object.h"
#include "php_indexed_iterator.h"

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
ZEND_BEGIN_ARG_INFO_EX(Indexed_construct_arginfo, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, size, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, data, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

PHP_METHOD(Indexed, __construct)
{
	php_indexed_t *pi = PHP_INDEXED_FETCH(getThis());
	HashTable     *data = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l|H", &pi->size, &data) != SUCCESS) {
		return;
	}

	pi->data = (zval*) ecalloc(sizeof(zval), pi->size);

	php_indexed_set_data(pi, data);	
}
/* }}} */

/* {{{ */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Indexed_count_arginfo, 0, 0, IS_LONG, NULL, 1)
ZEND_END_ARG_INFO()

PHP_METHOD(Indexed, count) {
	php_indexed_t *pi = PHP_INDEXED_FETCH(getThis());

	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	RETURN_LONG(pi->size);
} /* }}} */

/* {{{ */
ZEND_BEGIN_ARG_INFO_EX(Indexed_set_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, index)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

PHP_METHOD(Indexed, offsetSet) {
	php_indexed_t *pi = PHP_INDEXED_FETCH(getThis());
	zend_long index;
	zval *value;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "lz", &index, &value) != SUCCESS) {
		return;
	}

	PHP_INDEXED_CHECK(pi, index);

	if (Z_TYPE(pi->data[index]) != IS_UNDEF) {
		zval_ptr_dtor(&pi->data[index]);
	}

	ZVAL_COPY(&pi->data[index], value);
} /* }}} */

/* {{{ */
ZEND_BEGIN_ARG_INFO_EX(Indexed_get_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()

PHP_METHOD(Indexed, offsetGet) 
{
	php_indexed_t *pi = PHP_INDEXED_FETCH(getThis());
	zend_long index;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &index) != SUCCESS) {
		return;
	}
	
	PHP_INDEXED_CHECK(pi, index);

	if (Z_TYPE(pi->data[index]) != IS_UNDEF)
		ZVAL_COPY(return_value, &pi->data[index]);
} /* }}} */

/* {{{ */
ZEND_BEGIN_ARG_INFO_EX(Indexed_unset_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()

PHP_METHOD(Indexed, offsetUnset) 
{
	php_indexed_t *pi = PHP_INDEXED_FETCH(getThis());
	zend_long index;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &index) != SUCCESS) {
		return;
	}
	
	PHP_INDEXED_CHECK(pi, index);
	
	if (Z_TYPE(pi->data[index]) != IS_UNDEF) {
		zval_ptr_dtor(&pi->data[index]);
		ZVAL_UNDEF(&pi->data[index]);
	}
} /* }}} */

/* {{{ */
ZEND_BEGIN_ARG_INFO_EX(Indexed_exists_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()

PHP_METHOD(Indexed, offsetExists) 
{
	php_indexed_t *pi = PHP_INDEXED_FETCH(getThis());
	zend_long index;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &index) != SUCCESS) {
		return;
	}
	
	PHP_INDEXED_CHECK(pi, index);

	RETURN_BOOL(Z_TYPE(pi->data[index]) != IS_UNDEF);
} /* }}} */

/* {{{ */
ZEND_BEGIN_ARG_INFO_EX(Indexed_resize_arginfo, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, size, IS_LONG, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(Indexed, resize)
{
	php_indexed_t *pi = PHP_INDEXED_FETCH(getThis());
	zend_long resize;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &resize) != SUCCESS) {
		return;
	}

	php_indexed_resize(pi, resize);
} /* }}} */

/* {{{ */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Indexed_flip_arginfo, 0, 0, IS_OBJECT, "Indexed", 1)
ZEND_END_ARG_INFO()

PHP_METHOD(Indexed, flip)
{
	if (zend_parse_parameters_none() != SUCCESS) {
		return;	
	}

	php_indexed_flip(getThis(), return_value);
}
/* }}} */

/* {{{ */
static zend_function_entry Indexed_methods[] = {
	PHP_ME(Indexed, __construct,  Indexed_construct_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Indexed, count,        Indexed_count_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Indexed, offsetSet,    Indexed_set_arginfo,	 ZEND_ACC_PUBLIC)
	PHP_ME(Indexed, offsetGet,    Indexed_get_arginfo,	 ZEND_ACC_PUBLIC)
	PHP_ME(Indexed, offsetUnset,  Indexed_unset_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Indexed, offsetExists, Indexed_exists_arginfo,    ZEND_ACC_PUBLIC)
	PHP_ME(Indexed, resize,	      Indexed_resize_arginfo,    ZEND_ACC_PUBLIC)
	PHP_ME(Indexed, flip,	      Indexed_flip_arginfo,      ZEND_ACC_PUBLIC)
	PHP_FE_END
}; /* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(indexed)
{
	zend_class_entry ce;
	
	INIT_CLASS_ENTRY(ce, "Indexed", Indexed_methods);
	Indexed_ce = zend_register_internal_class(&ce);
	Indexed_ce->create_object = php_indexed_create;
	Indexed_ce->get_iterator = php_indexed_iterator;
	Indexed_ce->ce_flags |= ZEND_ACC_FINAL;

	zend_class_implements(
		Indexed_ce, 2,
		spl_ce_ArrayAccess, spl_ce_Countable);

	php_indexed_minit();
	
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
