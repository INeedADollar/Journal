import typing
from PyQt5.QtCore import pyqtSignal
from ui_generated.ui_journal import Ui_journal
from PyQt5.QtWidgets import QWidget

class Journal(QWidget):
    journal_selected = pyqtSignal(object)

    def __init__(self, parent):
        super().__init__(parent)

        self.setup_ui()

    def setup_ui(self):
        self.ui = Ui_journal()
        self.ui.setupUi(self)

    def set_journal_name(self, journal_name):
        self.ui.journalName.setText(journal_name)

    def get_journal_name(self):
        return self.ui.journalName.text()
    
    def select_journal(self):
        self.setStyleSheet("""
            QWidget {
                background-color: #ffffff80;
            }
        """)

    def unselect_journal(self):
        self.setStyleSheet("""
            QWidget {
                background-color: transparent;
            }
        """)

    def mousePressEvent(self, event):
        self.select_journal()
        self.journal_selected.emit(self)
        return super().mousePressEvent(event)