/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scamlett <scamlett@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/24 18:06:11 by scamlett          #+#    #+#             */
/*   Updated: 2026/08/02 15:49:52 by scamlett         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// Calculate the number of milliseconds since the simulation started.
// Used for timestamps and timeouts.

int	get_time_ms(t_sim *sim)
{
	struct timeval	now;
	int				sec;
	int				usec;

	gettimeofday(&now, NULL);
	sec = (int)(now.tv_sec - sim->base_time.tv_sec);
	usec = (int)(now.tv_usec - sim->base_time.tv_usec);
	if (usec < 0)
	{
		sec--;
		usec += 1000000;
	}
	return (sec * 1000 + usec / 1000);
}

// Sleep for ms milliseconds, checking sim->stop every 500 µs so
// the coder wakes up early when the monitor signals a burnout.

void	ms_sleep(int ms, t_sim *sim)
{
	int	start;

	start = get_time_ms(sim);
	while (!sim->stop && get_time_ms(sim) - start < ms)
		usleep(500);
}

// Thread-safe log: print timestamp, coder ID, and message.
// The log mutex ensures that messages from different threads don't interleave.

void	log_msg(t_sim *sim, char *msg, int coder_id)
{
	pthread_mutex_lock(&sim->log_mutex);
	printf("%d %d %s\n", get_time_ms(sim), coder_id, msg);
	pthread_mutex_unlock(&sim->log_mutex);
}

// Read the top heap element without removing it.
// Called by the monitor to check the priority of waiting coders.

int	heap_peek(t_heap *h, int *coder_id, int *priority)
{
	if (h->size == 0)
		return (-1);
	if (coder_id)
		*coder_id = h->data[0].coder_id;
	if (priority)
		*priority = h->data[0].priority;
	return (0);
}
