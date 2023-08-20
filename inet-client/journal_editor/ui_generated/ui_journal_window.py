# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ui/journal_window.ui'
#
# Created by: PyQt5 UI code generator 5.15.9
#
# WARNING: Any manual changes made to this file will be lost when pyuic5 is
# run again.  Do not edit this file unless you know what you are doing.


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_journalWindow(object):
    def setupUi(self, journalWindow):
        journalWindow.setObjectName("journalWindow")
        journalWindow.resize(467, 300)
        self.verticalLayout_3 = QtWidgets.QVBoxLayout(journalWindow)
        self.verticalLayout_3.setObjectName("verticalLayout_3")
        self.buttonContainer = QtWidgets.QWidget(journalWindow)
        self.buttonContainer.setMinimumSize(QtCore.QSize(0, 50))
        self.buttonContainer.setMaximumSize(QtCore.QSize(16777215, 50))
        self.buttonContainer.setObjectName("buttonContainer")
        self.horizontalLayout_2 = QtWidgets.QHBoxLayout(self.buttonContainer)
        self.horizontalLayout_2.setContentsMargins(0, 0, 0, 0)
        self.horizontalLayout_2.setObjectName("horizontalLayout_2")
        self.goBackward = QtWidgets.QPushButton(self.buttonContainer)
        self.goBackward.setMinimumSize(QtCore.QSize(0, 50))
        font = QtGui.QFont()
        font.setPointSize(12)
        self.goBackward.setFont(font)
        self.goBackward.setStyleSheet("QPushButton {\n"
"    padding: 10px;\n"
"}")
        self.goBackward.setObjectName("goBackward")
        self.horizontalLayout_2.addWidget(self.goBackward)
        spacerItem = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout_2.addItem(spacerItem)
        self.saveChanges = QtWidgets.QPushButton(self.buttonContainer)
        self.saveChanges.setMinimumSize(QtCore.QSize(0, 50))
        font = QtGui.QFont()
        font.setPointSize(12)
        self.saveChanges.setFont(font)
        self.saveChanges.setStyleSheet("QPushButton {\n"
"    padding: 10px;\n"
"}")
        self.saveChanges.setObjectName("saveChanges")
        self.horizontalLayout_2.addWidget(self.saveChanges)
        spacerItem1 = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout_2.addItem(spacerItem1)
        self.goForward = QtWidgets.QPushButton(self.buttonContainer)
        self.goForward.setMinimumSize(QtCore.QSize(0, 50))
        font = QtGui.QFont()
        font.setPointSize(12)
        self.goForward.setFont(font)
        self.goForward.setStyleSheet("QPushButton {\n"
"    padding: 10px;\n"
"}")
        self.goForward.setObjectName("goForward")
        self.horizontalLayout_2.addWidget(self.goForward)
        self.verticalLayout_3.addWidget(self.buttonContainer)
        self.horizontalLayout = QtWidgets.QHBoxLayout()
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.verticalLayout_2 = QtWidgets.QVBoxLayout()
        self.verticalLayout_2.setSpacing(0)
        self.verticalLayout_2.setObjectName("verticalLayout_2")
        self.leftPage = QtWidgets.QTextEdit(journalWindow)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.leftPage.setFont(font)
        self.leftPage.setStyleSheet("QTextEdit {\n"
"    border: 0.5px solid black;\n"
"    border-bottom: none;\n"
"}")
        self.leftPage.setObjectName("leftPage")
        self.verticalLayout_2.addWidget(self.leftPage)
        self.leftPageNumber = QtWidgets.QLineEdit(journalWindow)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.leftPageNumber.setFont(font)
        self.leftPageNumber.setStyleSheet("QLineEdit {\n"
"    border: 0.5px solid black;\n"
"    border-top: none;\n"
"}")
        self.leftPageNumber.setText("")
        self.leftPageNumber.setAlignment(QtCore.Qt.AlignRight|QtCore.Qt.AlignTrailing|QtCore.Qt.AlignVCenter)
        self.leftPageNumber.setReadOnly(True)
        self.leftPageNumber.setObjectName("leftPageNumber")
        self.verticalLayout_2.addWidget(self.leftPageNumber)
        self.horizontalLayout.addLayout(self.verticalLayout_2)
        self.verticalLayout = QtWidgets.QVBoxLayout()
        self.verticalLayout.setSpacing(0)
        self.verticalLayout.setObjectName("verticalLayout")
        self.rightPage = QtWidgets.QTextEdit(journalWindow)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.rightPage.setFont(font)
        self.rightPage.setStyleSheet("QTextEdit {\n"
"    border: 0.5px solid black;\n"
"    border-bottom: none;\n"
"}")
        self.rightPage.setObjectName("rightPage")
        self.verticalLayout.addWidget(self.rightPage)
        self.rightPageNumber = QtWidgets.QLineEdit(journalWindow)
        font = QtGui.QFont()
        font.setPointSize(12)
        self.rightPageNumber.setFont(font)
        self.rightPageNumber.setStyleSheet("QLineEdit {\n"
"    border: 0.5px solid black;\n"
"    border-top: none;\n"
"}")
        self.rightPageNumber.setReadOnly(True)
        self.rightPageNumber.setObjectName("rightPageNumber")
        self.verticalLayout.addWidget(self.rightPageNumber)
        self.horizontalLayout.addLayout(self.verticalLayout)
        self.verticalLayout_3.addLayout(self.horizontalLayout)

        self.retranslateUi(journalWindow)
        QtCore.QMetaObject.connectSlotsByName(journalWindow)

    def retranslateUi(self, journalWindow):
        _translate = QtCore.QCoreApplication.translate
        journalWindow.setWindowTitle(_translate("journalWindow", "Form"))
        self.goBackward.setText(_translate("journalWindow", "<--"))
        self.saveChanges.setText(_translate("journalWindow", "Save changes"))
        self.goForward.setText(_translate("journalWindow", "-->"))


if __name__ == "__main__":
    import sys
    app = QtWidgets.QApplication(sys.argv)
    journalWindow = QtWidgets.QWidget()
    ui = Ui_journalWindow()
    ui.setupUi(journalWindow)
    journalWindow.show()
    sys.exit(app.exec_())
