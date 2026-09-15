#include "codexion.h"

t_codex	*codex_return(void)
{
	static t_codex	codex;

	return (&codex);
}

static void    cleanup(t_codex *codex)
{
    int    i;

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
		pthread_t **coder_threads, t_codex **codex)
{
	pthread_join(*monitor_thread, NULL);
	cleanup(*codex);
	free(*coder_threads);
}

int	main(int argc, char **argv)
{
	t_codex		*codex;
	pthread_t	*coder_threads;
	pthread_t	monitor_thread;
	int			i;

	i = -1;
	codex = codex_return();
	pthread_mutex_init(&codex->mutex_sim, NULL);
	if (argc != 9)
		return (printf("wrong number of arguments\n"), 1);
	if (parser_validator(argv) == 1 || parser_last_validator(argv) == 1)
		return (1);
	if (dongles_init() == -1 || coders_init() == -1)
		return (printf("Memory allocation issue\n"), 1);
	coder_threads = malloc(codex->number_of_coders * sizeof(pthread_t));
	if (!(coder_threads))
		return (printf("Memory allocation issue\n"), 1);
	pthread_create(&monitor_thread, NULL, monitor_journey, NULL);
	while (++i < codex->number_of_coders)
		pthread_create(&coder_threads[i], NULL, coder_jrney, &codex->coders[i]);
	i = -1;
	while (++i < codex->number_of_coders)
		pthread_join(coder_threads[i], NULL);
	caller(&monitor_thread, &coder_threads, &codex);
	return (0);
}
