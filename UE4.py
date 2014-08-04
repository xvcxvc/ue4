import os
import shlex, subprocess
import sys

def execute():
	wd=os.getcwd()
	os.environ['UE4_DIR']=wd
	# Insert below the location of your own Qt5 64-bit install:
	os.environ['QTDIR']=os.environ['DROPBOX']+'/Qt/qt5_msvc2012_64_opengl'
	os.environ['VSDIR']=os.environ['ProgramFiles(x86)']+'/Microsoft Visual Studio 11.0';
	args=[os.environ['VSDIR']+'/Common7/IDE/devenv.exe','./UE4.sln']

	pid=subprocess.Popen(args).pid

if __name__ == "__main__":
	execute()
