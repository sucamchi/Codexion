/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scamlett <scamlett@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 18:06:11 by scamlett          #+#    #+#             */
/*   Updated: 2026/07/24 14:53:24 by scamlett         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// Return elapsed time in milliseconds since simulation start

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

// Precise sleep using usleep polling to avoid drift

void	ms_sleep(int ms, t_sim *sim)
{
	int	start;

	start = get_time_ms(sim);
	while (get_time_ms(sim) - start < ms)
		usleep(500);
}

// Peek at the minimum element without removing it

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

// Free the internal data array and reset the heap

void	heap_clear(t_heap *h)
{
	if (h->data)
		free(h->data);
	h->data = NULL;
	h->size = 0;
	h->capacity = 0;
}
