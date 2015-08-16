--TEST--
Indexed flip
--SKIPIF--
<?php if (!extension_loaded("indexed")) print "skip"; ?>
--FILE--
<?php
$array = new Indexed(2, [
	"Hello",
	"World"
]);

var_dump($array->flip());
?>
--EXPECT--
object(Indexed)#2 (2) {
  [0]=>
  string(5) "World"
  [1]=>
  string(5) "Hello"
}




