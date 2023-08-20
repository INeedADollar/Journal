import typing
from PyQt5 import QtCore, QtGui
from ui_generated.ui_journal import Ui_journal
from PyQt5.QtWidgets import QWidget

class Journal(QWidget):
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
        self.parentWidget.set_selected_journal(self)
        return super().mousePressEvent(event)