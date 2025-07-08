#ifndef GAMING_H
#define GAMING_H

#include "config.h"

// Gaming-specific build functions
int gaming_optimized_build(void);
int server_optimized_build(void);
int developer_optimized_build(void);
int custom_build_wizard(void);

// GPU and gaming driver functions
int install_gaming_gpu_drivers(void);
int setup_vulkan_support(void);
int install_opencl_support(void);
int install_gaming_libraries(void);
int install_emulation_software(void);
int install_emulationstation(void);
int install_es_themes(void);
int configure_retroarch_optimizations(void);
int configure_emulator_optimizations(void);
int install_steam_gaming_tools(void);
int configure_gpu_performance(void);
int install_box86_box64(void);
int setup_gaming_desktop(void);
int test_gpu_performance(void);

// Kernel gaming optimizations
int configure_kernel_interactive(void);
int choose_kernel_version(void);
int apply_kernel_patches(void);
int apply_gaming_kernel_optimizations(void);
int clean_kernel_build(void);

// Source management functions
int choose_kernel_source(void);
int choose_uboot_source(void);
int download_custom_patches(void);
int manage_source_cache(void);
int update_all_sources(void);
int clean_source_downloads(void);
int show_source_information(void);

// Additional menu stubs
void uboot_menu(void);
void rootfs_menu(void);
void image_creation_menu(void);
void system_config_menu(void);
void advanced_options_menu(void);

// RootFS menu functions
int install_desktop_environment(void);
int install_gaming_packages_rootfs(void);
int install_development_tools(void);
int install_media_codecs(void);
int configure_user_accounts(void);
int install_additional_software(void);
int customize_system_settings(void);
int package_rootfs(void);
int clean_rootfs_build(void);

// Image creation functions
int create_emmc_image(void);
int create_compressed_image(void);
int flash_to_sd_card(void);
int flash_to_emmc(void);
int verify_image_integrity(void);
int create_live_usb(void);
int backup_current_system(void);
int image_management_tools(void);

// System configuration functions
int configure_network_settings(void);
int configure_boot_parameters(void);
int configure_performance_profiles(void);
int configure_gpio_hardware(void);
int configure_audio_video(void);
int configure_security_settings(void);
int configure_locale_timezone(void);
int configure_services_daemons(void);
int view_system_information(void);
int apply_orangepi_optimizations(void);

// Advanced options functions
int custom_kernel_configuration(void);
int manual_package_selection(void);
int cross_compilation_settings(void);
int debug_logging_options(void);
int performance_tuning_advanced(void);
int custom_script_execution(void);
int backup_recovery_tools(void);
int system_monitoring_setup(void);
int expert_build_options(void);
int reset_factory_defaults(void);

// API and Authentication
void api_setup_menu(void);
void set_github_token(void);
void set_gitlab_token(void);
void set_arm_credentials(void);

#endif // GAMING_H
