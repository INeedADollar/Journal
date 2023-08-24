from journals_window import JournalsWindow
from PyQt5.QtWidgets import QApplication, QMessageBox
from requester import Requester

import sys

client_requester = None

def cleanup():
    client_requester.stop()

def show_requester_critical_error(message):
    QMessageBox.critical(None, "Error", message)
    QApplication.exit(0)

def main():
    app = QApplication(sys.argv)
    app.aboutToQuit.connect(cleanup)

    global client_requester
    client_requester = Requester("127.0.0.2", 5000)  # de completat adresa si portul
    client_requester.critical.connect(show_requester_critical_error)
    client_requester.start()

    journals_window = JournalsWindow(client_requester)
    journals_window.show()

    return app.exec_()

if __name__ == "__main__":
    main()
