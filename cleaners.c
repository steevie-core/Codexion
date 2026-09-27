/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cleaners.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: limelo-c <limelo-c@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/25 23:43:49 by limelo-c            #+#    #+#           */
/*   Updated: 2026/09/25 23:43:49 by limelo-c           ###   ########.fr     */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

t_codex	*codex_return(void)
{
	static t_codex	codex;

	return (&codex);
}

static void	stop_and_join(pthread_t *monitor_thread,
							pthread_t *coder_threads, int created)
{
	int	i;

	pthread_mutex_lock(&codex_return()->mutex_sim);
	codex_return()->sim_stopped = 1;
	pthread_mutex_unlock(&codex_return()->mutex_sim);
	wakeup_thread(codex_return());
	i = 0;
	while (i < created)
	{
		pthread_join(coder_threads[i], NULL);
		i++;
	}
	pthread_join(*monitor_thread, NULL);
}

static void	cleanup(t_codex *codex)
{
	int	i;

	i = 0;
	while (i < codex->number_of_coders)
	{
		pthread_mutex_destroy(&codex->dongles[i].mutex);
		pthread_mutex_destroy(&codex->coders[i].mutex);
		pthread_cond_destroy(&codex->dongles[i].thread_sleep);
		i++;
	}
	free_heap(&codex->waiting_heap);
	pthread_mutex_destroy(&codex->mutex_sim);
	pthread_mutex_destroy(&codex->mutex_sched);
	free(codex->dongles);
	free(codex->coders);
}

void	caller(pthread_t *monitor_thread, pthread_t **coder_threads,
				int flag, int created)
{
	if (flag == 1)
		pthread_join(*monitor_thread, NULL);
	else if (created >= 0)
		stop_and_join(monitor_thread, *coder_threads, created);
	free(*coder_threads);
	cleanup(codex_return());
}
