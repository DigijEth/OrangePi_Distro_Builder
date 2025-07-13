/*
 * ═══════════════════════════════════════════════════════════════════════════════════════════
 *                  ORANGE PI 5 PLUS ULTIMATE INTERACTIVE BUILDER
 *                           Setec Labs Presents: v0.1.1a
 *                               By: Digijeth
 * ═══════════════════════════════════════════════════════════════════════════════════════════
 * 
 * LEGAL NOTICE:
 * This software is provided by Setec Labs for legitimate purposes only. NO games, BIOS files,
 * or copyrighted software will be installed. Setec Labs does not support piracy in any form.
 * Users are responsible for complying with all applicable laws and regulations.
 * 
 * PROJECT FEATURES:
 * • Interactive menu-driven interface for ease of use
 * • Multiple Ubuntu versions (20.04 LTS through 25.04)
 * • Full Mali G610 GPU support with hardware acceleration
 * • Custom distributions: Desktop, Server, or Emulation-focused
 * • LibreELEC, EmulationStation, and RetroPie integration options
 * • Comprehensive error handling and recovery
 * • Build progress tracking and logging
 * 
 * INTEGRATED PROJECTS:
 * • Joshua-Riek Ubuntu Rockchip: https://github.com/Joshua-Riek/ubuntu-rockchip
 * • JeffyCN Mali Drivers: https://github.com/JeffyCN/mirrors/raw/libmali/
 * • LibreELEC: https://libreelec.tv/
 * • EmulationStation: https://emulationstation.org/
 * • RetroPie: https://retropie.org.uk/
 * 
 * CHANGES FROM v0.1.0a:
 * • Fixed /proc mounting issues in chroot environments
 * • Enhanced locale configuration during rootfs creation
 * • Improved GitHub authentication handling
 * • Added wrapper script for filesystem mounting
 * • Better error handling and cleanup on failures
 * 
 * ═══════════════════════════════════════════════════════════════════════════════════════════
 */

#include "builder.h"
#include "modules/debug.h"

#if DEBUG_ENABLED
    // Initialize debug system
    debug_init();
#endif

// Global variables
FILE *log_fp = NULL;
FILE *error_log_fp = NULL;
build_config_t *global_config = NULL;
volatile sig_atomic_t interrupted = 0;
menu_state_t menu_state = {0};

// Ubuntu release information
ubuntu_release_t ubuntu_releases[] = {
    {"20.04", "focal", "Ubuntu 20.04 LTS (Focal Fossa)", "5.4", 1, 1, "ubuntu-20.04"},
    {"22.04", "jammy", "Ubuntu 22.04 LTS (Jammy Jellyfish)", "5.15", 1, 1, "ubuntu-22.04"},
    {"24.04", "noble", "Ubuntu 24.04 LTS (Noble Numbat)", "6.8", 1, 1, "ubuntu-24.04"},
    {"25.04", "plucky", "Ubuntu 25.04 (Plucky Puffin)", "6.9", 0, 1, "ubuntu-25.04"},
    {"25.10", "vivid", "Ubuntu 25.10 (Vibrant Vervet)", "6.10", 0, 0, "ubuntu-devel"},
    {"", "", "", "", 0, 0, ""}  // Sentinel
};

// Mali driver information
mali_driver_t mali_drivers[] = {
    {
        "Mali G610 CSF Firmware", 
        "https://github.com/JeffyCN/mirrors/raw/libmali/firmware/g610/mali_csffw.bin",
        "mali_csffw.bin",
        1  // Required
    },
    {
        "Mali G610 Wayland Driver", 
        "https://github.com/JeffyCN/mirrors/raw/libmali/lib/aarch64-linux-gnu/libmali-valhall-g610-g6p0-wayland-gbm.so",
        "libmali-valhall-g610-g6p0-wayland-gbm.so",
        1  // Required
    },
    {
        "Mali G610 X11+Wayland Driver", 
        "https://github.com/JeffyCN/mirrors/raw/libmali/lib/aarch64-linux-gnu/libmali-valhall-g610-g6p0-x11-wayland-gbm.so",
        "libmali-valhall-g610-g6p0-x11-wayland-gbm.so",
        1  // Required
    },
    {
        "Mali G610 Vulkan Driver", 
        "https://github.com/JeffyCN/mirrors/raw/libmali/lib/aarch64-linux-gnu/libmali-valhall-g610-g6p0-wayland-gbm-vulkan.so",
        "libmali-valhall-g610-g6p0-wayland-gbm-vulkan.so",
        0  // Optional
    },
    {
        "", "", "", 0  // Sentinel
    }
};

