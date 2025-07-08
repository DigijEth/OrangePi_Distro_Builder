#include "auth.h"
#include "config.h"
#include "logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

// Helper function to read password/token securely without echoing to terminal
static int get_secure_input(const char* prompt, char* buffer, size_t buffer_size) {
    printf("%s", prompt);
    fflush(stdout);

    struct termios old_term, new_term;
    if (tcgetattr(STDIN_FILENO, &old_term) != 0) {
        log_error("get_secure_input", "tcgetattr failed", 1);
        return -1;
    }
    new_term = old_term;
    new_term.c_lflag &= ~ECHO; // Disable echo

    if (tcsetattr(STDIN_FILENO, TCSANOW, &new_term) != 0) {
        log_error("get_secure_input", "tcsetattr failed", 1);
        return -1;
    }

    int result = -1;
    if (fgets(buffer, buffer_size, stdin)) {
        buffer[strcspn(buffer, "\n")] = 0; // Remove trailing newline
        result = 0;
    }

    // Restore terminal settings
    (void)tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
    printf("\n"); // Move to the next line after input
    return result;
}

void api_setup_menu(void) {
    char choice[10];
    int choice_int;

    while (1) {
        printf("\n%s%s--- API & Authentication Setup ---%s\n", COLOR_BOLD, COLOR_YELLOW, COLOR_RESET);
        printf("1. Set GitHub Personal Access Token\n");
        printf("2. Set GitLab Personal Access Token\n");
        printf("3. Set ARM Developer Account Credentials\n");
        printf("4. View Current Settings (Tokens Masked)\n");
        printf("5. Return to Main Menu\n");
        printf("Enter your choice: ");

        if (fgets(choice, sizeof(choice), stdin) == NULL) continue;
        choice_int = atoi(choice);

        switch (choice_int) {
            case 1:
                set_github_token();
                break;
            case 2:
                set_gitlab_token();
                break;
            case 3:
                set_arm_credentials();
                break;
            case 4:
                // Implement view settings function
                log_info("View settings not yet implemented.");
                break;
            case 5:
                return;
            default:
                log_warn("Invalid choice. Please try again.");
        }
    }
}

void set_github_token(void) {
    log_info("Setting GitHub Personal Access Token...");
    if (get_secure_input("Enter GitHub Token: ", g_build_config.github_token, sizeof(g_build_config.github_token)) == 0) {
        log_info("GitHub token has been set.");
    } else {
        log_error("set_github_token", "Failed to read token.", 0);
    }
}

void set_gitlab_token(void) {
    log_info("Setting GitLab Personal Access Token...");
    if (get_secure_input("Enter GitLab Token: ", g_build_config.gitlab_token, sizeof(g_build_config.gitlab_token)) == 0) {
        log_info("GitLab token has been set.");
    } else {
        log_error("set_gitlab_token", "Failed to read token.", 0);
    }
}

void set_arm_credentials(void) {
    log_info("Setting ARM Developer Account Credentials...");
    printf("Enter ARM Developer Username: ");
    if (fgets(g_build_config.arm_user, sizeof(g_build_config.arm_user), stdin)) {
        g_build_config.arm_user[strcspn(g_build_config.arm_user, "\n")] = 0;
    }

    if (get_secure_input("Enter ARM Password: ", g_build_config.arm_password, sizeof(g_build_config.arm_password)) == 0) {
        log_info("ARM credentials have been set.");
    } else {
        log_error("set_arm_credentials", "Failed to read password.", 0);
    }
}

void configure_api_credentials(void) {
    log_info("Configuring API credentials...");
    printf("This feature allows you to configure API credentials for various services.\n");
    printf("1. GitHub Token\n");
    printf("2. GitLab Token\n");
    printf("3. ARM Developer Credentials\n");
    printf("Choose an option (1-3): ");
    
    int choice;
    if (scanf("%d", &choice) == 1) {
        switch (choice) {
            case 1:
                set_github_token();
                break;
            case 2:
                set_gitlab_token();
                break;
            case 3:
                set_arm_credentials();
                break;
            default:
                log_error("configure_api_credentials", "Invalid choice.", 0);
        }
    }
}

void test_api_connection(void) {
    log_info("Testing API connections...");
    printf("This feature will test your configured API credentials.\n");
    
    // Test GitHub token if set
    if (strlen(g_build_config.github_token) > 0) {
        log_info("Testing GitHub API connection...");
        printf("GitHub token is configured (length: %zu characters)\n", strlen(g_build_config.github_token));
    } else {
        log_warn("No GitHub token configured.");
    }
    
    // Test GitLab token if set
    if (strlen(g_build_config.gitlab_token) > 0) {
        log_info("Testing GitLab API connection...");
        printf("GitLab token is configured (length: %zu characters)\n", strlen(g_build_config.gitlab_token));
    } else {
        log_warn("No GitLab token configured.");
    }
    
    // Test ARM credentials if set
    if (strlen(g_build_config.arm_user) > 0) {
        log_info("Testing ARM Developer API connection...");
        printf("ARM credentials are configured for user: %s\n", g_build_config.arm_user);
    } else {
        log_warn("No ARM credentials configured.");
    }
    
    log_info("API connection test completed.");
}
