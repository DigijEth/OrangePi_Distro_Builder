#ifndef AUTH_H
#define AUTH_H

// Function to display the API setup menu
void api_setup_menu(void);

// Functions to set credentials for different services
void set_github_token(void);
void set_gitlab_token(void);
void set_arm_credentials(void);

// Additional API functions
void configure_api_credentials(void);
void test_api_connection(void);

#endif // AUTH_H