// Create .env template file (builder.c version - wrapper)
void create_env_template_builder(void) {
    // Just call the system.c version
    create_env_template();
}

// Validate GitHub token format
int validate_github_token(const char* token) {
    if (!token || strlen(token) == 0) {
        return 0;  // No token
    }
    
    size_t len = strlen(token);
    
    // Check for valid GitHub token formats
    if (strncmp(token, "ghp_", 4) == 0) {
        // Classic personal access token
        return (len == 40) ? 1 : 0;  // Should be exactly 40 characters
    } else if (strncmp(token, "github_pat_", 11) == 0) {
        // Fine-grained personal access token
        return (len >= 50) ? 1 : 0;  // Should be at least 50 characters
    } else if (strncmp(token, "gho_", 4) == 0) {
        // OAuth token
        return (len >= 36) ? 1 : 0;
    } else if (strncmp(token, "ghu_", 4) == 0) {
        // User-to-server token
        return (len >= 36) ? 1 : 0;
    } else if (strncmp(token, "ghs_", 4) == 0) {
        // Server-to-server token
        return (len >= 36) ? 1 : 0;
    } else if (strncmp(token, "ghr_", 4) == 0) {
        // Refresh token
        return (len >= 36) ? 1 : 0;
    }
    
    return 0;  // Unknown format
}

// Get token type description
const char* get_token_type_description(const char* token) {
    if (!token || strlen(token) == 0) {
        return "No token";
    }
    
    if (strncmp(token, "ghp_", 4) == 0) {
        return "Classic Personal Access Token";
    } else if (strncmp(token, "github_pat_", 11) == 0) {
        return "Fine-grained Personal Access Token";
    } else if (strncmp(token, "gho_", 4) == 0) {
        return "OAuth Token";
    } else if (strncmp(token, "ghu_", 4) == 0) {
        return "User-to-server Token";
    } else if (strncmp(token, "ghs_", 4) == 0) {
        return "Server-to-server Token";
    } else if (strncmp(token, "ghr_", 4) == 0) {
        return "Refresh Token";
    }
    
    return "Unknown token format";
}

// Test GitHub token validity
int test_github_token(const char* token) {
    if (!token || strlen(token) == 0) {
        return 0;
    }
    
    char cmd[512];
    int result;
    
    // Test the token by making a simple API call
    snprintf(cmd, sizeof(cmd), 
             "curl -s -f -H \"Authorization: token %s\" "
             "https://api.github.com/user >/dev/null 2>&1", 
             token);
    
    result = system(cmd);
    
    return (result == 0) ? 1 : 0;
}

// Configure git to use GitHub token
int configure_git_with_token(const char* token) {
    if (!token || strlen(token) == 0) {
        return 0;
    }
    
    char cmd[1024];
    error_context_t error_ctx = {0};
    
    // Configure git credential helper to use the token
    snprintf(cmd, sizeof(cmd), 
             "git config --global credential.\"https://github.com\".helper "
             "'!f() { echo \"username=x-access-token\"; echo \"password=%s\"; }; f'", 
             token);
    
    if (execute_command_safe(cmd, 0, &error_ctx) != 0) {
        return 0;
    }
    
    // Configure URL rewriting for HTTPS
    snprintf(cmd, sizeof(cmd),
             "git config --global url.\"https://x-access-token:%s@github.com/\".insteadOf \"https://github.com/\"",
             token);
    
    if (execute_command_safe(cmd, 0, &error_ctx) != 0) {
        return 0;
    }
    
    // Configure URL rewriting for SSH to HTTPS (for repositories that use git@ URLs)
    snprintf(cmd, sizeof(cmd),
             "git config --global url.\"https://x-access-token:%s@github.com/\".insteadOf \"git@github.com:\"",
             token);
    
    if (execute_command_safe(cmd, 0, &error_ctx) != 0) {
        return 0;
    }
    
    return 1;
}

