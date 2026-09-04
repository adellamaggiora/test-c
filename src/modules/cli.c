#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli.h"
#include "report.h"
#include "tube_test/dead_time.h"
#include "tube_test/gamma.h"
#include "tube_test/starting_voltage.h"

#define INPUT_BUFFER_SIZE 128
#define TUBE_CODE_BUFFER_SIZE 66

typedef struct
{
    size_t tube_count;
    char (*tube_codes)[TUBE_CODE_BUFFER_SIZE];
    const TubeConfig *tube_config;
    const char *tube_profile;
    bool gamma_completed;
    bool starting_voltage_completed;
    bool dead_time_completed;
    GammaDoseRateTestResults gamma;
    StartingVoltageTestResults starting_voltage;
    DeadTimeTestResults dead_time;
} CliSession;

typedef enum
{
    READ_LINE_OK = 0,
    READ_LINE_EOF,
    READ_LINE_TOO_LONG
} ReadLineResult;

static ReadLineResult read_line(
    FILE *input,
    char *buffer,
    size_t buffer_size)
{
    if (fgets(buffer, (int)buffer_size, input) == NULL)
    {
        return READ_LINE_EOF;
    }

    char *newline = strchr(buffer, '\n');
    if (newline != NULL)
    {
        *newline = '\0';
        return READ_LINE_OK;
    }

    if (feof(input))
    {
        return READ_LINE_OK;
    }

    int character;
    do
    {
        character = fgetc(input);
    }
    while (character != '\n' && character != EOF);

    return READ_LINE_TOO_LONG;
}

static bool parse_number(
    const char *text,
    size_t minimum,
    size_t maximum,
    size_t *number)
{
    if (text == NULL || text[0] == '\0')
    {
        return false;
    }

    errno = 0;
    char *end = NULL;
    unsigned long parsed = strtoul(text, &end, 10);

    while (end != NULL && *end == ' ')
    {
        end++;
    }

    if (errno != 0 || end == text || end == NULL || *end != '\0' ||
        parsed < minimum || parsed > maximum || parsed > SIZE_MAX)
    {
        return false;
    }

    *number = (size_t)parsed;
    return true;
}

static bool prompt_number(
    FILE *input,
    FILE *output,
    size_t minimum,
    size_t maximum,
    const char *prompt,
    size_t *number)
{
    char buffer[INPUT_BUFFER_SIZE];

    while (true)
    {
        fprintf(output, "%s", prompt);
        fflush(output);

        ReadLineResult read_result = read_line(
            input,
            buffer,
            sizeof(buffer));

        if (read_result == READ_LINE_EOF)
        {
            return false;
        }

        if (read_result == READ_LINE_OK &&
            parse_number(buffer, minimum, maximum, number))
        {
            return true;
        }

        fprintf(
            output,
            "Valore non valido. Inserire un numero da %zu a %zu.\n",
            minimum,
            maximum);
    }
}

static bool tube_code_already_used(
    const CliSession *session,
    size_t current_index,
    const char *tube_code)
{
    for (size_t i = 0; i < current_index; i++)
    {
        if (strcmp(session->tube_codes[i], tube_code) == 0)
        {
            return true;
        }
    }

    return false;
}

static bool prompt_tube_codes(
    FILE *input,
    FILE *output,
    CliSession *session)
{
    for (size_t i = 0; i < session->tube_count; i++)
    {
        while (true)
        {
            fprintf(
                output,
                "Codice tubo %zu/%zu: ",
                i + 1,
                session->tube_count);
            fflush(output);

            ReadLineResult read_result = read_line(
                input,
                session->tube_codes[i],
                TUBE_CODE_BUFFER_SIZE);

            if (read_result == READ_LINE_EOF)
            {
                return false;
            }

            if (read_result != READ_LINE_OK ||
                !tube_code_is_valid(session->tube_codes[i]))
            {
                fprintf(
                    output,
                    "Codice non valido. Usare al massimo 64 caratteri "
                    "alfanumerici.\n");
                continue;
            }

            if (tube_code_already_used(
                    session,
                    i,
                    session->tube_codes[i]))
            {
                fprintf(output, "Codice gia inserito. Usare un codice unico.\n");
                continue;
            }

            break;
        }
    }

    return true;
}

static bool prompt_tube_type(
    FILE *input,
    FILE *output,
    const Config *config,
    CliSession *session)
{
    size_t choice;

    fprintf(output, "Tipo di tubo:\n");
    fprintf(output, "1. Small\n");
    fprintf(output, "2. Large\n");

    if (!prompt_number(input, output, 1, 2, "Scelta: ", &choice))
    {
        return false;
    }

    if (choice == 1)
    {
        session->tube_config = &config->small_tube;
        session->tube_profile = "small";
    }
    else
    {
        session->tube_config = &config->large_tube;
        session->tube_profile = "large";
    }

    return true;
}

static const char *completion_flag(bool completed)
{
    return completed ? "[x]" : "[ ]";
}

static bool all_tests_completed(const CliSession *session)
{
    return session->gamma_completed &&
        session->starting_voltage_completed &&
        session->dead_time_completed;
}

