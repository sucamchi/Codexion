/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scamlett <scamlett@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 18:05:03 by scamlett          #+#    #+#             */
/*   Updated: 2026/08/02 13:00:29 by scamlett         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// Initialize a dongle's mutex, condition variable, and waiting queue.
int	dongle_init(t_dongle *d)
{
	memset(d, 0, sizeof(*d));
	if (pthread_mutex_init(&d->mutex, NULL) != 0)
		return (-1);
	if (pthread_cond_init(&d->cond, NULL) != 0)
	{
		pthread_mutex_destroy(&d->mutex);
		return (-1);
	}
	if (heap_init(&d->wait_queue) != 0)
	{
		pthread_cond_destroy(&d->cond);
		pthread_mutex_destroy(&d->mutex);
		return (-1);
	}
	return (0);
}

// Sleep on the dongle's condition variable until the coder can take it.
// Uses `pthread_cond_timedwait` if the dongle is cooling down.

static void	dongle_do_wait(t_dongle *d, int first, int cid, t_sim *sim)
{
	struct timespec	ts;

	if (first == cid && d->taken_by == 0
		&& d->cooldown_until > get_time_ms(sim))
	{
		ts.tv_sec = d->cooldown_until / 1000;
		ts.tv_nsec = (d->cooldown_until % 1000) * 1000000;
		pthread_cond_timedwait(&d->cond, &d->mutex, &ts);
	}
	else
		pthread_cond_wait(&d->cond, &d->mutex);
}

// Lock the dongle mutex, add the coder to the waiting queue,
// and wait until the coder can take the dongle.

int	dongle_take(t_dongle *d, int cid, int prio, t_sim *sim)
{
	int	first;

	pthread_mutex_lock(&d->mutex);
	heap_push(&d->wait_queue, cid, prio);
	heap_peek(&d->wait_queue, &first, NULL);
	if (first == cid)
		pthread_cond_broadcast(&d->cond);
	while (!sim->stop)
	{
		heap_peek(&d->wait_queue, &first, NULL);
		if (first == cid && d->taken_by == 0
			&& get_time_ms(sim) >= d->cooldown_until)
			break ;
		dongle_do_wait(d, first, cid, sim);
	}
	if (!sim->stop)
	{
		heap_pop(&d->wait_queue);
		d->taken_by = cid;
	}
	pthread_mutex_unlock(&d->mutex);
	return (!sim->stop);
}

// Mark the dongle free, start its cooldown, and wake every waiting coder.

void	dongle_release(t_dongle *d, t_sim *sim)
{
	pthread_mutex_lock(&d->mutex);
	d->taken_by = 0;
	d->cooldown_until = get_time_ms(sim) + sim->config.dongle_cooldown;
	pthread_cond_broadcast(&d->cond);
	pthread_mutex_unlock(&d->mutex);
}

// Destroy the dongle's mutex, condition variable, and heap storage.

void	dongle_cleanup(t_dongle *d)
{
	pthread_cond_destroy(&d->cond);
	pthread_mutex_destroy(&d->mutex);
	heap_clear(&d->wait_queue);
}
