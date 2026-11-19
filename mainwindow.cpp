#include "mainwindow.h"
#include "M_MEdit_ui.h"

#include <QCloseEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QTextStream>
#include <QTimer>
#include <QInputDialog>
#include <QKeyEvent>
#include <QTextCursor>
#include <QTextFormat>
#include <QRegularExpression>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , previewTimer(new QTimer(this))
    , ui(new Ui::MainWindow)
{
    // 初始化界面，并在文档内容修改时更新窗口标题上的修改标记。
    ui->setupUi(this);
    ui->textEdit->setFont(QFont("Consolas", 14));
    setCurrentFile(QString());
    connect(ui->textEdit->document(), &QTextDocument::modificationChanged,
            this, &QWidget::setWindowModified);

    ui->actionTogglePreview->setCheckable(true);
    ui->actionTogglePreview->setChecked(true);

    connect(ui->textEdit, &QPlainTextEdit::textChanged, this, [this]()
    {
        previewTimer->start(150);
    });

    ui->previewBrowser->setFont(QFont("Consolas", 14));
    connect(previewTimer, &QTimer::timeout, this, [this]()
    {
        ui->previewBrowser->setMarkdown(ui->textEdit->toPlainText());
    });
}

// 释放界面对象占用的内存。
MainWindow::~MainWindow()
{
    delete ui;
}

// “新建”：先处理未保存内容，再清空编辑区并切换到未命名文档。
void MainWindow::on_actionC_triggered()
{
    if (!maybeSave())
        return;

    ui->textEdit->clear();
    setCurrentFile(QString());
}

// “打开”：选择文件并将其文本载入编辑区。
void MainWindow::on_actionOpen_triggered()
{
    if (!maybeSave())
        return;

    const QString fileName = QFileDialog::getOpenFileName(
        this, tr("打开文件"), QString(), tr("Markdown 文件 (*.md);;所有文件 (*)"));
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("打开失败"),
                             tr("无法打开文件：%1\n%2").arg(fileName, file.errorString()));
        return;
    }

    QTextStream in(&file);
    ui->textEdit->setPlainText(in.readAll());
    setCurrentFile(fileName);
}

// “保存”：已有文件名时覆盖保存，否则弹出“另存为”对话框。
void MainWindow::on_actionSave_triggered()
{
    if (currentFile.isEmpty())
        saveAs();
    else
        saveFile(currentFile);
}

// “另存为”：通过文件对话框选择保存位置和文件名。
void MainWindow::on_actionSave_As_triggered()
{
    saveAs();
}

// “退出”：关闭主窗口，关闭前会由 closeEvent 检查未保存内容。
void MainWindow::on_actionExit_triggered()
{
    close();
}

// 撤销编辑区中的上一步操作。
void MainWindow::on_actionUndo_triggered()
{
    ui->textEdit->undo();
}

// 剪切编辑区中选中的文本。
void MainWindow::on_actionCut_triggered()
{
    ui->textEdit->cut();
}

// 将剪贴板文本粘贴到编辑区。
void MainWindow::on_actionPaste_triggered()
{
    ui->textEdit->paste();
}

// 检查文档是否有未保存的修改，并询问用户保存、放弃或取消操作。
bool MainWindow::maybeSave()
{
    if (!ui->textEdit->document()->isModified())
        return true;

    const QMessageBox::StandardButton answer = QMessageBox::warning(
        this, tr("未保存的更改"),
        tr("文档已修改，是否保存更改？"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
        QMessageBox::Save);

    if (answer == QMessageBox::Save) {
        if (currentFile.isEmpty())
            return saveAs();
        return saveFile(currentFile);
    }
    return answer == QMessageBox::Discard;
}

// 将编辑区内容写入指定文件；成功后更新当前文件名和修改状态。
bool MainWindow::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("保存失败"),
                             tr("无法保存文件：%1\n%2").arg(fileName, file.errorString()));
        return false;
    }

    QTextStream out(&file);
    out << ui->textEdit->toPlainText();
    if (out.status() != QTextStream::Ok) {
        QMessageBox::warning(this, tr("保存失败"),
                             tr("写入文件时出错：%1").arg(fileName));
        return false;
    }

    setCurrentFile(fileName);
    return true;
}

// 弹出“另存为”对话框，并将内容保存到用户指定的文件。
bool MainWindow::saveAs()
{
    const QString fileName = QFileDialog::getSaveFileName(
        this, tr("另存为"), currentFile, tr("Markdown 文件 (*.md);;所有文件 (*)"));
    if (fileName.isEmpty())
        return false;
    return saveFile(fileName);
}

// 更新当前文件路径、文档修改状态和窗口标题。
void MainWindow::setCurrentFile(const QString &fileName)
{
    currentFile = fileName;
    ui->textEdit->document()->setModified(false);
    const QString shownName = currentFile.isEmpty()
        ? tr("未命名")
        : QFileInfo(currentFile).fileName();
    setWindowTitle(tr("%1[*] - Markdown 编辑器").arg(shownName));
    setWindowModified(false);
}

// 处理窗口关闭事件：有未保存修改时先询问用户。
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
        event->accept();
    else
        event->ignore();
}

// “查找”功能的槽函数。
void MainWindow::on_actionFind_triggered()
{
    bool ok = false;
    const QString searchText = QInputDialog::getText(
        this, tr("查找"), tr("请输入要查找的文本："), QLineEdit::Normal,
        QString(), &ok);
    
    if (!ok || searchText.isEmpty())
        return;
    
    bool found = ui->textEdit->find(searchText);

    if (!found) 
    {
        QTextCursor cursor = ui->textEdit->textCursor();
        cursor.movePosition(QTextCursor::Start);
        ui->textEdit->setTextCursor(cursor);
        found = ui->textEdit->find(searchText);
    }

    if (!found) 
    {
        QMessageBox::information(this, tr("查找"), tr("未找到文本：%1").arg(searchText));
    }
}

// “替换”功能的槽函数，当前未实现。
void MainWindow::on_actionReplace_triggered()
{

}

// “放大”功能的槽函数，当前未实现。
void MainWindow::on_actionZoomIn_triggered()
{

}

// “缩小”功能的槽函数，当前未实现。
void MainWindow::on_actionZoomOut_triggered()
{

}

// “重置缩放”功能的槽函数，当前未实现。
void MainWindow::on_actionZoomReset_triggered()
{

}

// “自动换行”功能的槽函数，当前未实现。
void MainWindow::on_actionWordWrap_triggered()
{
    
}

// 根据菜单动作的勾选状态显示或隐藏 Markdown 预览。
void MainWindow::on_actionTogglePreview_triggered()
{
    ui->previewBrowser->setVisible(ui->actionTogglePreview->isChecked());
}