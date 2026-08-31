#include "codexion.h"

t_codex* codex_return() 
{
    static t_codex codex;
    return (&codex);
}

long timeofday_converter()
{
    struct timeval s;

    gettimeofday(&s, NULL);
    return ((s.tv_sec * 1000) + (s.tv_usec / 1000));
}

int dongles_init()
{
    t_dongle *dongle_arr;
    t_codex *codex;
    int i;

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
        codex->dongles[i].dongle_availability = available;
        codex->dongles[i].released_time = 0;
        i++;
    }
    return (0);
}

int coders_init()
{
    t_codex *codex;
    t_coder *coder_arr;
    int i;

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
        codex->coders[i].left_dongle = (i - 1 + codex->number_of_coders) % codex->number_of_coders;
        i++;
    }
    return (0);
}
void dongle_order(t_coder *coder, int *first_dongle, int *second_dongle)
{
    if (coder->right_dongle < coder->left_dongle)
    {
        *first_dongle = coder->right_dongle;
        *second_dongle = coder->left_dongle;
    }
    else
    {
        *first_dongle = coder->left_dongle;
        *second_dongle = coder->right_dongle;
    }
}

int sim_is_stopped(t_codex *codex)
{   
    int     sim_stopper;
    sim_stopper = 0;
    pthread_mutex_lock(&codex->mutex_sim);
    sim_stopper = codex->sim_stopped;
    pthread_mutex_unlock(&codex->mutex_sim);
    return (sim_stopper);
}

int get_dongle(int i)
{
    t_codex *codex;
    struct timespec tp;

    codex = codex_return();
    pthread_mutex_lock(&codex->dongles[i].mutex);
    while ((codex->dongles[i].dongle_availability != available || timeofday_converter() - codex->dongles[i].released_time < codex->dongle_cooldown) && sim_is_stopped(codex) == 0)
    {
        clock_gettime(CLOCK_REALTIME, &tp);
        tp.tv_nsec = tp.tv_nsec + (codex->dongle_cooldown * 1000000);
        tp.tv_sec = tp.tv_sec + (tp.tv_nsec / 1000000000);
        tp.tv_nsec = tp.tv_nsec % 1000000000;
        pthread_cond_timedwait(&codex->dongles[i].thread_sleep, &codex->dongles[i].mutex, &tp);
    }
    if (sim_is_stopped(codex) == 1)
    {
        pthread_mutex_unlock(&codex->dongles[i].mutex);
        return (0);
    }
    else
    {
        codex->dongles[i].dongle_availability = taken;
        pthread_mutex_unlock(&codex->dongles[i].mutex);
        return (1);        
    }
}

void let_dongle(int i)
{
    t_codex *codex;

    codex = codex_return();
    pthread_mutex_lock(&codex->dongles[i].mutex);
    codex->dongles[i].dongle_availability = available;
    codex->dongles[i].released_time = timeofday_converter();
    pthread_cond_signal(&codex->dongles[i].thread_sleep);
    pthread_mutex_unlock(&codex->dongles[i].mutex);
}

void let_both_dongles(t_coder *coder)
{
    let_dongle(coder->left_dongle);
    let_dongle(coder->right_dongle);
}

int get_both_dongles(t_coder *coder)
{
    int first_dongle;
    int second_dongle;
    int first_check;

    dongle_order(coder, &first_dongle, &second_dongle);
    first_check = get_dongle(first_dongle);
    if (first_check == 0)
        return (0);
    if (first_check == 1)
    {
        printf("%ld %ld has taken a dongle\n", timeofday_converter(), coder->coder_id);
        if (get_dongle(second_dongle) == 1)
        {
            printf("%ld %ld has taken a dongle\n", timeofday_converter(), coder->coder_id);
            return (1);
        }
        else
        {
            let_dongle(first_dongle);
            return (0);
        }
    }
    return (0);
}
void *monitor_journey(void *arg)
{
    t_codex *codex;
    int i;
    long last_compile_locked;

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
            if (timeofday_converter() - last_compile_locked > codex->time_to_burnout)
            {
                printf("%ld %ld has burned out\n", timeofday_converter(), codex->coders[i].coder_id);
                pthread_mutex_lock(&codex->mutex_sim);
                codex->sim_stopped = 1;
                pthread_mutex_unlock(&codex->mutex_sim);
                return (NULL);
            }
            i++;
        }
        usleep(2000);
    }
    return (NULL);
}

