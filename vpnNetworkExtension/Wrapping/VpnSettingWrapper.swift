import Foundation
import vpncore


class VpnSettingWrapper : SwiftContext
{
    private var mProfile : VpnProfile
    public var profile :VpnProfile { get { return mProfile }}
    
    public init(profile : VpnProfile)
    {
        mProfile = profile
    }
}


func vpnSettingWrapper_Username(_ setting : UnsafeMutableRawPointer?,
                                _ workspace : UnsafeMutableRawPointer?,
                                _ callback : (@convention(c) (_ ws : UnsafeMutableRawPointer?, _ string : UnsafePointer<Int8>?) -> Void)?)
{
    guard let callback = callback else { return}
    guard let val : VpnSettingWrapper = SwiftContext.fromValue(setting) else  { return }
    callback(workspace, val.profile.username)
}

func vpnSettingWrapper_Password(_ setting : UnsafeMutableRawPointer?,
                                _ workspace : UnsafeMutableRawPointer?,
                                _ callback : (@convention(c) (_ ws : UnsafeMutableRawPointer?, _ string : UnsafePointer<Int8>?) -> Void)?)
{
    guard let callback = callback else { return}
    guard let val : VpnSettingWrapper = SwiftContext.fromValue(setting) else  { return }
    callback(workspace, val.profile.password)
}

func vpnSettingWrapper_Gateway(_ setting : UnsafeMutableRawPointer?,
                                _ workspace : UnsafeMutableRawPointer?,
                                _ callback : (@convention(c) (_ ws : UnsafeMutableRawPointer?, _ string : UnsafePointer<Int8>?) -> Void)?)
{
    guard let callback = callback else { return}
    guard let val : VpnSettingWrapper = SwiftContext.fromValue(setting) else  { return }
    callback(workspace, val.profile.gateway)
}

func vpnSettingWrapper_ServerId(_ setting : UnsafeMutableRawPointer?,
                                _ workspace : UnsafeMutableRawPointer?,
                                _ callback : (@convention(c) (_ ws : UnsafeMutableRawPointer?, _ string : UnsafePointer<Int8>?) -> Void)?)
{
    guard let callback = callback else { return}
    guard let val : VpnSettingWrapper = SwiftContext.fromValue(setting) else  { return }
    callback(workspace, val.profile.serverId)
}
