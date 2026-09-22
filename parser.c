/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: limelo-c <limelo-c@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/19 15:55:14 by limelo-c          #+#    #+#             */
/*   Updated: 2026/09/19 15:55:14 by limelo-c         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	validator_scheduler(const char *str)
{
	t_codex	*codex;

	codex = codex_return();
	if (strcmp(str, "fifo") == 0)
	{
		codex->scheduler = fifo_sched;
		return (1);
	}
	if (strcmp(str, "edf") == 0)
	{
		codex->scheduler = edf_sched;
		return (1);
	}
	else
		return (-1);
}

int	parser_validator(char **argv)
{
	t_codex	*codex;
	int		i;

	codex = codex_return();
	i = 1;
	while (i < 7)
	{
		if (validator_numeric(argv[i]) == -1)
			return (printf("Argument %d is of wrong format\n", i), 1);
		else if (i == 1)
			codex->number_of_coders = validator_numeric(argv[i]);
		else if (i == 2)
			codex->time_to_burnout = validator_numeric(argv[i]);
		else if (i == 3)
			codex->time_to_compile = validator_numeric(argv[i]);
		else if (i == 4)
			codex->time_to_debug = validator_numeric(argv[i]);
		else if (i == 5)
			codex->time_to_refactor = validator_numeric(argv[i]);
		else if (i == 6)
			codex->number_of_compiles_required = validator_numeric(argv[i]);
		i++;
	}
	return (0);
}

int	parser_last_validator(char **argv)
{
	t_codex	*codex;

	codex = codex_return();
	if (validator_numeric_donglecd(argv[7]) == -1)
		return (printf("Argument 7 is of wrong format\n"), 1);
	else
		codex->dongle_cooldown = validator_numeric_donglecd(argv[7]);
	if (validator_scheduler(argv[8]) == -1)
		return (printf("Argument 8 is of wrong format\n"), 1);
	return (0);
}
