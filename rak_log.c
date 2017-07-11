/* *************
 * Auth: yanchonggaodian
 * Date: 20170630 
 * Company: RAK
 * *************/
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <libgen.h>
#include <pthread.h>
#include <time.h>

#include "rak_log.h"



static pthread_mutex_t logmutex = PTHREAD_MUTEX_INITIALIZER;


typedef struct __log_t__{
	const char              *module;
	log_outputv_callback_t   log_output_v;
	log_outputvs_callback_t  log_output_vs;
	rak_log_level_t         level;
	void                    *priv;
}log_t;


const char* get_loglevel_str(rak_log_level_t level)
{
	switch(level){
		case RAK_LOG_FATAL:   return "FATAL";
		case RAK_LOG_ERROR:   return "ERROR";
		case RAK_LOG_WARN:    return "WARN ";
		case RAK_LOG_INFO:    return "INFO ";
		case RAK_LOG_DEBUG:   return "DEBUG";
		case RAK_LOG_TRACE:   return "TRACE";
		case RAK_LOG_FINE:    return "FINE ";
		default:
			return " ";
	}
}
rak_log_t module_log_init(const char *module, int level, log_outputv_callback_t log_outputv_callback, log_outputvs_callback_t log_outputvs_callback)
{
	log_t * log = malloc(sizeof(log_t));
	if(log){
		log->module          = module;
		log->level           = level;
		log->log_output_v    = log_outputv_callback;
		log->log_output_vs   = log_outputvs_callback;
		log->priv            = NULL;
	}
	return (rak_log_t)log;
}

rak_log_t module_log_destroy(rak_log_t log)
{
	if(log){
		free(log);
		log=NULL;
	}
	return log;
}

int module_log_set_level(rak_log_t log, rak_log_level_t level)
{
	return ((log)? (((log_t*)log)->level = level) : RAK_LOG_STATUS_ERR);
}

rak_log_level_t module_log_get_level(rak_log_t log)
{
	return (log? ((log_t*)log)->level : RAK_LOG_STATUS_ERR);
}

int module_log_set_module(rak_log_t log, const char* module)
{
	return ((log)? ((((log_t*)log)->module = module), 0) : -1);
}

const char * module_log_get_module(rak_log_t log)
{
	return (log? ((log_t*)log)->module : NULL);
}

int module_log_set_log_outputv_func(rak_log_t log, log_outputv_callback_t func)
{
	return (log? ((((log_t*)log)->log_output_v = func), 0) : -1);
}

log_outputv_callback_t module_log_get_log_outputv_func(rak_log_t log)
{
	return (log? ((log_t*)log)->log_output_v : NULL);
}

int module_log_set_log_outputvs_func(rak_log_t log, log_outputvs_callback_t func)
{
	return (log? ((((log_t*)log)->log_output_vs = func), 0) : -1);
}

log_outputvs_callback_t module_log_get_log_outputvs_func(rak_log_t log)
{
	return (log? ((log_t*)log)->log_output_vs : NULL);
}

int module_log_set_privdata(rak_log_t log, void * data)
{
	return (log? ((((log_t*)log)->priv = data), 0) : -1);
}

void * module_log_get_privdata(rak_log_t log)
{
	return (log? ((((log_t*)log)->priv)) : NULL);
}

