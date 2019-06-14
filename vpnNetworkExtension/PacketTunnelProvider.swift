import Foundation
import NetworkExtension
import vpncore

class PacketTunnelProvider: NEPacketTunnelProvider {
    private var mPendingStartCompletion: ((Error?) -> Void)?
    private var mPendingStopCompletion: (() -> Void)?
    private var mNetworkSetting : NetworkSettingData?
    private var mStarted  = false

    private static var sInstance : PacketTunnelProvider?
    public static var networkSetting : NetworkSettingData?
    {
        get { return sInstance?.mNetworkSetting; }
    }
    
    override init()
    {
        super.init()
        NSLog("Network provider init")
        PacketTunnelProvider.sInstance = self
        // C++ library loading
        WrapHelper.initialize()
        WrapHelper.registerFunction()
    }
    
    deinit
    {
        NSLog("Network provider deinit")
        PacketTunnelProvider.sInstance = nil
        WrapHelper.finalize()
    }
    
    public static var Instance : PacketTunnelProvider?
    {
        get { return sInstance }
    }
    
    public func processCode(_ code: Int)
    {
        Log.i("Code is changed to \(code)")
        ErrorNotifier.notify(code, 0)
        
        if (mPendingStartCompletion != nil)
        {
            Log.v("Pending Start completion result \(code)")
            mPendingStartCompletion!(code != 1 ? Exception.general("General error") : nil)
            mPendingStartCompletion = nil
        }
        if (mPendingStopCompletion != nil)
        {
            Log.v("Pending Stop completion result \(code)")
            mPendingStopCompletion!();
            mPendingStopCompletion = nil
        }
    }
    
    public func getEstablish(_ onDone : @escaping (Bool) -> Void)-> Bool {
        
        guard let conf = mNetworkSetting?.generateConfig() else { return false }
        setTunnelNetworkSettings(conf, completionHandler: { [weak self](err : Error?) -> Void in
            guard let self = self else { return }
            self.mStarted = true
            self.tunToUDP()
            onDone(err == nil)
        })
        return true
    }
    
    public func udpToTun(_ data : Data) {
        self.packetFlow.writePackets([data], withProtocols: [AF_INET as NSNumber])
    }
    
    override func startTunnel(options: [String : NSObject]?, completionHandler: @escaping (Error?) -> Void)
    {
        NSLog("Network provider -- start Tunnel")
        
        if let url = FileManager.logFileURL {
            if let _ = options?["first"]  {
                WrapHelper.setLogFile(url.path, false);
            } else {
                WrapHelper.setLogFile(url.path, true);
            }
        }
        
        var profile : VpnProfile?
        do {
            let content = options?["vpnProfile"] as! String;
            NSLog(content)
            profile =  try VpnProfile.fromJson(content)
        } catch {
            completionHandler(Exception.general("profile is not exist"))
            NSLog("[ERROR] profile is not exist")
            return
        }

        mNetworkSetting = NetworkSettingData(profile?.gateway)

        
        if (!initialize_charon()) {
            completionHandler(Exception.general("profile is not exist"))
            NSLog("[ERROR] initialize charon error")
            return
        }

        Log.i("Charon started")
        let setting = VpnSettingWrapper(profile: profile!)
        mPendingStartCompletion = completionHandler;
        initiate_service(setting.pointer)
    }
    
    override func stopTunnel(with reason: NEProviderStopReason, completionHandler: @escaping () -> Void)
    {
        NSLog("Network provider -- stop Tunnel")
        mStarted = false
        mNetworkSetting = nil
        mPendingStopCompletion = completionHandler
        deinitialize_charon();
    }
    
    
    private func tunToUDP() {
        self.packetFlow.readPackets { [weak self] (packets: [Data], protocols: [NSNumber]) in
            guard let self = self else { return }
            if (!self.mStarted) { return }
            
            for packet in packets {
                let buffer = UnsafeMutablePointer<UInt8>.allocate(capacity: packet.count)
                packet.copyBytes(to: buffer, count : packet.count)
                send_tun_data(buffer, Int32(packet.count))
                buffer.deallocate()               
            }
            // Recursive to keep reading
            self.tunToUDP()
        }
    }
}
