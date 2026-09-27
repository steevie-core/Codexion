/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   schedulers.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: limelo-c <limelo-c@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/19 15:55:19 by limelo-c          #+#    #+#             */
/*   Updated: 2026/09/19 15:55:19 by limelo-c         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	free_heap(t_heap *heap)
{
	free(heap->list);
}

static void	scheduler_comparison(t_coder *a, t_coder *b)
{
	if (a->coder_id < b->coder_id)
	{
		pthread_mutex_lock(&a->mutex);
		pthread_mutex_lock(&b->mutex);
	}
	else
	{
		pthread_mutex_lock(&b->mutex);
		pthread_mutex_lock(&a->mutex);
	}
}

int	priority_coder(t_coder *a, t_coder *b)
{
	long		deadline_a;
	long		deadline_b;
	long		arrival_a;
	long		arrival_b;
	t_scheduler	sched;

	if (a == b)
		return (0);
	sched = codex_return()->scheduler;
	scheduler_comparison(a, b);
	arrival_a = a->arrival;
	deadline_a = a->last_compile;
	arrival_b = b->arrival;
	deadline_b = b->last_compile;
	pthread_mutex_unlock(&b->mutex);
	pthread_mutex_unlock(&a->mutex);
	if (sched == fifo_sched)
	{
		if (arrival_a != arrival_b)
			return (arrival_a < arrival_b);
		return (a->coder_id < b->coder_id);
	}
	if (deadline_a != deadline_b)
		return (deadline_a < deadline_b);
	return (a->coder_id < b->coder_id);
}

void	sift_up(t_heap *heap)
{
	int	i;
	int	parent;

	i = heap->size;
	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (priority_coder(heap->list[i], heap->list[parent]) == 0)
			break ;
		swap_coder(&heap->list[i], &heap->list[parent]);
		i = parent;
	}
}

void	sift_down(t_heap *heap)
{
	int	i;
	int	left_child;
	int	right_child;
	int	best;

	i = 0;
	while (1)
	{
		left_child = 2 * i + 1;
		right_child = 2 * i + 2;
		best = i;
		if (left_child < heap->size
			&& priority_coder(heap->list[left_child], heap->list[best]))
			best = left_child;
		if (right_child < heap->size
			&& priority_coder(heap->list[right_child], heap->list[best]))
			best = right_child;
		if (best == i)
			break ;
		else
		{
			swap_coder(&heap->list[i], &heap->list[best]);
			i = best;
		}
	}
}
