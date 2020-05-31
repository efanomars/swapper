#!/usr/bin/env python3

#  Copyright © 2019  Stefano Marsili, <stemars@gmx.ch>
# 
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public
#  License along with this program; if not, see <http://www.gnu.org/licenses/>

# File:   uninstall_libstmm-swapper-xml.py

# Removes all files installed by the install_libstmm-swapper-xml.py script.

import sys
import os
import subprocess

def main():
	import argparse
	oParser = argparse.ArgumentParser("Removes all files created by install_libstmm-swapper-xml.py")
	oParser.add_argument("--no-clean", help="Don't remove build folder", action="store_true"\
						, default=False, dest="bNoClean")
	oParser.add_argument("--no-uninstall", help="Don't uninstall", action="store_true"\
						, default=False, dest="bNoUninstall")
	oParser.add_argument("-y", "--no-prompt", help="No prompt comfirmation", action="store_true"\
						, default=False, dest="bNoPrompt")
	oParser.add_argument("--installdir", help="install dir (default=/usr/local)", metavar='INSTALLDIR'\
						, default="/usr/local", dest="sInstallDir")
	oParser.add_argument("--no-sudo", help="don't use sudo to uninstall", action="store_true"\
						, default=False, dest="bDontSudo")
	oArgs = oParser.parse_args()

	sInstallDir = os.path.abspath(os.path.expanduser(oArgs.sInstallDir))

	sScriptDir = os.path.dirname(os.path.abspath(__file__))
	os.chdir(sScriptDir)
	os.chdir("..")

	try:
		sEnvDestDir = os.environ["DESTDIR"]
	except KeyError:
		sEnvDestDir = ""

	sInstallDir = sEnvDestDir + sInstallDir

	if oArgs.bNoUninstall:
		sInstallDir = str(False)
	elif (sEnvDestDir != "") and not oArgs.bNoPrompt:
		print("Warning: DESTDIR value is prepended to the installation dir!")

	if not oArgs.bNoPrompt:
		print("Uninstall from dir: " + sInstallDir + "   Remove build folders: " + str(not oArgs.bNoClean))

	while not oArgs.bNoPrompt:
		sAnswer = input("Are you sure? (yes/no) >").strip()
		if sAnswer.casefold() == "yes":
			break
		elif sAnswer.casefold() == "no":
			return #-----------------------------------------------------------

	if oArgs.bDontSudo:
		sSudo = ""
	else:
		sSudo = "sudo"

	if not oArgs.bNoUninstall:
		subprocess.check_call("{} rm -r -f            {}/include/stmm-swapper-xml".format(sSudo, sInstallDir).split())
		subprocess.check_call("{} rm    -f             {}/lib/libstmm-swapper-xml.so*".format(sSudo, sInstallDir), shell=True)
		subprocess.check_call("{} rm    -f             {}/lib/libstmm-swapper-xml.a".format(sSudo, sInstallDir).split())
		subprocess.check_call("{} rm    -f      {}/lib/pkgconfig/stmm-swapper-xml.pc".format(sSudo, sInstallDir).split())
		subprocess.check_call("{} rm -r -f       {}/share/doc/libstmm-swapper-xml".format(sSudo, sInstallDir).split())
		subprocess.check_call("{} rm -r -f {}/share/stmm-games/themes/swapper/simple_s.thm".format(sSudo, sInstallDir).split())

	if not oArgs.bNoClean:
		os.chdir("..")
		subprocess.check_call("{} rm -r -f libstmm-swapper-xml/build".format(sSudo).split())

	if not oArgs.bDontSudo:
		subprocess.check_call("sudo ldconfig".split())


if __name__ == "__main__":
	main()

