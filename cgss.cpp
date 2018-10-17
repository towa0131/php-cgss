#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

extern "C"{
	#include "php.h"
	#include "php_ini.h"
	#include "zend.h"
	#include "ext/standard/info.h"
	#include "Zend/zend_exceptions.h"
}

#include <iostream>

#include "php_cgss.hpp"
#include "cgss_api.h"

static int le_cgss;

using namespace std;

zend_class_entry *cgss_exception;

PHP_FUNCTION(hca2wav)
{

	zend_string *result;
	zend_string *hcaFile;
	zend_string *waveFile;

	zend_long key1;
	zend_long key2;

	const char *cHcaFile;
	const char *cWaveFile;

	ZEND_PARSE_PARAMETERS_START(2, 4)
		Z_PARAM_STR(hcaFile)
		Z_PARAM_STR(waveFile)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(key1)
		Z_PARAM_LONG(key2)
	ZEND_PARSE_PARAMETERS_END();

	cHcaFile = reinterpret_cast<const char*>((unsigned char *) ZSTR_VAL(hcaFile));
	cWaveFile = reinterpret_cast<const char*>((unsigned char *) ZSTR_VAL(waveFile));

	cgss::CHcaDecoderConfig decoderConfig;
	decoderConfig.decodeFunc = cgss::CDefaultWaveGenerator::Decode16BitS;
	decoderConfig.waveHeaderEnabled = TRUE;

	if(key1 != NULL && key2 != NULL){
		decoderConfig.cipherConfig.keyParts.key1 = (uint32_t)key1;
		decoderConfig.cipherConfig.keyParts.key2 = (uint32_t)key2;
	}else{
		decoderConfig.cipherConfig.keyParts.key1 = 0x12345678;
		decoderConfig.cipherConfig.keyParts.key2 = 0x90abcdef;
	}

	try {
		cgss::CFileStream fileIn(cHcaFile, cgss::FileMode::OpenExisting, cgss::FileAccess::Read),
			fileOut(cWaveFile, cgss::FileMode::Create, cgss::FileAccess::Write);
		cgss::CHcaDecoder hcaDecoder(&fileIn, decoderConfig);

		uint32_t read = 1;
		static const uint32_t bufferSize = 1024;
		uint8_t buffer[bufferSize];
		while (read > 0) {
			read = hcaDecoder.Read(buffer, bufferSize, 0, bufferSize);
			if (read > 0) {
				fileOut.Write(buffer, bufferSize, 0, read);
			}
		}
	} catch (const cgss::CException &ex) {
		zend_throw_exception_ex(cgss_exception, ex.GetOpResult() TSRMLS_CC, ex.GetExceptionMessage().c_str());
		RETURN_FALSE;
	}
		
	RETURN_TRUE;
}

PHP_MINIT_FUNCTION(cgss)
{
	REGISTER_MAIN_LONG_CONSTANT("CGSS_HCA_KEY_1", 0xF27E3B22, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("CGSS_HCA_KEY_2", 0x00003657, CONST_PERSISTENT | CONST_CS);

	zend_class_entry cgss_ex;
	INIT_CLASS_ENTRY(cgss_ex, "CGSSException", NULL);
	cgss_exception = zend_register_internal_class_ex(&cgss_ex, zend_exception_get_default());
	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(cgss)
{
	return SUCCESS;
}

PHP_RINIT_FUNCTION(cgss)
{
	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(cgss)
{
	return SUCCESS;
}

PHP_MINFO_FUNCTION(cgss)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "cgss support", "enabled");
	php_info_print_table_end();
}

const zend_function_entry cgss_functions[] = {
	PHP_FE(hca2wav,	NULL)
	PHP_FE_END
};

zend_module_entry cgss_module_entry = {
	STANDARD_MODULE_HEADER,
	"cgss",
	cgss_functions,
	PHP_MINIT(cgss),
	PHP_MSHUTDOWN(cgss),
	PHP_RINIT(cgss),
	PHP_RSHUTDOWN(cgss),
	PHP_MINFO(cgss),
	PHP_CGSS_VERSION,
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_CGSS
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(cgss)
#endif