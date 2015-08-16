--TEST--
Indexed resize
--SKIPIF--
<?php if (!extension_loaded("indexed")) print "skip"; ?>
--FILE--
<?php 
$array = new Indexed(2);

$array[0] = "Hello";
$array[1] = "World";

var_dump($array);

$array->resize(1);

var_dump($array);
?>
--EXPECT--
object(Indexed)#1 (2) {
  [0]=>
  string(5) "Hello"
  [1]=>
  string(5) "World"
}
object(Indexed)#1 (1) {
  [0]=>
  string(5) "Hello"
}

