/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sim.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scamlett <scamlett@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 18:06:02 by scamlett          #+#    #+#             */
/*   Updated: 2026/07/23 18:06:03 by scamlett         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// Allocate and initialise all dongles

static int	init_dongles(t_sim *sim, int (*cmp)(long, long))
{
	int	i;
	int	n;

	n = sim->config.num_coders;
	sim->dongles = malloc(sizeof(t_dongle) * (unsigned long)n);
	if (!sim->dongles)
		return (-1);
	i = 0;
	while (i < n)
	{
		if (dongle_init(&sim->dongles[i], i + 1, n, cmp) != 0)
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

// Allocate and initialise coder structs, assign left/right dongles.
// Coder i uses dongles[i] (left) and dongles[(i+1)%n] (right).
// For 1 coder, left == right (same dongle, takes it once).

static int	init_coders(t_sim *sim)
{
	int	i;
	int	n;

	n = sim->config.num_coders;
	sim->coders = malloc(sizeof(t_coder) * (unsigned long)n);
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

// Initialise simulation state. sim->config must be set beforehand.

int	init_sim(t_sim *sim, int (*cmp)(long, long))
{
	sim->dongles = NULL;
	sim->coders = NULL;
	sim->stop = 0;
	sim->ready_count = 0;
	sim->start_time = -1;
	if (init_dongles(sim, cmp) != 0)
		return (-1);
	if (init_coders(sim) != 0)
	{
		free(sim->dongles);
		sim->dongles = NULL;
		return (-1);
	}
	pthread_mutex_init(&sim->log_mutex, NULL);
	pthread_mutex_init(&sim->sim_mutex, NULL);
	pthread_cond_init(&sim->start_cond, NULL);
	return (0);
}

// Free all simulation resources

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
	log_cleanup(sim);
}
