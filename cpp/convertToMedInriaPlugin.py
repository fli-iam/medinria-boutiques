import os
import re
import string

namePrefix = "medBoutiques"
namePrefixCapital = namePrefix.upper()

nameMap = {
	"executionwidget": "Execution",
	"invocationwidget": "Invocation",
	"invocationguiwidget": "InvocationGUI",
	"searchtoolswidget": "SearchTools",
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
								line = linePrefix + namePrefixCapital + baseNameCapital.replace("WIDGET", "") + "_H\n"
								isDefine = True

						isInclude = False

						include_search = re.search('#include "(.*).h"', line)
						if include_search:
							includedFileName = include_search.group(1)
							line = line.replace(includedFileName, namePrefix + nameMap[includedFileName])
							isInclude = True

						if not isDefine and not isInclude:

							for oldName, newName in nameMap.items():
								line = line.replace(newName + "Widget", namePrefix + newName)

						outputFile.write(line)
						# print(line, file=outputFile)