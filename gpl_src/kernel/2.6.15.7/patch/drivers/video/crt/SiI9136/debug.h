#ifndef _AST_DEBUG_H_
#define _AST_DEBUG_H_


#define AST_DEBUG /* FIXME: undefine for release version */
// #undef AST_DEBUG

#ifdef AST_DEBUG
int sii9136_debug_level(void);

enum _debug_level {
        DEBUG_LEVEL_TRACE = 0,
        DEBUG_LEVEL_DEBUG,
        DEBUG_LEVEL_INFO,
        DEBUG_LEVEL_WARNING,
        DEBUG_LEVEL_ERROR,
};
#define DPRINT(level, fmt, args...) { \
                if (DEBUG_LEVEL_##level >= sii9136_debug_level()) \
                        printk("Sii9136:" fmt, ##args); \
        }

#else
#define DPRINT(level, fmt, args...) do {} while (0)
#endif /* AST_DEBUG */

#define P_ERR(fmt, args...) DPRINT(ERROR, fmt, ## args);
#define P_WARN(fmt, args...) DPRINT(WARNING, fmt, ## args);
#define P_INFO(fmt, args...) DPRINT(INFO, fmt, ## args);
#define P_DBG(fmt, args...) DPRINT(DEBUG, fmt, ## args);
#define P_TRACE(fmt, args...) DPRINT(TRACE, fmt, ## args);
#endif  /* #define _AST_DEBUG_H_ */

