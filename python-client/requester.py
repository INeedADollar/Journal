from PyQt5.QtWidgets import QMessageBox
import socket

class Requester(object):
    def __init__(self, server_addr, server_port):
        self.server_addr = server_addr
        self.server_port = server_port
        self.is_socket_connected = False

        self.init_connection()

    def init_connection(self):
        self.client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        tries = 0

        while tries < 3:
            tries += 1
            try:
                self.client_socket.connect((self.server_addr, self.server_port))
            except Exception as e:
                print(f"Connection with server could not be established {tries} times. Exception: {e}")

        if tries == 3:
            QMessageBox.critical(self, "Connection error", "Connection with server could not be established. Closing...")
            exit(0)

        self.is_socket_connected = True

    def is_connected(self):
        return self.is_socket_connected
    
    def generate_id(self):
        pass

    def create_journal(self, journal_name):
        pass

    def retrieve_journal(self, journal_name):
        pass

    def retrieve_journals(self, journal_name):
        pass

    def import_journal(self, journal_name, journal_path):
        pass

    def modify_journal(self, journal_name):
        pass

    def delete_journal(self, journal_name):
        pass

    def disconnect(self):
        pass
