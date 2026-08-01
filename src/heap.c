/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scamlett <scamlett@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 18:05:36 by scamlett          #+#    #+#             */
/*   Updated: 2026/08/01 12:13:41 by scamlett         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// Allocate storage for a min-heap with the requested capacity.

int	heap_init(t_heap *h, int capacity)
{
	h->data = malloc(sizeof(t_heap_elem) * (unsigned int)capacity);
	if (!h->data)
		return (-1);
	h->size = 0;
	h->capacity = capacity;
	return (0);
}

// Move a newly inserted element upward until the heap order is restored.

static void	heap_sift_up(t_heap *h, int i)
{
	int			p;
	t_heap_elem	tmp;

	while (i > 0)
	{
		p = (i - 1) / 2;
		if (h->data[i].priority >= h->data[p].priority)
			break ;
		tmp = h->data[i];
		h->data[i] = h->data[p];
		h->data[p] = tmp;
		i = p;
	}
}

// Move the root element downward after a pop until the heap order is restored.

static void	heap_sift_down(t_heap *h, t_heap_elem el, int i)
{
	int	c;

	while (1)
	{
		c = 2 * i + 1;
		if (c >= h->size)
			break ;
		if (c + 1 < h->size && h->data[c + 1].priority
			< h->data[c].priority)
			c++;
		if (el.priority <= h->data[c].priority)
			break ;
		h->data[i] = h->data[c];
		i = c;
	}
	h->data[i] = el;
}

// Insert one coder into the priority queue.

int	heap_push(t_heap *h, int coder_id, int priority)
{
	if (h->size >= h->capacity)
		return (-1);
	h->data[h->size].coder_id = coder_id;
	h->data[h->size].priority = priority;
	heap_sift_up(h, h->size);
	h->size++;
	return (0);
}

// Remove the coder with the smallest priority from the queue.

int	heap_pop(t_heap *h)
{
	t_heap_elem	el;

	if (h->size == 0)
		return (-1);
	h->size--;
	el = h->data[h->size];
	heap_sift_down(h, el, 0);
	return (0);
}
