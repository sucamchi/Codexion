/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scamlett <scamlett@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 18:05:36 by scamlett          #+#    #+#             */
/*   Updated: 2026/08/04 13:52:03 by scamlett         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// Initialize the heap with capacity for two coders.
// Each dongle is shared by exactly two neighbors and a coder only re-queues
// after being popped, so the waiting queue never exceeds 2 entries.

int	heap_init(t_heap *h)
{
	h->data = malloc(sizeof(t_heap_elem) * 2);
	if (!h->data)
		return (-1);
	h->size = 0;
	return (0);
}

// Insert one coder into the heap with its priority.
// With at most 2 elements, a single swap keeps the min at the root.

int	heap_push(t_heap *h, int coder_id, long priority)
{
	t_heap_elem	tmp;

	h->data[h->size].coder_id = coder_id;
	h->data[h->size].priority = priority;
	if (h->size == 1 && h->data[1].priority < h->data[0].priority)
	{
		tmp = h->data[0];
		h->data[0] = h->data[1];
		h->data[1] = tmp;
	}
	h->size++;
	return (0);
}

// Pop the coder with the smallest priority from the queue.
// With at most 2 elements, the remaining entry is already at the root.

int	heap_pop(t_heap *h)
{
	if (h->size == 0)
		return (-1);
	h->size--;
	return (0);
}

// Read the top heap element without removing it.
// Called by dongle_take to check if the caller is the first in line.

int	heap_peek(t_heap *h, int *coder_id)
{
	if (h->size == 0)
		return (-1);
	if (coder_id)
		*coder_id = h->data[0].coder_id;
	return (0);
}
