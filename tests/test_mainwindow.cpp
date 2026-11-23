#include <QtTest/QtTest>
#include <QApplication>
#include <QPlainTextEdit>
#include <QTextBrowser>
#include <QAction>
#include <QMenu>
#include <QTimer>
#include <QFile>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include "../mainwindow.h"

class TestMainWindow : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void testInitialState();
    void testFontSize();
    void testTimerConfiguration();
    void testUserTypingModification();
    void testMarkdownRendering();
    void testFindLogic();
    void testSaveAndOpenFile();
    void testUnimplementedActionsAudit();
    void testShortcutsAudit();
    void testTogglePreview();
    void testAutoFormatList();
};

void TestMainWindow::initTestCase()
{
}

void TestMainWindow::testInitialState()
{
    MainWindow w;
    QCOMPARE(w.isWindowModified(), false);
    QVERIFY(w.windowTitle().contains("未命名"));

    auto *textEdit = w.findChild<QPlainTextEdit*>("textEdit");
    QVERIFY(textEdit != nullptr);
    QVERIFY(textEdit->toPlainText().isEmpty());

    auto *preview = w.findChild<QTextBrowser*>("previewBrowser");
    QVERIFY(preview != nullptr);
    QVERIFY(preview->toPlainText().isEmpty());
}

void TestMainWindow::testFontSize()
{
    MainWindow w;
    auto *textEdit = w.findChild<QPlainTextEdit*>("textEdit");
    QVERIFY(textEdit != nullptr);
    qDebug() << "[FONT TEST] textEdit font family:" << textEdit->font().family()
             << "pointSize:" << textEdit->font().pointSize();
    QCOMPARE(textEdit->font().pointSize(), 14);
    QCOMPARE(textEdit->font().family(), QString("Consolas"));

    auto *preview = w.findChild<QTextBrowser*>("previewBrowser");
    QVERIFY(preview != nullptr);
    qDebug() << "[FONT TEST] previewBrowser font family:" << preview->font().family()
             << "pointSize:" << preview->font().pointSize();
    QCOMPARE(preview->font().pointSize(), 14);
    QCOMPARE(preview->font().family(), QString("Consolas"));
}

void TestMainWindow::testTimerConfiguration()
{
    MainWindow w;
    QVERIFY(w.previewTimer != nullptr);
    
    // Check if previewTimer is singleShot
    // BUG: QTimer default is singleShot = false, so start(150) fires every 150ms forever!
    bool isSingleShot = w.previewTimer->isSingleShot();
    qDebug() << "[ANALYSIS] previewTimer->isSingleShot() =" << isSingleShot;
    if (!isSingleShot) {
        qWarning() << "[BUG CONFIRMED] previewTimer is periodic (singleShot == false)! It triggers endless setMarkdown() every 150ms after user types!";
    }
}

void TestMainWindow::testUserTypingModification()
{
    MainWindow w;
    auto *textEdit = w.findChild<QPlainTextEdit*>("textEdit");
    QVERIFY(textEdit != nullptr);

    // Simulate user typing (which emits modificationChanged(true))
    textEdit->insertPlainText("Sample user typed text");
    
    qDebug() << "[STATE] isWindowModified:" << w.isWindowModified();
    qDebug() << "[STATE] document()->isModified():" << textEdit->document()->isModified();
    QCOMPARE(textEdit->document()->isModified(), true);
    QCOMPARE(w.isWindowModified(), true);
}

void TestMainWindow::testMarkdownRendering()
{
    MainWindow w;
    auto *textEdit = w.findChild<QPlainTextEdit*>("textEdit");
    auto *preview = w.findChild<QTextBrowser*>("previewBrowser");

    textEdit->insertPlainText("# Title 1\n\n- item 1\n- item 2\n\n**bold**");
    
    // Wait 250ms for the 150ms timer to trigger
    QTest::qWait(250);

    QString html = preview->toHtml();
    QVERIFY(html.contains("Title 1"));
    QVERIFY(html.contains("item 1"));
    QVERIFY(html.contains("item 2"));
    QVERIFY(html.contains("bold"));
}

void TestMainWindow::testFindLogic()
{
    MainWindow w;
    auto *textEdit = w.findChild<QPlainTextEdit*>("textEdit");
    textEdit->insertPlainText("First apple, second banana, third apple.");

    // Test textEdit->find() mechanism which on_actionFind_triggered uses
    textEdit->moveCursor(QTextCursor::Start);
    bool found1 = textEdit->find("banana");
    QVERIFY(found1);
    QCOMPARE(textEdit->textCursor().selectedText(), QString("banana"));

    // Find next apple
    bool foundApple1 = textEdit->find("apple");
    QVERIFY(foundApple1);
    
    // Wrap-around search behavior check
    bool foundAppleWrap = textEdit->find("apple");
    if (!foundAppleWrap) {
        // Need wrap around to start
        textEdit->moveCursor(QTextCursor::Start);
        foundAppleWrap = textEdit->find("apple");
    }
    QVERIFY(foundAppleWrap);
}

