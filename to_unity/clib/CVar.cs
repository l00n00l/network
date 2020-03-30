using System.Runtime.InteropServices;

public class CVarIterator
{
    [DllImport("network_shared")]
    extern static ulong create_dict_iterator(ulong data_id);

    [DllImport("network_shared")]
    extern static void remove_dict_iterator(ulong iter_id);

    [DllImport("network_shared")]
    extern static System.IntPtr dict_iterator_get(ulong iter_id);

    ulong id;
    public CVarIterator(ulong data_id)
    {
        id = create_dict_iterator(data_id);
    }

    ~CVarIterator()
    {
        remove_dict_iterator(id);
    }

    public string get_data()
    {
        return Marshal.PtrToStringAuto(dict_iterator_get(id));
    }
}

public class CVar
{
    [DllImport("network_shared")]
    extern static ulong gen_net_dict();

    [DllImport("network_shared")]
    extern static void remove_net_dict(ulong id);

    [DllImport("network_shared")]
    extern static void remove_net_value(
        ulong id,
        [MarshalAs(UnmanagedType.LPStr)]string key,
        ulong key_size);
    static void remove_net_value(ulong id, string key)
    {
        remove_net_value(id, key, (ulong)key.Length);
    }

    [DllImport("network_shared")]
    extern static bool net_var_is_int(
        ulong id,
        [MarshalAs(UnmanagedType.LPStr)]string key,
        ulong key_size);
    static bool net_var_is_int(ulong id, string key)
    {
        return net_var_is_int(id, key, (ulong)key.Length);
    }

    [DllImport("network_shared")]
    extern static bool net_var_is_uint(
        ulong id,
        [MarshalAs(UnmanagedType.LPStr)]string key,
        ulong key_size);
    static bool net_var_is_uint(ulong id, string key)
    {
        return net_var_is_uint(id, key, (ulong)key.Length);
    }

    [DllImport("network_shared")]
    extern static bool net_var_is_float(
        ulong id,
        [MarshalAs(UnmanagedType.LPStr)]string key,
        ulong key_size);
    static bool net_var_is_float(ulong id, string key)
    {
        return net_var_is_float(id, key, (ulong)key.Length);
    }

    [DllImport("network_shared")]
    extern static bool net_var_is_string(
        ulong id,
        [MarshalAs(UnmanagedType.LPStr)]string key,
        ulong key_size);
    static bool net_var_is_string(ulong id, string key)
    {
        return net_var_is_string(id, key, (ulong)key.Length);
    }

    [DllImport("network_shared")]
    extern static long net_var_get_long(
        ulong id,
        [MarshalAs(UnmanagedType.LPStr)]string key,
        ulong key_size);
    static long net_var_get_long(ulong id, string key)
    {
        return net_var_get_long(id, key, (ulong)key.Length);
    }

    [DllImport("network_shared")]
    extern static ulong net_var_get_ulong(
        ulong id,
        [MarshalAs(UnmanagedType.LPStr)]string key,
        ulong key_size);
    static ulong net_var_get_ulong(ulong id, string key)
    {
        return net_var_get_ulong(id, key, (ulong)key.Length);
    }

    [DllImport("network_shared")]
    extern static double net_var_get_float(
        ulong id,
        [MarshalAs(UnmanagedType.LPStr)]string key,
        ulong key_size);
    static double net_var_get_float(ulong id, string key)
    {
        return net_var_get_float(id, key, (ulong)key.Length);
    }

    [DllImport("network_shared")]
    extern static System.IntPtr net_var_get_string(
        ulong id,
        [MarshalAs(UnmanagedType.LPStr)]string key,
        ulong key_size);

    static string net_var_get_string(ulong id, string key)
    {
        return Marshal.PtrToStringAuto(net_var_get_string(id, key, (ulong)key.Length));
    }

    [DllImport("network_shared")]
    extern static void net_var_set_long(
    ulong id,
    [MarshalAs(UnmanagedType.LPStr)]string key,
    ulong key_size, long value);

    [DllImport("network_shared")]
    extern static void net_var_set_ulong(
    ulong id,
    [MarshalAs(UnmanagedType.LPStr)]string key,
    ulong key_size, ulong value);

    [DllImport("network_shared")]
    extern static void net_var_set_float(
    ulong id,
    [MarshalAs(UnmanagedType.LPStr)]string key,
    ulong key_size, double value);

    [DllImport("network_shared")]
    extern static void net_var_set_string(
    ulong id,
    [MarshalAs(UnmanagedType.LPStr)]string key,
    ulong key_size,
    [MarshalAs(UnmanagedType.LPStr)]string value,
    ulong value_size);

    private ulong id;

    public ulong get_id()
    {
        return id;
    }

    public CVar()
    {
        id = gen_net_dict();
    }
    public CVar(ulong dataId)
    {
        id = dataId;
    }
    ~CVar()
    {
        remove_net_dict(id);
    }
    public void remove_value(string key)
    {
        remove_net_value(id, key);
    }
    public bool is_long(string key)
    {
        return net_var_is_int(id, key);
    }
    public bool is_ulong(string key)
    {
        return net_var_is_uint(id, key);
    }
    public bool is_float(string key)
    {
        return net_var_is_float(id, key);
    }
    public bool is_string(string key)
    {
        return net_var_is_string(id, key);
    }
    public long get_long(string key)
    {
        return net_var_get_long(id, key);
    }
    public ulong get_ulong(string key)
    {
        return net_var_get_ulong(id, key);
    }
    public double get_float(string key)
    {
        return net_var_get_float(id, key);
    }
    public string get_string(string key)
    {
        return net_var_get_string(id, key);
    }

    public void set_long(string key, long value)
    {
        net_var_set_long(id, key, (ulong)key.Length, value);
    }

    public void set_ulong(string key, ulong value)
    {
        net_var_set_ulong(id, key, (ulong)key.Length, value);
    }

    public void set_float(string key, double value)
    {
        net_var_set_float(id, key, (ulong)key.Length, value);
    }

    public void set_string(string key, string value)
    {
        net_var_set_string(id, key, (ulong)key.Length, value, (ulong)value.Length);
    }
}
