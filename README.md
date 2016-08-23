# php_snowflake

[中文文档](https://github.com/Sxdd/php_snowflake/wiki/%E4%B8%AD%E6%96%87%E6%96%87%E6%A1%A3)
## What is php_snowflake?

[Twitter SnowFlake](https://github.com/twitter/snowflake) PHP version of the algorithm

## Requires
PHP >= 5.6  (5.5 the following self-testing)

## Description

### NTS
```
0　2　　　　　　    15　　　　　　 　20   28　　　   32
---+----------------+--------------+----+----------+
00 |timestamp(ms)  | service_no 　 |pid | sequence |
---+----------------+--------------+----+----------+
```

### TS
```
0　2　　　　　 　   15　　　　　　 　20   28　　　   32
---+----------------+--------------+----+----------+
00 |timestamp(ms)  | service_no 　 |tid | sequence |
---+----------------+--------------+----+----------+
```

## Installation
```
phpize
./configure --with-php-config=/you/phppath/php-config
make
make install
```
## Example
```
$service_no = 999;
for ($i=0; $i < 10; $i++) { 
        echo PhpSnowFlake::nextId($service_no)."\n";
}
/*

00146523488416500999000634280001
00146523488416500999000634280002
00146523488416500999000634280003
00146523488416500999000634280004
00146523488416500999000634280005
00146523488416600999000634280001
00146523488416600999000634280002
00146523488416600999000634280003
00146523488416600999000634280004
00146523488416600999000634280005

*/
```
## License
Copyright (c) 2016 by [Towers](http://towers.pub) released under MIT License.


