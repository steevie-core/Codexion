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

static void	got_dongles(t_coder *coder, int right, int left)
{
	codex_return()->dongles[coder->left_dongle]
	.dongle_availability = taken;
	codex_return()->dongles[coder->right_dongle]
	.dongle_availability = taken;
	printer(coder);
	printer(coder);
	pthread_mutex_unlock(&codex_return()->mutex_sim);
	pthread_mutex_unlock(&codex_return()->dongles[right].mutex);
	pthread_mutex_unlock(&codex_return()->dongles[left].mutex);
	pthread_mutex_unlock(&codex_return()->mutex_sched);
}

static void	lock_dongle_id(t_coder *coder, int right, int left)
{
	if (left > right)
	{
		right = coder->left_dongle;
		left = coder->right_dongle;
	}
	pthread_mutex_lock(&codex_return()->dongles[left].mutex);
	pthread_mutex_lock(&codex_return()->dongles[right].mutex);
	pthread_mutex_lock(&codex_return()->mutex_sim);
}

static int	coder_selected(t_coder *selected, t_coder *coder,
	int right, int left)
{
	if (coder == selected)
	{
		left = coder->left_dongle;
		right = coder->right_dongle;
		lock_dongle_id(coder, right, left);
		if (codex_return()->sim_stopped)
		{
			pthread_mutex_unlock(&codex_return()->mutex_sim);
			pthread_mutex_unlock(&codex_return()->dongles[right].mutex);
			pthread_mutex_unlock(&codex_return()->dongles[left].mutex);
			pthread_mutex_unlock(&codex_return()->mutex_sched);
			return (0);
		}
		got_dongles(coder, right, left);
		return (1);
	}
	if (selected != NULL)
		heap_push(&codex_return()->waiting_heap, selected);
	pthread_mutex_unlock(&codex_return()->mutex_sched);
	return (0);
}

static int	try_to_get(t_coder *coder, int right, int left)
{
	t_coder	*selected;

	if (is_sim_stopped())
		return (0);
	pthread_mutex_lock(&codex_return()->mutex_sched);
	selected = is_coder_ready(codex_return());
	if (coder_selected(selected, coder, right, left) == 1)
		return (1);
	return (0);
}

int	get_both_dongles(t_coder *coder)
{
	if (coder->left_dongle == coder->right_dongle)
		return (0);
	pthread_mutex_lock(&codex_return()->mutex_sched);
	heap_push(&codex_return()->waiting_heap, coder);
	pthread_mutex_unlock(&codex_return()->mutex_sched);
	while (1)
	{
		if (is_sim_stopped())
			return (0);
		if (try_to_get(coder, coder->right_dongle, coder->left_dongle) == 1)
			return (1);
		usleep(100);
	}
	return (0);
}
