using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class ConnectServer : MonoBehaviour
{
    public TcpSession session;

    public void MsgHandler(ulong session_id, ulong data_id, string proto_name)
    {

    }

    private void Awake()
    {
        Network.RegistHandler(MsgHandler);
    }

    // Start is called before the first frame update
    void Start()
    {
    }

    // Update is called once per frame
    void Update()
    {
        CVar msg = new CVar();
        var s = "hello world!";
        var size = s.Length.ToString();
        while (size.Length < 4)
        {
            size = "0" + size;
        }
        msg.set_string("size", size);
        msg.set_string("data", s);
        session.SendMessage(msg);
    }

}
