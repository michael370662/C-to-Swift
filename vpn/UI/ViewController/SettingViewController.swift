//
//  SettingViewController.swift
//  vpn
//
//  Created by ponytech on 6/5/2019.
//  Copyright © 2019年 ponytech. All rights reserved.
//

import UIKit

class SettingViewController: UIViewController, UITableViewDelegate,
UITableViewDataSource{
    let items = ["settingview_select_language","settingview_edit_config","settingview_show_log"]
    
    override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
    }
   
    func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return items.count
    }
    
    func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = tableView.dequeueReusableCell(withIdentifier: "SettingCell", for: indexPath) as! SettingTableViewCell
         cell.lb_Title.text = NSLocalizedString("\(items[indexPath.item])", comment: "")
        return cell
    }
    
    func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        switch indexPath.item {
        case 0:
            performSegue(withIdentifier: "MoveToSelectLang", sender: self)
        case 1:
            performSegue(withIdentifier: "MoveToModifyConfig", sender: self)
        case 2:
            performSegue(withIdentifier: "MoveToShowLog", sender: self)
            break
        default:
            return
        }
    }
    
    @IBAction func onClickReturn(_ sender: UIButton) {
        self.dismiss(animated: true)
    }
}
