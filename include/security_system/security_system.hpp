#ifndef SECURITY_SYSTEM_H
#define SECURITY_SYSTEM_H

#include <security_system/config.h>
#include <finger_vascular_biometry/DBHandler.hpp>
#include "ui_security_system.h"
#include <QFileDialog>
#include <QStandardPaths>
#include <opencv2/highgui.hpp>

#ifdef BCMHOST
#include <wiringPi.h>
#endif

class SecuritySystem : public QMainWindow, private Ui::mainWindow
{
    Q_OBJECT

public:
    explicit SecuritySystem(QMainWindow* parent = nullptr);

#ifdef BCMHOST
private:
    cv::Mat takeImage();
#endif

private slots:
    void onLoginButtonClicked();
    void onRegisterButtonClicked();
};

#endif