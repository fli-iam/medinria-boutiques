import os
import re
import string

# This script converts BoutiqueGUI standalone code to a medInria plugin
# It duplicates and rename the BoutiquesGUI .h and .cpp files, and rename classes to follow medInria naming standards
# The output files go to the medInriaPlugin directory
# Those files can then be copied to medInria-sources/src/plugins/legacy/medBoutiques/boutiquesGUI/ and the medInria plugin should compile


# Prefix of the plugin
namePrefix = "medBoutiques"
namePrefixCapital = namePrefix.upper()

# Standalone file names to medInria plugin file names (the new names will be prefixed with the namePrefix)
nameMap = {
	"executionwidget": "ExecutionWidget",
	"invocationwidget": "InvocationWidget",
	"invocationguiwidget": "InvocationGUIWidget",
	"searchtoolswidget": "SearchToolsWidget",
	"dropwidget": "DropWidget",
	"abstractfilehandler": "AbstractFileHandler",
	"filehandler": "FileHandler",
	"configuration": "Configuration",
	"installer": "Installer"
}

# List the src/ directory and duplicates each file in nameMap, rename it and output it to medInriaPlugin/
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

						# Rename include guards
						isDefine = False
						for linePrefix in ["#ifndef ", "#define ", "#endif // "]:
							if line.startswith(linePrefix + baseNameCapital + "_H"):
								line = linePrefix + namePrefixCapital + baseNameCapital + "_H\n"
								isDefine = True

						isInclude = False

						# Rename includes
						include_search = re.search('#include "(.*).h"', line)
						if include_search:
							includedFileName = include_search.group(1)
							if includedFileName in nameMap:
								line = line.replace(includedFileName, namePrefix + nameMap[includedFileName])
							isInclude = True

						# Comment #define BOUTIQUE_GUI_STANDALONE
						if line.startswith("#define BOUTIQUE_GUI_STANDALONE"):
							line = line.replace("#define BOUTIQUE_GUI_STANDALONE", "// #define BOUTIQUE_GUI_STANDALONE")

						# Rename class names
						if not isDefine and not isInclude:

							for oldName, newName in nameMap.items():
								# line = line.replace(newName, namePrefix + newName)
								line = re.sub(r"\b%s\b" % newName , namePrefix + newName, line)

						# Write file
						outputFile.write(line)
						# print(line, file=outputFile)