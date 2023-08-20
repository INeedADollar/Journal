from ui_generated.ui_journal_window import Ui_journalWindow
from PyQt5.QtWidgets import QWidget, QStatusBar, QMessageBox, QApplication

import zipfile
import io

class JournalWindow(QWidget):
    def __init__(self, journal_path, parent=None):
        super().__init__(parent=parent)
        self.journal_path = journal_path
        self.journal_pages = []
        self.left_page_number = 0

        self.__setup_ui()

    def __setup_ui(self):        
        self.ui = Ui_journalWindow()
        self.ui.setupUi(self)

        self.setWindowTitle(f"Editing {self.journal_path}")

        self.status_bar = QStatusBar(self)
        self.ui.verticalLayout_3.addWidget(self.status_bar)

        self.status_bar.showMessage("Loading journal...")
        self.load_journal()
        
        self.ui.goBackward.clicked.connect(self.__go_backward_on_click)
        self.ui.saveChanges.clicked.connect(self.__save_changes_on_click)
        self.ui.goForward.clicked.connect(self.__go_forward_on_click)

    def __set_pages(self):
        self.ui.leftPage(self.journal_pages[self.left_page_number])
        self.ui.leftPageNumber.setText(f"{self.left_page_number + 1}")
        
        self.ui.rightPage(self.journal_pages[self.left_page_number + 1])
        self.ui.rightPageNumber.setText(f"{self.left_page_number + 2}")

    def __go_backward_on_click(self):
        if self.left_page_number == 1:
            return
        
        self.left_page_number -= 2
        if self.left_page_number == 1:
            self.ui.goBackward.setDisabled(True)

        self.__set_pages()

    def __save_changes_on_click(self):
        self.status_bar.showMessage("Saving...")
        try:
            with zipfile.ZipFile(self.journal_path, "w", zipfile.ZIP_DEFLATED) as journal_zip:
                for i, journal_page in enumerate(self.journal_pages):
                    if journal_page:
                        journal_zip.writestr(f"{i}.txt", journal_page)
        except:
            print("Could not save changes.")
            self.status_bar.showMessage("Could not save changes.")
            return 

    def __add_new_pages(self):
        for i in range(2):
            self.journal_pages.append("")

    def __go_forward_on_click(self):
        journal_pages_count = len(self.journal_pages)
        if journal_pages_count == 100:
            return
        
        self.left_page_number += 2
        if journal_pages_count + 2 <= 100:
            self.__add_new_pages()

        if len(self.journal_pages) == 100:
            self.ui.goForward.setDisabled(True)

        self.__set_pages()

    def load_journal(self):
        try:
            with zipfile.ZipFile(self.journal_path) as journal_zip:
                for entry in journal_zip.infolist():
                    with journal_zip.open(entry) as open_entry:
                        content = open_entry.read()
                        self.journal_pages.append(content)
        except:
            self.status_bar.showMessage("Something bad happened while loading journal file.")
            QApplication.exit(0)
            self.__add_new_pages()

        self.__set_pages()
