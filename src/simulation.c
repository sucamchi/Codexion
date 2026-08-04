/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scamlett <scamlett@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 18:06:02 by scamlett          #+#    #+#             */
/*   Updated: 2026/08/04 13:38:55 by scamlett         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// Allocate and initialize every dongle, cleaning up if one fails.

static int	init_dongles(t_sim *sim)
{
	int	i;
	int	n;

	n = sim->config.num_coders;
	sim->dongles = malloc(sizeof(t_dongle) * (unsigned int)n);
	if (!sim->dongles)
		return (-1);
	i = 0;
	while (i < n)
	{
		if (dongle_init(&sim->dongles[i]) != 0)
		{
			while (--i >= 0)
				dongle_cleanup(&sim->dongles[i]);
			free(sim->dongles);
			sim->dongles = NULL;
			return (-1);
		}
		i++;
	}
	return (0);
}

// Allocate coders and assign each coder its left and right dongle.
// For one coder, both pointers refer to the same dongle.

static int	init_coders(t_sim *sim)
{
	int	i;
	int	n;

	n = sim->config.num_coders;
	sim->coders = malloc(sizeof(t_coder) * (unsigned int)n);
	if (!sim->coders)
		return (-1);
	i = 0;
	while (i < n)
	{
		sim->coders[i].id = i + 1;
		sim->coders[i].compiles_done = 0;
		sim->coders[i].last_compile_start = -1;
		sim->coders[i].sim = sim;
		sim->coders[i].left_dongle = &sim->dongles[i];
		if (n == 1)
			sim->coders[i].right_dongle = &sim->dongles[0];
		else
			sim->coders[i].right_dongle = &sim->dongles[(i + 1) % n];
		i++;
	}
	return (0);
}

// Free all dongles and their mutexes/condvars, then free the array.
// This is called when coder initialization fails, to avoid memory leaks.

static void	free_dongles(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->config.num_coders)
	{
		dongle_cleanup(&sim->dongles[i]);
		i++;
	}
	free(sim->dongles);
	sim->dongles = NULL;
}

// Set the initial simulation state and create the shared sync objects.

int	init_sim(t_sim *sim)
{
	gettimeofday(&sim->base_time, NULL);
	if (init_dongles(sim) != 0)
		return (-1);
	if (init_coders(sim) != 0)
	{
		free_dongles(sim);
		return (-1);
	}
	pthread_mutex_init(&sim->log_mutex, NULL);
	pthread_mutex_init(&sim->sim_mutex, NULL);
	pthread_cond_init(&sim->start_cond, NULL);
	return (0);
}

// Join all threads and free the simulation state.
// Cleans up memory, mutexes and logging.

void	cleanup_sim(t_sim *sim)
{
	int	i;
	int	n;

	n = sim->config.num_coders;
	i = 0;
	while (i < n)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
	pthread_join(sim->monitor_thread, NULL);
	i = 0;
	while (i < n)
	{
		dongle_cleanup(&sim->dongles[i]);
		i++;
	}
	free(sim->dongles);
	free(sim->coders);
	pthread_cond_destroy(&sim->start_cond);
	pthread_mutex_destroy(&sim->sim_mutex);
	pthread_mutex_destroy(&sim->log_mutex);
}
