import Foundation
import UIKit


public extension UITextField {
    enum textType: String {
        case Alphabet = "[a-zA-Z0-9]*"
        case NumberOnly = "[0-9]*"
        case IP = "[a-z0-9\\.]*"
    }

    
    static func isValidText(textType:UITextField.textType, replacementString string: String)-> Bool{
        
        let pred = NSPredicate(format: "SELF MATCHES %@", textType.rawValue)
        return pred.evaluate(with:string)
    }
}
