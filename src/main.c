/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scamlett <scamlett@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 18:05:49 by scamlett          #+#    #+#             */
/*   Updated: 2026/07/25 13:57:47 by scamlett         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// Parse a single integer argument, checking it's at least 'min'
// Also checks that the string has no letters, no negative sign
// and is not empty. Returns -1 on error, 0 on success.

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

// Parse scheduler string: returns FIFO or EDF, -1 on error

static int	parse_scheduler(char *s)
{
	if (strcmp(s, "fifo") == 0)
		return (FIFO);
	if (strcmp(s, "edf") == 0)
		return (EDF);
	return (-1);
}

// Parse and validate all 8 command-line arguments

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

// Create all threads, wait for them to be ready, set start time

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

// Entry point: parse arguments, start simulation, launch threads, cleanup

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