static int module_log_output_prefix(char* dest, log_t* log, rak_log_level_t level, 
							 const char * filename, const char* funcname, const int lineno)
{
	int prefix_len;
	time_t time_l = time(NULL) + 8*60*60;
	struct tm tm;
	gmtime_r(&time_l, &tm);

	prefix_len  = dest ? strftime(dest,24,"[ %Y-%m-%d %H:%M:%S ]",&tm) : 23;
	prefix_len += snprintf(dest ? dest + prefix_len : NULL, dest? prefix_len : 0, " ");
	int prefix_len_flag = prefix_len;
	if(log->module)
		prefix_len += snprintf(dest ? dest + prefix_len : NULL, dest? prefix_len : 0, "%s", log->module);
	const char *strlevel = get_loglevel_str(level);
	if(log->module && *strlevel != ' ')
		prefix_len += snprintf(dest ? dest + prefix_len : NULL, dest? prefix_len : 0, "_");
	if(*strlevel != ' ')
		prefix_len += snprintf(dest ? dest + prefix_len : NULL, dest? prefix_len : 0, "%s", strlevel);
	if((log->module || *strlevel != ' ') && (filename || funcname)) {
		prefix_len += snprintf(dest ? dest + prefix_len : NULL, dest? prefix_len : 0, " ");
	}
	if(filename || funcname){
		if(filename){
			prefix_len += snprintf(dest ? dest + prefix_len : NULL, dest? prefix_len : 0, "%s", basename((char*)filename));
		}
		if(filename && funcname)
			prefix_len += snprintf(dest ? dest + prefix_len : NULL, dest? prefix_len : 0, "->");
		if(funcname)
			prefix_len += snprintf(dest ? dest + prefix_len : NULL, dest? prefix_len : 0, "%s", funcname);
		if(lineno>=0)
			prefix_len += snprintf(dest ? dest + prefix_len : NULL, dest? prefix_len : 0, " lineno %d", lineno);
	}
	if(prefix_len > prefix_len_flag)
		prefix_len += snprintf(dest ? dest + prefix_len : NULL, dest? prefix_len : 0, "::");
	
	return prefix_len;
}

int module_log_output_vs(char**outaddr, rak_log_t plog, rak_log_level_t level, 
                         const char * filename, const char* funcname, const int lineno, 
						 const char *fmt, va_list va_l)
{
	int ret = 0;
	pthread_mutex_lock(&logmutex);
	log_t *log=(log_t*)plog;
	if(level <= log->level){
		int prefix_len, len;
		{
			va_list va_dest;
			va_copy(va_dest, va_l);
			len = (prefix_len = module_log_output_prefix(NULL, log, level, filename, funcname, lineno)) + vsnprintf(NULL, 0, fmt, va_l);
			va_l = va_dest;
		}
		if(outaddr && len>=0){

			char * p;
			p = realloc(*outaddr, len+1);
			if(p){
				
				*outaddr = p;
			}else{

				module_log_output_strfree(outaddr);
				*outaddr = realloc(*outaddr, len+1);
			}

			prefix_len = module_log_output_prefix(*outaddr, log, level, filename, funcname, lineno);

			len = prefix_len + vsprintf(((char*)(*outaddr)) + prefix_len, fmt, va_l);

		}
		ret = len;
	}
	pthread_mutex_unlock(&logmutex);
	return ret;
}

int module_log_output_s(char**outaddr, rak_log_t plog, rak_log_level_t level, 
                        const char *filename, const char* funcname, const int lineno, 
                        const char *fmt,...)
{
	
	int ret;
	va_list ap;
	va_start(ap, fmt);
	if(((log_t*)plog)->log_output_vs)
		ret = ((log_t*)plog)->log_output_vs(outaddr, plog, level, filename, funcname, lineno, fmt, ap);
	else
		ret = module_log_output_vs(outaddr, plog, level, filename, funcname, lineno, fmt, ap);
	
	va_end(ap);
	return ret;
}

void module_log_output_strfree(char**outaddr)
{
	if(*outaddr)
		free(*outaddr),*outaddr=NULL;
}


int module_log_output_v(rak_log_t plog, rak_log_level_t level, 
						const char * filename, const char* funcname, const int lineno, 
						const char *fmt, va_list va_l)
{
	int ret = 0;
	pthread_mutex_lock(&logmutex);
	log_t *log = (log_t*)plog;
	if(level <= log->level){
		int len = module_log_output_prefix(NULL, log, level, filename, funcname, lineno);
		char* pbuf = malloc(len + 1 );
		if(pbuf){
			len = module_log_output_prefix(pbuf, log, level, filename, funcname, lineno);
			printf("%s",pbuf);
		}
		free(pbuf);
		ret = len + vprintf(fmt, va_l);
	}
	pthread_mutex_unlock(&logmutex);
	return ret;
}



int module_log_output(rak_log_t plog, rak_log_level_t level, 
						const char * filename, const char* funcname, const int lineno, 
						const char *fmt,...)
{
	int ret;
	va_list ap;
	va_start(ap, fmt);
	if(((log_t*)plog)->log_output_v){
		ret = ((log_t*)plog)->log_output_v(plog, level, filename, funcname, lineno, fmt, ap);
	}else{
		ret = module_log_output_v(plog, level, filename, funcname, lineno, fmt, ap);
	}
	va_end(ap);
	return ret;
}




