using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class message_handler : MonoBehaviour
{
    Dictionary<string, Dictionary<ulong, List<CVar>>> msgData = new Dictionary<string, Dictionary<ulong, List<CVar>>>();

    public void MsgHandler(ulong session_id, ulong data_id, string proto_name)
    {
        msgData[proto_name][session_id].Add(new CVar(data_id));
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

    }
}
