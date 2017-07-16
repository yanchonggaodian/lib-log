/* *************
 * Auth: yanchonggaodian
 * Date: 20170630 
 * Company: RAK
 * *************/

#ifndef __LOG_H__
#define __LOG_H__
#include <stdarg.h>

#ifdef __cplusplus  
extern "C"  {
#endif 
typedef void *log_t;    
typedef enum{//// if initialed log level is initedlevel, then log don't output msg when thecurrentloglevel is bigger than initedlevel.
	LOG_STATUS_ERR = -1,// only for the case: when type of return is log_level_t, return it on failed.
	LOG_OFF,            // only for the case: when initial log_t object, close all log info.
	LOG_FATAL,          // Fatal error log level
	LOG_ERROR,          // General error log level
	LOG_WARN,           // Warning log level
	LOG_INFO,           // important info log level
	LOG_DEBUG,          // debug info log level
	LOG_TRACE,          // Trace debug info log level
	LOG_FINE,           // Fine debug info log level
}log_level_t;    

/* *************************************************************************************************************
 * plog:  the value of log_t object, returned by module_log_init. 
 * level: thecurrentloglevel 
 * ************************************************************************************************************/
#define LOG_P(plog, level,...) module_log_output((log_t)(long)(plog), (log_level_t)((long)(level)), __FILE__, __func__, __LINE__, (const char*)(long)__VA_ARGS__)



/* *************************************************************************************************************
 * pdest: @char**;          *pdest will be chg and realloc memory, output string will be stored into *pdest
 * plog:  @log_t;           the value returned by module_log_init.
 * level: @log_level_t;     level>=LOG_FATAL && level<=LOG_FINE.
 * *************************************************************************************************************/
#define LOG_S(pdest, plog, level,...) module_log_output_s((char**)(long)(pdest), ((log_t)(long)(plog)), (log_level_t)(long)(level), __FILE__, __func__, __LINE__, __VA_ARGS__)


#ifdef __cplusplus  
#define LOG(FIRST,SECOND,...) LOG_INLINE(__FILE__, __func__, __LINE__, FIRST, SECOND, __VA_ARGS__)
#else
#define LOG(FIRST,SECOND,...)      _Generic((SECOND), log_t:LOG_S(FIRST,SECOND,__VA_ARGS__,NULL), int:LOG_P(FIRST,SECOND,__VA_ARGS__), default:LOG_S(FIRST,SECOND,__VA_ARGS__,NULL))
#endif

/* **************************************************************************************************************
 * the type for declare a function pointer for usr custom log function.
 * in the function instance, usr can get usr's custom data pointer by module_log_get_privdata(plog);
 * in any case, usr can set usr's custom data by module_log_set_privdata(plog, (void *) data);
 * outaddr: @char**;          *outaddr will be chg and realloc memory, output string will be stored into *outaddr
 * plog:    @log_t;           the value returned by module_log_init.
 * level:   @log_level_t;     level>=LOG_FATAL && level<=LOG_FINE.
 * filename:@const char*;     the name of file.
 * funcname:@const char*;     the name of the function.
 * lineno:  @const int;       the line number of the log in file.
 * fmt:     @const char*;     like vprintf function in c standard library.
 * va_l:    @va_list;         like vprintf function in c standard library.
 * *************************************************************************************************************/
typedef int (*log_outputvs_callback_t)(char**outaddr, log_t plog, log_level_t level, 
                        const char *filename, const char* funcname, const int lineno, 
                        const char *fmt,  va_list va_l);


/* *************************************************************************************************************
 * the type for declare a function pointer for usr custom log function.
 * in the function instance, usr can get usr's custom data pointer by module_log_get_privdata(plog);
 * in any case, usr can set usr's custom data by module_log_set_privdata(plog, (void *) data);
 * plog:    @log_t;           the value returned by module_log_init.
 * level:   @log_level_t;     level>=LOG_FATAL && level<=LOG_FINE.
 * filename:@const char*;     the name of file.
 * funcname:@const char*;     the name of the function.
 * lineno:  @const int;       the line number of the log in file.
 * fmt:     @const char*;     like vprintf function in c standard library.
 * va_l:    @va_list;         like vprintf function in c standard library.
 * *************************************************************************************************************/
typedef int (*log_outputv_callback_t)(log_t, log_level_t level, 
                        const char *filename, const char* funcname, const int lineno, 
                        const char *fmt, va_list va_l);

/* *******************************************************************************************************************************************
 * initial log resource. include: module_name, log_level, and custom function log_outputv_callback and log_outputvs_callback(the two func can 
 * be ingnored).
 * module:                @const char*;             module name, can be NULL if not output module name.
 * level:                 @log_level_t;             level>=LOG_OFF && level<=LOG_ALL.
 * log_outputv_callback:  @log_outputv_callback_t;  custom function for module_log_output, should be NULL, if usr NOT instance log_outputv_callback_t.
 * log_outputvs_callback: @log_outputvs_callback_t; custom function for module_log_output, should be NULL, if usr NOT instance log_outputvs_callback_t.
 * ******************************************************************************************************************************************/
 log_t module_log_init(const char *module, log_level_t level, log_outputv_callback_t log_outputv_callback, log_outputvs_callback_t log_outputvs_callback);

/* after module_log_init, you must call it for destroy the log hnd, when you are sure that do not use the log again.*/
log_t module_log_destroy(log_t log);


/* after LOG_S or module_log_output_s, you must call it for free memory, when you are sure that do not use the it again.*/
void module_log_output_strfree(char**outaddr);

/* for change log level */
int module_log_set_level(log_t log, log_level_t level);
/* for change log module name */
int module_log_set_module(log_t log, const char* module);
/* for set log_outputv_callback_t type custom function*/
int module_log_set_log_outputv_func(log_t log, log_outputv_callback_t func);
/* for set log_outputvs_callback_t type custom function*/
int module_log_set_log_outputvs_func(log_t log, log_outputvs_callback_t func);
/* for set custom data for custom function */
int module_log_set_privdata(log_t log, void * data);


/* for get log level */
log_level_t module_log_get_level(log_t log);
/* for get log module name */
const char * module_log_get_module(log_t log);
/* for get log log_outputv_callback_t type custom function pointer. */
log_outputv_callback_t module_log_get_log_outputv_func(log_t log);
/* for get log log_outputvs_callback_t type custom function pointer. */
log_outputvs_callback_t module_log_get_log_outputvs_func(log_t log);
/* for get custom data for custom function */
void * module_log_get_privdata(log_t log);



/* *******************************************************************************************************************************************
 * the function will output log msg into *outaddr if the level less than or equal to the level set by module_log_init or module_log_set_level.
 * outaddr: @char**;          *outaddr will be chg and realloc memory, log msg will be stored into *outaddr,
 *                            NULL may result in Segmentation fault expect for instance log_outputvs_callback_t function.
 * plog:    @log_t;           the value returned by module_log_init.
 * level:   @log_level_t;     level>=LOG_FATAL && level<=LOG_FINE.
 * filename:@const char*;     the name of file.
 * funcname:@const char*;     the name of the function.
 * lineno:  @const int;       the line number of the log in file.
 * fmt:     @const char*;     like vprintf function in c standard library.
 * va_l:    @va_list;         like vprintf function in c standard library.
 * ******************************************************************************************************************************************/
int module_log_output_s(char**outaddr, log_t plog, log_level_t level,
                        const char *filename, const char* funcname, const int lineno,
                        const char *fmt,...);



/* *****************************************************************************************************************************************
 * the function will output log msg into stdout if the level less than or equal to the level set by module_log_init or module_log_set_level.
 * plog:    @log_t;           the value returned by module_log_init.
 * level:   @log_level_t;     level>=LOG_FATAL && level<=LOG_FINE.
 * filename:@const char*;     the name of file.
 * funcname:@const char*;     the name of the function.
 * lineno:  @const int;       the line number of the log in file.
 * fmt:     @const char*;     like vprintf function in c standard library.
 * va_l:    @va_list;         like vprintf function in c standard library.
 * ****************************************************************************************************************************************/
int module_log_output(log_t plog, log_level_t level, 
						const char * filename, const char* funcname, const int lineno, 
						const char *fmt,...);

#ifdef __cplusplus  
}
#endif 

#ifdef __cplusplus  
extern "C" {
int module_log_output_vs(char**outaddr, log_t plog, log_level_t level,
                         const char * filename, const char* funcname, const int lineno,
                         const char *fmt, va_list va_l);


int module_log_output_v(log_t plog, log_level_t level,
                        const char * filename, const char* funcname, const int lineno,
                        const char *fmt, va_list va_l);
}

inline int LOG_INLINE(const char * filename, const char* funcname, const int lineno,
		char**outaddr, log_t plog, log_level_t level,
		const char *fmt, ...){
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret = module_log_output_vs(outaddr, plog, level, filename, funcname, lineno, fmt, ap);
	va_end(ap);
	return ret;
}
inline int LOG_INLINE(const char * filename, const char* funcname, const int lineno,
		log_t plog, log_level_t level,
		const char *fmt,...){
	int ret;
	va_list ap;
	va_start(ap, fmt);
	
	ret = module_log_output_v(plog, level, filename, funcname, lineno, fmt, ap);
	va_end(ap);
	return ret;
}
#endif

#endif
