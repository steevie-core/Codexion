#include "codexion.h"

int	sim_is_stopped(t_codex *codex)
{
	int	sim_stopper;

	sim_stopper = 0;
	pthread_mutex_lock(&codex->mutex_sim);
	sim_stopper = codex->sim_stopped;
	pthread_mutex_unlock(&codex->mutex_sim);
	return (sim_stopper);
}

void	wakeup_thread(t_codex *codex)
{
	int	i;

	i = 0;
	while (i < codex->number_of_coders)
	{
		pthread_mutex_lock(&codex->dongles[i].mutex);
		pthread_cond_broadcast(&codex->dongles[i].thread_sleep);
		pthread_mutex_unlock(&codex->dongles[i].mutex);
		i++;
	}
}

int	burnout_handle(t_codex *codex, long last_compile_locked, int i)
{
	if (timeofday_converter() - last_compile_locked > codex->time_to_burnout)
	{
		pthread_mutex_lock(&codex_return()->mutex_sim);
		printf("%ld %ld has burned out\n",
			timeofday_converter(), codex->coders[i].coder_id);
		codex->sim_stopped = 1;
		pthread_mutex_unlock(&codex->mutex_sim);
		wakeup_thread(codex);
		return (1);
	}
	return (0);
}

int	coders_are_done(t_codex *codex)
{
	int	i;
	int	done;

	i = 0;
	done = 0;
	while (i < codex->number_of_coders)
	{
		pthread_mutex_lock(&codex->coders[i].mutex);
		done = codex->coders[i].coder_compiles_num
			>= codex->number_of_compiles_required;
		pthread_mutex_unlock(&codex->coders[i].mutex);
		if (!done)
			return (0);
		i++;
	}
	pthread_mutex_lock(&codex->mutex_sim);
	codex->sim_stopped = 1;
	pthread_mutex_unlock(&codex->mutex_sim);
	wakeup_thread(codex);
	return (1);
}

void	*monitor_journey(void *arg)
{
	t_codex	*codex;
	int		i;
	long	last_compile_locked;

	(void)arg;
	codex = codex_return();
	last_compile_locked = 0;
	i = 0;
	while (1)
	{
		i = 0;
		while (i < codex->number_of_coders)
		{
			pthread_mutex_lock(&codex->coders[i].mutex);
			last_compile_locked = codex->coders[i].last_compile;
			pthread_mutex_unlock(&codex->coders[i].mutex);
			if (burnout_handle(codex, last_compile_locked, i) == 1)
				return (NULL);
			i++;
		}
		usleep(2000);
		if (coders_are_done(codex) == 1)
			return (NULL);
	}
	return (NULL);
}