// Initialize build configuration
void init_build_config(build_config_t *config) {
    if (!config) return;
    
    // Set default values
    strcpy(config->kernel_version, "6.8.0");  // Use a stable kernel version
    strcpy(config->build_dir, BUILD_DIR);
    strcpy(config->output_dir, "/tmp/opi5plus_output");
    strcpy(config->cross_compile, "aarch64-linux-gnu-");
    strcpy(config->arch, "arm64");
    strcpy(config->defconfig, "rockchip_defconfig");
    
    // Ubuntu release - default to a stable, supported version
    strcpy(config->ubuntu_release, "24.04");
    strcpy(config->ubuntu_codename, "noble");
    
    // Distribution type
    config->distro_type = DISTRO_DESKTOP;
    config->emu_platform = EMU_NONE;
    
    // Build options
    config->jobs = sysconf(_SC_NPROCESSORS_ONLN);
    if (config->jobs <= 0) config->jobs = 4;
    
    // Check .env for custom settings
    FILE *fp = fopen(".env", "r");
    if (fp) {
        char line[512];
        while (fgets(line, sizeof(line), fp)) {
            if (strncmp(line, "BUILD_JOBS=", 11) == 0) {
                int jobs = atoi(line + 11);
                if (jobs > 0 && jobs <= 128) {
                    config->jobs = jobs;
                }
            } else if (strncmp(line, "OUTPUT_DIR=", 11) == 0) {
                char *value = line + 11;
                char *nl = strchr(value, '\n');
                if (nl) *nl = '\0';
                strncpy(config->output_dir, value, sizeof(config->output_dir) - 1);
                config->output_dir[sizeof(config->output_dir) - 1] = '\0';
            }
        }
        fclose(fp);
    }
    
    config->verbose = 0;
    config->clean_build = 0;
    config->continue_on_error = 0;
    config->log_level = LOG_LEVEL_INFO;
    
    // GPU options
    config->install_gpu_blobs = 1;
    config->enable_opencl = 1;
    config->enable_vulkan = 1;
    
    // Component selection
    config->build_kernel = 1;
    config->build_rootfs = 1;
    config->build_uboot = 1;
    config->create_image = 1;
    
    // Image settings
    strcpy(config->image_size, "8192");
    strcpy(config->hostname, "orangepi");
    strcpy(config->username, "orangepi");
    strcpy(config->password, "orangepi");
}

// Create required directories
int ensure_directories_exist(build_config_t *config) {
    char cmd[MAX_CMD_LEN * 2];  // Increased buffer size
    error_context_t error_ctx = {0};
    
    // Create directories one at a time to avoid buffer overflow
    const char *dirs[] = {
        "%s/rootfs/boot",
        "%s/rootfs/etc",
        "%s/rootfs/lib",
        "%s/rootfs/usr/bin",
        "%s/rootfs/usr/lib",
        NULL
    };
    
    for (int i = 0; dirs[i] != NULL; i++) {
        snprintf(cmd, sizeof(cmd), "mkdir -p ");
        int len = strlen(cmd);
        snprintf(cmd + len, sizeof(cmd) - len, dirs[i], config->output_dir);
        
        if (execute_command_safe(cmd, 0, &error_ctx) != 0) {
            LOG_ERROR("Failed to create output directory");
            return ERROR_FILE_NOT_FOUND;
        }
    }
    
    // Create build directory
    if (create_directory_safe(config->build_dir, &error_ctx) != 0) {
        LOG_ERROR("Failed to create build directory");
        return ERROR_FILE_NOT_FOUND;
    }
    
    LOG_DEBUG("All required directories created successfully");
    return ERROR_SUCCESS;
}

