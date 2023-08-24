from ui_generated.ui_journal_window import Ui_journalWindow
from PyQt5.QtWidgets import QWidget, QStatusBar, QMessageBox
from PyQt5.QtCore import pyqtSignal
from enums import CommandTypes, OperationStatus

import zipfile
import io

class JournalWindow(QWidget):
    journal_received = pyqtSignal(bytes)

    def __init__(self, journal_name, read_only, requester, journals_window=None, parent=None):
        super().__init__(parent=parent)
        self.journal_name = journal_name
        self.journal_pages = []
        self.left_page_number = 0
        self.changes_made = False

        self.read_only = read_only
        self.journals_window = journals_window
        self.requester = requester
        self.requester.register_response_callback(self.__message_received, self)

        self.__setup_ui()

    def __set_disabled(self, enabled):
        self.ui.leftPage.setReadOnly(enabled)
        self.ui.leftPageNumber.setReadOnly(enabled)
        self.ui.rightPage.setReadOnly(enabled)
        self.ui.rightPageNumber.setReadOnly(enabled)

        self.ui.goBackward.setDisabled(enabled)
        self.ui.saveChanges.setDisabled(enabled)
        self.ui.goForward.setDisabled(enabled)

    def __setup_ui(self):        
        self.ui = Ui_journalWindow()
        self.ui.setupUi(self)

        self.setWindowTitle(self.journal_name)

        self.status_bar = QStatusBar(self)
        self.ui.verticalLayout_3.addWidget(self.status_bar)

        self.status_bar.showMessage("Retrieving journal content...")
        self.__set_disabled(True)

        if not self.read_only:
            self.ui.leftPage.textChanged.connect(self.__left_page_content_changed)
            self.ui.rightPage.textChanged.connect(self.__right_page_content_changed)
        
        self.ui.goBackward.clicked.connect(self.__go_backward_on_click)
        self.ui.saveChanges.clicked.connect(self.__save_changes_on_click)
        self.ui.goForward.clicked.connect(self.__go_forward_on_click)

        self.journal_received.connect(self.__handle_journal_retrieved)

    def __set_modified(self, index):
        current_page = self.journal_pages[self.left_page_number]
        if current_page["modified"]:
            current_page["modified"] = True

    def __left_page_content_changed(self):
        self.journal_pages[self.left_page_number]["content"] = self.ui.leftPage.toPlainText()
        self.__set_modified(self.left_page_number)

    def __right_page_content_changed(self):
        self.journal_pages[self.left_page_number + 1]["content"] = self.ui.rightPage.toPlainText()
        self.__set_modified(self.left_page_number + 1)

    def __set_pages(self):
        self.ui.leftPage.setText(self.journal_pages[self.left_page_number]["content"])
        self.ui.leftPageNumber.setText(f"{self.left_page_number + 1}")
        
        self.ui.rightPage.setText(self.journal_pages[self.left_page_number + 1]["content"])
        self.ui.rightPageNumber.setText(f"{self.left_page_number + 2}")

    def __go_backward_on_click(self):
        if self.left_page_number == 1:
            return
        
        self.left_page_number -= 2
        if self.left_page_number == 1:
            self.ui.goBackward.setDisabled(True)

        self.__set_pages()

    def __save_changes_on_click(self):
        if not self.changes_made or self.read_only:
            return
                
        self.status_bar.showMessage("Saving...")
        try:
            journal_zip_buffer = io.BytesIO()
            with zipfile.ZipFile(journal_zip_buffer, "w", zipfile.ZIP_DEFLATED) as journal_zip:
                for i, journal_page in enumerate(self.journal_pages):
                    if journal_page["modified"]:
                        journal_zip.writestr(f"{i}.txt", journal_page["content"])
        except:
            print("Could not save changes.")
            self.status_bar.showMessage("Could not save changes.")
            return 
        
        self.requester.modify_journal(self.journal_name, journal_zip_buffer.getvalue())

    def __add_new_pages(self):
        if len(self.journal_pages) % 2 == 1:
            self.journal_pages.append({
                "content": "",
                "modified": False
            })
            return
        
        for i in range(2):
            self.journal_pages.append({
                "content": "",
                "modified": False
            })

    def __go_forward_on_click(self):
        journal_pages_count = len(self.journal_pages)
        if journal_pages_count == 100:
            return
        
        self.left_page_number += 2
        if journal_pages_count + 2 <= 100 and not self.read_only:
            self.__add_new_pages()

        if len(self.journal_pages) == 100:
            self.ui.goForward.setDisabled(True)

        self.__set_pages()

    def __message_received(self, message):
        if message["command_type"] == CommandTypes.RETRIEVE_JOURNAL:
            self.__handle_retrieve_journal(message)
        elif message["command_type"] == CommandTypes.MODIFY_JOURNAL:
            self.__handle_modify_journal(message)

    def __handle_journal_retrieved(self, journal_content):
        with open("test.zip", "wb+") as f:
            f.write(journal_content)
            
        try:
            with zipfile.ZipFile(io.BytesIO(journal_content)) as journal_zip:
                for entry in journal_zip.infolist():
                    with journal_zip.open(entry) as open_entry:
                        content = open_entry.read()
                        self.journal_pages.append({
                            "content": content,
                            "modified": False
                        })
        except Exception as e:
            print(e)
            print("Received journal zip is not a valid one.")
            self.__add_new_pages()

        if not self.read_only:
            self.__set_disabled(False)
        else:
            self.ui.goForward.setEnabled(True)

        self.ui.goBackward.setDisabled(True)
        self.__set_pages()

    def __handle_retrieve_journal(self, message):
        if message["status"] == OperationStatus.OPERATION_FAIL:
            QMessageBox.critical(self, "Error", "Failed to retrieve journal content.")
            self.close()
            self.journals_window.show()
            return

        self.journal_received.emit(message["additional_data"])

    def __handle_modify_journal(self, message):
        self.status_bar.showMessage(message["status_message"], 5)
