from PyQt5.QtWidgets import QMessageBox, QApplication
from PyQt5.QtCore import QObject, pyqtSignal
from journals_window import JournalsWindow
from journal_window import JournalWindow

from enums import CommandTypes, OperationStatus
from parse import search

import socket
import threading
import time

journals_window_commands = (CommandTypes.CREATE_JOURNAL, CommandTypes.DELETE_JOURNAL, CommandTypes.IMPORT_JOURNAL, CommandTypes.RETRIEVE_JOURNALS)
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
        self.stop_requester = False

    # encoding problem when receiving bytes that are not text
    def __read_message(self):
        message_read = self.client_socket.recv(110).decode()
        print(message_read)
        parsed_header = search("Header\ncommand-type<::::>{}\ncontent-length<::::>{}\nuser-id<::::>{}\n", message_read)
        print(parsed_header)
        
        try:
            command_type = CommandTypes[parsed_header[0]]
        except KeyError:
            command_type = CommandTypes.INVALID_COMMAND

        content_length = int(parsed_header[1])
        print(content_length)

        message_parts = message_read.split("Content\n");
        current_length = len(message_parts[1])
        content = message_parts[1].encode()

        while current_length < content_length:
            size_to_read = 1024
            if current_length + size_to_read > content_length:
                size_to_read = content_length - current_length

            if command_type == CommandTypes.RETRIEVE_JOURNAL:
                size_to_read = 1024

            bytes_read = self.client_socket.recv(size_to_read)

            current_length += len(bytes_read)
            content += bytes_read

        response = {
            "command_type": command_type
        }

        start_tag = b'<journal_response_value>'
        end_tag = b'</journal_response_value>'

        start_positions = [i for i in range(len(content)) if content[i : i + len(start_tag)] == start_tag]
        end_positions = [i for i in range(len(content)) if content[i :  i + len(end_tag)] == end_tag]

        contents = []
        for start, end in zip(start_positions, end_positions):
            value = content[start + len(start_tag) : end]
            contents.append(value)

        if len(contents) < 2:
            return response

        try:
            response["status"] = OperationStatus[contents[0]]
        except KeyError:
            response["status"] = OperationStatus.OPERATION_FAIL if (len(contents[1]) == 0 or
                    b"could not" in contents[1] or
                    b"failed" in contents[1]) else OperationStatus.OPERATION_SUCCESFUL

        response["status_message"] = contents[1].decode()
        print(contents, content, len(content))
        if len(contents) == 3:
            response["additional_data"] = contents[2]

        return response   

    def __start(self):
        self.client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        tries = 0

        while tries < 3:
            tries += 1
            try:
                self.client_socket.connect((self.server_addr, self.server_port))
                time.sleep(1)
                break;
            except Exception as e:
                print(f"Connection with server could not be established {tries} times. Exception: {e}")

        if tries == 3:
            self.critical.emit("Connection with server could not be established. Closing...")
            return

        self.is_socket_connected = True
        self.__get_id()

        while not self.stop_requester:
            message = self.__read_message()
            print(message)
            self.__handle_message(message)       

    def start(self):
        requester_thread = threading.Thread(target=self.__start)
        requester_thread.start()

    def stop(self):
        self.stop_requester = True
        self.disconnect()

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
                self.id = int(id_file.read())
        except:
            self.id = self.generate_id()

        self.retrieve_journals()

    def __save_id(self, id):
        print("SAVE ID");
        self.id = id

        try:
            with open("user.id", "w+") as id_file:
                id_file.write(f"{id}")

            print("Id retrieved succesfully.")
        except:
            print("Failed to write id to id file")

    def __handle_message(self, message):
        print("handle", message)
        if message["command_type"] in journals_window_commands:
            print("Sending response")
            if self.journals_window_callback is not None:
                print("Sending response")
                self.journals_window_callback(message)
        elif message["command_type"] in journal_window_command:
            if self.journal_window_callback is not None:
                self.journal_window_callback(message)
        elif message["command_type"] == CommandTypes.GENERATE_ID:
            try:
                id = int(message["additional_data"])
                self.__save_id(id)
            except Exception as e:
                print(e);
                print("Failed to get an id.")
                self.critical.emit("Failed to register this app with the server. Exitting...")
        elif message["command_type"] == CommandTypes.INVALID_COMMAND:
            print(f"Invalid message received. Message: {message}")

    def __create_content(self, content):
        if content is None or not isinstance(content, dict):
            print(f"Could not create content due to invalid content object {content}")
            return
        
        content_bytes = b''
        for key in content.keys():
            content_bytes += f"{key}=<journal_request_value>".encode() + content[key] + b'</journal_request_value>\n'

        return content_bytes
    
    def __create_message(self, command_type, content = None):
        content_length = 0
        content_part = b'Content\n'
        if content is not None:
            content_length = len(content)
            content_part += content

        return f"Header\ncommand-type<::::>{command_type.name}\ncontent-length<::::>{content_length}\nuser-id<::::>{self.id}\n".encode() + content_part
        
    def generate_id(self):
        message = self.__create_message(CommandTypes.GENERATE_ID)
        self.__send_message(message)

    def create_journal(self, journal_name):
        content_object = {
            "journal-name": journal_name.encode()
        }

        content = self.__create_content(content_object)
        message = self.__create_message(CommandTypes.CREATE_JOURNAL, content=content)
        self.__send_message(message)

    def retrieve_journal(self, journal_name):
        content_object = {
            "journal-name": journal_name.encode()
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
            "journal-name": journal_name.encode()
        }

        try:
            with open(journal_path, 'rb') as journal_file:
                content_object["journal-data"] = journal_file.read()
        except:
            print(f"Journal {journal_path} could not be opened or read.")
            return
        
        content = self.__create_content(content_object)
        message = self.__create_message(CommandTypes.IMPORT_JOURNAL, content=content)
        self.__send_message(message)


    def modify_journal(self, journal_name, new_content):
        content_object = {
            "journal-name": journal_name.encode(),
            "new-content": new_content
        }

        content = self.__create_content(content_object)
        message = self.__create_message(CommandTypes.MODIFY_JOURNAL_JOURNAL, content=content)
        self.__send_message(message)

    def delete_journal(self, journal_name):
        content_object = {
            "journal-name": journal_name.encode()
        }

        content = self.__create_content(content_object)
        message = self.__create_message(CommandTypes.DELETE_JOURNAL, content=content)
        self.__send_message(message)

    def disconnect(self):
        message = self.__create_message(CommandTypes.DISCONNECT_CLIENT)
        self.__send_message(message)
        time.sleep(1)
        self.client_socket.close()

    def __send_message(self, message):
        print(message)
        self.client_socket.send(message)
