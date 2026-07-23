/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: scamlett <scamlett@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 18:06:11 by scamlett          #+#    #+#             */
/*   Updated: 2026/07/23 18:06:12 by scamlett         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// Return current time in milliseconds since epoch

long	get_time_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((long)tv.tv_sec * 1000 + (long)tv.tv_usec / 1000);
}

// Precise sleep using usleep polling to avoid drift

void	ms_sleep(long ms)
{
	long	start;

	start = get_time_ms();
	while (get_time_ms() - start < ms)
		usleep(500);
}

// Convert string to positive integer
// Returns 0 on success (value in *out), -1 on invalid input

int	str_to_int(char *s, int *out)
{
	long	n;
	int		i;

	i = 0;
	n = 0;
	if (!s || s[0] == '\0')
		return (-1);
	while (s[i] >= '0' && s[i] <= '9')
	{
		n = n * 10 + (s[i] - '0');
		if (n > 2147483647L)
			return (-1);
		i++;
	}
	if (s[i] != '\0')
		return (-1);
	*out = (int)n;
	return (0);
}

// Min-heap comparator: negative if a < b, 0 if equal, positive if a > b

int	cmp_min(long a, long b)
{
	if (a < b)
		return (-1);
	if (a > b)
		return (1);
	return (0);
}