// Process command line arguments
void process_args(int argc, char *argv[], build_config_t *config) {
    int i;
    
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("Usage: %s [OPTIONS]\n", argv[0]);
            printf("Options:\n");
            printf("  --kernel-version VERSION   Kernel version (default: %s)\n", config->kernel_version);
            printf("  --build-dir DIR           Build directory (default: %s)\n", config->build_dir);
            printf("  --output-dir DIR          Output directory (default: %s)\n", config->output_dir);
            printf("  --jobs N                  Number of parallel jobs (default: %d)\n", config->jobs);
            printf("  --ubuntu VERSION          Ubuntu release (default: %s)\n", config->ubuntu_release);
            printf("  --disable-gpu             Disable Mali GPU support\n");
            printf("  --disable-opencl          Disable OpenCL support\n");
            printf("  --disable-vulkan          Disable Vulkan support\n");
            printf("  --no-kernel               Skip kernel building\n");
            printf("  --no-rootfs               Skip rootfs building\n");
            printf("  --no-uboot                Skip U-Boot building\n");
            printf("  --no-image                Skip image creation\n");
            printf("  --clean                   Clean previous build\n");
            printf("  --verbose                 Verbose output\n");
            printf("  --help                    Show this help\n");
            exit(0);
        } else if (strcmp(argv[i], "--kernel-version") == 0) {
            if (i + 1 < argc) {
                strncpy(config->kernel_version, argv[i + 1], sizeof(config->kernel_version) - 1);
                config->kernel_version[sizeof(config->kernel_version) - 1] = '\0';
                i++;
            }
        } else if (strcmp(argv[i], "--build-dir") == 0) {
            if (i + 1 < argc) {
                strncpy(config->build_dir, argv[i + 1], sizeof(config->build_dir) - 1);
                config->build_dir[sizeof(config->build_dir) - 1] = '\0';
                i++;
            }
        } else if (strcmp(argv[i], "--output-dir") == 0) {
            if (i + 1 < argc) {
                strncpy(config->output_dir, argv[i + 1], sizeof(config->output_dir) - 1);
                config->output_dir[sizeof(config->output_dir) - 1] = '\0';
                i++;
            }
        } else if (strcmp(argv[i], "--jobs") == 0) {
            if (i + 1 < argc) {
                config->jobs = atoi(argv[i + 1]);
                i++;
            }
        } else if (strcmp(argv[i], "--ubuntu") == 0) {
            if (i + 1 < argc) {
                strncpy(config->ubuntu_release, argv[i + 1], sizeof(config->ubuntu_release) - 1);
                config->ubuntu_release[sizeof(config->ubuntu_release) - 1] = '\0';
                ubuntu_release_t *release = find_ubuntu_release(config->ubuntu_release);
                if (release) {
                    strncpy(config->ubuntu_codename, release->codename, sizeof(config->ubuntu_codename) - 1);
                    config->ubuntu_codename[sizeof(config->ubuntu_codename) - 1] = '\0';
                }
                i++;
            }
        } else if (strcmp(argv[i], "--disable-gpu") == 0) {
            config->install_gpu_blobs = 0;
        } else if (strcmp(argv[i], "--disable-opencl") == 0) {
            config->enable_opencl = 0;
        } else if (strcmp(argv[i], "--disable-vulkan") == 0) {
            config->enable_vulkan = 0;
        } else if (strcmp(argv[i], "--no-kernel") == 0) {
            config->build_kernel = 0;
        } else if (strcmp(argv[i], "--no-rootfs") == 0) {
            config->build_rootfs = 0;
        } else if (strcmp(argv[i], "--no-uboot") == 0) {
            config->build_uboot = 0;
        } else if (strcmp(argv[i], "--no-image") == 0) {
            config->create_image = 0;
        } else if (strcmp(argv[i], "--clean") == 0) {
            config->clean_build = 1;
        } else if (strcmp(argv[i], "--verbose") == 0) {
            config->verbose = 1;
        } else {
            printf("Unknown option: %s\n", argv[i]);
        }
    }
}

