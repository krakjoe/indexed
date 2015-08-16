--TEST--
Indexed properties
--SKIPIF--
<?php if (!extension_loaded("indexed")) print "skip"; ?>
--FILE--
<?php
$array = new Indexed(2, [
	"Hello",
	"World"
]);

$array->property = $array;

var_dump($array);
?>
--EXPECT--
object(Indexed)#1 (3) {
  [0]=>
  string(5) "Hello"
  [1]=>
  string(5) "World"
  ["property"]=>
  *RECURSION*
}





