#include "codexion.h"

void	swap_coder(t_coder **a, t_coder **b)
{
	t_coder	*tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

void	heap_push(t_heap *heap, t_coder *coder)
{
	heap->list[heap->size] = coder;
	sift_up(heap);
	heap->size++;
}

t_coder	*heap_pop(t_heap *heap)
{
	t_coder	*ret;

	ret = heap->list[0];
	heap->size--;
	heap->list[0] = heap->list[heap->size];
	sift_down(heap);
	return (ret);
}

int	heap_peek(t_heap *heap)
{
	if (heap->size == 0)
		return (-1);
	return (heap->list[0]->coder_id);
}
