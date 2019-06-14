import Foundation


class WrapHelper
{
    
    public static func initialize()
    {
        on_lib_load()
    }
    
    public static func finalize()
    {
        on_lib_unload()
    }
    
    public static func registerFunction()
    {
        register_print_log(printLog)
        register_swift_destroy(swiftObjectDestrory)
        register_vpn_setting_username(vpnSettingWrapper_Username)
        register_vpn_setting_password(vpnSettingWrapper_Password)
        register_vpn_setting_gateway(vpnSettingWrapper_Gateway)
        register_vpn_setting_server_id(vpnSettingWrapper_ServerId)

        register_vpn_service_state(vpnService_State)
        
        register_tun_setting_set_addr(tunSetting_SetAddr)
        register_tun_setting_add_route(tunSetting_AddRoute)
        register_tun_setting_add_dns(tunSetting_AddDNS)
        register_tun_setting_establish(tunSetting_Establish)
        register_tun_setting_send_packet(tunSetting_SendPacket)
    }
    
    public static func setLogFile(_ path : String?, _ append : Bool)
    {
        set_log_file(path, append)
    }
}


fileprivate func printLog(_ message: UnsafePointer<CChar>?)
{
    if let stringMessage = String(validatingUTF8: message!) {
        NSLog(stringMessage)
    }
}

fileprivate func vpnService_State(_ state : Int32)
{
    if let instance = PacketTunnelProvider.Instance {
        instance.processCode(Int(state))
    }
}

fileprivate func tunSetting_SetAddr(_ addr: UnsafePointer<Int8>?, _ prefix : Int32)
{
    guard let net = PacketTunnelProvider.networkSetting else { return }
    if let addrStr = String(validatingUTF8: addr!) {
        net.setVirtualIp(addrStr, Int(prefix))
    }
}

fileprivate func tunSetting_AddRoute(_ addr: UnsafePointer<Int8>?, _ prefix : Int32)
{
    guard let net = PacketTunnelProvider.networkSetting else { return }
    if let addrStr = String(validatingUTF8: addr!) {
        net.addRoute(addrStr, Int(prefix))
    }
}

fileprivate func tunSetting_AddDNS(_ addr: UnsafePointer<Int8>?)
{
    guard let net = PacketTunnelProvider.networkSetting else { return }
    if let addrStr = String(validatingUTF8: addr!) {
        net.addDNS(addrStr)
    }
}

fileprivate func tunSetting_Establish() -> Bool
{
    var lock = NSCondition()
    lock.lock()
    defer { lock.unlock() }
    
    var value  = false
    guard let instance = PacketTunnelProvider.Instance else { return false }
    
    let ret = instance.getEstablish({ (arg : Bool) -> Void in
        lock.lock()
        NSLog("Get establish value \(arg)")
        value = arg
        lock.signal()
        lock.unlock()
    })
    if (!ret) { return false }
    
    lock.wait()
    NSLog("Get establish wait done")
    return value
}

fileprivate func tunSetting_SendPacket(_ addr: UnsafeRawPointer?, _ count : Int32)
{
    if (count == 0) { return }
    guard let addr = addr else { return }
    guard let instance = PacketTunnelProvider.Instance else { return }

    let data = Data(bytes: addr, count: Int(count))
    instance.udpToTun(data)
    
}
