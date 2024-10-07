#include <security_system/security_system.hpp>

#ifdef WIN32
#include <windows.h>

int WinMain([[maybe_unused]] HINSTANCE hInstance, [[maybe_unused]] HINSTANCE hPrevInstance, [[maybe_unused]] char*, [[maybe_unused]] int nShowCmd)
{
    int argc = 0;
    QApplication app(argc, 0);

    SecuritySystem securitySystem;
    securitySystem.show();

    return app.exec();
}
#else
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    SecuritySystem securitySystem;
    securitySystem.show();

    return app.exec();
}
#endif


SecuritySystem::SecuritySystem(QMainWindow* parent): QMainWindow(parent)
{
    // Initialize UI elements
    setupUi(this);

    // Connect UI elements to slots
    connect(loginButton, &QPushButton::clicked, this, &SecuritySystem::onLoginButtonClicked);
    connect(registerButton, &QPushButton::clicked, this, &SecuritySystem::onRegisterButtonClicked);
}

void SecuritySystem::onLoginButtonClicked()
{
    cv::Mat img, imgPre;

#ifdef BCMHOST
    img = takeImage();
#else
    // Open dialog to chose an image
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"),
        QStandardPaths::standardLocations(QStandardPaths::HomeLocation).at(0), tr("Image Files (*.png *.jpg *.jpeg *.bmp)"));

    if (fileName.isEmpty())
        return;

    // Read image
    img = cv::imread(fileName.toStdString().c_str(), cv::ImreadModes::IMREAD_GRAYSCALE);
#endif

    std::string username;
    if(DBHandler::Login(&username, img))
    {
        std::cout << "Login successful. Detected user " << username << std::endl;
    }
    else
    {
        std::cout << "Unrecognised user" << std::endl;
    }
}

void SecuritySystem::onRegisterButtonClicked()
{
    cv::Mat img, imgPre;

#ifdef BCMHOST
    img = takeImage();
#else
    // Open dialog to chose an image
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"),
        QStandardPaths::standardLocations(QStandardPaths::HomeLocation).at(0), tr("Image Files (*.png *.jpg *.jpeg *.bmp)"));

    if (fileName.isEmpty())
        return;

    // Read image
    img = cv::imread(fileName.toStdString().c_str(), cv::ImreadModes::IMREAD_GRAYSCALE);
#endif

    Biometry::PreprocessImage(img, imgPre);
    KeypointsTuple features = Biometry::KAZEDetector(imgPre);
    int result = DBHandler::WriteEntry(features, usernameLineEdit->text().toStdString());
    if (result == 0)
    {
        std::cout << "Register successful" << std::endl;
    }
    else
    {
        std::cout << "Register failed" << std::endl;
    }
}

#ifdef BCMHOST
cv::Mat SecuritySystem::takeImage()
{
    VideoCapture vc(0);
    Mat img, img2;

    // Activate Lights ////////////////
    digitalWrite(4, HIGH);
    digitalWrite(5, HIGH);
    ///////////////////////////////////

    // Take image /////////////////////
    vc.read(img);
    cvtColor(img, img, COLOR_BGR2GRAY);
    ///////////////////////////////////

    // Reduce image to a region ///////
    Rect fingerRegion = Rect(255, 0, 465-255, img.rows-40);
    img2 = img(fingerRegion);
    ///////////////////////////////////

    // Deactivate Lights //////////////
    digitalWrite(4, LOW);
    digitalWrite(5, LOW);
    ///////////////////////////////////
    
    return img2.clone();
}
#endif
