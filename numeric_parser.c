/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   numeric_parser.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: limelo-c <limelo-c@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/19 15:55:09 by limelo-c          #+#    #+#             */
/*   Updated: 2026/09/19 15:55:09 by limelo-c         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	overflow_checker(long value)
{
	if (value > INT_MAX)
		return (-1);
	return (0);
}

int	validator_numeric_donglecd(const char *str)
{
	size_t	i;
	size_t	len;
	long	value;

	i = 0;
	len = strlen(str);
	value = 0;
	while (i < len)
	{
		if (str[i] >= '0' && str[i] <= '9')
		{
			value = value * 10 + (str[i] - '0');
			if (value > INT_MAX)
				return (printf("Overflow value, "), -1);
			i++;
		}
		else
			return (-1);
	}
	if (overflow_checker(value) == -1)
		return (-1);
	else
		return (value);
}

int	validator_numeric(const char *str)
{
	size_t	i;
	size_t	counter_zero;
	long	value;

	i = 0;
	counter_zero = 0;
	value = 0;
	if (str[0] == '\0')
		return (-1);
	while (i < strlen(str))
	{
		if (str[i] == '0')
			counter_zero++;
		if (str[i] >= '0' && str[i] <= '9')
		{
			value = value * 10 + (str[i] - '0');
			if (value > INT_MAX)
				return (printf("Overflow value, "), -1);
			i++;
		}
		else
			return (-1);
	}
	if (counter_zero == strlen(str) || overflow_checker(value) == -1)
		return (-1);
	else
		return (value);
}
