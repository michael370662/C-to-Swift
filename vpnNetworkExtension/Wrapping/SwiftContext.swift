import Foundation


public class SwiftContext
{
    public typealias DestroyFunc = @convention(c) (UnsafeMutableRawPointer?) -> Void
    
    private var mSwiftPointer : UnsafeMutableRawPointer?
    
    public var pointer : UnsafeMutableRawPointer? {
        get { return mSwiftPointer }
    }
    
    public init()
    {
        mSwiftPointer = UnsafeMutableRawPointer(Unmanaged<SwiftContext>.passRetained(self).toOpaque())
    }
    
    public func onClose()
    {
        if (mSwiftPointer == nil) { return }
        _ = Unmanaged<SwiftContext>.fromOpaque(mSwiftPointer!).takeRetainedValue()
        mSwiftPointer = nil
    }
    
    public static func fromValue<T : SwiftContext>(_ swiftContextPointer : UnsafeMutableRawPointer?) -> T?
    {
        if let val = swiftContextPointer {
            let context = Unmanaged<SwiftContext>.fromOpaque(val).takeUnretainedValue()
            return context as? T
        }
        return nil
    }    
}

func swiftObjectDestrory(_ swiftContext : UnsafeMutableRawPointer?)
{
    if let val : SwiftContext = SwiftContext.fromValue(swiftContext){
        val.onClose()
    }
}


