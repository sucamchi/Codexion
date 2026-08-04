/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scamlett <scamlett@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 18:03:54 by scamlett          #+#    #+#             */
/*   Updated: 2026/08/04 12:38:24 by scamlett         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// Choose the order for locking the two dongles.
// Even coders take the right dongle first; odd coders take the left one first.

static void	coder_pick_dongles(t_coder *c, t_dongle **first, t_dongle **second)
{
	if (c->id % 2 == 0)
	{
		*first = c->right_dongle;
		*second = c->left_dongle;
	}
	else
	{
		*first = c->left_dongle;
		*second = c->right_dongle;
	}
}

// Attempt to take both dongles if possible, otherwise wait.
// smaller prio value = higher priority.

static int	coder_take_dongles(t_coder *c, t_sim *sim)
{
	t_dongle	*first;
	t_dongle	*second;
	int			prio;

	if (sim->config.scheduler == EDF)
		prio = c->last_compile_start + sim->config.time_to_burnout;
	else
		prio = get_time_ms(sim);
	coder_pick_dongles(c, &first, &second);
	if (first == second)
	{
		if (!dongle_take(first, c->id, prio, sim))
			return (0);
		log_msg(sim, "has taken a dongle", c->id);
		log_msg(sim, "has taken a dongle", c->id);
		return (1);
	}
	if (!dongle_take(first, c->id, prio, sim))
		return (0);
	log_msg(sim, "has taken a dongle", c->id);
	if (!dongle_take(second, c->id, prio, sim))
		return (0);
	log_msg(sim, "has taken a dongle", c->id);
	return (1);
}

// Run one full work cycle: take dongles, compile, release, debug, and refactor.

static int	coder_do_cycle(t_coder *c, t_sim *sim)
{
	if (!coder_take_dongles(c, sim))
		return (0);
	c->last_compile_start = get_time_ms(sim);
	log_msg(sim, "is compiling", c->id);
	ms_sleep(sim->config.time_to_compile, sim);
	dongle_release(c->left_dongle, sim);
	if (c->left_dongle != c->right_dongle)
		dongle_release(c->right_dongle, sim);
	c->compiles_done++;
	if (sim->stop || c->compiles_done >= sim->config.num_compiles_required)
		return (1);
	log_msg(sim, "is debugging", c->id);
	ms_sleep(sim->config.time_to_debug, sim);
	if (sim->stop)
		return (1);
	log_msg(sim, "is refactoring", c->id);
	ms_sleep(sim->config.time_to_refactor, sim);
	return (1);
}

// Main loop for a coder thread: wait for start, then repeat cycles until done
// or until the monitor stops the simulation.

void	*coder_lifecycle(void *arg)
{
	t_coder	*c;
	t_sim	*sim;

	c = (t_coder *)arg;
	sim = c->sim;
	sim_wait_start(sim);
	c->last_compile_start = 0;
	while (!sim->stop && c->compiles_done < sim->config.num_compiles_required)
	{
		if (!coder_do_cycle(c, sim))
			break ;
	}
	return (NULL);
}
