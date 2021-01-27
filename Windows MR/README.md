This extension was only developed for Unity, due to the restrictions for how Windows MR Headsets can be used.

To access the MR data in Unity, follow the VRTK instructions found at https://academy.vrtk.io/Documentation/HowToGuides/CameraRigs/ in order to set up the trackedAliases corresponding to the Windows MR controllers. Add the files LSL.cs and liblsl64.dll to the Assets folder. Then, create a new object in unity and attach the script RecordTrackedAlias to it. Add that object to the scene, and the data will be sent through SMOOTH.
