//
//  ModifyConfigViewController.swift
//  vpn
//
//  Created by ponytech on 16/5/2019.
//  Copyright © 2019年 ponytech. All rights reserved.
//

import UIKit
import vpncore

class ModifyConfigViewController: UIViewController ,UITextFieldDelegate{

    
    @IBOutlet weak var tf_ServerAddress: UITextField!
    @IBOutlet weak var tf_Username: UITextField!
    @IBOutlet weak var tf_Password: UITextField!
    @IBOutlet weak var tf_ServerIdentity: UITextField!
    
    @IBOutlet weak var lb_EmptyServerAddress: UILabel!
    @IBOutlet weak var lb_EmptyUsername: UILabel!
    @IBOutlet weak var lb_EmptyPassword: UILabel!
    @IBOutlet weak var lb_EmptyServerIdentity: UILabel!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        initConfig()
        
        tf_Username.delegate = self
        tf_Password.delegate = self
        tf_ServerAddress.delegate = self
        tf_ServerIdentity.delegate = self
        // Do any additional setup after loading the view.
    }
    
    func initConfig() -> Void {
        if let profile = UserDefaults.testProfile {
            tf_ServerAddress.text = profile.gateway
            tf_Username.text = profile.username
            tf_Password.text = profile.password
            tf_ServerIdentity.text = profile.serverId
        }
    }

    @IBAction func OnClickSaveConfig(_ sender: Any) {
        if !verifyInput() {
            return
        }
        
        let profile = SavableProfile()
        profile.password = tf_Password.text ?? String()
        profile.gateway = tf_ServerAddress.text ?? String()
        profile.username = tf_Username.text ?? String()
        profile.serverId = tf_ServerIdentity.text ?? String()
        
        UserDefaults.testProfile = profile
        self.dismiss(animated: true)
    }
    
    @IBAction func OnClickDeleteConfig(_ sender: Any) {
        UserDefaults.testProfile = nil
        self.dismiss(animated: true)
    }
    
    func verifyInput() -> Bool {
        var verify:Bool = true
        verify = (checkEmpty(textField: tf_ServerAddress,errorLabel:lb_EmptyServerAddress) && verify)
        verify = (checkEmpty(textField: tf_Username,errorLabel:lb_EmptyUsername) && verify)
        verify = (checkEmpty(textField: tf_Password,errorLabel:lb_EmptyPassword) && verify)
        verify = (checkEmpty(textField: tf_ServerIdentity,errorLabel:lb_EmptyServerIdentity) && verify)
        return verify
    }
    
    func checkEmpty(textField:UITextField,errorLabel:UILabel) -> Bool{
        if textField.text!.isEmpty{
            errorLabel.isHidden = false
            return false
        }
        errorLabel.isHidden = true
        return true
    }
    
    func textField(_ textField: UITextField, shouldChangeCharactersIn range: NSRange, replacementString string: String) -> Bool 
    {
        if tf_ServerAddress == textField{
            return UITextField.isValidText(textType: UITextField.textType.IP,replacementString: string)
        }
        return true
    }
        
    @IBAction func OnClickReturn(_ sender: Any) {
        self.dismiss(animated: true)
    }
}
