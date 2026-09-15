#include "codexion.h"

void	simulation_stopper_helper(long ms)
{
	long	slept;
	t_codex	*codex;
	int		sim_stopper;

	codex = codex_return();
	slept = 0;
	sim_stopper = 0;
	while (slept < ms && sim_stopper == 0)
	{
		usleep(1000);
		slept = slept + 1;
		sim_stopper = sim_is_stopped(codex);
	}
}

int	try_compile(t_coder *coder)
{
	t_codex	*codex;

	codex = codex_return();
	if (sim_is_stopped(codex) == 1)
		return (0);
	pthread_mutex_lock(&coder->mutex);
	coder->arrival = timeofday_converter();
	pthread_mutex_unlock(&coder->mutex);
	if (get_both_dongles(coder) == 1)
	{
		pthread_mutex_lock(&coder->mutex);
		coder->last_compile = timeofday_converter();
		pthread_mutex_unlock(&coder->mutex);
		pthread_mutex_lock(&codex_return()->mutex_sim);
		printf("%ld %ld is compiling\n",
			timeofday_converter(), coder->coder_id);
		pthread_mutex_unlock(&codex_return()->mutex_sim);
		simulation_stopper_helper(codex->time_to_compile);
		let_both_dongles(coder);
	}
	if (sim_is_stopped(codex) == 1)
		return (0);
	return (1);
}

int	try_debug(t_coder *coder)
{
	t_codex	*codex;

	codex = codex_return();
	if (sim_is_stopped(codex) == 0)
	{
		pthread_mutex_lock(&codex_return()->mutex_sim);
		printf("%ld %ld is debugging\n",
			timeofday_converter(), coder->coder_id);
		pthread_mutex_unlock(&codex_return()->mutex_sim);
		simulation_stopper_helper(codex->time_to_debug);
		return (1);
	}
	if (sim_is_stopped(codex) == 1)
		return (0);
	return (1);
}

int	try_refactor(t_coder *coder)
{
	t_codex	*codex;

	codex = codex_return();
	if (sim_is_stopped(codex) == 0)
	{
		pthread_mutex_lock(&codex_return()->mutex_sim);
		printf("%ld %ld is refactoring\n",
			timeofday_converter(), coder->coder_id);
		pthread_mutex_unlock(&codex_return()->mutex_sim);
		simulation_stopper_helper(codex->time_to_refactor);
		return (1);
	}
	if (sim_is_stopped(codex) == 1)
		return (0);
	return (1);
}

void	*coder_jrney(void *arg)
{
	t_coder	*coder;
	t_codex	*codex;
	int		sim_stopper;
	long	compile_count;

	codex = codex_return();
	coder = (t_coder *)arg;
	sim_stopper = sim_is_stopped(codex);
	compile_count = 0;
	while (compile_count < codex->number_of_compiles_required
		&& sim_stopper == 0)
	{
		if (!try_compile(coder))
			break ;
		pthread_mutex_lock(&coder->mutex);
		coder->coder_compiles_num++;
		compile_count = coder->coder_compiles_num;
		pthread_mutex_unlock(&coder->mutex);
		if (coders_are_done(codex))
			break ;
		if (!try_debug(coder))
			break ;
		if (!try_refactor(coder))
			break ;
		sim_stopper = sim_is_stopped(codex);
	}
	return (NULL);
}
