/*
  +----------------------------------------------------------------------+
  | php_snowflake  https://github.com/Sxdd/php_snowflake   |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2016 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:  Towers   (sxddwuwang@gmail.com)       |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// #define MAX_SEQUENCE 8191

#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <stdio.h>
#include <sys/syscall.h>
#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_snowflake.h"

PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("php_snowflake.twepoch",      "1380902400000", PHP_INI_SYSTEM, OnUpdateLong, twepoch,      zend_php_snowflake_globals, php_snowflake_globals)
	STD_PHP_INI_ENTRY("php_snowflake.sequence_bit", "8",             PHP_INI_SYSTEM, OnUpdateLong, sequence_bit, zend_php_snowflake_globals, php_snowflake_globals)
    STD_PHP_INI_ENTRY("php_snowflake.worker_bit",   "8",             PHP_INI_SYSTEM, OnUpdateLong, worker_bit,   zend_php_snowflake_globals, php_snowflake_globals)
    STD_PHP_INI_ENTRY("php_snowflake.service_bit",  "4",             PHP_INI_SYSTEM, OnUpdateLong, service_bit,  zend_php_snowflake_globals, php_snowflake_globals)
PHP_INI_END()


/* If you declare any globals in php_snowflake.h uncomment this:*/
ZEND_DECLARE_MODULE_GLOBALS(php_snowflake)

zend_class_entry *php_snowflake_ce;

static zend_long time_re_gen(zend_long last) {
	zend_long new_time;
	struct timeval tv;
	do {
		gettimeofday(&tv, 0);
		new_time = (zend_long)tv.tv_sec * 1000 + (zend_long)tv.tv_usec / 1000;
	} while (new_time <= last);

	return new_time;
}

static zend_long get_workid() {
#ifdef ZTS
	return syscall(SYS_gettid);
#else
	return getpid();
#endif
}

static zend_long time_gen() {
	struct timeval tv;
	gettimeofday(&tv, 0);
	return (zend_long)tv.tv_sec * 1000 + (zend_long)tv.tv_usec / 1000;
}

static zend_long next_id(zend_long service_no) {
#if PHP_API_VERSION < 20151012
	TSRMLS_FETCH();
#endif
	zend_long ts;

	ts = time_gen();

	if (ts == PHP_SNOWFLAKE_G(last_time_stamp)) {
		PHP_SNOWFLAKE_G(sequence) = (PHP_SNOWFLAKE_G(sequence)+1) & SEQMASK;
		if (PHP_SNOWFLAKE_G(sequence) == 0) {
			ts = time_re_gen(ts);
			PHP_SNOWFLAKE_G(sequence) = 1;
		}
	} else {
		PHP_SNOWFLAKE_G(sequence) = 1;
	}
	PHP_SNOWFLAKE_G(last_time_stamp) = ts;

	return ((ts - TWEPOCH) << TMSHIFT) |
			((service_no & SERVICEMASK) << SERVICESHIFT) |
			((PHP_SNOWFLAKE_G(worker_id) & WORKERMASK) << WORKERSHIFT) |
			(PHP_SNOWFLAKE_G(sequence));

	// if (ts < PHP_SNOWFLAKE_G(last_time_stamp)) {
	// 	strcpy(id, NULL);
	// } else {
	// 	sprintf(id, "00%ld%05ld%08ld%04d", ts, PHP_SNOWFLAKE_G(service_no), PHP_SNOWFLAKE_G(worker_id), PHP_SNOWFLAKE_G(sequence));
	// }
}

PHP_METHOD(PhpSnowFlake, nextId) {
	zend_long id = 0;
	zend_long service_no = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS()
#if PHP_API_VERSION < 20151012
		TSRMLS_CC
#endif
		, "l", &service_no) == FAILURE) {
		RETURN_FALSE;
	}

	if (service_no > SERVICEMASK | service_no < 0) {
		zend_error(E_ERROR, "service_no in the range of 0,%lld", SERVICEMASK);
	}

	id = next_id(service_no);

	RETVAL_LONG(id);
}

