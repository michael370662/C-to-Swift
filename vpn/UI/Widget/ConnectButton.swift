//
//  ConnectButton.swift
//  ios
//
//  Created by ponytechhk on 22/2/2019.
//  Copyright © 2019 ponytechhk. All rights reserved.
//

import Foundation
import UIKit
import Security

//MARK: class for the connetion button
class ConnectButton: UIButton {
    
    override func awakeFromNib() {
        
        super.awakeFromNib()
        
        enum ErrorsToThrow: Error {
            case noStatus
        }
        
        let screenSize = UIScreen.main.bounds
        let bthRadius = screenSize.width / 2
        
        self.backgroundColor = UIColor.orange
        
        self.frame.size = CGSize(width: bthRadius, height: bthRadius)
        
        self.frame = CGRect(x:  screenSize.width / 4 , y:  screenSize.height / 4 , width: bthRadius, height: bthRadius)
        self.layer.cornerRadius = 0.5 * self.bounds.size.width
        self.layer.borderColor = UIColor.lightGray.cgColor
        self.layer.borderWidth = 1.0
        self.clipsToBounds = true
        self.contentEdgeInsets.bottom = 4
    }
    
}
