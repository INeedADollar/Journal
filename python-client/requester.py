from PyQt5.QtWidgets import QMessageBox, QApplication
from PyQt5.QtCore import QObject, pyqtSignal
from journals_window import JournalsWindow
from journal_window import JournalWindow

from enums import CommandTypes, OperationStatus
from parse import parse

import socket
import threading
import time

journals_window_commands = (CommandTypes.CREATE_JOURNAL, CommandTypes.DELETE_JOURNAL, CommandTypes.IMPORT_JOURNAL, CommandTypes.RETRIEVE_JOURNAL)
journal_window_command = (CommandTypes.RETRIEVE_JOURNAL, CommandTypes.MODIFY_JOURNAL)

class Requester(QObject):
    critical = pyqtSignal(str)

    def __init__(self, server_addr, server_port):
        super().__init__()

        self.server_addr = server_addr
        self.server_port = server_port
        self.is_socket_connected = False

        self.journal_window_callback = None
        self.journals_window_callback = None

        self.id = -1

    # encoding problem when receiving bytes that are not text
    def __read_message(self):
        message_read = self.client_socket.recv(100).decode()
        parsed_header = parse("Header\ncommand-type<::::>{}\ncontent-length<::::>{}\nuser-id<::::>{}\n", message_read)
        
        try:
            command_type = CommandTypes[parsed_header[0]]
        except KeyError:
            command_type = CommandTypes.INVALID_COMMAND

        content_length = parsed_header[1]

        current_length = 0
        while current_length < content_length:
            size_to_read = 1024
            if current_length + size_to_read > content_length:
                size_to_read = content_length - current_length

            bytes_read = self.client_socket.recv(size_to_read)
            current_length += len(bytes_read)
            message_read += bytes_read.decode()

        message_parts = message_read.split("Content\n")
        response = {
            "command_type": command_type
        }

        if len(message_parts) < 2:
            return response
        
        format = "status=<journal_response_value>{}</journal_response_value>\nstatus-message=<journal_response_value>{}</journal_response_value>\n"
        if "additional-data" in message_parts[1]:
            format += "additional-data=<journal_response_value>{}</journal_response_value>\n"

        parsed_content = parse(format, message_parts[1])
        response["status_message"] = parsed_content[1]

        try:
            response["status"] = OperationStatus[parsed_content[0]]
        except KeyError:
            response["status"] = OperationStatus.OPERATION_FAIL if (len(response["status_message"]) == 0 or
                    "could not" in response["status_message"] or
                    "failed" in response["status_message"]) else OperationStatus.OPERATION_SUCCESFUL

        if len(parsed_content) == 3:
            response["additional_data"] = parsed_content[2]

        return response   

    def __start(self):
        self.client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        tries = 0

        while tries < 3:
            tries += 1
            try:
                self.client_socket.connect((self.server_addr, self.server_port))
                time.sleep(1)
            except Exception as e:
                print(f"Connection with server could not be established {tries} times. Exception: {e}")

        if tries == 3:
            self.critical.emit("Connection with server could not be established. Closing...")
            return

        self.is_socket_connected = True
        self.__get_id()

        while True:
            if self.journal_window_callback is None or self.journals_window_callback is None:
                time.sleep(1)
                continue

            message = self.__read_message()
            self.__handle_message(message)       

    def start(self):
        requester_thread = threading.Thread(target=self.__start)
        requester_thread.start()

    def is_connected(self):
        return self.is_socket_connected
    
    def register_response_callback(self, callback, window):
        if isinstance(window, JournalsWindow):
            self.journals_window_callback = callback

        if isinstance(window, JournalWindow):
            self.journal_window_callback = callback

    def __get_id(self):
        try:
            with open("user.id", 'r') as id_file:
                self.id = int(f.read())
        except:
            self.id = self.generate_id()

    def __save_id(self, id):
        self.id = id

        try:
            with open("user.id", "w+") as id_file:
                id_file.write(f"{id}")

            print("Id retrieved succesfully.")
        except:
            print("Failed to write id to id file")

    def __handle_message(self, message):
        if message["command_type"] in journals_window_commands:
            if self.journals_window_callback is not None:
                self.journals_window_callback(message)
        elif message["command_type"] in journal_window_command:
            if self.journal_window_callback is not None:
                self.journal_window_callback(message)
        elif message["command_type"] == CommandTypes.GENERATE_ID:
            try:
                id = int(message["additional_content"])
                self.__save_id(id)
            except:
                print("Failed to get an id.")
                self.critical.emit("Failed to register this app with the server. Exitting...")
        elif message["command_type"] == CommandTypes.INVALID_COMMAND:
            print(f"Invalid message received. Message: {message}")

    def __create_content(self, content):
        if content is None or not isinstance(content, dict):
            print(f"Could not create content due to invalid content object {content}")
            return
        
        content_str = ""
        for key in content.keys():
            content_str += f"{key}=<journal_request_value>{content[key]}</journal_request_value>\n"

        return content_str
    
    def __create_message(self, command_type, content = None):
        content_length = -1
        content_part = "Content\n"
        if content is not None:
            content_length = len(content)
            content_part += content

        return f"Header\ncommand-type<::::>{command_type.name}\ncontent-length<::::>{content_length}\nuser-id<::::>{self.id}\n{content_part}"
        
    def generate_id(self):
        message = self.__create_message(CommandTypes.GENERATE_ID)
        self.__send_message(message)

    def create_journal(self, journal_name):
        content_object = {
            "journal-name": journal_name
        }

        content = self.__create_content(content_object)
        message = self.__create_message(CommandTypes.CREATE_JOURNAL, content=content)
        self.__send_message(message)

    def retrieve_journal(self, journal_name):
        content_object = {
            "journal-name": journal_name
        }

        content = self.__create_content(content_object)
        message = self.__create_message(CommandTypes.RETRIEVE_JOURNAL, content=content)
        self.__send_message(message)

    def retrieve_journals(self):
        message = self.__create_message(CommandTypes.RETRIEVE_JOURNALS)
        self.__send_message(message)

    def import_journal(self, journal_name, journal_path):
        if not journal_path.endswith(".zip"):
            print(f"Given file {journal_path} is not a zip, so import journal could not proceed.")
            return
        
        content_object = {
            "journal-name": journal_name
        }

        try:
            with open(journal_path, 'rb') as journal_file:
                content_object["journal-content"] = journal_file.read()
        except:
            print(f"Journal {journal_path} could not be opened or read.")
            return
        
        content = self.__create_content(content_object)
        message = self.__create_message(CommandTypes.IMPORT_JOURNAL, content=content)
        self.__send_message(message)


    def modify_journal(self, journal_name, new_content):
        content_object = {
            "journal-name": journal_name,
            "new-content": new_content
        }

        content = self.__create_content(content_object)
        message = self.__create_message(CommandTypes.MODIFY_JOURNAL_JOURNAL, content=content)
        self.__send_message(message)

    def delete_journal(self, journal_name):
        content_object = {
            "journal-name": journal_name
        }

        content = self.__create_content(content_object)
        message = self.__create_message(CommandTypes.MODIFY_JOURNAL, content=content)
        self.__send_message(message)

    def disconnect(self):
        message = self.__create_message(CommandTypes.DISCONNECT_CLIENT)
        self.__send_message(message)
        time.sleep(1)
        self.client_socket.close()

    def __send_message(self, message):
        self.client_socket.send(message.encode())
