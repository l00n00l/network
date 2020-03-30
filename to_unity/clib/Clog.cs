using UnityEngine;
using System.Runtime.InteropServices;


public class Clog
{
    delegate void log_handler(string data, ulong data_size);
    [DllImport("network_shared")]
    extern static void regist_log_info_func(log_handler handler);

    [DllImport("network_shared")]
    extern static void regist_log_debug_func(log_handler handler);

    [DllImport("network_shared")]
    extern static void regist_log_error_func(log_handler handler);
    public static void LogInfo(string data, ulong size)
    {
        Debug.Log(data);
    }
    public static void LogDebug(string data, ulong size)
    {
        Debug.Log(data);
    }
    public static void LogError(string data, ulong size)
    {
        Debug.LogError(data);
    }

    public static void Init()
    {
        regist_log_info_func(LogInfo);
        regist_log_debug_func(LogDebug);
        regist_log_error_func(LogError);
    }
}
