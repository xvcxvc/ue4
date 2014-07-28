trueSKY UE4 plugin
======

How to build it
---
* If you've not done so already, run the trueSKY UE4 plugin installer from [Simul](http://simul.co/truesky/truesky-on-unrealengine-4/). This will install the trueSKY binaries and shaders you will need to run the plugin. The plugin source (only) can be obtained from [GitHub](https://github.com/simul/ue4).

* Run GenerateProjectFiles_AllPlatforms.bat from UE4. This will generate projects and the solution file (UE4.sln)

* Use UE4.sln (UE4.vcxproj).

* UE4 project contains the "TrueSkyPlugin" folder in Engine/UE4/Plugins.


**IMPORTANT**: To successfully build the UE4 plugin, you need to copy "[HowTo]/Engine/Source/Runtime/Renderer/*" files into the
apropriate location in UE4. It contains modified Epic source code to enable custom sky rendering.


* Build the UE4 project.

TrueSkyPlugin loads "TrueSkyUI.dll" and "UE4PluginRenderInterface.dll" from [UE4]\Engine\Plugins\TrueSkyPlugin\Binaries\Win64

TrueSkyUI.dll further loads the Qt libraries. For that, the PATH environment-variable is extended temporarily by the plugin to include "../../Plugins/TrueSkyPlugin/Binaries/Win64"


How to run it
---
* Run UE4, either standalone or with the debugger.
* When the UE4 editor is running, go to Windows->Plugin and enable TrueSky plugin. Restart the editor! The plugin adds the "TrueSky" section into the "Window" menu.
* Press "Window->Add Sequence to Scene" -- this adds a TrueSkySequenceActor to the current level. This actor provides a reference to a sequence asset which is rendered. Choose the actor (from "Scene Outliner" window). In the "Details" window, set the reference to a TrueSky Sequence asset (read below how to create one) in the "Active Sequence" property.
* Select "Window->Toggle Rendering".

Note: If you delete TrueSkySequenceActor from the level, the sky rendering is still active. However, immediately after you select the "Window" menu it will be detected that the actor is missing and the rendering stops. You can't toggle rendering back until you add a new TrueSkySequenceActor to the level.

* To create a new TrueSkySeqence asset, go to the "Content Browser" window. Press "New Asset" button (or do a right mouse click inside the window) to open an asset selection window. Choose "Miscellaneous / TrueSky Sequence Asset". A new asset will be created. Now you can rename/save/delete it.

* To edit the TrueSkySequence asset just double-click on it - the editor window opens (TrueSkyUI_MD.dll).

* You can see changes to the properties (e.g. "preview") only if the edited asset is also assigned to the level's TrueSkySequenceActor! The trueSky plugin renderer uses only the asset which is referenced from that actor. If you are editing some other asset (which is not assigned to the TrueSky actor of the current level) then you won't see any visualization of it.

* Your changes in TrueSky properties are saved to the asset when you close the editing window. If the plugin has been compiled with autosaving on (#define ENABLE_AUTO_SAVING), it saves changes automatically every X seconds (currently X=4).

* If the TrueSkySequence asset has been changed, it can be saved to the disc by right-clicking on it and choosing "Save". You are also prompted about any unsaved assets when closing Unreal Editor.

* You can edit any number of TrueSkySequence assets at once. However, only that which is also set to the current level's TrueSkySequenceActor is visible in editor's rendering window.

* A TrueSkySequenceActor can be added to the level only through "Window->Add Sequence to Scene" menu command. This prevents more than one such actor to be present on the level.

--------------------------------------------------------------------------------------------------



