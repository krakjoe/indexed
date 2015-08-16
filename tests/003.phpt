--TEST--
Indexed clone
--SKIPIF--
<?php if (!extension_loaded("indexed")) print "skip"; ?>
--FILE--
<?php 
$array = new Indexed(2);

$array[0] = "Hello";
$array[1] = "World";

var_dump($array);

$clone = clone $array;

var_dump($clone);
?>
--EXPECT--
object(Indexed)#1 (2) {
  [0]=>
  string(5) "Hello"
  [1]=>
  string(5) "World"
}
object(Indexed)#2 (2) {
  [0]=>
  string(5) "Hello"
  [1]=>
  string(5) "World"
}


