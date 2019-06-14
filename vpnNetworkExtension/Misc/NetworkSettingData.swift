import Foundation
import NetworkExtension
import vpncore

class NetworkSettingData
{
    private var mGateway : String = String()
    private var mVirtualIp : NEIPv4Settings?
    private var mRoutes : [NEIPv4Route] = [NEIPv4Route]()
    private var mDNS : [String] = [String]()
    
    public init(_ gateway : String?)
    {
        if let gateway = gateway {
            let list = gateway.components(separatedBy: ":")
            if (list.count > 0) {
                Log.v("The gateway setting is \(list[0])")
                mGateway = list[0]
            }
        }
    }
    
    public func setVirtualIp(_ address : String, _ prefix: Int) {
        let subnet = IPSubnet.subnet(address, prefix)
        Log.i("The virtual ip = \(address) : \(subnet)")
        mVirtualIp = NEIPv4Settings(addresses: [address], subnetMasks: [subnet])
    }
    
    public func addRoute(_ address : String, _ prefix: Int) {
        let subnet = IPSubnet.subnet(address, prefix)
        Log.i("Add route = \(address) : \(subnet)")
        mRoutes.append(NEIPv4Route(destinationAddress: address, subnetMask: subnet))
    }
    
    public func addDNS(_ address : String) {
        Log.i("Add DNS = \(address)")
        mDNS.append(address)
    }
    
    public func generateConfig() ->  NEPacketTunnelNetworkSettings?
    {
        guard let ip = mVirtualIp else { return nil }

        ip.includedRoutes = mRoutes

        let config = NEPacketTunnelNetworkSettings(tunnelRemoteAddress: mGateway)
        config.mtu = 1400
        config.ipv4Settings = ip;
        
        let DNSSettings = NEDNSSettings(servers: mDNS)
        DNSSettings.matchDomains = [""]
        DNSSettings.matchDomainsNoSearch = false
        config.dnsSettings = DNSSettings
        
        return config
    }
    
}
