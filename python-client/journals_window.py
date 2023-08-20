from ui_generated.ui_journals_window import Ui_journalsWindow
from journal_window import JournalWindow
from journal import Journal
from import_dialog import ImportDialog

from PyQt5.QtWidgets import QWidget, QInputDialog, QMessageBox, QApplication
from enums import CommandTypes, OperationStatus

class JournalsWindow(QWidget):
    def __init__(self, requester, parent=None):
        super().__init__(parent=parent)
        self.requester = requester
        self.requester.register_response_callback(self.__message_received, self)

        self.journals = []
        self.selected_journal = None
        self.processed_journal = None

        self.__setup_ui()

    def __setup_ui(self):        
        self.ui = Ui_journalsWindow()
        self.ui.setupUi(self)
        
        self.setWindowTitle("Journals")
        self.resize(1000, 500)
        
        self.ui.statusLabel.setText("Fetching journals...");
    
        self.ui.createJournal.clicked.connect(self.__create_journal_on_click)
        self.ui.deleteJournal.clicked.connect(self.__delete_journal_on_click)
        self.ui.importJournal.clicked.connect(self.__import_journal_on_click)
        self.ui.viewJournal.clicked.connect(self.__view_journal_on_click)
        self.ui.editJournal.clicked.connect(self.__edit_journal_on_click)
        self.ui.exit.clicked.connect(self.__exit_on_click)

    def set_selected_journal(self, journal):
        self.selected_journal = journal

    def __create_journal_on_click(self):
        if self.processed_journal is not None:
            QMessageBox.information(self, "Creation in progress", "A journal is already creating.")
            return

        journal_name = QInputDialog.getText(self, "Input", "Input journal name")
        if journal_name[0] == "":
            QMessageBox.information(self, "Empty input", "Journal name should not be empty.")
            return
        
        self.processed_journal = journal_name
        self.requester.create_journal(journal_name)

    def __show_no_journal_selected_message(self):
        message = "No journal selected. Create or import one."
        if len(self.journals) > 0:
            message = "No journal selected. Select, create or import one"
            
        QMessageBox.information(self, "Select journal", message)

    def __delete_journal_on_click(self):
        if self.selected_journal is None:
            self.__show_no_journal_selected_message()
            return
        
        self.processed_journal = self.selected_journal.get_journal_name()
        self.requester.delete_journal(self.processed_journal)

    def __import_journal_on_click(self):
        def get_import_args():
            journal_name = import_dialog.get_journal_name()
            journal_path = import_dialog.get_journal_path()

            if journal_name == "" or journal_path == "":
                QMessageBox.information(self, "Invalid input", "Input journal name and/or journal path.")
                return
            
            self.processed_journal = journal_name
            self.requester.import_journal(journal_name, journal_path)

        import_dialog = ImportDialog(self)
        import_dialog.closed.connect(get_import_args)
        import_dialog.show()

    def __show_journal(self, read_only=False):
        if self.selected_journal is None:
            self.__show_no_journal_selected_message()
            return
        
        journal_name = self.selected_journal.get_journal_name()
        self.requester.retrieve_journal(journal_name)

        journal_window = JournalWindow(journal_name, read_only, self.requester, self)
        journal_window.show()

        self.hide()

    def __view_journal_on_click(self):
        self.__show_journal()

    def __edit_journal_on_click(self):
        self.__show_journal(True)

    def __exit_on_click(self):
        QApplication.exit(0)

    def __message_received(self, message):
        if message["command_type"] == CommandTypes.CREATE_JOURNAL:
            self.__handle_create_journal(message)
        elif message["command_type"] == CommandTypes.DELETE_JOURNAL:
            self.__handle_delete_journal(message)
        elif message["command_type"] == CommandTypes.IMPORT_JOURNAL:
            self.__handle_import_journal(message)
        elif message["command_type"] == CommandTypes.RETRIEVE_JOURNALS:
            self.__handle_retrieve_journals(message)

    def __add_new_journal(self):
        self.journals.append(self.processed_journal)

        journals_count = len(self.journals)
        if len(self.journals) == 1:
            self.__remove_status_label()

        journal_widget = Journal(self)
        journal_widget.set_journal_name(self.processed_journal)

        i = journals_count / 10
        last_row_count = journals_count % 10
        if last_row_count == 0:
            i += 1

        self.ui.gridLayout.addWidget(journal_widget, i, last_row_count + 1)

    def __handle_create_journal(self, message):
        if message["operation_status"] == OperationStatus.OPERATION_FAIL:
            QMessageBox.information(self, "Creation failed", message["status_message"])
            self.processed_journal = None

        self.__add_new_journal()

    def __delete_journal(self):
        try:
            index = self.journals.index(self)   
        except ValueError:
            print(f"Could not find journal {self.processed_journal} in journals list.")
            self.processed_journal = None
            return
        
        journal_item = self.ui.gridLayout.itemAt(index)
        if journal_item is None:
            print("Journal widget not found in layout.")
            self.processed_journal = None
            return
        
        self.ui.gridLayout.removeWidget(journal_item)
        journal_item.widget.hide()
        self.processed_journal = None

    def __handle_delete_journal(self, message):
        if message["operation_status"] == OperationStatus.OPERATION_FAIL:
            QMessageBox.information(self, "Deletion failed", message["status_message"])
            self.processed_journal = None

        self.__delete_journal()

    def __handle_import_journal(self, message):
        if message["operation_status"] == OperationStatus.OPERATION_FAIL:
            QMessageBox.information(self, "Import failed", message["status_message"])
            self.processed_journal = None
            return
        
        self.__add_new_journal()

    def __remove_status_label(self):
        self.ui.gridLayout.removeWidget(self.ui.statusLabel)
        self.ui.statusLabel.hide()

    def __handle_retrieve_journals(self, message):
        self.journals = message.split(";")
        if len(self.journals) == 0:
            self.ui.statusLabel.setText("There are no journals yet. Create or import one.")
            return 
        
        self.__remove_status_label()

        i = 0
        for j, journal in enumerate(self.journals):
            journalWidget = Journal(self)
            if j % 10 == 0 and i > 0:
                i += 1

            self.ui.gridLayout.addWidget(journalWidget, i, j)
            journalWidget.show()

    def showEvent(self, event):
        return super().showEvent(event)
    