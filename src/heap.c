/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scamlett <scamlett@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 18:05:36 by scamlett          #+#    #+#             */
/*   Updated: 2026/07/23 18:05:37 by scamlett         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// Initialise a min-heap with given capacity and comparison function

int	heap_init(t_heap *h, int capacity, int (*cmp)(long a, long b))
{
	h->data = malloc(sizeof(t_heap_elem) * (unsigned long)capacity);
	if (!h->data)
		return (-1);
	h->size = 0;
	h->capacity = capacity;
	h->cmp = cmp;
	return (0);
}

// Sift-up: bubble element at index up to restore heap property

static void	heap_sift_up(t_heap *h, int i)
{
	int			p;
	t_heap_elem	tmp;

	while (i > 0)
	{
		p = (i - 1) / 2;
		if (h->cmp(h->data[i].priority, h->data[p].priority) >= 0)
			break ;
		tmp = h->data[i];
		h->data[i] = h->data[p];
		h->data[p] = tmp;
		i = p;
	}
}

// Sift-down: bubble element from index down to restore heap property

static void	heap_sift_down(t_heap *h, t_heap_elem el, int i)
{
	int	c;

	while (1)
	{
		c = 2 * i + 1;
		if (c >= h->size)
			break ;
		if (c + 1 < h->size && h->cmp(h->data[c + 1].priority,
				h->data[c].priority) < 0)
			c++;
		if (h->cmp(el.priority, h->data[c].priority) <= 0)
			break ;
		h->data[i] = h->data[c];
		i = c;
	}
	h->data[i] = el;
}

// Push element onto heap. Returns 0 on success, -1 if full.

int	heap_push(t_heap *h, int coder_id, long priority)
{
	if (h->size >= h->capacity)
		return (-1);
	h->data[h->size].coder_id = coder_id;
	h->data[h->size].priority = priority;
	heap_sift_up(h, h->size);
	h->size++;
	return (0);
}

// Pop minimum element from heap. Use heap_peek first to read the value.

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
