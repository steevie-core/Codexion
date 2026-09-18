#include "codexion.h"

int	heap_creation(t_heap *heap)
{
	heap->list = malloc(heap->capacity * sizeof(t_coder *));
	if (!heap->list)
		return (-1);
	heap->size = 0;
	return (0);
}

void	free_heap(t_heap *heap)
{
	free(heap->list);
}

int	priority_coder(t_coder	*a, t_coder	*b, t_heap *heap)
{
	t_codex	*codex;
	long	deadline_a;
	long	deadline_b;

	codex = codex_return();
	deadline_a = a->last_compile + codex->time_to_burnout;
	deadline_b = b->last_compile + codex->time_to_burnout;
	if (codex->scheduler == fifo_sched)
	{
		if (a->arrival != b->arrival)
			return (a->arrival < b->arrival);
		return (a->coder_id < b->coder_id);
	}
	if (codex->scheduler == edf_sched)
	{
		if (deadline_a != deadline_b)
			return (deadline_a < deadline_b);
		else if (a->coder_id == heap->last_dgl_granted)
			return (0);
		else if (b->coder_id == heap->last_dgl_granted)
			return (1);
		else
			return (a->coder_id < b->coder_id);
	}
	return (0);
}

void	sift_up(t_heap *heap)
{
	int	i;
	int	parent;

	i = heap->size;
	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (priority_coder(heap->list[i], heap->list[parent], heap) == 0)
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
			&& priority_coder(heap->list[left_child], heap->list[best], heap))
			best = left_child;
		if (right_child < heap->size
			&& priority_coder(heap->list[right_child], heap->list[best], heap))
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
