/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: limelo-c <limelo-c@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/19 15:54:01 by limelo-c          #+#    #+#             */
/*   Updated: 2026/09/19 15:54:01 by limelo-c         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <errno.h>
# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <limits.h>
# include <time.h>
# include <sys/time.h>
# include <unistd.h>

typedef enum e_scheduler
{
	fifo_sched,
	edf_sched
}	t_scheduler;

typedef enum e_dongle_stat
{
	available,
	taken
}	t_dongle_stat;

typedef struct s_heap
{
	struct s_coder		**list;
	int					size;
	int					capacity;
}	t_heap;

typedef struct s_dongle
{
	pthread_mutex_t		mutex;
	long				released_time;
	pthread_cond_t		thread_sleep;
	t_dongle_stat		dongle_availability;
}	t_dongle;

typedef struct s_coder
{
	pthread_mutex_t		mutex;
	long				coder_id;
	long				coder_compiles_num;
	long				last_compile;
	int					left_dongle;
	int					right_dongle;
	long				arrival;
}	t_coder;

typedef struct s_codex
{
	long				number_of_coders;
	long				time_to_burnout;
	long				time_to_compile;
	long				time_to_debug;
	long				time_to_refactor;
	long				number_of_compiles_required;
	long				dongle_cooldown;
	t_scheduler			scheduler;
	t_dongle			*dongles;
	t_coder				*coders;
	t_heap				waiting_heap;
	int					sim_stopped;
	pthread_mutex_t		mutex_sim;
	pthread_mutex_t		mutex_sched;
	long				start_time;
}	t_codex;

t_codex					*codex_return(void);
int						overflow_checker(long value);
int						validator_numeric_donglecd(const char *str);
int						validator_numeric(const char *str);
int						validator_scheduler(const char *str);
int						parser_validator(char **argv);
int						parser_last_validator(char **argv);
int						dongles_init(void);
int						coders_init(void);
long					timeofday_converter(void);
void					let_dongle(int i);
void					let_both_dongles(t_coder *coder);
int						get_both_dongles(t_coder *coder);
void					simulation_stopper_helper(long ms);
int						try_compile(t_coder *coder);
int						try_debug(t_coder *coder);
int						try_refactor(t_coder *coder);
void					*coder_jrney(void *arg);
int						sim_is_stopped(t_codex *codex);
void					wakeup_thread(t_codex *codex);
int						burnout_handle(t_codex *codex, long last_compile_locked,
							int c_done, int i);
int						coders_are_done(t_codex *codex);
void					*monitor_journey(void *arg);
int						priority_coder(t_coder	*a, t_coder	*b);
void					swap_coder(t_coder **a, t_coder **b);
void					sift_up(t_heap *heap);
void					sift_down(t_heap *heap);
t_coder					*heap_pop(t_heap *heap);
void					heap_push(t_heap *heap, t_coder *coder);
int						heap_creation(t_heap *heap);
void					free_heap(t_heap *heap);
t_coder					*is_coder_ready(t_codex *codex);
void					printer(t_coder *coder);
int						both_dongles_permission(t_coder *coder);
int						is_sim_stopped(void);
void					caller(pthread_t *monitor_thread,
							pthread_t **coder_threads, int flag, int created);

#endif
