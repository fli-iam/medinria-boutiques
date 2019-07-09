import sys
import random
import re
from os import path
import subprocess
from subprocess import Popen
from PySide2 import QtGui, QtCore, QtWidgets
from PySide2.QtCore import Qt, QAbstractTableModel, QAbstractItemModel, QModelIndex
from PySide2.QtGui import QColor
from PySide2.QtCore import QObject, Signal, Slot

class SearchToolsWidget(QtWidgets.QWidget):

	toolSelected = Signal()
	toolDeselected = Signal()

	def __init__(self):
		super().__init__()

		self.searchLineEdit = QtWidgets.QLineEdit()
		self.searchLineEdit.setPlaceholderText("Search a tool in Boutiques...")
		self.button = QtWidgets.QPushButton("Search")

		# Search input
		self.searchGroupBox = QtWidgets.QGroupBox()
		self.searchLayout = QtWidgets.QHBoxLayout()
		self.searchLayout.addWidget(self.searchLineEdit)
		self.searchLayout.addWidget(self.button)
		self.searchGroupBox.setLayout(self.searchLayout)
		
		self.loadingLabel = QtWidgets.QLabel("Loading...")
		self.loadingLabel.hide()
		self.createTable()

		self.infoLabel = QtWidgets.QLabel("Tool Info")
		self.infoLabel.hide()
		self.info = QtWidgets.QTextEdit()
		self.info.setReadOnly(True)
		self.info.hide()

		self.layout = QtWidgets.QVBoxLayout()
		self.layout.addWidget(self.searchGroupBox)
		self.layout.addWidget(self.loadingLabel)
		self.layout.addWidget(self.table)
		self.layout.addWidget(self.infoLabel)
		self.layout.addWidget(self.info)
		self.setLayout(self.layout)

		self.button.clicked.connect(self.searchBoutiquesTools)
		self.searchLineEdit.returnPressed.connect(self.searchBoutiquesTools)

		self.createProcess()

	def createTable(self):
		self.table = QtWidgets.QTableWidget()
		self.table.setRowCount(0)
		self.table.setColumnCount(4)
		self.table.move(0,0)
		self.table.setSelectionBehavior(QtWidgets.QAbstractItemView.SelectRows)
		self.table.horizontalHeader().setSectionResizeMode(QtWidgets.QHeaderView.Stretch)
		self.table.setEditTriggers(QtWidgets.QAbstractItemView.NoEditTriggers)
		self.table.setHorizontalHeaderLabels(["ID", "Title", "Description", "Downloads"])
		self.table.itemSelectionChanged.connect(self.selectionChanged)
		self.table.hide()

	def createProcess(self):
		self.process = QtCore.QProcess(self)
		self.process.finished.connect(self.processFinished)

	def selectionChanged(self):
		print("selectionChanged")
		tool = self.getSelectedTool()
		print(tool)
		if tool is None:
			return
		result = subprocess.run(["bosh", "pprint", tool["id"]], capture_output=True)
		self.info.setText(result.stdout.decode("utf-8"))

		self.infoLabel.show()
		self.info.show()
		self.toolSelected.emit()

	def searchBoutiquesTools(self):
		self.loadingLabel.show()
		self.table.hide()
		self.infoLabel.hide()
		self.info.hide()

		searchQuery = self.searchLineEdit.text()
		args = ["search", "-m 50", searchQuery]
		self.process.start('bosh', args)
		self.toolDeselected.emit()
		self.searchResults = []

	def processFinished(self):
		self.loadingLabel.hide()
		self.table.show()

		output = self.process.readAll().data().decode(sys.stdout.encoding)
		output = re.sub(r"\x1b\[[0-9;]*[mGKF]", "", output)
		lines = output.splitlines()

		if len(lines) < 2:
			return

		line = lines[1]
		idIndex = line.find("ID")
		titleIndex = line.find("TITLE")
		descriptionIndex = line.find("DESCRIPTION")
		downloadsIndex = line.find("DOWNLOADS")
		self.table.setRowCount(len(lines)-2)
		self.searchResults = []

		n = 0
		for line in lines[2:]:
			id = line[idIndex:titleIndex].strip()
			title = line[titleIndex:descriptionIndex].strip()
			description = line[descriptionIndex:downloadsIndex].strip()
			downloads = line[downloadsIndex:].strip()

			self.table.setItem(n, 0, QtWidgets.QTableWidgetItem(id))
			self.table.setItem(n, 1, QtWidgets.QTableWidgetItem(title))
			self.table.setItem(n, 2, QtWidgets.QTableWidgetItem(description))
			self.table.setItem(n, 3, QtWidgets.QTableWidgetItem(downloads))

			self.searchResults.append({"id": id, "title": title, "description": description, "downloads": downloads})

			n += 1

	def getSelectedTool(self):
		currentRow = self.table.currentRow()
		return self.searchResults[currentRow] if currentRow >= 0 and currentRow < len(self.searchResults) else None

