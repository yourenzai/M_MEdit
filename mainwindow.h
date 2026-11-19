#pragma once

#include <QMainWindow>
#include <QString>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QCloseEvent;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QTimer *previewTimer;

private slots:
    void on_actionC_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSave_As_triggered();
    void on_actionExit_triggered();
    void on_actionUndo_triggered();
    void on_actionCut_triggered();
    void on_actionPaste_triggered();
    void on_actionFind_triggered();
    void on_actionReplace_triggered();
    void on_actionZoomIn_triggered();
    void on_actionZoomOut_triggered();
    void on_actionZoomReset_triggered();
    void on_actionWordWrap_triggered();
    void on_actionTogglePreview_triggered();

private:
    bool maybeSave();
    bool saveFile(const QString &fileName);
    bool saveAs();
    void setCurrentFile(const QString &fileName);

    void closeEvent(QCloseEvent *event) override;

    Ui::MainWindow *ui;
    QString currentFile;
};