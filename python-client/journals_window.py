from PyQt5 import QtGui
from .ui_generated.ui_journals_window import Ui_journalsWindow
from PyQt5.QtWidgets import QWidget

class JournalsWindow(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent=parent)
        self.journals = []
        self.setup_ui()

    def setup_ui(self):
        ui = Ui_journalsWindow()
        ui.setupUi(self)

        ui.statusLabel.setText("Fetching journals...");

    def showEvent(self, event):
        self.fetch_journals()
        return super().showEvent(event)