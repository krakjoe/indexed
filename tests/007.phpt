--TEST--
Indexed iterator
--SKIPIF--
<?php if (!extension_loaded("indexed")) print "skip"; ?>
--FILE--
<?php
$array = new Indexed(2, [
	"Hello",
	"World"
]);

foreach ($array as $k => $v)
	var_dump($k, $v);
?>
--EXPECT--
int(0)
string(5) "Hello"
int(1)
string(5) "World"



