dnl $Id$
dnl config.m4 for extension list

PHP_ARG_ENABLE(indexed, whether to enable indexed support,
[  --enable-indexed           Enable indexed support])

if test "$PHP_INDEXED" != "no"; then
  PHP_NEW_EXTENSION(indexed, indexed.c object.c iterator.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