class InvocationWidget(QtWidgets.QWidget):
	def __init__(self, searchToolsWidget):
		super().__init__()

		self.searchToolsWidget = searchToolsWidget
		self.layout = QtWidgets.QVBoxLayout()

		self.generateInvocationButton = QtWidgets.QPushButton("Generate invocation file")
		self.fullInvocationCheckbox = QtWidgets.QCheckBox("Full invocation")

		self.invocationGroupBox = QtWidgets.QGroupBox()
		self.invocationLayout = QtWidgets.QHBoxLayout()
		self.invocationLayout.addWidget(self.generateInvocationButton)
		self.invocationLayout.addWidget(self.fullInvocationCheckbox)
		self.invocationGroupBox.setLayout(self.invocationLayout)

		self.openInvocationButton = QtWidgets.QPushButton("Open invocation file")

		self.invocationEditor = QtWidgets.QTextEdit()
		self.saveInvocationButton = QtWidgets.QPushButton("Save invocation file")

		self.layout.addWidget(self.invocationGroupBox)
		self.layout.addWidget(self.openInvocationButton)
		self.layout.addWidget(self.invocationEditor)
		self.layout.addWidget(self.saveInvocationButton)

		self.generateInvocationButton.clicked.connect(self.generateInvocationFile)
		self.openInvocationButton.clicked.connect(self.openInvocationFile)
		self.saveInvocationButton.clicked.connect(self.saveInvocationFile)
		
		self.setLayout(self.layout)

	def generateInvocationFile(self):
		tool = self.searchToolsWidget.getSelectedTool()
		if tool is None:
			return
		args = ["bosh", "example"]
		if self.fullInvocationCheckbox.isChecked():
			args.append("--complete")
		args.append(tool["id"])
		result = subprocess.run(args, capture_output=True)
		output = result.stdout.decode("utf-8")
		self.invocationEditor.setText(output)

	def openInvocationFile(self):
		name = QtWidgets.QFileDialog.getOpenFileName(self, 'Open Invocation File')
		
		file = open(name[0],'r')

		with file:
			text = file.read()
			self.invocationEditor.setText(text)

	def saveInvocationFile(self):
		name = QtWidgets.QFileDialog.getSaveFileName(self, 'Save Invocation File')
		
		file = open(name[0],'w')

		with file:
			file.write(self.invocationEditor.toPlainText())

	@Slot()
	def toolSelected(self):
		self.show()

	@Slot()
	def toolDeselected(self):
		self.hide()

class ExecutionWidget(QtWidgets.QWidget):
	def __init__(self, searchToolsWidget, invocationWidget):
		super().__init__()
		self.searchToolsWidget = searchToolsWidget
		self.invocationWidget = invocationWidget
		self.layout = QtWidgets.QVBoxLayout()

		self.executeButton = QtWidgets.QPushButton("Execute tool")
		self.cancelButton = QtWidgets.QPushButton("Cancel execution")
		self.cancelButton.hide()

		self.output = QtWidgets.QTextEdit()
		self.output.setReadOnly(True)

		self.layout.addWidget(self.executeButton)
		self.layout.addWidget(self.cancelButton)
		self.layout.addWidget(self.output)

		self.executeButton.clicked.connect(self.executeTool)
		self.cancelButton.clicked.connect(self.cancelExecution)
		
		self.process = QtCore.QProcess(self)
		self.process.readyRead.connect(self.dataReady)

		self.process.started.connect(self.processStarted)
		self.process.finished.connect(self.processFinished)

		self.setLayout(self.layout)

	def executeTool(self):
		tool = self.searchToolsWidget.getSelectedTool()
		if tool is None:
			return

		temporaryInvocationFilePath = path.join(QtCore.QDir.tempPath(), "invocation.json")

		invocationFile = open(temporaryInvocationFilePath,'w')

		with invocationFile:
			invocationFile.write(self.invocationWidget.invocationEditor.toPlainText())

		args = ["exec", "launch", "-s", tool["id"], temporaryInvocationFilePath]

		self.process.start('bosh', args)

		args.insert(0, 'bosh')
		self.output.clear()
		self.print(" ".join(args) + "\n\n")

	def cancelExecution(self):
		self.process.kill()
		self.executeButton.show()
		self.cancelButton.hide()

	def print(self, text):
		cursor = self.output.textCursor()
		cursor.movePosition(cursor.End)
		cursor.insertText(text)
		self.output.ensureCursorVisible()

	def processStarted(self):
		self.print("Process started...\n\n")
		self.executeButton.hide()
		self.cancelButton.show()

	def processFinished(self):
		self.print("\n\nProcess finished.")
		self.executeButton.show()
		self.cancelButton.hide()

	def dataReady(self):
		output = self.process.readAll().data().decode(sys.stdout.encoding)
		output = re.sub(r"\x1b\[[0-9;]*[mGKF]", "", output)
		self.print(output)

	@Slot()
	def toolSelected(self):
		self.show()

	@Slot()
	def toolDeselected(self):
		self.hide()

class BoutiquesGUI(QtWidgets.QWidget):
	def __init__(self):
		super().__init__()

		self.searchToolsWidget = SearchToolsWidget()
		self.invocationWidget = InvocationWidget(self.searchToolsWidget)
		self.executionWidget = ExecutionWidget(self.searchToolsWidget, self.invocationWidget)
		
		self.invocationWidget.hide()
		self.executionWidget.hide()

		self.searchToolsWidget.toolSelected.connect(self.invocationWidget.toolSelected)
		self.searchToolsWidget.toolSelected.connect(self.executionWidget.toolSelected)
		self.searchToolsWidget.toolDeselected.connect(self.invocationWidget.toolDeselected)
		self.searchToolsWidget.toolDeselected.connect(self.executionWidget.toolDeselected)

		self.layout = QtWidgets.QVBoxLayout()
		self.layout.addWidget(self.searchToolsWidget)
		self.layout.addWidget(self.invocationWidget)
		self.layout.addWidget(self.executionWidget)

		self.setLayout(self.layout)


if __name__ == "__main__":
	app = QtWidgets.QApplication([])

	widget = BoutiquesGUI()
	widget.resize(800, 600)
	widget.show()

	sys.exit(app.exec_())