static void print_menu(FILE *output, const CliSession *session)
{
    fprintf(
        output,
        "\n=== %zu tubi | profilo %s ===\n",
        session->tube_count,
        session->tube_profile);
    fprintf(output, "1. %s Gamma\n",
            completion_flag(session->gamma_completed));
    fprintf(output, "2. %s Starting voltage\n",
            completion_flag(session->starting_voltage_completed));
    fprintf(output, "3. %s Dead time\n",
            completion_flag(session->dead_time_completed));
    fprintf(output, "4. Run all\n");
    fprintf(
        output,
        "5. Print report%s\n",
        all_tests_completed(session)
            ? ""
            : " [bloccato: completare tutti i test]");
    fprintf(output, "0. Esci\n");
}

static const GammaDoseRateTestResult *find_gamma_result(
    const GammaDoseRateTestResults *results,
    size_t tube_index)
{
    if (results == NULL || results->test_results == NULL)
    {
        return NULL;
    }

    for (size_t i = 0; i < results->count; i++)
    {
        if (results->test_results[i].tube_index == tube_index)
        {
            return &results->test_results[i];
        }
    }

    return NULL;
}

static void print_gamma_results(
    FILE *output,
    const CliSession *session)
{
    fprintf(output, "Gamma completato:\n");

    for (size_t i = 0; i < session->tube_count; i++)
    {
        const GammaDoseRateTestResult *result = find_gamma_result(
            &session->gamma,
            i);

        if (result == NULL)
        {
            continue;
        }

        fprintf(
            output,
            "  %s: media=%.2f, riferimento=%s, esito=%s\n",
            session->tube_codes[i],
            result->avg,
            result->ref_tube ? "si" : "no",
            result->test_passed ? "PASS" : "FAIL");
    }
}

static bool run_gamma_test(
    FILE *output,
    FILE *error_output,
    CliSession *session,
    size_t configured_reference_tube_index)
{
    free(session->gamma.test_results);

    size_t reference_tube_index = configured_reference_tube_index;
    if (reference_tube_index >= session->tube_count)
    {
        reference_tube_index = 0;
    }

    session->gamma = test_gamma_tubes(
        &session->tube_config->gamma_test_params,
        session->tube_count,
        reference_tube_index);
    session->gamma_completed =
        session->gamma.test_results != NULL &&
        session->gamma.count == session->tube_count;

    if (!session->gamma_completed)
    {
        fprintf(error_output, "Errore durante il test gamma.\n");
        return false;
    }

    print_gamma_results(output, session);
    return true;
}

static void print_starting_voltage_results(
    FILE *output,
    const CliSession *session)
{
    fprintf(output, "Starting voltage completato:\n");

    for (size_t i = 0; i < session->starting_voltage.count; i++)
    {
        const StartingVoltageTestResult *result =
            &session->starting_voltage.test_results[i];
        fprintf(
            output,
            "  %s: stato=%s",
            session->tube_codes[result->tube_index],
            starting_voltage_status_string(result->status));

        if (result->status == STARTING_VOLTAGE_FOUND)
        {
            fprintf(
                output,
                ", tensione=%d V, eventi=%u, deviazione=%.2f%%",
                result->starting_voltage,
                result->detected_events,
                result->deviation_percent);
        }

        fprintf(
            output,
            ", esito=%s\n",
            result->test_passed ? "PASS" : "FAIL");
    }
}

static bool run_starting_voltage_test(
    FILE *output,
    FILE *error_output,
    CliSession *session)
{
    free_starting_voltage_test_results(&session->starting_voltage);

    session->starting_voltage = test_starting_voltage_tubes(
        &session->tube_config->starting_voltage_test_params,
        session->tube_count);
    session->starting_voltage_completed =
        session->starting_voltage.error == STARTING_VOLTAGE_TEST_OK &&
        session->starting_voltage.test_results != NULL &&
        session->starting_voltage.count == session->tube_count;

    if (!session->starting_voltage_completed)
    {
        fprintf(
            error_output,
            "Errore durante il test starting voltage (codice %d).\n",
            session->starting_voltage.error);
        return false;
    }

    print_starting_voltage_results(output, session);
    return true;
}

static void print_dead_time_results(
    FILE *output,
    const CliSession *session)
{
    fprintf(output, "Dead time completato:\n");

    for (size_t i = 0; i < session->dead_time.count; i++)
    {
        const DeadTimeTestResult *result = &session->dead_time.test_results[i];
        fprintf(
            output,
            "  %s: misurato=%.2f us, riferimento=%.2f us, "
            "deviazione=%.2f%%, esito=%s\n",
            session->tube_codes[result->tube_index],
            result->measured_dead_time_us,
            result->reference_dead_time_us,
            result->deviation_percent,
            result->test_passed ? "PASS" : "FAIL");
    }
}

