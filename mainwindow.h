#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSpinBox>
#include <QGridLayout>
#include <QButtonGroup>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QVector>

#include "testswriter.h"
#include "deletebutton.h"

#define _GNU_SOURCE

#define MIN_QUESTIONS_COUNT 1
#define MAX_QUESTIONS_COUNT 10000
#define START_QUESTIONS_COUNT 10

#define MAX_FILENAME_LENGTH 65

#define TESTS_FOLDER "Tests"
#define TEST_FILENAME "test"
#define FILE_EXTENSION ".json"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class DeleteButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void showDialog();
    QString truncateFilename(const QString &filename);
    void addFileWidget(const QString &filename);
    void addAnswerWidget(const QString &answer);
    void saveCurQuestion();
    void restoreCurQuestion(const TestsWriter::Question &question);
    void showCurQuestion();
    void clearQuestionFields();
    std::string getVacantFilename();

signals:
    void removeLineSignal(DeleteButton *const btn);

public slots:
    void removeLine(DeleteButton *const btn);

private slots:
    void initQuestionsCount();
    void on_qCreateBtn_clicked();
    void switchQuestion();
    void addAnswer();
    void addAnswerField();
    void on_singleChoiceRB_clicked();
    void on_multipleChoiceRB_clicked();
    void on_addFileBtn_clicked();
    void on_textAnswerRB_clicked();
    void on_goPrevBtn_clicked();
    void on_goNextBtn_clicked();
    void on_resetTestBtn_clicked();
    void on_saveTestBtn_clicked();

private:
    Ui::MainWindow *ui;
    TestsWriter writer;

    QString filepath = "C:\\";

    QDialog *dialog;
    QSpinBox *qInitCountSB;

    QButtonGroup *answerTypeButtons;
    QButtonGroup *answerButtons;

    size_t questionsCount, curQuestionIndex;
    TestsWriter::Question::AnswerType curAnswerType;

    QWidget *questionSwitchersWidget;
    QGridLayout *questionSwitchersLayout;
    QVector<QPushButton *> questionSwitchers;
//    QSpacerItem *questionSwitchersHSpacer;

    QWidget *answersWidget;
    QGridLayout *answersLayout;
    QPushButton *addAnswerBtn;
    QLineEdit *answerField;
    QVector<QPair<QWidget *, DeleteButton *>> answers; // заменить все * на умные указатели
    QVector<QSpacerItem *> answersHSpacers;
    QSpacerItem *answersVSpacer, *answersHSpacer;
    size_t answersCountOfAllTime;

    QWidget *filesWidget;
    QGridLayout *filesLayout;
    QVector<QPair<QLabel *, DeleteButton *>> files;
    QVector<QSpacerItem *> filesHSpacers;
    QSpacerItem *filesVSpacer;
    size_t filesCountOfAllTime;
};

#endif // MAINWINDOW_H