void simulation_stopper_helper(long ms)
{
    long slept;
    t_codex *codex;
    int     sim_stopper;

    codex = codex_return();
    slept = 0;
    sim_stopper = 0;
    while (slept < ms && sim_stopper == 0)
    {
        usleep(1000);
        slept = slept + 1;
        sim_is_stopped(codex);
    }
}

int try_compile(t_coder *coder)
{
    t_codex *codex;

    codex = codex_return();
    if (sim_is_stopped(codex) == 1)
        return (0);
    if (get_both_dongles(coder) == 1)
    {
        pthread_mutex_lock(&coder->mutex);
        coder->last_compile = timeofday_converter();
        pthread_mutex_unlock(&coder->mutex);
        printf("%ld %ld is compiling\n", timeofday_converter(), coder->coder_id);
        simulation_stopper_helper(codex->time_to_compile);
        let_both_dongles(coder);        
    }
    if (sim_is_stopped(codex) == 1)
        return (0);
    return (1);
}
int try_debug(t_coder *coder)
{
    t_codex *codex;

    codex = codex_return();
    if (sim_is_stopped(codex) == 1)
        return (0);
    if (sim_is_stopped(codex) == 0)
    {
        printf("%ld %ld is debugging\n", timeofday_converter(), coder->coder_id);
        simulation_stopper_helper(codex->time_to_debug);
        return (1);
    }
    if (sim_is_stopped(codex) == 1)
        return (0);
    return (1);
}
int try_refactor(t_coder *coder)
{
    t_codex *codex;

    codex = codex_return();
    if (sim_is_stopped(codex) == 1)
        return (0);
    if (sim_is_stopped(codex) == 0)
    {
        printf("%ld %ld is refactoring\n", timeofday_converter(), coder->coder_id);
        simulation_stopper_helper(codex->time_to_refactor);
        return (1);
    }
    if (sim_is_stopped(codex) == 1)
        return (0);
    return (1);
}
void *coder_journey(void *arg)
{
    t_coder *coder;
    t_codex *codex;
    int     sim_stopper;
    int     is_compiled;

    codex = codex_return();
    coder = (t_coder *)arg;
    is_compiled = try_compile(coder);
    sim_stopper = 0;
    while (coder->coder_compiles_num < codex->number_of_compiles_required && sim_stopper == 0)
    {
        if (!is_compiled)
            break;
        coder->coder_compiles_num++;
        if (!try_debug(coder))
            break;
        if (!try_refactor(coder))
            break;
    }
    return (NULL);
}

int main(int argc, char **argv)
{
    t_codex *codex;
    pthread_t *coder_threads;
    pthread_t monitor_thread;
    int i;

    i = 0;
    codex = codex_return();
    pthread_mutex_init(&codex->mutex_sim, NULL);
    coder_threads = malloc(codex->number_of_coders * sizeof(pthread_t));
    if (!(coder_threads))
        return (printf("Memory allocation issue\n"), 1);
    if (argc != 9)
        return (printf("wrong number of arguments\n"), 1);
    if (parser_validator(argv) == 1 || parser_last_validator(argv) == 1)
        return (1);
    if (dongles_init() == -1 || coders_init() == -1)
        return (printf("Memory allocation issue\n"), 1);
    pthread_create(&monitor_thread, NULL, monitor_journey, NULL);
    while (i < codex->number_of_coders)
    {
        pthread_create(&coder_threads[i], NULL, coder_journey, &codex->coders[i]);
        i++;
    }
    i = 0;
    while (i < codex->number_of_coders)
    {
        pthread_join(coder_threads[i], NULL);
        i++;
    }
    codex->sim_stopped = 1;
    pthread_join(monitor_thread, NULL);
    // i = 0;
    // free(codex->dongles);
    // while (i < codex->number_of_coders)
    // { 
    //     pthread_mutex_destroy(&codex->dongles[i].mutex);
    //     pthread_mutex_destroy(&coder_threads[i]);
    //     i++;
    // }
    // get_both_dongles(&codex->coders[0]);
    // long t1;
    // long t2;

    // t1 = timeofday_converter();
    // usleep(500000);  // sleep 500,000 microseconds = 500ms
    // t2 = timeofday_converter();
    // printf("elapsed: %ld ms (expected ~500)\n", t2 - t1);
    return (0);
}