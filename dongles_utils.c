/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles_utils.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: limelo-c <limelo-c@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/25 23:44:55 by limelo-c          #+#    #+#             */
/*   Updated: 2026/09/25 23:44:55 by limelo-c         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	printer(t_coder *coder)
{
	printf("%ld %ld has taken a dongle\n",
		timeofday_converter() - codex_return()->start_time,
		coder->coder_id);
}

int	is_sim_stopped(void)
{
	int	stopped;

	stopped = 0;
	pthread_mutex_lock(&codex_return()->mutex_sim);
	stopped = codex_return()->sim_stopped;
	pthread_mutex_unlock(&codex_return()->mutex_sim);
	return (stopped);
}

int	both_dongles_permission(t_coder *coder)
{
	int	left_check;
	int	right_check;
	int	left;
	int	right;

	left = coder->left_dongle;
	right = coder->right_dongle;
	if (left > right)
	{
		left = coder->right_dongle;
		right = coder->left_dongle;
	}
	pthread_mutex_lock(&codex_return()->dongles[left].mutex);
	pthread_mutex_lock(&codex_return()->dongles[right].mutex);
	left_check = codex_return()->dongles[coder->left_dongle]
		.dongle_availability == available && timeofday_converter()
		- codex_return()->dongles[coder->left_dongle].released_time
		>= codex_return()->dongle_cooldown;
	right_check = codex_return()->dongles[coder->right_dongle]
		.dongle_availability == available && timeofday_converter()
		- codex_return()->dongles[coder->right_dongle].released_time
		>= codex_return()->dongle_cooldown;
	pthread_mutex_unlock(&codex_return()->dongles[right].mutex);
	pthread_mutex_unlock(&codex_return()->dongles[left].mutex);
	return (left_check && right_check);
}

t_coder	*is_coder_ready(t_codex *codex)
{
	t_coder	*selected;
	t_coder	**skipped_coders;
	int		skipped_counter;
	int		i;

	skipped_coders = malloc((codex->number_of_coders) * sizeof(t_coder *));
	if (!skipped_coders)
		return (NULL);
	selected = NULL;
	skipped_counter = 0;
	while (codex->waiting_heap.size > 0)
	{
		selected = heap_pop(&codex->waiting_heap);
		if (both_dongles_permission(selected))
			break ;
		skipped_coders[skipped_counter] = selected;
		selected = NULL;
		skipped_counter++;
	}
	i = 0;
	while (i < skipped_counter)
		heap_push(&codex->waiting_heap, skipped_coders[i++]);
	free(skipped_coders);
	return (selected);
}
