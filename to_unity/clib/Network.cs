using UnityEngine;
using System.Runtime.InteropServices;

public class Network : MonoBehaviour
{
    public delegate void proto_handler(
        ulong session_id,
        ulong cvar_id,
        string proto_name);

    [DllImport("network_shared", EntryPoint = "set_msg_handler")]
    public extern static void RegistHandler(proto_handler handler);

    [DllImport("network_shared")]
    extern static bool init([MarshalAs(UnmanagedType.LPStr)]string proto_path);

    [DllImport("network_shared")]
    extern static void run();

    [DllImport("network_shared")]
    extern static void stop();

    public string protoPath;
    bool inited = false;

    private void init()
    {
        Clog.Init();
        inited = init(protoPath);
        if (!inited)
        {
            Debug.LogError("init network error");
        }
    }

    private void Awake()
    {
        init();
    }

    private void LateUpdate()
    {
        if (inited)
        {
            run();
        }
        else
        {
            init();
        }
    }


    private void OnDestroy()
    {
        if (inited)
            stop();
    }
}
