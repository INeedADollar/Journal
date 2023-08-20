# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'journal.ui'
#
# Created by: PyQt5 UI code generator 5.15.9
#
# WARNING: Any manual changes made to this file will be lost when pyuic5 is
# run again.  Do not edit this file unless you know what you are doing.


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_journal(object):
    def setupUi(self, journal):
        journal.setObjectName("journal")
        journal.resize(150, 176)
        journal.setMinimumSize(QtCore.QSize(150, 150))
        journal.setMaximumSize(QtCore.QSize(150, 176))
        self.verticalLayout = QtWidgets.QVBoxLayout(journal)
        self.verticalLayout.setObjectName("verticalLayout")
        self.horizontalLayout = QtWidgets.QHBoxLayout()
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.journalIcon = QtWidgets.QLabel(journal)
        self.journalIcon.setMinimumSize(QtCore.QSize(100, 100))
        self.journalIcon.setMaximumSize(QtCore.QSize(100, 100))
        self.journalIcon.setText("")
        self.journalIcon.setPixmap(QtGui.QPixmap(":/journal/assets/journal-icon.png"))
        self.journalIcon.setScaledContents(True)
        self.journalIcon.setAlignment(QtCore.Qt.AlignCenter)
        self.journalIcon.setObjectName("journalIcon")
        self.horizontalLayout.addWidget(self.journalIcon)
        self.verticalLayout.addLayout(self.horizontalLayout)
        self.journalName = QtWidgets.QLabel(journal)
        self.journalName.setMinimumSize(QtCore.QSize(0, 50))
        self.journalName.setMaximumSize(QtCore.QSize(16777215, 50))
        self.journalName.setText("")
        self.journalName.setAlignment(QtCore.Qt.AlignCenter)
        self.journalName.setObjectName("journalName")
        self.verticalLayout.addWidget(self.journalName)

        self.retranslateUi(journal)
        QtCore.QMetaObject.connectSlotsByName(journal)

    def retranslateUi(self, journal):
        _translate = QtCore.QCoreApplication.translate
        journal.setWindowTitle(_translate("journal", "Form"))
import resources.resources_rc


if __name__ == "__main__":
    import sys
    app = QtWidgets.QApplication(sys.argv)
    journal = QtWidgets.QWidget()
    ui = Ui_journal()
    ui.setupUi(journal)
    journal.show()
    sys.exit(app.exec_())