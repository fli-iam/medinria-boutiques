import sys
import random
import os
import re
import json
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

class InvocationGUIWidget(QtWidgets.QWidget):

	invocationChanged = Signal()

	def __init__(self, searchToolsWidget):
		super().__init__()
		self.searchToolsWidget = searchToolsWidget
		self.layout = QtWidgets.QVBoxLayout()

		self.rootWidget = QtWidgets.QWidget()
		self.rootWidget.setSizePolicy(QtWidgets.QSizePolicy.Policy.Maximum, QtWidgets.QSizePolicy.Policy.Preferred)
		self.rootWidget.setMinimumSize(800, 400)

		self.scrollArea = QtWidgets.QScrollArea(self.rootWidget)
		self.scrollArea.setVerticalScrollBarPolicy( Qt.ScrollBarAlwaysOn )
		self.scrollArea.setSizePolicy(QtWidgets.QSizePolicy.Policy.Maximum, QtWidgets.QSizePolicy.Policy.Preferred)
		self.scrollArea.setWidgetResizable( True )
		self.scrollArea.setGeometry(0, 0, 800, 400 )
		# self.scrollArea.setSizeConstraint()

		self.layout.addWidget(self.rootWidget)

		self.group = None
		self.setLayout(self.layout)
	
	def makeLambda(arg1, ):
		return lambda: x

	def buttonClicked(self, id, name):
		self.invocationJSON[id] = QtWidgets.QFileDialog.getOpenFileName(self, 'Select ' + name)[0]
		self.invocationChanged.emit()
	
	def listChanged(self, id, value):
		try:
			self.invocationJSON[id] = json.loads("[" + value + "]")
			self.invocationChanged.emit()
		except ValueError as e:
			return

	def textChanged(self, id, value):
		self.invocationJSON[id] = value
		self.invocationChanged.emit()

	def valueChanged(self, id, value):
		self.invocationJSON[id] = value
		self.invocationChanged.emit()

	def stateChanged(self, id, value):
		print('change: ' + id + ', value: ' + str(value))
		self.invocationJSON[id] = value
		self.invocationChanged.emit()

	def createGroupAndLayout(self, name):
		group = QtWidgets.QGroupBox()
		group.setTitle(name)
		groupLayout = QtWidgets.QVBoxLayout()
		group.setLayout(groupLayout)
		return (group, groupLayout)

	def parseDescriptor(self, invocationJSON):

		self.invocationJSON = invocationJSON

		if self.group is not None:
			# self.layout.removeWidget(self.group)
			self.scrollArea.takeWidget()
			self.group.deleteLater()
			self.group = None

		self.group = QtWidgets.QWidget()
		# self.group.setSizePolicy(QtWidgets.QSizePolicy.Policy.Maximum, QtWidgets.QSizePolicy.Policy.Maximum)
		# self.group.setMinimumSize(800, 400)
		self.scrollArea.setWidget(self.group)

		groupLayout = QtWidgets.QVBoxLayout()
		# groupLayout.setSizeConstraint(QtWidgets.QLayout.SetMaximumSize)
		self.group.setLayout(groupLayout)
		# self.layout.addWidget(self.group)

		tool = self.searchToolsWidget.getSelectedTool()
		
		cacheDirectory = os.path.expanduser('~/.cache/boutiques/')
		descriptorFileName = tool["id"].replace(".", "-") + ".json"
		descriptorFile = open(path.join(cacheDirectory, descriptorFileName), 'r')

		with descriptorFile:
			try:
				description = json.loads(descriptorFile.read())
			except ValueError as e:
				return

			(mainInputsGroup, mainInputsLayout) = self.createGroupAndLayout("Main parameters")
			(optionalInputsGroup, optionalInputsLayout) = self.createGroupAndLayout("Optional parameters")

			idToGroupAndLayout = {}

			idToOptional = {}

			for inputObject in description["inputs"]:
				if "id" in inputObject and "optional" in inputObject:
					idToOptional[inputObject["id"]] = inputObject["optional"]

			destinationLayouts = []

			if "groups" in description:
				for groupObject in description["groups"]:
					if "members" in groupObject:
						groupName = groupObject["name"]
						groupAndLayout = self.createGroupAndLayout(groupName)
						groupIsOptional = True
						for member in groupObject["members"]:
							idToGroupAndLayout[member] = groupAndLayout
							if member in idToOptional and not idToOptional[member]:
								groupIsOptional = False
								break

						destinationLayouts.append({"groupAndLayout": groupAndLayout, "layout": optionalInputsLayout if groupIsOptional else mainInputsLayout})

			for inputObject in description["inputs"]:

				widget = None
				
				if inputObject["type"] == "File":
					widget = QtWidgets.QPushButton("Select " + inputObject["name"])
					widget.clicked.connect( ( lambda inputObject: lambda: self.buttonClicked(inputObject["id"], inputObject["name"]) )(inputObject) )

				if inputObject["type"] == "String" or inputObject["type"] == "Number":

					widget = QtWidgets.QGroupBox()
					layout = QtWidgets.QHBoxLayout()
					label = QtWidgets.QLabel(inputObject["name"] + ":")
					layout.addWidget(label)

					subWidget = None
					if "list" in inputObject and inputObject["list"]:
						subWidget = QtWidgets.QLineEdit()
						subWidget.setPlaceholderText("Comma seperated " + ("strings" if inputObject["type"] == "String" else "numbers") + ".")
						listString = json.dumps(self.invocationJSON[inputObject["id"]])
						subWidget.setText(listString)
						subWidget.textChanged.connect( (lambda inputObject, subWidget: lambda: self.listChanged(inputObject["id"], subWidget.text()) )(inputObject, subWidget) )
					else:
						if inputObject["type"] == "String":
							subWidget = QtWidgets.QLineEdit()
							subWidget.setPlaceholderText(inputObject["description"])
							subWidget.setText(self.invocationJSON[inputObject["id"]])
							subWidget.textChanged.connect( (lambda inputObject, subWidget: lambda: self.textChanged(inputObject["id"], subWidget.text()) )(inputObject, subWidget) )
						else:
							if "integer" in inputObject and inputObject["integer"]:
								subWidget = QtWidgets.QSpinBox()
							else:
								subWidget = QtWidgets.QDoubleSpinBox()
							if "minimum" in inputObject:
								subWidget.setMinimum(inputObject["minimum"])
							if "maximum" in inputObject:
								subWidget.setMaximum(inputObject["maximum"])
							subWidget.setValue(self.invocationJSON[inputObject["id"]])
							subWidget.valueChanged.connect( (lambda inputObject, subWidget: lambda: self.valueChanged(inputObject["id"], subWidget.value()))(inputObject, subWidget) )

					widget.setToolTip(inputObject["description"])

					layout.addWidget(subWidget)

					widget.setLayout(layout)
					
				if inputObject["type"] == "Flag":
					widget = QtWidgets.QCheckBox(inputObject["name"])
					widget.setToolTip(inputObject["description"])
					widget.setCheckState(Qt.Checked if self.invocationJSON[inputObject["id"]] else Qt.Unchecked)
					widget.stateChanged.connect( (lambda inputObject, widget: lambda: self.stateChanged(inputObject["id"], widget.isChecked()) )(inputObject, widget) )

				if inputObject["id"] in idToGroupAndLayout:
					idToGroupAndLayout[inputObject["id"]][1].addWidget(widget)
				elif "optional" in inputObject and inputObject["optional"]:
					optionalInputsLayout.addWidget(widget)
				else:
					mainInputsLayout.addWidget(widget)

			for destinationLayout in destinationLayouts:
				destinationLayout['layout'].addWidget(destinationLayout['groupAndLayout'][0])

			groupLayout.addWidget(mainInputsGroup)
			groupLayout.addWidget(optionalInputsGroup)
		

