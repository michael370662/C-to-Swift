//
//  SelectLanguageViewController.swift
//  vpn
//
//  Created by ponytech on 7/5/2019.
//  Copyright © 2019年 ponytech. All rights reserved.
//

import UIKit

class SelectLanguageViewController: UIViewController, UITableViewDelegate,
UITableViewDataSource {
     let items = ["langview_simplified_chinese","langview_traditional_chinese","langview_english"]
    
    override func viewDidLoad() {
        super.viewDidLoad()
        ///(Incomplete) setupNotifications for change language immediately in current view
        //NotificationHelper.shareInstance.setupNotifications(viewController: self)
        
        // Do any additional setup after loading the view.
    }
    
    func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return items.count
    }
    
    func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = tableView.dequeueReusableCell(withIdentifier: "LanguageCell", for: indexPath) as! LanguageTableViewCell

        cell.lb_Title.text = NSLocalizedString("\(items[indexPath.item])", comment: "")
    
        return cell
    }
    
    func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath)
    {
        changeLanguage(index:indexPath.item)
    }

    func changeLanguage(index : Int) -> Void {
    
        var lang : String = ""
        switch index {
        case 0:
            lang = "zh-Hans"
        case 1:
            lang = "zh-Hant"
        default:
            lang = "en"
        }
        LanguageHelper.shareInstance.setLanguage(language:lang)
    }
    
    @IBAction func onClickReturn(_ sender: UIButton) {
        self.dismiss(animated: true)
    }
}
