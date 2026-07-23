/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scamlett <scamlett@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 18:05:30 by scamlett          #+#    #+#             */
/*   Updated: 2026/07/23 18:05:31 by scamlett         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// Peek at the minimum element without removing it

int	heap_peek(t_heap *h, int *coder_id, long *priority)
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