class InvocationWidget(QtWidgets.QWidget):
	def __init__(self, searchToolsWidget):
		super().__init__()

		self.searchToolsWidget = searchToolsWidget
		self.layout = QtWidgets.QVBoxLayout()

		# self.generateInvocationButton = QtWidgets.QPushButton("Generate invocation file")
		# self.fullInvocationCheckbox = QtWidgets.QCheckBox("Full invocation")

		# self.invocationGroupBox = QtWidgets.QGroupBox()
		# self.invocationLayout = QtWidgets.QHBoxLayout()
		# self.invocationLayout.addWidget(self.generateInvocationButton)
		# self.invocationLayout.addWidget(self.fullInvocationCheckbox)
		# self.invocationGroupBox.setLayout(self.invocationLayout)

		self.invocationGUIWidget = InvocationGUIWidget(searchToolsWidget)

		self.openInvocationButton = QtWidgets.QPushButton("Open invocation file")

		self.invocationEditor = QtWidgets.QTextEdit()
		self.saveInvocationButton = QtWidgets.QPushButton("Save invocation file")

		# self.layout.addWidget(self.invocationGroupBox)
		self.layout.addWidget(self.openInvocationButton)
		self.layout.addWidget(self.invocationGUIWidget)
		self.layout.addWidget(self.invocationEditor)
		self.layout.addWidget(self.saveInvocationButton)

		# self.generateInvocationButton.clicked.connect(self.generateInvocationFile)
		self.openInvocationButton.clicked.connect(self.openInvocationFile)
		self.saveInvocationButton.clicked.connect(self.saveInvocationFile)
		self.invocationGUIWidget.invocationChanged.connect(self.invocationChanged)
		
		self.setLayout(self.layout)

	def generateInvocationFile(self):
		tool = self.searchToolsWidget.getSelectedTool()
		if tool is None:
			return
		args = ["bosh", "example"]
		# if self.fullInvocationCheckbox.isChecked():
		args.append("--complete")
		args.append(tool["id"])
		result = subprocess.run(args, capture_output=True)
		output = result.stdout.decode("utf-8")
		try:
			self.invocationJSON = json.loads(output)
		except ValueError as e:
			self.invocationJSON = None
			pass
		self.invocationEditor.setText(output)

	def openInvocationFile(self):
		name = QtWidgets.QFileDialog.getOpenFileName(self, 'Open Invocation File')
		
		file = open(name[0],'r')

		with file:
			text = file.read()
			self.invocationEditor.setText(text)

	def invocationChanged(self):
		output = json.dumps(self.invocationJSON, indent=4)
		self.invocationEditor.setText(output)

	def saveInvocationFile(self):
		name = QtWidgets.QFileDialog.getSaveFileName(self, 'Save Invocation File')
		
		file = open(name[0],'w')

		with file:
			file.write(self.invocationEditor.toPlainText())

	@Slot()
	def toolSelected(self):
		self.show()
		self.generateInvocationFile()
		self.invocationGUIWidget.parseDescriptor(self.invocationJSON)

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

