/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scamlett <scamlett@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 18:05:03 by scamlett          #+#    #+#             */
/*   Updated: 2026/07/23 18:05:03 by scamlett         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// Initialise a dongle: mutex, condition variable, and wait-queue heap

int	dongle_init(t_dongle *d, int id, int cap, int (*cmp)(long a, long b))
{
	d->id = id;
	d->taken_by = 0;
	d->cooldown_until = 0;
	if (pthread_mutex_init(&d->mutex, NULL) != 0)
		return (-1);
	if (pthread_cond_init(&d->cond, NULL) != 0)
	{
		pthread_mutex_destroy(&d->mutex);
		return (-1);
	}
	if (heap_init(&d->wait_queue, cap, cmp) != 0)
	{
		pthread_cond_destroy(&d->cond);
		pthread_mutex_destroy(&d->mutex);
		return (-1);
	}
	return (0);
}

// Check if dongle can be taken: not held and cooldown has expired

static int	dongle_is_available(t_dongle *d, long now)
{
	return (d->taken_by == 0 && now >= d->cooldown_until);
}

// Wait helper: if at front, timedwait until cooldown; else plain cond_wait

static void	dongle_do_wait(t_dongle *d, int at_front, long cooldown_end)
{
	struct timespec	ts;

	if (!at_front)
	{
		pthread_cond_wait(&d->cond, &d->mutex);
		return ;
	}
	ts.tv_sec = cooldown_end / 1000;
	ts.tv_nsec = (cooldown_end % 1000) * 1000000;
	pthread_cond_timedwait(&d->cond, &d->mutex, &ts);
}

// Acquire dongle: join wait queue, wait until at front and available.
// Returns 1 on success, 0 if simulation stopped.

int	dongle_take(t_dongle *d, int cid, long prio, t_sim *sim)
{
	int	fid;

	pthread_mutex_lock(&d->mutex);
	heap_push(&d->wait_queue, cid, prio);
	heap_peek(&d->wait_queue, &fid, NULL);
	if (fid == cid)
		pthread_cond_broadcast(&d->cond);
	while (!sim->stop)
	{
		heap_peek(&d->wait_queue, &fid, NULL);
		if (fid == cid && dongle_is_available(d, get_time_ms()))
			break ;
		if (fid == cid && d->taken_by == 0 && d->cooldown_until > get_time_ms())
			dongle_do_wait(d, 1, d->cooldown_until);
		else
			pthread_cond_wait(&d->cond, &d->mutex);
	}
	if (!sim->stop)
	{
		heap_pop(&d->wait_queue);
		d->taken_by = cid;
	}
	pthread_mutex_unlock(&d->mutex);
	return (!sim->stop);
}

// Release dongle: mark free, set cooldown, wake up all waiters

void	dongle_release(t_dongle *d, t_sim *sim)
{
	pthread_mutex_lock(&d->mutex);
	d->taken_by = 0;
	d->cooldown_until = get_time_ms() + sim->config.dongle_cooldown;
	pthread_cond_broadcast(&d->cond);
	pthread_mutex_unlock(&d->mutex);
}
