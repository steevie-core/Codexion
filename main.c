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
	pthread_create(&monitor_thread, NULL, monitor_journey, NULL);
	codex_return()->start_time = timeofday_converter();
	while (++i < codex_return()->number_of_coders)
		pthread_create(&coder_thrds[i], NULL,
			coder_jrney, &codex_return()->coders[i]);
	i = -1;
	while (++i < codex_return()->number_of_coders)
		pthread_join(coder_thrds[i], NULL);
	caller(&monitor_thread, &coder_thrds, codex_return());
	return (0);
}
