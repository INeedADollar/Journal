#include "interface.h"
#include "requests.h"

#include <signal.h>


void stop_client(int signal) {
    log_info("Stopping client...");
    STOP_CLIENT = 1;
}


int main(int argc, char * argv[]) {
	signal(SIGINT, stop_client);
	init_requests("", 5000);
	init_interface();

	return 0;
}