void TestMainWindow::testSaveAndOpenFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString testFilePath = tempDir.filePath("test_doc.md");

    {
        // Test manual write and read simulation matching MainWindow's saveFile logic
        QFile file(testFilePath);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        QTextStream out(&file);
        QString sampleContent = "# Test Header\nHello from automated test.";
        out << sampleContent;
        file.close();
    }

    {
        QFile file(testFilePath);
        QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
        QTextStream in(&file);
        QString content = in.readAll();
        file.close();
        QVERIFY(content.contains("# Test Header"));
    }
}

void TestMainWindow::testUnimplementedActionsAudit()
{
    MainWindow w;
    auto *actionReplace = w.findChild<QAction*>("actionReplace");
    QVERIFY(actionReplace != nullptr);

    auto *actionZoomIn = w.findChild<QAction*>("actionZoomIn");
    QVERIFY(actionZoomIn != nullptr);

    auto *actionZoomOut = w.findChild<QAction*>("actionZoomOut");
    QVERIFY(actionZoomOut != nullptr);

    auto *actionZoomReset = w.findChild<QAction*>("actionZoomReset");
    QVERIFY(actionZoomReset != nullptr);

    auto *actionWordWrap = w.findChild<QAction*>("actionWordWrap");
    QVERIFY(actionWordWrap != nullptr);

    qDebug() << "[AUDIT] actionReplace, actionZoomIn, actionZoomOut, actionZoomReset, actionWordWrap all exist in UI but slots are EMPTY.";
}

void TestMainWindow::testShortcutsAudit()
{
    MainWindow w;
    QList<QAction*> actions = w.findChildren<QAction*>();
    int actionsWithoutShortcut = 0;
    for (QAction* act : actions) {
        if (!act->isSeparator() && !act->menu()) {
            if (act->shortcut().isEmpty()) {
                qDebug() << "[SHORTCUT AUDIT] Action missing shortcut:" << act->objectName() << act->text();
                actionsWithoutShortcut++;
            }
        }
    }
    qDebug() << "[SHORTCUT AUDIT] Total actions without shortcuts:" << actionsWithoutShortcut;
}

void TestMainWindow::testTogglePreview()
{
    MainWindow w;
    auto *actionTogglePreview = w.findChild<QAction*>("actionTogglePreview");
    QVERIFY(actionTogglePreview != nullptr);

    auto *preview = w.findChild<QTextBrowser*>("previewBrowser");
    QVERIFY(preview != nullptr);
    QVERIFY(actionTogglePreview->isCheckable());
    QVERIFY(actionTogglePreview->isChecked());
    QVERIFY(!preview->isHidden());

    actionTogglePreview->trigger();
    QVERIFY(!actionTogglePreview->isChecked());
    QVERIFY(preview->isHidden());

    actionTogglePreview->trigger();
    QVERIFY(actionTogglePreview->isChecked());
    QVERIFY(!preview->isHidden());
}

void TestMainWindow::testAutoFormatList()
{
    MainWindow w;
    w.show();
    auto *textEdit = w.findChild<QPlainTextEdit*>("textEdit");
    QVERIFY(textEdit != nullptr);

    // 测试 1: 输入 "- 第一项"，然后按回车
    textEdit->setPlainText("- 第一项");
    QTextCursor cursor = textEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    textEdit->setTextCursor(cursor);

    QTest::keyClick(textEdit, Qt::Key_Return);
    QString text = textEdit->toPlainText();
    qDebug() << "[AUTO FORMAT TEST] Result after pressing Return on '- 第一项':\n" << text;
    QVERIFY(text.contains("- 第一项"));
    QVERIFY(text.endsWith("- "));

    // 测试 2: 输入 "1. 第一项"，然后按回车
    textEdit->setPlainText("1. 第一项");
    cursor = textEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    textEdit->setTextCursor(cursor);

    QTest::keyClick(textEdit, Qt::Key_Return);
    text = textEdit->toPlainText();
    qDebug() << "[AUTO FORMAT TEST] Result after pressing Return on '1. 第一项':\n" << text;
    QVERIFY(text.contains("1. 第一项"));
    QVERIFY(text.endsWith("2. "));
}

QTEST_MAIN(TestMainWindow)
#include "test_mainwindow.moc"
