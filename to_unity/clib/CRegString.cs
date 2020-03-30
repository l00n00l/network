using System.Runtime.InteropServices;

public class CRegString
{
    [DllImport("network_shared")]
    extern static ulong create_regstring();

    [DllImport("network_shared")]
    extern static void remove_regstring(ulong id);

    [DllImport("network_shared")]
    extern static bool regstring_parse_regex(ulong id,
        [MarshalAs(UnmanagedType.LPStr)]string regstr,
        ulong regstr_size);

    [DllImport("network_shared")]
    extern static void regstring_set(ulong id,
        [MarshalAs(UnmanagedType.LPStr)]string name,
        [MarshalAs(UnmanagedType.LPStr)]string data,
        ulong data_size);

    [DllImport("network_shared")]
    extern static System.IntPtr regstring_get(ulong id);

    ulong id;
    CRegString()
    {
        id = create_regstring();
    }

    ~CRegString()
    {
        remove_regstring(id);
    }

    public bool ParseString(string regex_str)
    {
        return regstring_parse_regex(id, regex_str, (ulong)regex_str.Length);
    }

    public string Str()
    {
        return Marshal.PtrToStringAuto(regstring_get(id));
    }
}