PHP_METHOD(PhpSnowFlake, configure) {
    array_init(return_value);
    add_assoc_long(return_value, "worker_id", PHP_SNOWFLAKE_G(worker_id));
    add_assoc_long(return_value, "last_time_stamp", PHP_SNOWFLAKE_G(last_time_stamp));
    add_assoc_long(return_value, "sequence", PHP_SNOWFLAKE_G(sequence));
    add_assoc_long(return_value, "twepoch", PHP_SNOWFLAKE_G(twepoch));
    add_assoc_long(return_value, "sequence_bit", PHP_SNOWFLAKE_G(sequence_bit));
    add_assoc_long(return_value, "worker_bit", PHP_SNOWFLAKE_G(worker_bit));
    add_assoc_long(return_value, "service_bit", PHP_SNOWFLAKE_G(service_bit));
    add_assoc_long(return_value, "tm_bit", PHP_SNOWFLAKE_G(tm_bit));
    add_assoc_long(return_value, "seqmask", PHP_SNOWFLAKE_G(seqmask));
    add_assoc_long(return_value, "seqshift", PHP_SNOWFLAKE_G(seqshift));
    add_assoc_long(return_value, "workermask", PHP_SNOWFLAKE_G(workermask));
    add_assoc_long(return_value, "workershift", PHP_SNOWFLAKE_G(workershift));
    add_assoc_long(return_value, "servicemask", PHP_SNOWFLAKE_G(servicemask));
    add_assoc_long(return_value, "serviceshift", PHP_SNOWFLAKE_G(serviceshift));
    add_assoc_long(return_value, "tmshift", PHP_SNOWFLAKE_G(tmshift));
}

/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and
   unfold functions in source code. See the corresponding marks just before
   function definition, where the functions purpose is also documented. Please
   follow this convention for the convenience of others editing your code.
*/

/* {{{ ARG_INFO
 */
ZEND_BEGIN_ARG_INFO_EX(php_snowflake_next_id, 0, 0, 1)
	ZEND_ARG_INFO(0, service_no)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_snowflake_configure, 0, 0, 0)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ php_php_snowflake_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_php_snowflake_init_globals(zend_php_snowflake_globals *php_snowflake_globals)
{
	php_snowflake_globals->global_value = 0;
	php_snowflake_globals->global_string = NULL;
}
*/
/* }}} */


/* {{{ php_snowflake_functions[]
 *
 * Every user visible function must have an entry in php_snowflake_methods[].
 */
