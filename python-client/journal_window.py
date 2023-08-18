from .ui_generated.ui_journal_window import Ui_journalWindow
from PyQt5.QtWidgets import QWidget

class JournalWindow(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent=parent)
        self.setup_ui()

    def setup_ui(self):
        ui = Ui_journalWindow()
        ui.setupUi(self)
