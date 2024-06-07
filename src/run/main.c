#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <occlum_pal_api.h>
#include <sys/prctl.h>

void current_timespec(char *boot_stage) {
    struct timespec ts_asm;
    int ret;

    // Get time using inline assembly for syscall
    __asm__ volatile (
        "mov $228, %%eax\n"   // syscall number for clock_gettime (228 on x86_64)
        "mov $0, %%edi\n"     // CLOCK_REALTIME
        "mov %1, %%rsi\n"     // Pointer to timespec struct
        "syscall\n"           // Make the system call
        "mov %%eax, %0\n"     // Store the return value in ret
        : "=r" (ret)          // Output operand
        : "r" (&ts_asm)       // Input operand
        : "eax", "edi", "rsi" // Clobbered registers
    );

    if (ret == 0) {
        fprintf(stderr, "boot_stage: %s at: %ld.%09ld seconds\n", boot_stage, ts_asm.tv_sec, ts_asm.tv_nsec);
    } else {
        fprintf(stderr, "[ERROR] current_time (assembly)");
    }
}
int main(int argc, char *argv[]) {
    // Parse arguments
    if (argc < 2) {
        fprintf(stderr, "[ERROR] occlum-run: at least one argument must be provided\n\n");
        fprintf(stderr, "Usage: occlum-run <executable> [<args>]\n");
        return EXIT_FAILURE;
    }

    char **cmd_args = &argv[1];
    char *cmd_path = strdup(argv[1]);
    extern const char **environ;

    // Change cmd_args[0] from program path to program name in place (e.g., "/bin/abc" to "abc")
    char *cmd_path_tmp = strdup(cmd_path);
    const char *program_name = (const char *) basename(cmd_path_tmp);
    memset(cmd_args[0], 0, strlen(cmd_args[0]));
    memcpy(cmd_args[0], program_name, strlen(program_name));
    fprintf(stderr, "start to boot program: %s\n", program_name);

    // Check Occlum PAL version
    int pal_version = occlum_pal_get_version();
    if (pal_version <= 0) {
        return EXIT_FAILURE;
    }

    // Init Occlum PAL
    struct occlum_pal_attr attr = OCCLUM_PAL_ATTR_INITVAL;
    attr.log_level = getenv("OCCLUM_LOG_LEVEL");
    current_timespec("start occlum_pal_init");
    if (occlum_pal_init(&attr) < 0) {
        return EXIT_FAILURE;
    }
    current_timespec("finish occlum_pal_init");
    // Use Occlum PAL to execute the cmd
    struct occlum_stdio_fds io_fds = {
        .stdin_fd = STDIN_FILENO,
        .stdout_fd = STDOUT_FILENO,
        .stderr_fd = STDERR_FILENO,
    };
    int exit_status = 0;
    int libos_tid = 0;
    struct occlum_pal_create_process_args create_process_args = {
        .path = (const char *) cmd_path,
        .argv = (const char **) cmd_args,
        .env = environ,
        .stdio = (const struct occlum_stdio_fds *) &io_fds,
        .pid = &libos_tid,
    };
    current_timespec("start occlum_pal_create_process (user program)");
    if (occlum_pal_create_process(&create_process_args) < 0) {
        // Command not found or other internal errors
        return 127;
    }
    current_timespec("finish occlum_pal_create_process (user program)");

    struct occlum_pal_exec_args exec_args = {
        .pid = libos_tid,
        .exit_value = &exit_status,
    };
    current_timespec("start occlum_pal_exec (user program)");
    if (occlum_pal_exec(&exec_args) < 0) {
        // Command not found or other internal errors
        return 127;
    }
    current_timespec("finish occlum_pal_exec (user program)");

    // Convert the exit status to a value in a shell-like encoding
    if (WIFEXITED(exit_status)) { // terminated normally
        exit_status = WEXITSTATUS(exit_status) & 0x7F; // [0, 127]
    } else { // killed by signal
        exit_status = 128 + WTERMSIG(exit_status); // [128 + 1, 128 + 64]
    }

    // Destroy Occlum PAL
    current_timespec("start occlum_pal_destroy");
    occlum_pal_destroy();
    current_timespec("finish occlum_pal_destroy");

    return exit_status;
}
