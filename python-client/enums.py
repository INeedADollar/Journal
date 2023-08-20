from enum import Enum

class OperationStatus(Enum):
    OPERATION_SUCCESFUL = 0
    OPERATION_FAIL = 1

class CommandTypes(Enum):
    GENERATE_ID = 0
    CREATE_JOURNAL = 1
    RETRIEVE_JOURNAL = 2
    RETRIEVE_JOURNALS = 3
    IMPORT_JOURNAL = 4
    MODIFY_JOURNAL = 5
    DELETE_JOURNAL = 6
    DISCONNECT_CLIENT = 7
    INVALID_COMMAND = 8