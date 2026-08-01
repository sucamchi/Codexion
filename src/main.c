/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scamlett <scamlett@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 18:05:49 by scamlett          #+#    #+#             */
/*   Updated: 2026/08/01 21:42:26 by scamlett         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// Check that a string is a valid non-negative integer and at least 'min'.
// Returns -1 on error, 0 on success.

static int	is_valid_int(char *s, int *val, int min)
{
	int	i;

	i = 0;
	if (!s || s[0] == '\0')
		return (-1);
	while (s[i])
	{
		if (s[i] < '0' || s[i] > '9')
			return (-1);
		i++;
	}
	*val = atoi(s);
	if (*val < min)
		return (-1);
	return (0);
}

// Convert the scheduler name into the internal FIFO or EDF value.

static int	parse_scheduler(char *s)
{
	if (strcmp(s, "fifo") == 0)
		return (FIFO);
	if (strcmp(s, "edf") == 0)
		return (EDF);
	return (-1);
}

// Read and validate all command-line arguments into the config struct.

int	parse_args(int argc, char **argv, t_config *cfg)
{
	if (argc != 9)
		return (-1);
	if (is_valid_int(argv[1], &cfg->num_coders, 1))
		return (-1);
	if (is_valid_int(argv[2], &cfg->time_to_burnout, 1))
		return (-1);
	if (is_valid_int(argv[3], &cfg->time_to_compile, 1))
		return (-1);
	if (is_valid_int(argv[4], &cfg->time_to_debug, 1))
		return (-1);
	if (is_valid_int(argv[5], &cfg->time_to_refactor, 1))
		return (-1);
	if (is_valid_int(argv[6], &cfg->num_compiles_required, 1))
		return (-1);
	if (is_valid_int(argv[7], &cfg->dongle_cooldown, 0))
		return (-1);
	cfg->scheduler = parse_scheduler(argv[8]);
	if (cfg->scheduler < 0)
		return (-1);
	return (0);
}

// Start every thread, wait for the barrier, then release them all.

static void	launch_threads(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->config.num_coders)
	{
		pthread_create(&sim->coders[i].thread, NULL, coder_lifecycle,
			&sim->coders[i]);
		i++;
	}
	pthread_create(&sim->monitor_thread, NULL, monitor_run, sim);
	pthread_mutex_lock(&sim->sim_mutex);
	while (sim->ready_count < sim->config.num_coders + 1)
		pthread_cond_wait(&sim->start_cond, &sim->sim_mutex);
	sim->started = 1;
	pthread_cond_broadcast(&sim->start_cond);
	pthread_mutex_unlock(&sim->sim_mutex);
}

// Program entry point: parse input, initialize the simulation, run it,
// and clean up.

int	main(int argc, char **argv)
{
	t_sim	sim;

	if (parse_args(argc, argv, &sim.config) != 0)
	{
		fprintf(stderr, "invalid arguments\n");
		return (1);
	}
	if (init_sim(&sim) != 0)
	{
		fprintf(stderr, "Error: memory allocation failed\n");
		return (1);
	}
	launch_threads(&sim);
	cleanup_sim(&sim);
	return (0);
}
