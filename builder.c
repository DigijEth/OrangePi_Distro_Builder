/*
 * ═══════════════════════════════════════════════════════════════════════════════════════════
 *                     ORANGE PI 5 PLUS CUSTOM UBUNTU BUILDER
 * ═══════════════════════════════════════════════════════════════════════════════════════════
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "src/config.h"
#include "src/logging.h"
#include "src/system_utils.h"
#include "src/dependencies.h"
#include "src/kernel.h"
#include "src/gpu.h"
#include "src/rootfs.h"
#include "src/uboot.h"
#include "src/image.h"
#include "src/gaming.h"
#include "src/auth.h"

// Global build configuration instance
build_config_t g_build_config;

// Function prototypes
void print_usage(const char *program_name);
void print_header(void);
void main_menu(void);
void full_build_menu(void);
void kernel_menu(void);
void uboot_menu(void);
void rootfs_menu(void);
void gaming_gpu_menu(void);
void image_creation_menu(void);
void source_management_menu(void);
void system_config_menu(void);
void advanced_options_menu(void);

void initialize_build_config(void) {
    memset(&g_build_config, 0, sizeof(build_config_t));
    strncpy(g_build_config.kernel_git_url, KERNEL_GIT_URL_DEFAULT, sizeof(g_build_config.kernel_git_url) - 1);
    strncpy(g_build_config.kernel_branch, KERNEL_BRANCH_DEFAULT, sizeof(g_build_config.kernel_branch) - 1);
    strncpy(g_build_config.uboot_git_url, UBOOT_GIT_URL_DEFAULT, sizeof(g_build_config.uboot_git_url) - 1);
    strncpy(g_build_config.uboot_branch, UBOOT_BRANCH_DEFAULT, sizeof(g_build_config.uboot_branch) - 1);
    // Set other default values if needed
}

int main(int argc, char *argv[]) {
    // Suppress unused parameter warnings
    (void)argc;
    (void)argv;
    
    if (check_root() != 0) {
        log_error("main", "This program must be run as root.", 1);
        return 1;
    }

    // Initialize logging
    init_logging();

    // Initialize build configuration
    initialize_build_config();

    print_header();

    // For now, we will call the main menu directly.
    // In the future, command-line arguments could be used to script the build process.
    main_menu();

    log_info("Builder finished.");
    return 0;
}

void main_menu(void) {
    char choice[10];
    int choice_int;

    while (1) {
        printf("\n%s%s═══════════════════════════════════════════════════════════════════════════════════════════%s\n", COLOR_BOLD, COLOR_CYAN, COLOR_RESET);
        printf("%s%s                     ORANGE PI 5 PLUS CUSTOM UBUNTU BUILDER (v%s)%s\n", COLOR_BOLD, COLOR_CYAN, VERSION, COLOR_RESET);
        printf("%s%s═══════════════════════════════════════════════════════════════════════════════════════════%s\n", COLOR_BOLD, COLOR_CYAN, COLOR_RESET);
        printf("\n%s%s--- Main Menu ---%s\n", COLOR_BOLD, COLOR_YELLOW, COLOR_RESET);
        printf("1.  %sFull Build (All Steps)%s\n", COLOR_GREEN, COLOR_RESET);
        printf("2.  %sKernel Management%s\n", COLOR_BLUE, COLOR_RESET);
        printf("3.  %sU-Boot Management%s\n", COLOR_BLUE, COLOR_RESET);
        printf("4.  %sRootFS Management%s\n", COLOR_BLUE, COLOR_RESET);
        printf("5.  %sGPU & Gaming Setup%s\n", COLOR_MAGENTA, COLOR_RESET);
        printf("6.  %sCreate Bootable Image%s\n", COLOR_CYAN, COLOR_RESET);
        printf("7.  %sSource Management%s\n", COLOR_YELLOW, COLOR_RESET);
        printf("8.  %sDependency Management%s\n", COLOR_WHITE, COLOR_RESET);
        printf("9.  %sSystem Configuration%s\n", COLOR_WHITE, COLOR_RESET);
        printf("10. %sAdvanced Options%s\n", COLOR_WHITE, COLOR_RESET);
        printf("11. %sAPI Setup%s\n", COLOR_YELLOW, COLOR_RESET);
        printf("12. %sExit%s\n", COLOR_RED, COLOR_RESET);
        printf("\nEnter your choice: ");

        if (fgets(choice, sizeof(choice), stdin) == NULL) continue;
        choice_int = atoi(choice);

        switch (choice_int) {
            case 1:
                full_build_menu();
                break;
            case 2:
                kernel_menu();
                break;
            case 3:
                uboot_menu();
                break;
            case 4:
                rootfs_menu();
                break;
            case 5:
                gaming_gpu_menu();
                break;
            case 6:
                image_creation_menu();
                break;
            case 7:
                source_management_menu();
                break;
            case 8:
                dependencies_menu();
                break;
            case 9:
                system_config_menu();
                break;
            case 10:
                advanced_options_menu();
                break;
            case 11:
                api_setup_menu();
                break;
            case 12:
                printf("\n%sThank you for using Orange Pi Ubuntu Builder!%s\n", COLOR_GREEN, COLOR_RESET);
                return;
            default:
                log_warn("Invalid choice. Please try again.");
        }
    }
}

void print_usage(const char *program_name) {
    printf("Usage: %s [options]\n", program_name);
    printf("This program is interactive. Run without arguments to see the main menu.\n");
    // Future command-line options can be documented here.
}

void print_header(void) {
    printf("%s%s", COLOR_BOLD, COLOR_CYAN);
    printf("═══════════════════════════════════════════════════════════════════════════════════════════\n");
    printf("                     ORANGE PI 5 PLUS CUSTOM UBUNTU BUILDER (v%s)\n", VERSION);
    printf("═══════════════════════════════════════════════════════════════════════════════════════════\n");
    printf("%s", COLOR_RESET);
}

void full_build_menu(void) {
    char choice[10];
    int choice_int;

    while (1) {
        printf("\n%s%s--- Full Build Options ---%s\n", COLOR_BOLD, COLOR_GREEN, COLOR_RESET);
        printf("1. Quick Build (Recommended settings)\n");
        printf("2. Gaming Build (Optimized for gaming performance)\n");
        printf("3. Server Build (Minimal, no GUI)\n");
        printf("4. Developer Build (All debugging tools)\n");
        printf("5. Custom Build (Choose your options)\n");
        printf("6. Return to Main Menu\n");
        printf("Enter your choice: ");

        if (fgets(choice, sizeof(choice), stdin) == NULL) continue;
        choice_int = atoi(choice);

        switch (choice_int) {
            case 1:
                log_info("Starting Quick Build...");
                if (check_dependencies() != 0) break;
                // Quick build with standard options
                if (build_kernel() != 0) break;
                if (build_and_install_uboot(NULL) != 0) break;
                if (build_rootfs(ROOTFS_PATH) != 0) break;
                if (install_gpu_drivers_legacy(ROOTFS_PATH) != 0) break;
                if (create_boot_image(NULL) != 0) break;
                log_info("Quick Build Completed Successfully!");
                break;
            case 2:
                gaming_optimized_build();
                break;
            case 3:
                server_optimized_build();
                break;
            case 4:
                developer_optimized_build();
                break;
            case 5:
                custom_build_wizard();
                break;
            case 6:
                return;
            default:
                log_warn("Invalid choice. Please try again.");
        }
    }
}

void gaming_gpu_menu(void) {
    char choice[10];
    int choice_int;

    while (1) {
        printf("\n%s%s--- Gaming & GPU Setup ---%s\n", COLOR_BOLD, COLOR_MAGENTA, COLOR_RESET);
        printf("1. Install Gaming GPU Drivers (Mali G610 + Panfrost)\n");
        printf("2. Setup Vulkan Support\n");
        printf("3. Install OpenCL Support\n");
        printf("4. Install Gaming Libraries (SDL2, OpenGL ES)\n");
        printf("5. Install Emulation Software (RetroArch, PPSSPP)\n");
        printf("6. Install Steam Link & Gaming Tools\n");
        printf("7. Configure GPU Performance Profiles\n");
        printf("8. Install Box86/Box64 for x86 games\n");
        printf("9. Setup Gaming Desktop Environment\n");
        printf("10. Test GPU Performance\n");
        printf("11. Return to Main Menu\n");
        printf("Enter your choice: ");

        if (fgets(choice, sizeof(choice), stdin) == NULL) continue;
        choice_int = atoi(choice);

        switch (choice_int) {
            case 1:
                install_gaming_gpu_drivers();
                break;
            case 2:
                setup_vulkan_support();
                break;
            case 3:
                install_opencl_support();
                break;
            case 4:
                install_gaming_libraries();
                break;
            case 5:
                install_emulation_software();
                break;
            case 6:
                install_steam_gaming_tools();
                break;
            case 7:
                configure_gpu_performance();
                break;
            case 8:
                install_box86_box64();
                break;
            case 9:
                setup_gaming_desktop();
                break;
            case 10:
                test_gpu_performance();
                break;
            case 11:
                return;
            default:
                log_warn("Invalid choice. Please try again.");
        }
    }
}

void source_management_menu(void) {
    char choice[10];
    int choice_int;

    while (1) {
        printf("\n%s%s--- Source Code Management ---%s\n", COLOR_BOLD, COLOR_YELLOW, COLOR_RESET);
        printf("1. Choose Kernel Source\n");
        printf("2. Choose U-Boot Source\n");
        printf("3. Download Custom Patches\n");
        printf("4. Manage Local Source Cache\n");
        printf("5. Update All Sources\n");
        printf("6. Clean Source Downloads\n");
        printf("7. Show Source Information\n");
        printf("8. Return to Main Menu\n");
        printf("Enter your choice: ");

        if (fgets(choice, sizeof(choice), stdin) == NULL) continue;
        choice_int = atoi(choice);

        switch (choice_int) {
            case 1:
                choose_kernel_source();
                break;
            case 2:
                choose_uboot_source();
                break;
            case 3:
                download_custom_patches();
                break;
            case 4:
                manage_source_cache();
                break;
            case 5:
                update_all_sources();
                break;
            case 6:
                clean_source_downloads();
                break;
            case 7:
                show_source_information();
                break;
            case 8:
                return;
            default:
                log_warn("Invalid choice. Please try again.");
        }
    }
}

void kernel_menu(void) {
    char choice[10];
    int choice_int;

    while (1) {
        printf("\n%s%s--- Kernel Management ---%s\n", COLOR_BOLD, COLOR_BLUE, COLOR_RESET);
        printf("1. Download Kernel Source\n");
        printf("2. Configure Kernel (menuconfig)\n");
        printf("3. Build Kernel\n");
        printf("4. Install Kernel\n");
        printf("5. Choose Kernel Version/Branch\n");
        printf("6. Apply Custom Patches\n");
        printf("7. Gaming Kernel Optimizations\n");
        printf("8. Clean Kernel Build\n");
        printf("9. Return to Main Menu\n");
        printf("Enter your choice: ");

        if (fgets(choice, sizeof(choice), stdin) == NULL) continue;
        choice_int = atoi(choice);

        switch (choice_int) {
            case 1:
                if (check_dependencies() != 0) break;
                download_kernel_source();
                break;
            case 2:
                configure_kernel_interactive();
                break;
            case 3:
                if (check_dependencies() != 0) break;
                build_kernel();
                break;
            case 4:
                install_kernel(NULL);
                break;
            case 5:
                choose_kernel_version();
                break;
            case 6:
                apply_kernel_patches();
                break;
            case 7:
                apply_gaming_kernel_optimizations();
                break;
            case 8:
                clean_kernel_build();
                break;
            case 9:
                return;
            default:
                log_warn("Invalid choice. Please try again.");
        }
    }
}


// Additional menu implementations would continue here...
// For brevity, I'll add the key gaming functions