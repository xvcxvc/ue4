import os
import shlex, subprocess
import sys

def execute():
	wd=os.getcwd()
	os.environ['SIMUL']="C:/Simul/master/Simul"
	#os.environ['ue.bEnableFastIteration']='1'
	#os.environ['ue.bUseUnityBuild']='false'

	os.environ['QTDIR']=os.environ['DROPBOX']+'/Qt/qt5_msvc2012_64_opengl'
	os.environ['SIMUL_BUILD']='1'
	os.environ['VSDIR']=os.environ['ProgramFiles(x86)']+'/Microsoft Visual Studio 11.0';
	args=[os.environ['VSDIR']+'/Common7/IDE/devenv.exe','./UE4.sln']

        #Add to the PATH so we can debug without having the DLL's in the same directory as the exe:
	os.environ['PATH']=os.environ['QTDIR']+'/bin/plugins;'+os.environ['PATH']
	os.environ['PATH']=os.environ['QTDIR']+'/bin;'+os.environ['PATH']

	pid=subprocess.Popen(args).pid

if __name__ == "__main__":
	execute()
