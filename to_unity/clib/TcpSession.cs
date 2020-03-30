using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;

public class TcpSession : MonoBehaviour
{
    [DllImport("network_shared")]
    extern static bool session_valid(ulong session_id);

    [DllImport("network_shared")]
    extern static void disconnect(ulong session_id);

    [DllImport("network_shared", EntryPoint = "connect_server")]
    extern static ulong connect(
        [MarshalAs(UnmanagedType.LPStr)]string proto_name,
        [MarshalAs(UnmanagedType.LPStr)]string host,
        [MarshalAs(UnmanagedType.LPStr)]string port);

    [DllImport("network_shared", EntryPoint = "send_msg")]
    extern static void send(ulong session_id,
    [MarshalAs(UnmanagedType.LPStr)]string msg);

    [DllImport("network_shared", EntryPoint = "send_msg_by_dataid")]
    extern static void SendMsgByDataID(ulong session_id, ulong data_id,
[MarshalAs(UnmanagedType.LPStr)]string msg_name);

    [DllImport("network_shared")]
    extern static bool make_message(
    [MarshalAs(UnmanagedType.LPStr)]string proto_name,
    int side, ulong data_id,
    [MarshalAs(UnmanagedType.LPStr)]string to_name);

    public ulong id;
    public string protoName;
    public string host;
    public string port;

    List<CVar> msgList = new List<CVar>();

    public ulong get_id()
    {
        return this.id;
    }

    public void SendMessage(CVar msg)
    {
        msgList.Add(msg);
    }

    public bool Valid()
    {
        return session_valid(id);
    }

    public void ConnectTo(string proto_name, string host, string port)
    {
        id = connect(proto_name, host, port);
    }

    public void Disconnect()
    {
        if (Valid())
            disconnect(id);
    }

    private void Start()
    {
        ConnectTo(protoName, host, port);
    }

    private void Update()
    {
        if (Valid())
        {
            foreach (var msg in msgList)
            {
                if (make_message(protoName, 1, msg.get_id(), "[[message_data]]"))
                {
                    SendMsgByDataID(id, msg.get_id(), "[[message_data]]");
                }
            }

            msgList.Clear();
        }
    }

    private void OnDestroy()
    {
        Disconnect();
    }
}
