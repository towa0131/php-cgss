#ifndef PHP_CGSS_H
#define PHP_CGSS_H

extern zend_module_entry cgss_module_entry;
#define phpext_cgss_ptr &cgss_module_entry

#define PHP_CGSS_VERSION "0.1.0"

#ifdef PHP_WIN32
#	define PHP_CGSS_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_CGSS_API __attribute__ ((visibility("default")))
#else
#	define PHP_CGSS_API
#endif


#define CGSS_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(cgss, v)

#if defined(ZTS) && defined(COMPILE_DL_CGSS)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

#endif