// Start interactive build
int start_interactive_build(build_config_t *config) {
    int choice;
    
    print_header();
    print_legal_notice();
    
    while (1) {
        show_main_menu();
        
        choice = get_user_choice("Enter your choice", 0, 6);
        
        switch (choice) {
            case 0:  // Exit
                if (confirm_action("Exit the builder?")) {
                    LOG_INFO("Exiting builder");
                    return ERROR_SUCCESS;
                }
                break;
                
            case 1:  // Quick Setup
                show_quick_setup_menu();
                if (confirm_action("Proceed with quick setup?")) {
                    return perform_quick_setup(config);
                }
                break;
                
            case 2:  // Custom Build
                return perform_custom_build(config);
                
            case 3:  // Emulation Focus
                config->distro_type = DISTRO_EMULATION;
                show_emulation_menu();
                choice = get_user_choice("Select emulation platform", 0, 5);
                if (choice > 0) {
                    config->emu_platform = choice;
                    if (confirm_action("Proceed with emulation build?")) {
                        return perform_custom_build(config);
                    }
                }
                break;
                
            case 4:  // Documentation
                show_help_menu();
                break;
                
            case 5:  // System Requirements
                LOG_INFO("Checking system requirements...");
                check_root_permissions();
                check_dependencies();
                check_disk_space("/tmp", 15000);
                pause_screen();
                break;
                
            case 6:  // About
                clear_screen();
                print_header();
                printf("\n%s%sABOUT ORANGE PI 5 PLUS ULTIMATE INTERACTIVE BUILDER%s\n", COLOR_BOLD, COLOR_GREEN, COLOR_RESET);
                printf("════════════════════════════════════════════════════════════════════════\n");
                printf("\n");
                printf("Version: %s\n", VERSION);
                printf("License: GPLv3\n");
                printf("Author: Setec Labs\n");
                printf("\n");
                printf("Features:\n");
                printf("• Build custom Ubuntu distributions for Orange Pi 5 Plus\n");
                printf("• Full Mali G610 GPU support (OpenCL 2.2, Vulkan 1.2)\n");
                printf("• Multiple Ubuntu versions (20.04 LTS through 25.04)\n");
                printf("• Desktop, Server, Minimal, or Emulation-focused builds\n");
                printf("• Automated kernel compilation with Rockchip patches\n");
                printf("• U-Boot bootloader support\n");
                printf("• Legal emulation platform support (NO copyrighted content)\n");
                printf("\n");
                printf("For more information, see https://github.com/seteclabs/orangepi-builder\n");
                printf("\n");
                pause_screen();
                break;
        }
    }
    
    return ERROR_SUCCESS;
}