const zend_function_entry php_snowflake_methods[] = {
	PHP_ME(PhpSnowFlake, nextId, php_snowflake_next_id, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(PhpSnowFlake, configure, php_snowflake_configure, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_FE_END
};

#if PHP_API_VERSION < 20151012
zend_function_entry php_snowflake_functions[] = {
	PHP_FE_END
};
#else
#define php_snowflake_functions NULL
#endif
/* }}} */


static void php_snowflake_ctor(
#if PHP_API_VERSION < 20151012
	zend_php_snowflake_globals *iw TSRMLS_DC
#else
	zend_php_snowflake_globals *iw
#endif
)
{
	iw->sequence = 0;
	iw->worker_id = 0;//get_workid() & WORKERMASK;
}

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(php_snowflake)
{
	// module init
#ifdef ZTS
	ts_allocate_id(&php_snowflake_globals_id, sizeof(zend_php_snowflake_globals), (ts_allocate_ctor)php_snowflake_ctor, NULL);
#else
# if PHP_API_VERSION < 20151012
	php_snowflake_ctor(&php_snowflake_globals TSRMLS_CC);
# else
	php_snowflake_ctor(&php_snowflake_globals);
# endif
#endif

	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "PhpSnowFlake", php_snowflake_methods);
#if PHP_API_VERSION < 20151012
	php_snowflake_ce = zend_register_internal_class(&ce TSRMLS_CC);
#else
	php_snowflake_ce = zend_register_internal_class_ex(&ce, NULL);
#endif
	/* If you have INI entries, uncomment these lines */
	REGISTER_INI_ENTRIES();

    // 初始化掩码等设置
    PHP_SNOWFLAKE_G(tm_bit) = 64 - PHP_SNOWFLAKE_G(sequence_bit) - PHP_SNOWFLAKE_G(worker_bit) - PHP_SNOWFLAKE_G(service_bit);
    PHP_SNOWFLAKE_G(seqmask) = -1UL ^ (-1UL << PHP_SNOWFLAKE_G(sequence_bit));
    PHP_SNOWFLAKE_G(seqshift) = 0;
    PHP_SNOWFLAKE_G(workermask) = -1UL ^ (-1UL << PHP_SNOWFLAKE_G(worker_bit));
    PHP_SNOWFLAKE_G(workershift) = PHP_SNOWFLAKE_G(sequence_bit);
    PHP_SNOWFLAKE_G(servicemask) = -1UL ^ (-1UL << PHP_SNOWFLAKE_G(service_bit));
    PHP_SNOWFLAKE_G(serviceshift) = PHP_SNOWFLAKE_G(sequence_bit) + PHP_SNOWFLAKE_G(worker_bit);
    PHP_SNOWFLAKE_G(tmshift) = PHP_SNOWFLAKE_G(serviceshift) + PHP_SNOWFLAKE_G(service_bit);
	return SUCCESS;
}
/* }}} */



/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(php_snowflake)
{

	/* uncomment this line if you have INI entries */
	UNREGISTER_INI_ENTRIES();

	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(php_snowflake)
{
	// request init
// #ifndef ZTS
	if (!PHP_SNOWFLAKE_G(worker_id)) {
		PHP_SNOWFLAKE_G(worker_id) = get_workid();// & WORKERMASK;
	}
// #endif
#if defined(COMPILE_DL_PHP_SNOWFLAKE) && defined(ZTS) && PHP_API_VERSION >= 20151012
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(php_snowflake)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(php_snowflake)
{
    char buffer[100];
	php_info_print_table_start();
	php_info_print_table_header(2, "php_snowflake", "enabled");
	php_info_print_table_row(2, "version", PHP_SNOWFLAKE_VERSION);
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini */
	DISPLAY_INI_ENTRIES();

	php_info_print_table_start();
	php_info_print_table_header(2, "param", "value");
    sprintf(buffer, "%lld(%llx)", PHP_SNOWFLAKE_G(worker_id), PHP_SNOWFLAKE_G(worker_id));
	php_info_print_table_row(2, "worker id", buffer);
    sprintf(buffer, "%lld", PHP_SNOWFLAKE_G(last_time_stamp));
	php_info_print_table_row(2, "last timestamp", buffer);
    sprintf(buffer, "%lld", PHP_SNOWFLAKE_G(sequence));
	php_info_print_table_row(2, "sequence", buffer);
    sprintf(buffer, "%lld", PHP_SNOWFLAKE_G(twepoch));
	php_info_print_table_row(2, "twepoch", buffer);
    sprintf(buffer, "%lld", PHP_SNOWFLAKE_G(sequence_bit));
	php_info_print_table_row(2, "sequence bit", buffer);
    sprintf(buffer, "%lld", PHP_SNOWFLAKE_G(worker_bit));
	php_info_print_table_row(2, "worker bit", buffer);
    sprintf(buffer, "%lld", PHP_SNOWFLAKE_G(service_bit));
	php_info_print_table_row(2, "service bit", buffer);
    sprintf(buffer, "%lld", PHP_SNOWFLAKE_G(tm_bit));
	php_info_print_table_row(2, "tm bit", buffer);
    sprintf(buffer, "%llx", PHP_SNOWFLAKE_G(seqmask));
	php_info_print_table_row(2, "seqmask", buffer);
    sprintf(buffer, "%lld", PHP_SNOWFLAKE_G(seqshift));
	php_info_print_table_row(2, "seqshift", buffer);
    sprintf(buffer, "%llx", PHP_SNOWFLAKE_G(workermask));
	php_info_print_table_row(2, "workermask", buffer);
    sprintf(buffer, "%lld", PHP_SNOWFLAKE_G(workershift));
	php_info_print_table_row(2, "workershift", buffer);
    sprintf(buffer, "%llx", PHP_SNOWFLAKE_G(servicemask));
	php_info_print_table_row(2, "servicemask", buffer);
    sprintf(buffer, "%lld", PHP_SNOWFLAKE_G(serviceshift));
	php_info_print_table_row(2, "serviceshift", buffer);
    sprintf(buffer, "%lld", PHP_SNOWFLAKE_G(tmshift));
	php_info_print_table_row(2, "tmshift", buffer);
	php_info_print_table_end();
}
/* }}} */

/* {{{ php_snowflake_module_entry
 */
zend_module_entry php_snowflake_module_entry = {
	STANDARD_MODULE_HEADER,
	"php_snowflake",
	php_snowflake_functions,
	PHP_MINIT(php_snowflake),
	PHP_MSHUTDOWN(php_snowflake),
	PHP_RINIT(php_snowflake),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(php_snowflake),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(php_snowflake),
	PHP_SNOWFLAKE_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_PHP_SNOWFLAKE
#if defined(ZTS) && PHP_API_VERSION >= 20151012
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(php_snowflake)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
