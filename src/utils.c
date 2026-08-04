/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scamlett <scamlett@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/24 18:06:11 by scamlett          #+#    #+#             */
/*   Updated: 2026/08/04 14:09:17 by scamlett         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// Return current time in milliseconds

long	get_time_ms(void)
{
	struct timeval	now;

	gettimeofday(&now, NULL);
	return ((long)now.tv_sec * 1000 + now.tv_usec / 1000);
}

// Sleep for ms milliseconds, checking sim->stop every 500 µs so
// the coder wakes up early when the monitor signals a burnout.

void	ms_sleep(long ms, t_sim *sim)
{
	long	start;

	start = get_time_ms();
	while (!sim->stop && get_time_ms() - start < ms)
		usleep(500);
}

// Thread-safe log: print relative timestamp, coder ID, and message.
// The log mutex ensures that messages from different threads don't interleave.

void	log_msg(t_sim *sim, char *msg, int coder_id)
{
	pthread_mutex_lock(&sim->log_mutex);
	printf("%ld %d %s\n", get_time_ms() - sim->base_time_ms, coder_id, msg);
	pthread_mutex_unlock(&sim->log_mutex);
}
