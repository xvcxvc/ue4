trueSKY UE4 plugin
======

=== Build UE4 TrueSky plugin ===

* Run GenerateProjectFiles_AllPlatforms.bat from UE4. This will generate projects and the solution file (UE4.sln)

* Use UE4.sln (UE4.vcxproj).

* UE4 project contains "TrueSkyPlugin" folder in Engine/UE4/Plugins

* Build UE4 project.

TrueSkyPlugin loads "TrueSkyUI.dll" and "UE4PluginRenderInterface.dll" from %SIMUL%\exe\x64\VC11\Debug\
or %SIMUL%\exe\x64\VC11\Release\

TrueSkyUI.dll further loads Qt libraries. For that, the PATH env-variable is extended by
%QTDIR%\bin
%SIMUL%\exe\x64
%SIMUL%\exe\x64\VC11\Debug

It also loads Qt style-sheet from
%SIMUL%\PlugIns\UE4\TrueSkyUI\qss
(Note that it's all temporary solution!)

Therefore before running UE4 you must properly set QTDIR and SIMUL env-variables

I do it by starting VS using following batch file:

@echo off
set SIMUL=%CD%\Simul
set QTDIR=C:\Qt64\Qt5.2.0\5.2.0\msvc2012_64
cd "C:\Program Files (x86)\Microsoft Visual Studio 11.0\Common7\IDE\"
start devenv.exe

TrueSky plugin requires modified Unreal source files to successfully build. Read "Update 2" for more information.


=== Build TrueSkyUI.dll ===

* Use %SIMUL%/PlugIns/UE4/TrueSkyUI/TrueSkyUI.vcxproj  (use Debug_DLL or Release_DLL configuration)
It builds x64 DLL into %SIMUL%/exe/$(Platform)/VC11/$(Configuration)
* To build DLL you must build "qtwinmigrate" library first (in PlugIns/Qt/QtWinMigrate)


=== Build UE4PluginRenderInterface.dll

* Use %SIMUL%/PlugIns/UE4/PluginRenderInterface/UE4Plugin_x64_VC11.vcxproj
Build 64bit DLL in %SIMUL%/exe/$(Platform)/VC11/$(Configuration).



== How to run it?

* From VS2012 (to debug): Set any game project as a startup project (TrueSky game, for example). Do not run
UE4 directly as it displays just the game selection window, closes the process and runs a new one (so you
need to reattach debugger then).
Open your chosen game project properties and set "Debugging->Command Arguments" to contain the game name ("TrueSky"
for example). If not set it displays the game selection window.

* When UE4 editor is running (for the first time), go to Windows->Plugin and enable TrueSky plugin. Restart the editor!
* Plugin adds "TrueSky" section into "Window" menu.

* Press "Window->Add Sequence to Scene" -- this adds TrueSkySequenceActor to the current level. This actor provides a reference
to a sequence asset which is rendered. Choose the actor (from "Scene Outliner" window). In "Details" window, set reference to a
TrueSky Sequence asset (read below how to create one) in "Active Sequence" property. When correctly set, "Window->Toggle Rendering"
should be enabled. Press it to start the rendering.
Note: If you delete TrueSkySequenceActor from the level, the sky rendering is still active. However, immediately after you select
"Window" menu it will be detected that the actor is missing and the rendering stops. You can't toggle rendering back until you add
a new TrueSkySequenceActor to the level.

* To create a new TrueSkySeqence asset, go to "Content Browser" window. Press "New Asset" button (or do a right mouse click inside
the window) to open an asset selection window. Choose "Miscellaneous / TrueSky Sequence Asset". A new asset will be created.
Now you can rename/save/delete it.

* To edit TrueSkySequence asset just double-click on it. Editor window opens (TrueSkyUI.dll).

* You can see changes to the properties (e.g. "preview") only if edited asset is also assigned to the level's TrueSkySequenceActor!
TrueSky plugin renderer uses only  asset which is referenced from that actor.
If you are editing some other asset (which is not assigned to the TrueSky actor of the current level) then you can't see any visualization of it.

* Your changes in TrueSky properties are reliably(!) saved to the asset when you close the editing window.
If the plugin has been compiled with autosaving on (#define ENABLE_AUTO_SAVING), it saves changes automatically every X seconds (currently X=4).

* If TrueSkySequence asset has been changed, it can be saved to the disc by right-clicking on it and choosing "Save".
You are also prompted about any unsaved assets when closing Unreal Editor.

* You can edit any number of TrueSkySequence assets at once. However, only that which is also set to the current level's TrueSkySequenceActor
is visible in editor's rendering window.

* TrueSkySequenceActor can be added to the level only through "Window->Add Sequence to Scene" menu command.
This prevents more than one such actor to be present on the level.



-----------------------------------------------------------------------------------------------------------

Update 1:

* TrueSkyUI project has configurations to build EXE or DLL. Please setup your solution so it builds either Debug_DLL or
Release_DLL ( after opening you solution go to Build -> Configuration Manager, choose debur/relase solution
configuration and choose Debug_DLL/Release_DLL for TrueSkyUI )
[exe is for testing purposes only]

IMPORTANT!!!!!!

* To build current version of UE4 TrueSkyPlugin, you must manually move following file:

Engine/Source/Runtime/Windows/D3D11RHI/Private/D3D11RHIPrivate.h
into
Engine/Source/Runtime/Windows/D3D11RHI/Public/D3D11RHIPrivate.h

This is of course a temporary solution and hopefully will be fixed by Epic.


-----------------------------------------------------------------------------------------------------------

Update 2:

* To successfully build current version of UE4 plugin, you need to copy "[HowTo]/Engine/Source/Runtime/Renderer/*" files into
apropriate location in UE4. It contains modified Epic's source codes.


