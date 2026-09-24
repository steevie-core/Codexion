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

t_codex	*codex_return(void)
{
	static t_codex	codex;

	return (&codex);
}

static void	cleanup(t_codex *codex)
{
	int	i;

	i = 0;
	while (i < codex->number_of_coders)
	{
		free_heap(&codex->dongles[i].heap);
		pthread_mutex_destroy(&codex->dongles[i].mutex);
		pthread_mutex_destroy(&codex->coders[i].mutex);
		pthread_cond_destroy(&codex->dongles[i].thread_sleep);
		i++;
	}
	pthread_mutex_destroy(&codex->mutex_sim);
	free(codex->dongles);
	free(codex->coders);
}

static void	caller(pthread_t *monitor_thread,
		pthread_t **coder_threads, t_codex *codex)
{
	pthread_join(*monitor_thread, NULL);
	cleanup(codex);
	free(*coder_threads);
}

int	main(int argc, char **argv)
{
	pthread_t	*coder_thrds;
	pthread_t	monitor_thread;
	int			i;

	i = -1;
	pthread_mutex_init(&codex_return()->mutex_sim, NULL);
	if (argc != 9)
		return (printf("wrong number of arguments\n"), 1);
	if (parser_validator(argv) == 1 || parser_last_validator(argv) == 1)
		return (1);
	if (dongles_init() == -1 || coders_init() == -1)
		return (printf("Memory allocation issue\n"), 1);
	coder_thrds = malloc(codex_return()->number_of_coders * sizeof(pthread_t));
	if (!(coder_thrds))
		return (printf("Memory allocation issue\n"), 1);
	codex_return()->start_time = timeofday_converter();
	if (pthread_create(&monitor_thread, NULL, monitor_journey, NULL) != 0)
	{
		printf("Error creating monitor thread\n");
		free(coder_thrds);
		cleanup(codex_return());
		return (1);
	}
	while (++i < codex_return()->number_of_coders)
	{
		if (pthread_create(&coder_thrds[i], NULL,
				coder_jrney, &codex_return()->coders[i]) != 0)
		{
			printf("Error creating coder thread\n");
			pthread_mutex_lock(&codex_return()->mutex_sim);
			codex_return()->sim_stopped = 1;
			pthread_mutex_unlock(&codex_return()->mutex_sim);
			wakeup_thread(codex_return());
			while (--i >= 0)
				pthread_join(coder_thrds[i], NULL);
			pthread_join(monitor_thread, NULL);
			cleanup(codex_return());
			free(coder_thrds);
			return (1);
		}
	}
	i = -1;
	while (++i < codex_return()->number_of_coders)
		pthread_join(coder_thrds[i], NULL);
	caller(&monitor_thread, &coder_thrds, codex_return());
	return (0);
}
