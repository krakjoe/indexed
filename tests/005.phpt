--TEST--
Indexed initial data
--SKIPIF--
<?php if (!extension_loaded("indexed")) print "skip"; ?>
--FILE--
<?php 
$array = new Indexed(3, [
	"Hello",
	"World"
]);

$array[2] = ["Hello", "World"];

var_dump($array);
?>
--EXPECT--
object(Indexed)#1 (3) {
  [0]=>
  string(5) "Hello"
  [1]=>
  string(5) "World"
  [2]=>
  array(2) {
    [0]=>
    string(5) "Hello"
    [1]=>
    string(5) "World"
  }
}


