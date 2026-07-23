/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scamlett <scamlett@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 18:04:15 by scamlett          #+#    #+#             */
/*   Updated: 2026/07/23 18:17:27 by scamlett         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include <unistd.h>

// Scheduler constants

# define FIFO 0
# define EDF 1

// Heap element for the priority queue

typedef struct s_heap_elem
{
	int					coder_id;
	long				priority;
}						t_heap_elem;

// Priority queue (min-heap) for FIFO and EDF scheduling
// - FIFO: priority = arrival timestamp (earlier = higher priority)
// - EDF:  priority = burnout deadline (earlier = higher priority)

typedef struct s_heap
{
	t_heap_elem			*data;
	int					size;
	int					capacity;
	int					(*cmp)(long a, long b);
}						t_heap;

// Dongle: shared resource between two adjacent coders
// - taken_by: 0 = free, > 0 = coder ID holding it
// - cooldown_until: ms timestamp until cooldown expires

typedef struct s_dongle
{
	int					id;
	int					taken_by;
	long				cooldown_until;
	pthread_mutex_t		mutex;
	pthread_cond_t		cond;
	t_heap				wait_queue;
}						t_dongle;

// Parsed command-line configuration

typedef struct s_config
{
	int					num_coders;
	int					time_to_burnout;
	int					time_to_compile;
	int					time_to_debug;
	int					time_to_refactor;
	int					num_compiles_required;
	int					dongle_cooldown;
	int					scheduler;
}						t_config;

// Forward declarations for circular dependencies

typedef struct s_coder	t_coder;
typedef struct s_sim	t_sim;

// Coder: represented by a pthread
// - left_dongle / right_dongle: the two dongles this coder uses
// - last_compile_start: timestamp (ms) of most recent compile start

struct s_coder
{
	int					id;
	int					compiles_done;
	long				last_compile_start;
	pthread_t			thread;
	t_dongle			*left_dongle;
	t_dongle			*right_dongle;
	t_sim				*sim;
};

// Simulation state
// - start_time: reference timestamp for relative logging (ms)
// - stop: set by monitor when burnout occurs or all coders are done
// - sim_mutex: protects stop, ready_count
// - log_mutex: serializes log output
// - start_cond: synchronizes thread start

struct					s_sim
{
	t_config			config;
	t_dongle			*dongles;
	t_coder				*coders;
	long				start_time;
	int					stop;
	int					ready_count;
	pthread_mutex_t		log_mutex;
	pthread_mutex_t		sim_mutex;
	pthread_cond_t		start_cond;
	pthread_t			monitor_thread;
};

// heap.c

int						heap_init(t_heap *h, int capacity, int (*cmp)(long a,
								long b));
int						heap_push(t_heap *h, int coder_id, long priority);
int						heap_pop(t_heap *h);
int						heap_peek(t_heap *h, int *coder_id, long *priority);
void					heap_clear(t_heap *h);

// dongle.c

int						dongle_init(t_dongle *d, int id, int capacity,
							int (*cmp)(long a, long b));
int						dongle_take(t_dongle *d, int coder_id, long priority,
							t_sim *sim);
void					dongle_release(t_dongle *d, t_sim *sim);
void					dongle_cleanup(t_dongle *d);

// coder.c

void					*coder_lifecycle(void *arg);

// monitor.c

void					*monitor_run(void *arg);

// log.c

void					log_init(t_sim *sim);
void					log_msg(t_sim *sim, char *msg, int coder_id);
void					log_cleanup(t_sim *sim);

// utils.c

long					get_time_ms(void);
void					ms_sleep(long ms);
int						str_to_int(char *s, int *out);
int						cmp_min(long a, long b);

// main.c

int						parse_args(int ac, char **av, t_config *cfg);
int						init_sim(t_sim *sim, int (*cmp)(long, long));
void					cleanup_sim(t_sim *sim);

#endif
