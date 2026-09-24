/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_dongle.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: limelo-c <limelo-c@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/19 15:54:41 by limelo-c          #+#    #+#             */
/*   Updated: 2026/09/19 15:54:41 by limelo-c         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	dongle_order(t_coder *coder, int *first_dongle, int *second_dongle)
{
	if (coder->right_dongle < coder->left_dongle)
	{
		*first_dongle = coder->right_dongle;
		*second_dongle = coder->left_dongle;
	}
	else
	{
		*first_dongle = coder->left_dongle;
		*second_dongle = coder->right_dongle;
	}
}

int	wait_outcome(int i, t_codex *codex, t_coder *coder)
{
	if (sim_is_stopped(codex) == 1)
	{
		pthread_mutex_unlock(&codex->dongles[i].mutex);
		return (0);
	}
	else
	{
		codex->dongles[i].dongle_availability = taken;
		codex->dongles[i].heap.last_dgl_granted = coder->coder_id;
		heap_pop(&codex->dongles[i].heap);
		pthread_mutex_unlock(&codex->dongles[i].mutex);
		return (1);
	}
}

int	get_dongle(int i, t_coder *coder)
{
	t_codex			*codex;

	codex = codex_return();
	pthread_mutex_lock(&codex->dongles[i].mutex);
	while ((codex->dongles[i].dongle_availability != available
			|| timeofday_converter() - codex->dongles[i].released_time
			< codex->dongle_cooldown
			|| heap_peek(&codex->dongles[i].heap) != coder->coder_id)
		&& sim_is_stopped(codex) == 0)
	{
		pthread_mutex_unlock(&codex->dongles[i].mutex);
		usleep(100);
		pthread_mutex_lock(&codex->dongles[i].mutex);
	}
	return (wait_outcome(i, codex, coder));
}

void	printer(t_coder *coder)
{
	printf("%ld %ld has taken a dongle\n",
		timeofday_converter() - codex_return()->start_time,
		coder->coder_id);
}

int	get_both_dongles(t_coder *coder)
{
	int	first_dongle;
	int	second_dongle;
	int	first_check;

	dongle_order(coder, &first_dongle, &second_dongle);
	add_to_heaps(coder);
	first_check = get_dongle(first_dongle, coder);
	if (first_check == 0)
		return (0);
	if (first_check == 1)
	{
		pthread_mutex_lock(&codex_return()->mutex_sim);
		printer(coder);
		pthread_mutex_unlock(&codex_return()->mutex_sim);
		if (get_dongle(second_dongle, coder) == 1)
		{
			pthread_mutex_lock(&codex_return()->mutex_sim);
			printer(coder);
			pthread_mutex_unlock(&codex_return()->mutex_sim);
			return (1);
		}
		else
			return (let_dongle(first_dongle), 0);
	}
	return (0);
}