static bool run_dead_time_test(
    FILE *output,
    FILE *error_output,
    CliSession *session,
    const double *measured_dead_times_us)
{
    free_dead_time_test_results(&session->dead_time);

    session->dead_time = test_dead_times(
        &session->tube_config->dead_time_test_params,
        measured_dead_times_us,
        session->tube_count);
    session->dead_time_completed =
        session->dead_time.error == DEAD_TIME_TEST_OK &&
        session->dead_time.test_results != NULL &&
        session->dead_time.count == session->tube_count;

    if (!session->dead_time_completed)
    {
        fprintf(
            error_output,
            "Errore durante il test dead time (codice %d).\n",
            session->dead_time.error);
        return false;
    }

    print_dead_time_results(output, session);
    return true;
}

static void run_all_tests(
    FILE *output,
    FILE *error_output,
    CliSession *session,
    size_t gamma_reference_tube_index,
    const double *measured_dead_times_us)
{
    run_gamma_test(
        output,
        error_output,
        session,
        gamma_reference_tube_index);
    run_starting_voltage_test(output, error_output, session);
    run_dead_time_test(
        output,
        error_output,
        session,
        measured_dead_times_us);
}

static void print_reports(
    FILE *output,
    FILE *error_output,
    const CliSession *session,
    const Config *config)
{
    if (!all_tests_completed(session))
    {
        fprintf(
            output,
            "Report non disponibili: completare prima tutti e tre i test.\n");
        return;
    }

    size_t reports_written = 0;

    for (size_t i = 0; i < session->tube_count; i++)
    {
        TestReportData report_data = {
            .config = config,
            .tube_code = session->tube_codes[i],
            .tube_profile = session->tube_profile,
            .tube_index = i,
            .gamma = &session->gamma,
            .starting_voltage = &session->starting_voltage,
            .dead_time = &session->dead_time};

        char report_path[PATH_MAX];
        if (write_test_report(
                &report_data,
                report_path,
                sizeof(report_path)) != 0)
        {
            fprintf(
                error_output,
                "Errore durante la scrittura del report per %s.\n",
                session->tube_codes[i]);
            continue;
        }

        fprintf(output, "Report scritto in: %s\n", report_path);
        reports_written++;
    }

    fprintf(
        output,
        "Report generati: %zu/%zu.\n",
        reports_written,
        session->tube_count);
}

static void free_session(CliSession *session)
{
    free(session->tube_codes);
    free(session->gamma.test_results);
    free_starting_voltage_test_results(&session->starting_voltage);
    free_dead_time_test_results(&session->dead_time);
    *session = (CliSession){0};
}

int run_cli(
    FILE *input,
    FILE *output,
    FILE *error_output,
    const Config *config,
    size_t maximum_tubes,
    size_t gamma_reference_tube_index)
{
    const double measured_dead_times_us[] = {
        190.0,
        187.0,
        193.0,
        185.0,
        195.0,
        200.0};
    const size_t measured_dead_time_count =
        sizeof(measured_dead_times_us) / sizeof(measured_dead_times_us[0]);

    if (input == NULL || output == NULL || error_output == NULL ||
        config == NULL || maximum_tubes == 0 ||
        maximum_tubes > measured_dead_time_count)
    {
        return -1;
    }

    CliSession session = {0};
    char tube_count_prompt[INPUT_BUFFER_SIZE];
    snprintf(
        tube_count_prompt,
        sizeof(tube_count_prompt),
        "Numero di tubi da testare [1-%zu]: ",
        maximum_tubes);

    fprintf(output, "\nTube Test CLI\n");
    fprintf(output, "=============\n");

    if (!prompt_number(
            input,
            output,
            1,
            maximum_tubes,
            tube_count_prompt,
            &session.tube_count))
    {
        fprintf(error_output, "Input terminato prima di iniziare la sessione.\n");
        return -1;
    }

    session.tube_codes = calloc(
        session.tube_count,
        sizeof(*session.tube_codes));
    if (session.tube_codes == NULL)
    {
        fprintf(error_output, "Errore di allocazione della memoria.\n");
        return -1;
    }

    if (!prompt_tube_codes(input, output, &session) ||
        !prompt_tube_type(input, output, config, &session))
    {
        fprintf(error_output, "Input terminato prima di iniziare la sessione.\n");
        free_session(&session);
        return -1;
    }

    while (true)
    {
        size_t choice;
        print_menu(output, &session);

        if (!prompt_number(
                input,
                output,
                0,
                5,
                "Scelta: ",
                &choice))
        {
            fprintf(output, "\nInput terminato.\n");
            break;
        }

        switch (choice)
        {
            case 0:
                free_session(&session);
                return 0;
            case 1:
                run_gamma_test(
                    output,
                    error_output,
                    &session,
                    gamma_reference_tube_index);
                break;
            case 2:
                run_starting_voltage_test(output, error_output, &session);
                break;
            case 3:
                run_dead_time_test(
                    output,
                    error_output,
                    &session,
                    measured_dead_times_us);
                break;
            case 4:
                run_all_tests(
                    output,
                    error_output,
                    &session,
                    gamma_reference_tube_index,
                    measured_dead_times_us);
                break;
            case 5:
                print_reports(output, error_output, &session, config);
                break;
        }
    }

    free_session(&session);
    return 0;
}
