
#ifndef _MULTI_TIMER_H_
#define _MULTI_TIMER_H_

typedef void (*mtimer_handler_func)(void *);

struct mtimer_s {
	unsigned int ref_cnt; //Can only be 1 or 0
	timer_t timer_id;
	mtimer_handler_func handler;
	void *priv_data;
	int fired;
};

int create_timer(struct mtimer_s *mtimer, mtimer_handler_func func, void *priv_data);
int delete_timer(struct mtimer_s *mtimer);
void start_timer(struct mtimer_s *mtimer, int expireMS, int intervalMS);
void stop_timer(struct mtimer_s *mtimer);

#endif /* #ifndef _MULTI_TIMER_H_ */
