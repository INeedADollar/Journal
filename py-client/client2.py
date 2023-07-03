import socket 
import sys
 
def init_client():
    port = 80 

    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        print ("Socket successfully created")
    except socket.error as err:
        print ("The server socket cannot be open!")
    
    try:
        host_ip = socket.gethostbyname('www.google.com')
    except socket.gaierror:
        print ("there was an error resolving the host")
        sys.exit()
    
    s.connect((host_ip, port))
    print (s.recv(1024).decode())

    s.close() 


def send_command_and_print_response(socket_fd):
    command = input(">>> ")

    //nush cum se face sendu
    socket_fd = socket.socket()
    c = socket_fd.accept() 
    c.send(command.encode())

    if command == "generate-id":
        generate_id(header, socket_fd)
    elif command == "create-journal":
        create_journal(header, socket_fd)
    elif command == "retrieve-journal":
        retrieve_journal()
    elif command == "import-journal":
        import_journal()
    elif command == "modify-journal":
        modify_journal()
    elif command == "delete-journal":
        delete_journal(header, socket_fd)
    elif command == "disconnect":
        disconnect_client(header, socket_fd)
    else
        print("Invalid command!\n")


def generate_id(header, socket_fd):
    


def create_journal(header, socket_fd):


def retrieve_journal():


def import_journal():


def modify_journal():


def delete_journal(header, socket_fd):


def disconnect_client(header, socket_fd):



if __name__ == "__main__":
    