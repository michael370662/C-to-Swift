import Foundation

public class Log
{
    public enum Level : Int32 {
        case Error = 0
        case Warn = 1
        case Info = 2
        case Verbose = 3
        case Trace = 4
    }
    
    public static func e(_ message : String) { log(message, Level.Error)}
    public static func w(_ message : String) { log(message, Level.Warn)}
    public static func i(_ message : String) { log(message, Level.Info)}
    public static func v(_ message : String) { log(message, Level.Verbose)}
    public static func t(_ message : String) { log(message, Level.Trace)}
    
    
    
    public static func log(_ message : String, _ level: Level)
    {
        send_log_message(message, level.rawValue)
    }
}
