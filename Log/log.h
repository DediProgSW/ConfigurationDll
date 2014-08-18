#ifndef _LOG_H
#define	_LOG_H

#ifdef LOG_EXPORTS

#define	LOGAPI	__declspec(dllexport)

#else

#define	LOGAPI	__declspec(dllexport)

#endif // LOG_EXPORTS

#define	_USE_LOG_

#ifdef _USE_LOG_ 
#define	LOG_BEGIN	if (1)

#define	LOG_END		

enum log_level {
        LOG_EMERGE,
        LOG_INFO,
        LOG_DEBUG
};


#define	log_init(name) log_init_c(name)
	LOGAPI	int log_init_c(const char *name);

#define log_set_level(level) log_set_level_c(level)
LOGAPI  int log_set_level_c(enum log_level level);

#define	log_add(a, ...) log_level_add_ascii(LOG_EMERGE, a, __VA_ARGS__)
#define log_emerge_add(a, ...) log_level_add_ascii(LOG_EMERGE, a, __VA_ARGS__)
#define log_info_add(a, ...) log_level_add_ascii(LOG_INFO, a, __VA_ARGS__)
#define log_debug_add(a, ...) log_level_add_ascii(LOG_DEBUG, a, __VA_ARGS__)
        LOGAPI  int log_level_add_ascii(enum log_level level, const char *format, ...);

#define	log_exit() log_exit_c()
	LOGAPI  int log_exit_c(void);

#else

#define	LOG_BEGIN	if (0)

#define	LOG_END	


#define	log_init()	((void)0)

#define	log_add(a)	((void)0)

#define	log_exit()	((void)0)

#endif // _USE_LOG_






#endif // !_LOG_H
