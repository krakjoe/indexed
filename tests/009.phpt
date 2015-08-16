--TEST--
Indexed properties
--SKIPIF--
<?php if (!extension_loaded("indexed")) print "skip"; ?>
--FILE--
<?php
$array = new Indexed(1);

var_dump($array->property);
?>
--EXPECTF--
Fatal error: Uncaught Exception: properties on Indexed objects are not allowed in %s:4
Stack trace:
#0 {main}
  thrown in %s on line 4





