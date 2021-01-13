using System;
using System.Collections.Generic;
using System.IO;
using UnityEngine;
using System.Text;
using System.Runtime.Serialization.Formatters.Binary;
using Unity.Collections;
using Unity.Jobs;
using LSL;
using UnityEngine.XR;

public class RecordTrackedAlias : MonoBehaviour
{
    public GameObject trackedAlias;

    private GameObject hmd;

    public GameObject controllerR;
    private static List<string> trackedObservations = new List<string>();  
    private GameObject controllerL;
    liblsl.StreamInfo controllersOutletinfo;
    liblsl.StreamOutlet controllersOutlet;
    liblsl.StreamInfo markersOutletInfo;
    liblsl.StreamOutlet markersOutlet;
    float startTime;
    int counter = 0;

    private string trackingHeaderWithTime =
        "time_ms,hmd_x,hmd_y,hmd_z,hmd_rot_x,hmd_rot_y,hmd_rot_z,lc_x,lc_y,lc_z,lc_rot_x,lc_rot_y,lc_rot_z,rc_x,rc_y,rc_z,rc_rot_x,rc_rot_y,rc_rot_z\n";
    // Start is called before the first frame update
    private void Start()
    {
        var aliasObject = trackedAlias.transform.GetChild(0);
        hmd = aliasObject.transform.GetChild(1).gameObject;
        controllerL = aliasObject.transform.GetChild(2).gameObject;
        controllerR = aliasObject.transform.GetChild(3).gameObject;
        controllersOutletinfo = new liblsl.StreamInfo("Controllers", "Controller Coords", 18, 50.0, liblsl.channel_format_t.cf_float32, "PRIDMain0");
        controllersOutlet = new liblsl.StreamOutlet(controllersOutletinfo);
        markersOutletInfo = new liblsl.StreamInfo("Controller Markers", "Markers", 1, 0, liblsl.channel_format_t.cf_string, "PRIDMain1");
        markersOutlet = new liblsl.StreamOutlet(markersOutletInfo);
        startTime = Time.time;
    }

    // Update is called once per frame
    public static string Tracked6DString(Transform trackedTransform)
    {
        var position = trackedTransform.position;
        var rotation = trackedTransform.rotation;
        var outString =
            $"{position.x:F4},{position.y:F4},{position.z:F4},{rotation.eulerAngles.x:F4},{rotation.eulerAngles.y:F4},{rotation.eulerAngles.z:F4}";
        return outString;
    }

    public static float[] lslSample(Transform trackedTransform, float[] sample, int start)
    {
        var position = trackedTransform.position;
        var rotation = trackedTransform.rotation;
        float[] output = { position.x, position.y, position.z, rotation.eulerAngles.x, rotation.eulerAngles.y, rotation.eulerAngles.z };
        for (int i = start; i < start + 6; i++)
        {
            sample[i] = output[i - start];
        }
        return sample;
    }
        
    public static void SavePositionsAndRotationsToDiskAndAnalyze(string headerString, List<string> observationsList, string fileName)
    {

       
        try
        {
            using (var fs = new FileStream(Environment.GetFolderPath(Environment.SpecialFolder.Desktop)+ $"/{fileName}.csv", FileMode.OpenOrCreate, FileAccess.ReadWrite))
            {
                using (var fw = new StreamWriter(fs))
                {
                    //header
                    fw.Write(headerString);

                    foreach (var t in observationsList)
                    {
                        fw.WriteLine(t);
                    }
                    fw.Flush(); // Added
                }
            }
        }
        catch(IOException)
        {
            // ReSharper disable once Unity.PerformanceCriticalCodeInvocation
            Debug.LogError("FileStream not possible");
        }
        
    }

    private int _framesSinceLastSave = 0;
    private void  FixedUpdate()
    {
        float currentTime = Time.time;
        if (currentTime - startTime <= 15)
        {
            counter++;
            float[] sample = new float[18];
            sample = lslSample(hmd.transform, sample, 0);
            sample = lslSample(controllerL.transform, sample, 6);
            sample = lslSample(controllerR.transform, sample, 12);
            controllersOutlet.push_sample(sample);
            var x = GameMillisToString() +
                    "," +
                    Tracked6DString(hmd.transform) +
                    "," +
                    Tracked6DString(controllerL.transform) +
                    "," +
                    Tracked6DString(controllerR.transform);
            trackedObservations.Add(x);

            if (Input.GetKeyDown(KeyCode.Return) && _framesSinceLastSave >= 90)
            {
                var startTime = DateTimeOffset.UtcNow.ToUnixTimeMilliseconds();
                SavePositionsAndRotationsToDiskAndAnalyze(trackingHeaderWithTime, trackedObservations, $"recording_{startTime}_aliasTracking");
                Debug.Log("completed save. elapsed seconds:" + BCTools.DeltaTimeString(startTime) + "and the frame count for save was " + _framesSinceLastSave);
                _framesSinceLastSave = 0;
            }

            _framesSinceLastSave += 1;

        } else
        {
            Debug.Log("MR" +  counter.ToString());
        }
    }

    public static string GameMillisToString()
    {
        return (Time.time*1000f).ToString("F0");
    }



    public void LTrigger() 
    {
        String[] output = { "LTrigger" };
        markersOutlet.push_sample(output); }
    public void RTrigger()
    {
        String[] output = { "RTrigger" };
        markersOutlet.push_sample(output);
    }
    public void LGrip()
    {
        Debug.Log("LGrip");
        String[] output = { "LGrip" };
        markersOutlet.push_sample(output);
    }
    public void RGrip()
    {
        Debug.Log("RGrip");
        String[] output = { "RGrip" };
        markersOutlet.push_sample(output);
    }
    public void LTouchStart()
    {
        String[] output = { "LTouchStart" };
        markersOutlet.push_sample(output);
    }
    public void RTouchStart()
    {
        String[] output = { "RTouchStart" };
        markersOutlet.push_sample(output);
    }
    public void LTouchEnd()
    {
        String[] output = { "LTouchEnd" };
        markersOutlet.push_sample(output);
    }
    public void RTouchEnd()
    {
        String[] output = { "RTouchEnd" };
        markersOutlet.push_sample(output);
    }
    public void LPress()
    {
        String[] output = { "LPress" };
        markersOutlet.push_sample(output);
    }
    public void RPress()
    {
        String[] output = { "RPress" };
        markersOutlet.push_sample(output);
        float[] sample = new float[18];
        sample[3] = 361F;
        controllersOutlet.push_sample(sample);
    }
    public void LMenu()
    {
        Debug.Log("LMenu");
        String[] output = { "LMenu" };
        markersOutlet.push_sample(output);
    }
    public void RMenu()
    {
        Debug.Log("RMenu");
        String[] output = { "RMenu" };
        markersOutlet.push_sample(output);
    }
    public void LJoystick()
    {
        Debug.Log("LJoystick");
        String[] output = { "LJoystick" };
        markersOutlet.push_sample(output);
    }
    public void RJoystick()
    {
        Debug.Log("RJoystick");
        String[] output = { "RJoystick" };
        markersOutlet.push_sample(output);
    }
}
