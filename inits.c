#include "codexion.h"

long	timeofday_converter(void)
{
	struct timeval	s;

	gettimeofday(&s, NULL);
	return ((s.tv_sec * 1000) + (s.tv_usec / 1000));
}

int	dongles_init(void)
{
	t_dongle	*dongle_arr;
	t_codex		*codex;
	int			i;

	codex = codex_return();
	i = 0;
	dongle_arr = malloc(codex->number_of_coders * sizeof(t_dongle));
	if (!(dongle_arr))
		return (-1);
	codex->dongles = dongle_arr;
	while (i < codex->number_of_coders)
	{
		pthread_mutex_init(&codex->dongles[i].mutex, NULL);
		pthread_cond_init(&codex->dongles[i].thread_sleep, NULL);
		codex->dongles[i].heap.capacity = codex->number_of_coders;
		if (heap_creation(&codex->dongles[i].heap) == -1)
			return (-1);
		codex->dongles[i].heap.last_dgl_granted = 0;
		codex->dongles[i].dongle_availability = available;
		codex->dongles[i].released_time = 0;
		i++;
	}
	return (0);
}

int	coders_init(void)
{
	t_codex	*codex;
	t_coder	*coder_arr;
	int		i;

	codex = codex_return();
	i = 0;
	coder_arr = malloc(codex->number_of_coders * sizeof(t_coder));
	if (!(coder_arr))
		return (-1);
	codex->coders = coder_arr;
	while (i < codex->number_of_coders)
	{
		pthread_mutex_init(&codex->coders[i].mutex, NULL);
		codex->coders[i].coder_id = i + 1;
		codex->coders[i].coder_compiles_num = 0;
		codex->coders[i].last_compile = timeofday_converter();
		codex->coders[i].right_dongle = i;
		codex->coders[i].left_dongle = (i - 1 + codex->number_of_coders)
			% codex->number_of_coders;
		i++;
	}
	return (0);
}