// Perform quick setup - FIXED VERSION
int perform_quick_setup(build_config_t *config) {
    int result;
    
    // Set default quick setup options
    config->distro_type = DISTRO_DESKTOP;
    config->emu_platform = EMU_NONE;
    strcpy(config->ubuntu_release, "24.04");  // Use stable Noble instead of development version
    strcpy(config->ubuntu_codename, "noble");
    config->install_gpu_blobs = 1;
    config->enable_opencl = 1;
    config->enable_vulkan = 1;
    config->build_kernel = 1;
    config->build_rootfs = 1;
    config->build_uboot = 1;
    config->create_image = 1;
    
    clear_screen();
    print_header();
    show_build_summary(config);
    
    if (!confirm_action("Start quick setup build?")) {
        return ERROR_USER_CANCELLED;
    }
    
    LOG_INFO("Starting quick setup build...");
    
    // Ensure output directories exist
    result = ensure_directories_exist(config);
    if (result != ERROR_SUCCESS && !config->continue_on_error) {
        return result;
    }
    
    // Setup build environment
    result = setup_build_environment();
    if (result != ERROR_SUCCESS && !config->continue_on_error) {
        return result;
    }
    
    // Install prerequisites
    result = install_prerequisites();
    if (result != ERROR_SUCCESS && !config->continue_on_error) {
        return result;
    }
    
    // Download kernel source
    result = download_kernel_source(config);
    if (result != ERROR_SUCCESS && !config->continue_on_error) {
        return result;
    }
    
    // Configure kernel
    result = configure_kernel(config);
    if (result != ERROR_SUCCESS && !config->continue_on_error) {
        return result;
    }
    
    // Build kernel
    result = build_kernel(config);
    if (result != ERROR_SUCCESS && !config->continue_on_error) {
        return result;
    }
    
    // Download Mali blobs
    if (config->install_gpu_blobs) {
        result = download_mali_blobs(config);
        if (result != ERROR_SUCCESS && !config->continue_on_error) {
            return result;
        }
    }
    
    // Build rootfs
    if (config->build_rootfs) {
        result = build_ubuntu_rootfs(config);
        if (result != ERROR_SUCCESS && !config->continue_on_error) {
            return result;
        }
    }
    
    // Install kernel
    result = install_kernel(config);
    if (result != ERROR_SUCCESS && !config->continue_on_error) {
        return result;
    }
    
    // Install Mali drivers
    if (config->install_gpu_blobs) {
        result = install_mali_drivers(config);
        if (result != ERROR_SUCCESS && !config->continue_on_error) {
            return result;
        }
        
        if (config->enable_opencl) {
            result = setup_opencl_support(config);
            if (result != ERROR_SUCCESS && !config->continue_on_error) {
                return result;
            }
        }
        
        if (config->enable_vulkan) {
            result = setup_vulkan_support(config);
            if (result != ERROR_SUCCESS && !config->continue_on_error) {
                return result;
            }
        }
    }
    
    // Install system packages
    result = install_system_packages(config);
    if (result != ERROR_SUCCESS && !config->continue_on_error) {
        return result;
    }
    
    // Configure system services
    result = configure_system_services(config);
    if (result != ERROR_SUCCESS && !config->continue_on_error) {
        return result;
    }
    
    // Build U-Boot if requested
    if (config->build_uboot) {
        result = download_uboot_source(config);
        if (result != ERROR_SUCCESS && !config->continue_on_error) {
            return result;
        }
        
        result = build_uboot(config);
        if (result != ERROR_SUCCESS && !config->continue_on_error) {
            return result;
        }
    }
    
    // Create system image if requested
    if (config->create_image) {
        result = create_system_image(config);
        if (result != ERROR_SUCCESS && !config->continue_on_error) {
            return result;
        }
    }
    
    LOG_INFO("Quick setup build completed successfully!");
    
    // Show completion message
    clear_screen();
    print_header();
    printf("\n%s%sBUILD COMPLETED SUCCESSFULLY!%s\n", COLOR_BOLD, COLOR_GREEN, COLOR_RESET);
    printf("════════════════════════════════════════════════════════════════════════\n");
    printf("\nYour Orange Pi 5 Plus system has been built with the following:\n");
    printf("• Ubuntu %s (%s)\n", config->ubuntu_release, config->ubuntu_codename);
    printf("• Kernel %s with Mali G610 GPU support\n", config->kernel_version);
    printf("• Desktop environment with hardware acceleration\n");
    printf("• OpenCL 2.2 and Vulkan 1.2 support\n");
    printf("\nOutput files can be found in: %s\n", config->output_dir);
    if (config->create_image) {
        printf("\nTo flash to SD card:\n");
        printf("sudo dd if=%s/orangepi5plus-%s.img of=/dev/sdX bs=4M status=progress\n", 
               config->output_dir, config->ubuntu_codename);
    }
    printf("\n");
    pause_screen();
    
    return ERROR_SUCCESS;
}

