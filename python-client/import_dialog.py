from ui_generated.ui_import_dialog import Ui_ImportDialog
from PyQt5.QtWidgets import QDialog, QPushButton, QHBoxLayout, QFileDialog, QMessageBox, QStyle, QAction, QLineEdit
from PyQt5.QtCore import pyqtSignal

class ImportDialog(QDialog):
    closed = pyqtSignal()

    def __init__(self, parent):
        super().__init__(parent)

        self.journal_name = ""
        self.journal_path = ""

        self.__setup_ui()

    def get_journal_name(self):
        return self.journal_name
    
    def get_journal_path(self):
        return self.journal_path
    
    def __setup_ui(self):        
        self.ui = Ui_ImportDialog()
        self.ui.setupUi(self)

        self.setWindowTitle("Import journal")
        self.setModal(True)

        folder_icon = self.style().standardIcon(QStyle.StandardPixmap.SP_DirIcon)
        choose_journal_file = self.ui.zipFileChooser.addAction(folder_icon, QLineEdit.ActionPosition.TrailingPosition);
        choose_journal_file.triggered.connect(self.__handle_file_choose)

        self.ui.buttonBox.clicked.connect(self.__handle_button_click)

        self.resize(100, 1000)

    def __handle_file_choose(self):
        journal_zip_path = QFileDialog.getOpenFileName(self, "Choose journal file", ".", "Zip files (*.zip)")
        if journal_zip_path[0] == "":
            QMessageBox.information(self, "No file selected", "Choose a zip file to import.")
            return
        
        self.ui.zipFileChooser.setText(journal_zip_path[0])
        
    def __handle_button_click(self, button):
        if button.text() == "Ok":
            self.journal_name = self.ui.journalName.text()
            self.journal_path = self.ui.zipFileChooser.text()

        self.closed.emit()

    def closeEvent(self, event):
        self.closed.emit()
        return super().closeEvent(event)
        