from journal_window import JournalWindow
from PyQt5.QtWidgets import QApplication

import sys

def app_exiting():
    print("Exit")

def main():
    app = QApplication(sys.argv)
    app.aboutToQuit.connect(app_exiting)

    try:
        journal_arg_index = sys.argv.index("--journal")
        journal_path = sys.argv[journal_arg_index + 1]
    except ValueError:
        print("Journal path is not specified!")
        return -1
    
    journal_window = JournalWindow(journal_path)
    journal_window.show()

    return app.exec_()

if __name__ == "__main__":
    main()
