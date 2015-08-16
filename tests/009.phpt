--TEST--
Indexed properties
--SKIPIF--
<?php if (!extension_loaded("indexed")) print "skip"; ?>
--FILE--
<?php
$array = new Indexed(1);

$array->property = "something";
?>
--EXPECTF--
Fatal error: Uncaught Exception: properties on Indexed objects are not allowed in %s:4
Stack trace:
#0 {main}
  thrown in %s on line 4





