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
#include <stdlib.h>

#include "php_cgss.hpp"
#include "cgss.hpp"
#include "CAcbFile.h"
#include "common.h"

static int le_cgss;

using namespace std;
using namespace cgss;

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

	CHcaDecoderConfig config;
	config.decodeFunc = CDefaultWaveGenerator::Decode16BitS;
	config.waveHeaderEnabled = TRUE;

	if(key1 != NULL){
		config.cipherConfig.keyParts.key1 = (uint32_t)key1;
	}else{
		config.cipherConfig.keyParts.key1 = 0x12345678;
	}

	if(key2 != NULL){
		config.cipherConfig.keyParts.key2 = (uint32_t)key2;
	}else{
		config.cipherConfig.keyParts.key2 = 0x90abcdef;
	}

	try{
		CFileStream fileIn(cHcaFile, FileMode::OpenExisting, FileAccess::Read), fileOut(cWaveFile, FileMode::Create, FileAccess::Write);
		CHcaDecoder hcaDecoder(&fileIn, config);

		uint32_t read = 1;
		static const uint32_t bufferSize = 1024;
		uint8_t buffer[bufferSize];
		while(read > 0){
			read = hcaDecoder.Read(buffer, bufferSize, 0, bufferSize);
			if(read > 0){
				fileOut.Write(buffer, bufferSize, 0, read);
			}
		}
	}catch(const CException &ex){
		zend_throw_exception_ex(cgss_exception, ex.GetOpResult() TSRMLS_CC, ex.GetExceptionMessage().c_str());
		RETURN_FALSE;
	}

	RETURN_TRUE;
}


PHP_FUNCTION(acbunpack)
{
	zend_string *acbFile;

	const char *filePath;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_STR(acbFile)
	ZEND_PARSE_PARAMETERS_END();

	filePath = reinterpret_cast<const char*>((unsigned char *) ZSTR_VAL(acbFile));

	if(!cgssHelperFileExists(filePath)){
		zend_throw_exception_ex(cgss_exception, -1 TSRMLS_CC, "File %s does not exist or cannot be opened.", filePath);
		RETURN_FALSE;
	}

	try{
		CFileStream fileStream(filePath, FileMode::OpenExisting, FileAccess::Read);
		CAcbFile acb(&fileStream, filePath);

		acb.Initialize();

		const string extractDir = GetFilePath(filePath) + "_acb_" + GetFileName(filePath) + "/";
		MakeDirectories(extractDir);

		const auto &fileNames = acb.GetFileNames();

		uint32_t i = 0;

		for(const auto &fileName : fileNames){
			auto s = fileName;
			auto isCueNonEmpty = !s.empty();

			if(!isCueNonEmpty){
				s = CAcbFile::GetSymbolicFileNameFromCueId(i);
			}

			auto extractPath = extractDir + s;

			IStream *stream;

			if(isCueNonEmpty){
				stream = acb.OpenDataStream(s.c_str());
			}else{
				stream = acb.OpenDataStream(i);
			}

			if(stream){
				CFileStream fs(extractPath.c_str(), FileMode::Create, FileAccess::Write);
				common_utils::CopyStream(stream, &fs);
			}else{
				zend_throw_exception_ex(cgss_exception, -1 TSRMLS_CC, "Cue #%u (%s) cannot be retrieved.", i + 1, s.c_str());
				RETURN_FALSE;
			}

			delete stream;

			++i;
		}
	}catch (const CException &ex){
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
	PHP_FE(acbunpack,	NULL)
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
