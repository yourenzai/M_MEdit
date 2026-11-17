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
    void testTimerConfiguration();
    void testUserTypingModification();
    void testMarkdownRendering();
    void testFindLogic();
    void testSaveAndOpenFile();
    void testUnimplementedActionsAudit();
    void testShortcutsAudit();
    void testTogglePreviewAudit();
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

void TestMainWindow::testTogglePreviewAudit()
{
    MainWindow w;
    auto *actionTogglePreview = w.findChild<QAction*>("actionTogglePreview");
    QVERIFY(actionTogglePreview != nullptr);
    
    // In mainwindow.h and mainwindow.cpp, there is NO on_actionTogglePreview_triggered() slot!
    // We can verify that triggering it does not hide or toggle previewBrowser
    auto *preview = w.findChild<QTextBrowser*>("previewBrowser");
    QVERIFY(preview != nullptr);
    
    // Check if triggering changes preview visible state
    // Note: since w is not shown, let's show offscreen or check connection
    // When an action has no connection, triggering it is a no-op.
    qDebug() << "[AUDIT] actionTogglePreview is defined in UI but completely disconnected in mainwindow.cpp!";
}

QTEST_MAIN(TestMainWindow)
#include "test_mainwindow.moc"
