//gsoap_commands.c
#include "soapH.h"
#include "gsoap_commands.nsmap"

#include "../includes/commands.h"
#include "../includes/message.h"
#include <stdlib.h>


int ns__generate_id(struct soap* soap, command_result* response) {
    command_result* result = generate_id(soap->socket);
    memcpy((void*)response, (void*)result, sizeof(command_result));
    return SOAP_OK;
}


int ns__create_journal(struct create_journal_input* request, command_result* response) {
    command_result* result = create_journal(request->id, request->journal_name);
    memcpy((void*)response, (void*)result, sizeof(command_result));
    return SOAP_OK;
}


int ns__retrieve_journal(struct retrieve_journal_input* request, command_result* response) {
    command_result* result = retrieve_journal(request->id, request->journal_name);
    memcpy((void*)response, (void*)result, sizeof(command_result));
    return SOAP_OK;
}


int ns__import_journal(struct import_journal_input* request, command_result* response) {
    command_result* result = import_journal(request->id, request->journal_name, request->journal_data, request->journal_data_size);
    memcpy((void*)response, (void*)result, sizeof(command_result));
    return SOAP_OK;
}


int ns__modify_journal(struct modify_journal_input* request, command_result* response) {
    command_result* result = modify_journal(request->id, request->journal_name, request->journal_data, request->journal_data_size);
    memcpy((void*)response, (void*)result, sizeof(command_result));
    return SOAP_OK;
}


int ns__delete_journal(struct delete_journal_input* request, command_result* response) {
    command_result* result = delete_journal(request->id, request->journal_name, request->journal_data, request->journal_data_size);
    memcpy((void*)response, (void*)result, sizeof(command_result));
    return SOAP_OK;
}

int main() {
    struct soap soap;
    soap_init(&soap);

    if (soap_bind(&soap, NULL, 8080, 100) < 0) {
        soap_print_fault(&soap, stderr);
        exit(EXIT_FAILURE);
    }

    while (1) {
        soap_accept(&soap);
        if (soap_begin_serve(&soap) == SOAP_OK) {
            soap_serve(&soap);
        }
        soap_destroy(&soap);
        soap_end(&soap);
    }

    soap_done(&soap);
    return 0;
}