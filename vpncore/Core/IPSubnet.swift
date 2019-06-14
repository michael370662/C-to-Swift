import Foundation

public class IPSubnet
{
    
    public static func subnet(_ address : String, _ prefix : Int) -> String {
        let element = address.components(separatedBy: ".")
        if (element.count == 4) {
            return ipv4Subnet(prefix)
        }
       return String("255.255.255.255")
    }
    
    private static func ipv4Subnet(_ prefix : Int) -> String {
        var value = ""
        var pos = prefix
        
        for index in 0...3 {
            value = value + String(ipv4Mask(pos))
            if (index != 3) {
                value = value + "."
            }
            pos = pos < 8 ? 0 : pos - 8
        }
        return value
    }
    
    private static func ipv4Mask(_ pos: Int) -> Int {
        if (pos >= 8) { return 255 }
        switch(pos)
        {
        case 0 : return 0
        case 1 : return 128
        case 2 : return 192
        case 3 : return 224
        case 4 : return 240
        case 5 : return 248
        case 6 : return 252
        case 7 : return 254
        default: return 255
        }
    }
    
}
