--TEST--
Indexed gc
--SKIPIF--
<?php if (!extension_loaded("indexed")) print "skip"; ?>
--FILE--
<?php 
$array = new Indexed(3);

$array[0] = "Hello";
$array[1] = "World";
$array[2] = $array;

var_dump($array);
?>
--EXPECT--
object(Indexed)#1 (3) {
  [0]=>
  string(5) "Hello"
  [1]=>
  string(5) "World"
  [2]=>
  *RECURSION*
}


