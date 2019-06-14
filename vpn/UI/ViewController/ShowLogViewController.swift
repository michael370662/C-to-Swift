import UIKit
import vpncore

class ShowLogViewController: UIViewController
{
    @IBOutlet weak var mTextView: UITextView!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        // Do any additional setup after loading the view.
        mTextView.text = readContent()
    }

    @IBAction func onClickReturn(_ sender: Any) {
        self.dismiss(animated: true)
    }

    private func readContent() -> String
    {
        do
        {
            if let url = FileManager.logFileURL {
                return try String(contentsOf: url, encoding: .utf8)
            }
        }
        catch {}
        return String()
    }
}
