# PHP Master

Another one of **\*those\*** challenges.

**Target:** http://challs.xmas.htsp.ro:3000/  
**Author:** yakuhito  
**Category:** Web exploitation

![image](https://user-images.githubusercontent.com/6524684/102717287-a350fa00-42e1-11eb-94f4-58f11b0b0499.png)

---

When visiting the provided link (and accepting the lack of HTTPS), we are greeted with:

```php
<?php

include('flag.php');

$p1 = $_GET['param1'];
$p2 = $_GET['param2'];

if(!isset($p1) || !isset($p2)) {
    highlight_file(__FILE__);
    die();
}

if(strpos($p1, 'e') === false && strpos($p2, 'e') === false  && strlen($p1) === strlen($p2) && $p1 !== $p2 && $p1[0] != '0' && $p1 == $p2) {
    die($flag);
}

?>
```

The PHP script reads the value of two parameters `param1` and `param2`. If either of them is missing, the script just prints itself, which is exactly what we're seeing.

To obtain the flag, we have to provide two parameters where:

- Neither contains the character `e`

- Both have the same length

- They do not compare equal using `!==`

- The first parameter does not start with `0`

- The parameters compare equal with `==`

My initial thought as somewhat of a PHP noob was to (somehow) create strings that would evalue to dictionaries with overloaded `[]` operators.
The goal was to have `p1[0] != 0` produce some side effect that makes strings that were unequal before compare equal afterwards.

However, a quick study of [PHP comparison operators](https://www.php.net/manual/en/language.operators.comparison.php) directly points to type
juggling as the simple solution: `!==` checks for (in)equality directly, whereas `==` first performs type juggling.

There are many ways to solve this, and people have shared creative solutions in the discord. I personally wondered about the "does not contain `e`"
checks, which looks like an attempt to exclude scientific notation. But `E` is still allowed, which led me to the intended solution of e.g.

http://challs.xmas.htsp.ro:3000/?param1=1E2&param2=100

Other solutions people shared were:

- `-0.0`/`0.00` (signed and unsigned zero compare equal)

- `1.000000000000000000000`/`1.000000000000000000000001` (limited floating point accuracy)

- `1.0`/`001`

This is the first and only web challenge I completed this year. I briefly dabbled in some other ones, but I'm primarily missing a deeper understanding
of the "behind the scenes" to generate efficient intuitions about what to look for.

At least I completed one web challenge!
