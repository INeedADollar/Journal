# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ui/import_dialog.ui'
#
# Created by: PyQt5 UI code generator 5.15.9
#
# WARNING: Any manual changes made to this file will be lost when pyuic5 is
# run again.  Do not edit this file unless you know what you are doing.


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_ImportDialog(object):
    def setupUi(self, ImportDialog):
        ImportDialog.setObjectName("ImportDialog")
        ImportDialog.resize(356, 300)
        ImportDialog.setMinimumSize(QtCore.QSize(0, 0))
        ImportDialog.setMaximumSize(QtCore.QSize(1677345, 300))
        self.verticalLayout = QtWidgets.QVBoxLayout(ImportDialog)
        self.verticalLayout.setObjectName("verticalLayout")
        self.verticalLayout_2 = QtWidgets.QVBoxLayout()
        self.verticalLayout_2.setContentsMargins(6, 6, 6, 6)
        self.verticalLayout_2.setObjectName("verticalLayout_2")
        self.journalLabel = QtWidgets.QLabel(ImportDialog)
        self.journalLabel.setMinimumSize(QtCore.QSize(0, 50))
        self.journalLabel.setMaximumSize(QtCore.QSize(16777215, 50))
        font = QtGui.QFont()
        font.setPointSize(12)
        self.journalLabel.setFont(font)
        self.journalLabel.setAlignment(QtCore.Qt.AlignCenter)
        self.journalLabel.setObjectName("journalLabel")
        self.verticalLayout_2.addWidget(self.journalLabel)
        self.journalName = QtWidgets.QLineEdit(ImportDialog)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.journalName.setFont(font)
        self.journalName.setClearButtonEnabled(True)
        self.journalName.setObjectName("journalName")
        self.verticalLayout_2.addWidget(self.journalName)
        self.journalZipLabel = QtWidgets.QLabel(ImportDialog)
        self.journalZipLabel.setMinimumSize(QtCore.QSize(0, 50))
        self.journalZipLabel.setMaximumSize(QtCore.QSize(16777215, 50))
        font = QtGui.QFont()
        font.setPointSize(12)
        self.journalZipLabel.setFont(font)
        self.journalZipLabel.setAlignment(QtCore.Qt.AlignCenter)
        self.journalZipLabel.setObjectName("journalZipLabel")
        self.verticalLayout_2.addWidget(self.journalZipLabel)
        self.zipFileChooser = QtWidgets.QLineEdit(ImportDialog)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.zipFileChooser.setFont(font)
        self.zipFileChooser.setReadOnly(True)
        self.zipFileChooser.setObjectName("zipFileChooser")
        self.verticalLayout_2.addWidget(self.zipFileChooser)
        self.verticalLayout.addLayout(self.verticalLayout_2)
        self.buttonBox = QtWidgets.QDialogButtonBox(ImportDialog)
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtWidgets.QDialogButtonBox.Cancel|QtWidgets.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName("buttonBox")
        self.verticalLayout.addWidget(self.buttonBox)

        self.retranslateUi(ImportDialog)
        self.buttonBox.accepted.connect(ImportDialog.accept) # type: ignore
        self.buttonBox.rejected.connect(ImportDialog.reject) # type: ignore
        QtCore.QMetaObject.connectSlotsByName(ImportDialog)

    def retranslateUi(self, ImportDialog):
        _translate = QtCore.QCoreApplication.translate
        ImportDialog.setWindowTitle(_translate("ImportDialog", "Dialog"))
        self.journalLabel.setText(_translate("ImportDialog", "Input how the journal will be called:"))
        self.journalZipLabel.setText(_translate("ImportDialog", "Choose journal zip file:"))


if __name__ == "__main__":
    import sys
    app = QtWidgets.QApplication(sys.argv)
    ImportDialog = QtWidgets.QDialog()
    ui = Ui_ImportDialog()
    ui.setupUi(ImportDialog)
    ImportDialog.show()
    sys.exit(app.exec_())
