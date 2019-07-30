import os
import re
import string

namePrefix = "medBoutiques"
namePrefixCapital = namePrefix.upper()

nameMap = {
	"executionwidget": "ExecutionWidget",
	"invocationwidget": "InvocationWidget",
	"invocationguiwidget": "InvocationGUIWidget",
	"searchtoolswidget": "SearchToolsWidget",
	"dropwidget": "DropWidget",
	"abstractfilehandler": "AbstractFileHandler",
	"filehandler": "FileHandler",
}

for root, dirs, files in os.walk("src/", topdown=False):
	for name in files:
		(basename, extension) = os.path.splitext(name)
		if basename in nameMap:

			baseNameCapital = basename.upper()

			newName = namePrefix + nameMap[basename] + extension

			outputFile = open(os.path.join('medInriaPlugin/', newName),'w')

			with outputFile:

				with open(os.path.join(root, name), "r") as inputFile:
					for line in inputFile:

						isDefine = False
						for linePrefix in ["#ifndef ", "#define ", "#endif // "]:
							if line.startswith(linePrefix + baseNameCapital + "_H"):
								line = linePrefix + namePrefixCapital + baseNameCapital + "_H\n"
								isDefine = True

						isInclude = False

						include_search = re.search('#include "(.*).h"', line)
						if include_search:
							includedFileName = include_search.group(1)
							if includedFileName in nameMap:
								line = line.replace(includedFileName, namePrefix + nameMap[includedFileName])
							isInclude = True

						if not isDefine and not isInclude:

							for oldName, newName in nameMap.items():
								# line = line.replace(newName, namePrefix + newName)
								line = re.sub(r"\b%s\b" % newName , namePrefix + newName, line)

						outputFile.write(line)
						# print(line, file=outputFile)