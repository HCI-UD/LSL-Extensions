This extension has been implemented both in Unity and outside of Unity in Visual Studio. In both cases, due to the constraints of the Azure Kinect SDK, the extension has only been designed for a machine running Windows software. In both cases the extension was developed on a 64-bit machine, although the basic core of the code should still work, with possibly some altered requirements for the background setup described below. Note: For our purposes, in both designs the code is set up to send an extra body tracking sample and an extra skeleton sample, each of all 0's, on the 20th iteration, for synchronization purposes for the receiver programs. These extra samples are not integral to the structure of the code and can be deleted if desired. They are only necessary for the ColorTracking program.

For Unity:

First, download the unity package file located in Google Drive at https://drive.google.com/file/d/1SfzSfJEEZqYwNulKZFpTZGXcU4G46iVA/view?usp=sharing In your target project, in the toolbar at the top, choose Assets > Import Package > Custom Package, and select LSL.Azure.Kinect.unitypackage. When the dialogue pops up, select All, then Import. This will load all of the required files, all of which will be stored in the KinectLSL folder, with required resources being stored in the Plugins subfolder, and the actual implementation code stored in the main folder. To use the implementation, you will need to add one of two objects to the scene you wish to capture data from. If you wish to be able to see the body tracking in your scene, add the Cubes object, which has a physical cube object for each joint tracked by the Kinect. If you don't wish to have any visible objects in the scene, add the Kinect Recorder object. Notes regarding data captured: Both the Cubes and the Kinect Recorder objects have the same default configuration as to what data they capture. This configuration is: Color images in BGRA format, Depth Images, Infrared Images, and 3D Body Tracking data at 5 frames per second. To modify the data collection, go to Lines 179-207 in CubesScript.cs for the Cubes object. Lines 97-125 in KinectRecorder.cs for the Kinect Recorder object. Note: If the trackerInTwoD variable is set to true, the tracker data will be transformed into 2d locations in the space of the color image prior to being streamed through LSL, for the purpose of allowing visual representation of the body tracking in the image.

Outside of Unity:

While this extension was implemented in Visual Studio, the code should be able to work in another context if properly implemented. Open Visual Studio 2019 and select "Create a new project." Choose language C++, platform Windows, and project type Console, and select Empty Project, then choose Next. Choose the project name and location, along with the solution name, and leave the "Place solution and project in the same directory box" unchecked.

Next, into the created folder [project name], add the folders Azure Kinect Body Tracking SDK, Azure Kinect SDK v1.4.0, and liblsl-CPP. Additionally, there are two files used by the body tracking SDK, which, due to size, could not be added to github, but should be able to be found in the freely available SDK downloadable from Microsoft. These two are:

dnn_model_2_0.onnx, which needs to be added into both the Azure Kinect Body Tracking SDK > sdk > windows-desktop > amd64 > release > bin folder, and the Azure Kinect Body Tracking SDK > tools folder.

cudnn64_y.dll, which needs to be added into the Azure Kinect Body Tracking SDK > tools folder.

Next, in the Solution Explorer, right-click on the "Source Files" folder and choose Add > New Item. Select the file type of C++ file (.cpp), name it Reader.cpp, and select Add. Now, copy and paste the code from the file Reader.cpp, located in the Visual Studio 2019 folder, into that new file. Next, at the top, below the uppermost toolbar, change the machine type to x64. Next, in the toolbar at the top, select Project > [your Project Name] properties. 

You will need to make the following changes. 

In Configuration > Debugging: paste the following line into the Environment space: PATH=%PATH%;$(ProjectDir)..\Azure Kinect Body Tracking SDK\sdk\windows-desktop\amd64\release\bin;$(ProjectDir)..\Azure Kinect SDK v1.4.0\sdk\windows-desktop\amd64\release\bin;$(ProjectDir)..\liblsl-CPP\bin;$(ProjectDir)..\Azure Kinect Body Tracking SDK\tools 

In Configuration > C/C++ > General: paste the following line into the Additional Include Directories space: $(ProjectDir)..\liblsl-CPP\include;$(ProjectDir)..\Azure Kinect SDK v1.4.0\sdk\include;$(ProjectDir)..\Azure Kinect Body tracking SDK\sdk\include 

In Configuration > Linker > General: paste the following line into the Additional Library Directories space: $(ProjectDir)..\liblsl-CPP\lib;$(ProjectDir)..\Azure Kinect SDK v1.4.0\sdk\windows-desktop\amd64\release\lib;$(ProjectDir)..\Azure Kinect Body Tracking SDK\sdk\windows-desktop\amd64\release\lib 

In Configuration > Linker > Input: paste the following text at the beginning of the Additional Dependencies space (not deleting text, merely adding this text before it): k4abt.lib;k4a.lib;lsl.lib; 


The extension is automatically set up to record color images, depth images, infrared images, body tracking data, and audio data, with the audio data being sampled at roughly N Hz, and the rest of the data being sampled at 5 frames per second. To alter this setup, go to lines N1-N2.

Notes on Receiving Data:

The Lab Streaming Layer has various systems already set up to record multiple streams of data, which you can use if desired. Otherwise, sample python programs are provided in the Python Receivers folder, each of which focuses on one type of input, with the exception of the ColorTracking program, which combines body tracking data and color images to create overlays of the body tracking data on top of the color images. Also, we were unable to discover what format the audio data was streamed in, although we can confirm that each audio sample point is 28 bytes large, including data from all 7 microphones in the Kinect array. Now, in the decoding of color data, due to constraints of type size, in Unity, in all cases it was necessary to send data in a format that requires spiltting of short type variables into two char type variables, and in the case of NV12 and YUY2 formatted images, to then be further split into 4-bit samples. Depending on the endianness of your local machine, this may require some experimentation to discover the exact way to do this. On our local machine, the file UnityColor.py was able to translate each pair of shorts into 4 int8 variables and then use them to create the final image. In all cases, the memory was cast directly into an array and pushed through LSL, so consult the Microsoft documentation on the Kinect to study the exact data format.
