/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scamlett <scamlett@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 18:06:11 by scamlett          #+#    #+#             */
/*   Updated: 2026/08/01 14:37:43 by scamlett         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// Return the number of milliseconds since the simulation started.

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

// Sleep for approximately the requested number of milliseconds.

void	ms_sleep(int ms, t_sim *sim)
{
	int	start;

	start = get_time_ms(sim);
	while (!sim->stop && get_time_ms(sim) - start < ms)
		usleep(500);
}

// Read the top heap element without removing it.

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

// Free the heap storage and reset its fields.

void	heap_clear(t_heap *h)
{
	if (h->data)
		free(h->data);
	h->data = NULL;
	h->size = 0;
	h->capacity = 0;
}
