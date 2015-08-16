--TEST--
Indexed casting to array
--SKIPIF--
<?php if (!extension_loaded("indexed")) print "skip"; ?>
--FILE--
<?php 
$array = new Indexed(2, [
	"Hello",
	"World"
]);

var_dump((array) $array);
?>
--EXPECT--
array(2) {
  [0]=>
  string(5) "Hello"
  [1]=>
  string(5) "World"
}



