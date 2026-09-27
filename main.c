/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: limelo-c <limelo-c@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/19 15:55:00 by limelo-c          #+#    #+#             */
/*   Updated: 2026/09/19 15:55:00 by limelo-c         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	parser_valid(int argc, char **argv)
{
	if (argc != 9)
		return (printf("wrong number of arguments\n"), 1);
	if (parser_validator(argv) == 1 || parser_last_validator(argv) == 1)
		return (1);
	return (0);
}

static int	init_sim(void)
{
	int	i;

	pthread_mutex_init(&codex_return()->mutex_sim, NULL);
	pthread_mutex_init(&codex_return()->mutex_sched, NULL);
	if (dongles_init() == -1 || coders_init() == -1)
		return (printf("Memory allocation issue\n"), 1);
	codex_return()->start_time = timeofday_converter();
	i = 0;
	while (i < codex_return()->number_of_coders)
	{
		pthread_mutex_lock(&codex_return()->coders[i].mutex);
		codex_return()->coders[i].last_compile = codex_return()->start_time;
		pthread_mutex_unlock(&codex_return()->coders[i].mutex);
		i++;
	}
	return (0);
}

static int	create_coder_threads(pthread_t *coder_thrds, int *created)
{
	int	i;

	i = 0;
	while (i < codex_return()->number_of_coders)
	{
		if (pthread_create(&coder_thrds[i], NULL, coder_jrney,
				&codex_return()->coders[i]))
		{
			*created = i;
			return (1);
		}
		i++;
	}
	return (0);
}

static void	wait_coders(pthread_t *coder_thrds)
{
	int	i;

	i = 0;
	while (i < codex_return()->number_of_coders)
	{
		pthread_join(coder_thrds[i], NULL);
		i++;
	}
}

int	main(int argc, char **argv)
{
	pthread_t	*coder_thrds;
	pthread_t	monitor_thread;
	int			created;

	if (parser_valid(argc, argv) != 0)
		return (1);
	if (init_sim() != 0)
		return (1);
	coder_thrds = malloc(codex_return()->number_of_coders * sizeof(pthread_t));
	if (!coder_thrds)
		return (printf("Memory allocation issue\n"), 1);
	if (pthread_create(&monitor_thread, NULL, monitor_journey, NULL) != 0)
		return (caller(&monitor_thread, &coder_thrds, 0, -1), 1);
	created = 0;
	if (create_coder_threads(coder_thrds, &created) != 0)
	{
		caller(&monitor_thread, &coder_thrds, 0, created);
		return (1);
	}
	wait_coders(coder_thrds);
	caller(&monitor_thread, &coder_thrds, 1, 0);
	return (0);
}
