#include "movierendererwindow.h"
#include "movierenderer.h"
#include "ui_movierendererwindow.h"

#include <QProgressBar>
#include <QSettings>
#include <QFileDialog>
#include <QCloseEvent>

#include <QImageWriter>

MovieRendererWindow::MovieRendererWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MovieRendererWindow)
    , m_movieRenderer(new MovieRenderer)
{
    ui->setupUi(this);
    ui->centralwidget->setLayout(ui->verticalLayout);
    connect(ui->openQmlFileButton, SIGNAL(clicked()), this, SLOT(getQmlFile()));
    connect(ui->outputDirectoryPushButton, SIGNAL(clicked()), this, SLOT(getOutputDirectory()));
    connect(ui->outputDirectoryLineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(checkEnableRender()));
    connect(ui->qmlFileLineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(checkEnableRender()));
    connect(ui->renderMovieButton, SIGNAL(clicked()), this, SLOT(renderMovie()));
    connect(m_movieRenderer, SIGNAL(finished()), this, SLOT(handleMovieFinished()));

    // Statusbar
    m_progressLabel = new QLabel(this);
    m_progressLabel->setText("Render Progress");
    ui->statusbar->addWidget(m_progressLabel);
    m_progressBar = new QProgressBar(this);
    m_progressBar->setValue(0);
    m_progressBar->setMinimum(0);
    m_progressBar->setMaximum(100);
    ui->statusbar->addWidget(m_progressBar);
    connect(m_movieRenderer, SIGNAL(progressChanged(int)), m_progressBar, SLOT(setValue(int)));

    m_fileProgressLabel = new QLabel(this);
    m_fileProgressLabel->setText("Write Progress");
    ui->statusbar->addWidget(m_fileProgressLabel);
    m_fileProgressBar = new QProgressBar(this);
    m_fileProgressBar->setValue(0);
    m_fileProgressBar->setMinimum(0);
    m_fileProgressBar->setMaximum(100);
    ui->statusbar->addWidget(m_fileProgressBar);
    connect(m_movieRenderer, SIGNAL(fileProgressChanged(int)), m_fileProgressBar, SLOT(setValue(int)));
    setProgressBarsVisible(false);

    // Populate Output Format
    for (QByteArray format : QImageWriter::supportedImageFormats()) {
        ui->imageFormatComboBox->addItem(QString::fromLocal8Bit(format));
    }
    // Set Defaults
    readSettings();
}

MovieRendererWindow::~MovieRendererWindow()
{
    delete ui;
    delete m_movieRenderer;
}

void MovieRendererWindow::getQmlFile()
{
    QString qmlFile = QFileDialog::getOpenFileName(this, "QML file", QString(), QString("QML Files(*.qml)"));
    ui->qmlFileLineEdit->setText(qmlFile);
}

void MovieRendererWindow::getOutputDirectory()
{
    QString directory = QFileDialog::getExistingDirectory(this, "Output Directory");
    ui->outputDirectoryLineEdit->setText(directory);
}

void MovieRendererWindow::checkEnableRender()
{
    if (!ui->qmlFileLineEdit->text().isEmpty() &&
        !ui->outputDirectoryLineEdit->text().isEmpty() &&
        !m_movieRenderer->isRunning()) {
        if (QFile::exists(ui->qmlFileLineEdit->text())) {
            ui->renderMovieButton->setEnabled(true);
            return;
        }
    }

    ui->renderMovieButton->setEnabled(false);
}

void MovieRendererWindow::renderMovie()
{
    //Star Render
    m_movieRenderer->renderMovie(ui->qmlFileLineEdit->text(), ui->outputFilenameLineEdit->text(),
                                 ui->outputDirectoryLineEdit->text(), ui->imageFormatComboBox->currentText(),
                                 QSize(ui->widthSpinBox->value(), ui->heightSpinBox->value()), 1.0,
                                 ui->durationSpinBox->value() * 1000, ui->fpsSpinBox->value());

    //Disable button
    checkEnableRender();

    //Show progress bar
    setProgressBarsVisible(true);
}

void MovieRendererWindow::handleMovieFinished()
{
    //Enable Button
    checkEnableRender();

    //Hide progress bar
    setProgressBarsVisible(false);
}

void MovieRendererWindow::setProgressBarsVisible(bool isVisible)
{
    m_progressBar->setVisible(isVisible);
    m_progressLabel->setVisible(isVisible);
    m_fileProgressBar->setVisible(isVisible);
    m_fileProgressLabel->setVisible(isVisible);
}

void MovieRendererWindow::readSettings()
{
    ui->qmlFileLineEdit->setText(m_settings.value("qmlFile").toString());
    ui->widthSpinBox->setValue(m_settings.value("outputWidth", 1280).toInt());
    ui->heightSpinBox->setValue(m_settings.value("outputHeight", 720).toInt());
    ui->durationSpinBox->setValue(m_settings.value("duration", 2.0).toDouble());
    ui->fpsSpinBox->setValue(m_settings.value("fps", 24).toInt());
    ui->outputDirectoryLineEdit->setText(m_settings.value("outputDir", QDir::currentPath()).toString());
    ui->outputFilenameLineEdit->setText(m_settings.value("outputFilename", "output").toString());
    QString format = m_settings.value("format", "jpg").toString();
    ui->imageFormatComboBox->setCurrentIndex(ui->imageFormatComboBox->findText(format));
}

void MovieRendererWindow::writeSettings()
{
    m_settings.setValue("qmlFile", ui->qmlFileLineEdit->text());
    m_settings.setValue("outputWidth", ui->widthSpinBox->value());
    m_settings.setValue("outputHeight", ui->heightSpinBox->value());
    m_settings.setValue("duration", ui->durationSpinBox->value());
    m_settings.setValue("fps", ui->fpsSpinBox->value());
    m_settings.setValue("outputDir", ui->outputDirectoryLineEdit->text());
    m_settings.setValue("outputFilename", ui->outputFilenameLineEdit->text());
    m_settings.setValue("format", ui->imageFormatComboBox->currentText());
}


void MovieRendererWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}
