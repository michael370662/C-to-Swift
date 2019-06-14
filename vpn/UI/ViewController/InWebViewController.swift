import UIKit


class InWebViewController: UIViewController {

    override func viewDidLoad() {
        super.viewDidLoad()
    }
    
    @IBAction func onLogout(_ sender: UIButton) {
        let profile = DataCollection.getInstance().profile
        profile.username = nil
        profile.password = nil
        profile.server = nil
        profile.smsToken = nil
        profile.token = nil

        profile.save()
        profile.notifyChange()
    }
    
}
