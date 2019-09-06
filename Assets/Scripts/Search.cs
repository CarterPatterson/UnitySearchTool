using UnityEngine;
using UnityEditor;
using System;
using System.Text;
using System.Collections.Generic;
using System.Runtime.InteropServices;


public class Search : EditorWindow
{
    [DllImport("HandleGrabber")]
    public static extern IntPtr InitGrabber();
    [DllImport("HandleGrabber")]
    public static extern void DiscoverMenuItems();
    [DllImport("HandleGrabber")]
    public static extern int MenuItemCount();
    [DllImport("HandleGrabber")]
    [return: MarshalAs(UnmanagedType.BStr)]
    public static extern string RetrieveDataName(int index);
    [DllImport("HandleGrabber")]
    public static extern uint RetrieveDataID(int index);
    [DllImport("HandleGrabber")]
    public static extern void FreeMemory();
    [DllImport("HandleGrabber")]
    public static extern void ExecuteMenuItemAt(uint id);

    public struct DisplayInfo
    {
        public string name;
        public uint id;

        public DisplayInfo(string name, uint id)
        {
            this.name = name;
            this.id = id;
        }
    }

    string searchInput;
    int menuItemCount;
    List<string> names = new List<string>();
    List<uint> ids = new List<uint>();
    List<DisplayInfo> displayInfos = new List<DisplayInfo>();
    Vector2 scrollPos;


    [MenuItem("Window/Search")]
    private static void Search_Bar() => GetWindow<Search>("Search");

    public void Awake()
    {
        InitGrabber();
        DiscoverMenuItems();
        menuItemCount = MenuItemCount();
        Debug.Log("Found " + menuItemCount + " menu items");
        for (int i = 0; i < menuItemCount; i++)
        {
            names.Add(RetrieveDataName(i));
            ids.Add(RetrieveDataID(i));
        }
    }

    public void OnGUI()
    {
        scrollPos = EditorGUILayout.BeginScrollView(scrollPos);

        searchInput = EditorGUILayout.TextField(searchInput);

        if (searchInput != null)
        {
            for (int i = 0; i < names.Count; i++)
            {
                if (names[i].Contains(searchInput))
                {
                    displayInfos.Add(new DisplayInfo(names[i], ids[i]));
                }
            }

            for (int i = 0; i < displayInfos.Count; i++)
            {
                if (GUILayout.Button(displayInfos[i].name))
                {
                    ExecuteMenuItemAt(displayInfos[i].id);
                }
            }
        }

        EditorGUILayout.EndScrollView();
        displayInfos.Clear();
        Repaint();
    }

    public void OnDestroy()
    {
        FreeMemory();
    }
}
