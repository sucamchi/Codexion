/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   log.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scamlett <scamlett@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 18:05:43 by scamlett          #+#    #+#             */
/*   Updated: 2026/08/01 12:15:35 by scamlett         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// Initialize the mutex that protects console output.

void	log_init(t_sim *sim)
{
	pthread_mutex_init(&sim->log_mutex, NULL);
}

// Print one event with the elapsed time and coder ID without mixing output.

void	log_msg(t_sim *sim, char *msg, int coder_id)
{
	pthread_mutex_lock(&sim->log_mutex);
	printf("%d %d %s\n", get_time_ms(sim), coder_id, msg);
	pthread_mutex_unlock(&sim->log_mutex);
}

// Destroy the mutex used for logging.

void	log_cleanup(t_sim *sim)
{
	pthread_mutex_destroy(&sim->log_mutex);
}
