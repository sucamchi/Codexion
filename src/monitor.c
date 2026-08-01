/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scamlett <scamlett@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 18:05:55 by scamlett          #+#    #+#             */
/*   Updated: 2026/08/01 17:11:06 by scamlett         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// Wake every coder waiting on a dongle so they can notice the stop flag.

static void	monitor_wake_all(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->config.num_coders)
	{
		pthread_mutex_lock(&sim->dongles[i].mutex);
		pthread_cond_broadcast(&sim->dongles[i].cond);
		pthread_mutex_unlock(&sim->dongles[i].mutex);
		i++;
	}
}

// Check the simulation state and report if finished or if someone burned out.

static int	monitor_check_state(t_sim *sim)
{
	int	i;
	int	now;

	i = 0;
	while (i < sim->config.num_coders)
	{
		if (sim->coders[i].compiles_done < sim->config.num_compiles_required)
			break ;
		i++;
	}
	if (i == sim->config.num_coders)
		return (-1);
	now = get_time_ms(sim);
	i = 0;
	while (i < sim->config.num_coders)
	{
		if (sim->coders[i].compiles_done < sim->config.num_compiles_required
			&& sim->coders[i].last_compile_start >= 0 && now
			- sim->coders[i].last_compile_start > sim->config.time_to_burnout)
			return (sim->coders[i].id);
		i++;
	}
	return (0);
}

// Set the shared stop flag while holding the simulation mutex.

static void	set_stop(t_sim *sim)
{
	pthread_mutex_lock(&sim->sim_mutex);
	sim->stop = 1;
	pthread_mutex_unlock(&sim->sim_mutex);
}

// Join the start barrier and wait until main releases the simulation.

static void	monitor_wait_start(t_sim *sim)
{
	pthread_mutex_lock(&sim->sim_mutex);
	sim->ready_count++;
	if (sim->ready_count == sim->config.num_coders + 1)
		pthread_cond_broadcast(&sim->start_cond);
	while (!sim->started)
		pthread_cond_wait(&sim->start_cond, &sim->sim_mutex);
	pthread_mutex_unlock(&sim->sim_mutex);
}

// Monitor thread loop: wait for the start signal, then watch for burnout
// or completion.
void	*monitor_run(void *arg)
{
	t_sim	*sim;
	int		id;

	sim = (t_sim *)arg;
	monitor_wait_start(sim);
	while (!sim->stop)
	{
		id = monitor_check_state(sim);
		if (id < 0)
		{
			set_stop(sim);
			break ;
		}
		if (id > 0)
		{
			set_stop(sim);
			log_msg(sim, "burned out", id);
			monitor_wake_all(sim);
			return (NULL);
		}
		usleep(1000);
	}
	monitor_wake_all(sim);
	return (NULL);
}
