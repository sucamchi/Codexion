/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scamlett <scamlett@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 18:04:15 by scamlett          #+#    #+#             */
/*   Updated: 2026/08/02 13:58:20 by scamlett         ###   ########.fr       */
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

# define FIFO 0
# define EDF 1

// Heap element for the priority queue
// - coder_id: the ID of the coder waiting for the dongle
// - priority: the priority value (lower = higher priority)

typedef struct s_heap_elem
{
	int					coder_id;
	int					priority;
}						t_heap_elem;

// Heap structure for the priority queue
// - data: array of heap elements
// - size: current number of elements in the heap

typedef struct s_heap
{
	t_heap_elem			*data;
	int					size;
}						t_heap;

// Dongle: shared resource between two adjacent coders
// - taken_by: 0 = free, > 0 = coder ID holding it
// - cooldown_until: ms timestamp until cooldown expires
// - mutex: protects access to the dongle state
// - cond: condition variable for waiting coders
// - wait_queue: priority queue of waiting coders

typedef struct s_dongle
{
	int					taken_by;
	int					cooldown_until;
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
// - t_coder and t_sim reference each other, so we declare them first.

typedef struct s_coder	t_coder;
typedef struct s_sim	t_sim;

// Coder: represented by a pthread
// - id: unique identifier
// - compiles_done: number of completed compile cycles
// - last_compile_start: timestamp of last compile start (ms)
// - thread: pthread handle for the coder
// - left_dongle: pointer to the left dongle
// - right_dongle: pointer to the right dongle
// - sim: pointer to the shared simulation state

struct					s_coder
{
	int					id;
	int					compiles_done;
	int					last_compile_start;
	pthread_t			thread;
	t_dongle			*left_dongle;
	t_dongle			*right_dongle;
	t_sim				*sim;
};

// Simulation state
// - config: parsed command-line configuration
// - dongles: array of dongles (size = num_coders)
// - coders: array of coders (size = num_coders)
// - base_time: timestamp when the simulation started
// - started: flag indicating if the simulation has started
// - stop: flag indicating if the simulation should stop
// - ready_count: number of coders ready to start
// - log_mutex: mutex for logging
// - sim_mutex: mutex for shared simulation state
// - start_cond: condition variable for starting the simulation
// - monitor_thread: pthread handle for the monitor thread

struct					s_sim
{
	t_config			config;
	t_dongle			*dongles;
	t_coder				*coders;
	struct timeval		base_time;
	int					started;
	int					stop;
	int					ready_count;
	pthread_mutex_t		log_mutex;
	pthread_mutex_t		sim_mutex;
	pthread_cond_t		start_cond;
	pthread_t			monitor_thread;
};

// heap.c

int						heap_init(t_heap *h);
int						heap_push(t_heap *h, int coder_id, int priority);
int						heap_pop(t_heap *h);
int						heap_peek(t_heap *h, int *coder_id, int *priority);

// dongle.c

int						dongle_init(t_dongle *d);
int						dongle_take(t_dongle *d, int coder_id, int priority,
							t_sim *sim);
void					dongle_release(t_dongle *d, t_sim *sim);
void					dongle_cleanup(t_dongle *d);

// coder.c

void					*coder_lifecycle(void *arg);

// monitor.c

void					*monitor_run(void *arg);

// utils.c

void					sim_wait_start(t_sim *sim);
int						get_time_ms(t_sim *sim);
void					ms_sleep(int ms, t_sim *sim);
void					log_msg(t_sim *sim, char *msg, int coder_id);

// main.c

int						parse_args(int ac, char **av, t_config *cfg);
int						init_sim(t_sim *sim);
void					cleanup_sim(t_sim *sim);

#endif
