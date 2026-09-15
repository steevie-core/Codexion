#include "codexion.h"

void	let_dongle(int i)
{
	t_codex	*codex;

	codex = codex_return();
	pthread_mutex_lock(&codex->dongles[i].mutex);
	codex->dongles[i].dongle_availability = available;
	codex->dongles[i].released_time = timeofday_converter();
	pthread_cond_broadcast(&codex->dongles[i].thread_sleep);
	pthread_mutex_unlock(&codex->dongles[i].mutex);
}

void	let_both_dongles(t_coder *coder)
{
	let_dongle(coder->left_dongle);
	let_dongle(coder->right_dongle);
}
