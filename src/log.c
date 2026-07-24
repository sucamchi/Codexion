/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   log.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scamlett <scamlett@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 18:05:43 by scamlett          #+#    #+#             */
/*   Updated: 2026/07/24 14:53:02 by scamlett         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// Initialise the log mutex (called once at simulation start)

void	log_init(t_sim *sim)
{
	pthread_mutex_init(&sim->log_mutex, NULL);
}

// Thread-safe log: prints timestamp + coder ID + message
// Timestamp is relative to sim->start_time (ms)

void	log_msg(t_sim *sim, char *msg, int coder_id)
{
	pthread_mutex_lock(&sim->log_mutex);
	printf("%d %d %s\n", get_time_ms(sim), coder_id, msg);
	pthread_mutex_unlock(&sim->log_mutex);
}

// Destroy the log mutex

void	log_cleanup(t_sim *sim)
{
	pthread_mutex_destroy(&sim->log_mutex);
}
