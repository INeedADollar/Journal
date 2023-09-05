#include "interface.h"
#include "requests.h"
#include "logger.h"

#include <signal.h>


void stop_client(int signal) {
    log_info("Stopping client...");
	close_log_file();
	
    STOP_CLIENT_FLAG = 1;
}


int main(int argc, char * argv[]) {
	signal(SIGINT, stop_client);
	initialize_log_file("client.log");
	init_requests("130.61.59.146", 5000);
	init_interface();

	return 0;
}