// Perform custom build
int perform_custom_build(build_config_t *config) {
    int choice;
    
    while (1) {
        show_custom_build_menu();
        choice = get_user_choice("Select option", 0, 7);
        
        switch (choice) {
            case 0:  // Back
                return ERROR_SUCCESS;
                
            case 1:  // Distribution Type
                show_distro_selection_menu();
                choice = get_user_choice("Select distribution type", 1, 4);
                if (choice >= 1 && choice <= 4) {
                    config->distro_type = choice - 1;
                    printf("Distribution type set to: ");
                    switch (config->distro_type) {
                        case DISTRO_DESKTOP: printf("Desktop Edition\n"); break;
                        case DISTRO_SERVER: printf("Server Edition\n"); break;
                        case DISTRO_EMULATION: printf("Emulation Station\n"); break;
                        case DISTRO_MINIMAL: printf("Minimal System\n"); break;
                    }
                    pause_screen();
                }
                break;
                
            case 2:  // Ubuntu Version
                show_ubuntu_selection_menu();
                choice = get_user_choice("Select Ubuntu version", 1, 5);
                if (choice >= 1 && choice <= 5) {
                    ubuntu_release_t *release = &ubuntu_releases[choice - 1];
                    strcpy(config->ubuntu_release, release->version);
                    strcpy(config->ubuntu_codename, release->codename);
                    printf("Ubuntu version set to: %s (%s)\n", 
                           config->ubuntu_release, config->ubuntu_codename);
                    pause_screen();
                }
                break;
                
            case 3:  // Kernel Options
                printf("Current kernel version: %s\n", config->kernel_version);
                char buffer[64];
                get_user_input("Enter new kernel version (or press Enter to keep current): ", 
                              buffer, sizeof(buffer));
                if (strlen(buffer) > 0) {
                    strcpy(config->kernel_version, buffer);
                    printf("Kernel version set to: %s\n", config->kernel_version);
                }
                pause_screen();
                break;
                
            case 4:  // GPU Configuration
                while (1) {
                    show_gpu_options_menu(config);
                    int gpu_choice = get_user_choice("Select GPU option", 0, 5);
                    
                    switch (gpu_choice) {
                        case 0: goto gpu_done;
                        case 1: config->install_gpu_blobs = !config->install_gpu_blobs; break;
                        case 2: config->enable_opencl = !config->enable_opencl; break;
                        case 3: config->enable_vulkan = !config->enable_vulkan; break;
                        case 4:
                            config->install_gpu_blobs = 1;
                            config->enable_opencl = 1;
                            config->enable_vulkan = 1;
                            break;
                        case 5:
                            config->install_gpu_blobs = 0;
                            config->enable_opencl = 0;
                            config->enable_vulkan = 0;
                            break;
                    }
                }
gpu_done:
                break;
                
            case 5:  // Build Components
                printf("Build components:\n");
                printf("1. Kernel: %s\n", config->build_kernel ? "Yes" : "No");
                printf("2. Root filesystem: %s\n", config->build_rootfs ? "Yes" : "No");
                printf("3. U-Boot: %s\n", config->build_uboot ? "Yes" : "No");
                printf("4. System image: %s\n", config->create_image ? "Yes" : "No");
                choice = get_user_choice("Toggle which component (1-4, 0=done)", 0, 4);
                switch (choice) {
                    case 1: config->build_kernel = !config->build_kernel; break;
                    case 2: config->build_rootfs = !config->build_rootfs; break;
                    case 3: config->build_uboot = !config->build_uboot; break;
                    case 4: config->create_image = !config->create_image; break;
                }
                if (choice != 0) pause_screen();
                break;
                
            case 6:  // Image Settings
                show_image_settings_menu(config);
                break;
                
            case 7:  // Start Build
                show_build_summary(config);
                if (confirm_action("Start custom build?")) {
                    return perform_quick_setup(config);  // Reuse the build logic
                }
                break;
        }
    }
    
    return ERROR_SUCCESS;
}

// Main function
int main(int argc, char *argv[]) {
    build_config_t config = {0};
    int result = ERROR_SUCCESS;
    
    // Initialize configuration
    init_build_config(&config);
    global_config = &config;
    
    // Setup signal handlers
    setup_signal_handlers();
    
    // Process command line arguments
    process_args(argc, argv, &config);
    
    // Check if we have command line arguments that indicate non-interactive mode
    if (argc > 1) {
        // Non-interactive mode - validate and run
        result = validate_config(&config);
        if (result != ERROR_SUCCESS) {
            LOG_ERROR("Configuration validation failed");
            return result;
        }
        
        LOG_INFO("Starting non-interactive build...");
        result = perform_quick_setup(&config);
    } else {
        // Interactive mode
        result = start_interactive_build(&config);
    }
    
    // Cleanup
    if (log_fp) {
        fclose(log_fp);
    }
    if (error_log_fp) {
        fclose(error_log_fp);
    }
    
#if DEBUG_ENABLED
    debug_cleanup();
#endif
    
    return result;
}
