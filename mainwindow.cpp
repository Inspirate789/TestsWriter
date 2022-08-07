#include <iostream>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QRadioButton>
#include <QCheckBox>
#include <QMessageBox>
#include <fstream>
#include <filesystem>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "exceptions.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("TestsWriter");

    connect(this, &MainWindow::removeLineSignal, this, &MainWindow::removeLine);

    questionSwitchers = QVector<QPushButton *>();
//    questionSwitchersHSpacer = nullptr;

    answerTypeButtons = new QButtonGroup();
    answerTypeButtons->addButton(ui->singleChoiceRB);
    answerTypeButtons->addButton(ui->multipleChoiceRB);
    answerTypeButtons->addButton(ui->textAnswerRB);

    answerButtons = new QButtonGroup();

    questionsCount = 0;
    curQuestionIndex = 0;
    answersCountOfAllTime = 0;
    filesCountOfAllTime = 0;
    curAnswerType = TestsWriter::Question::NONE;

    addAnswerBtn = nullptr;
    answerField = nullptr;
    answersHSpacer = nullptr;
    answersVSpacer = nullptr;
    filesVSpacer = nullptr;

    questionSwitchersWidget = new QWidget();
    questionSwitchersLayout = new QGridLayout(questionSwitchersWidget);
    ui->questionsSwitcherSA->setWidget(questionSwitchersWidget);

    answersWidget = new QWidget();
    answersLayout = new QGridLayout(answersWidget);
    ui->answersSA->setWidget(answersWidget);

    filesWidget = new QWidget();
    filesLayout = new QGridLayout(filesWidget);
    ui->filesSA->setWidget(filesWidget);

    setDisabled(true);
    showDialog();
}

MainWindow::~MainWindow()
{
    delete answerTypeButtons;
    delete answerButtons;

    delete ui;
}

void MainWindow::showDialog()
{
    dialog = new QDialog(this);
    // this->setWindowFlags(Qt::WindowStaysOnBottomHint);
    dialog->setWindowFlag(Qt::WindowStaysOnTopHint);
    dialog->topLevelWidget();
    dialog->activateWindow();
    dialog->setWindowTitle(ui->centralwidget->windowTitle());
    QGridLayout *dialogLayout = new QGridLayout(dialog);
    QLabel *lbl = new QLabel("Количество вопросов: ");
    qInitCountSB = new QSpinBox(dialog);
    qInitCountSB->setRange(MIN_QUESTIONS_COUNT, MAX_QUESTIONS_COUNT);
    qInitCountSB->setValue(START_QUESTIONS_COUNT);
    QPushButton *qInitBtn = new QPushButton("Составить");
    dialogLayout->addWidget(lbl, 0, 0);
    dialogLayout->addWidget(qInitCountSB, 0, 1);
    dialogLayout->addWidget(qInitBtn, 1, 0, 1, 2);
    dialog->setLayout(dialogLayout);
    connect(qInitBtn, &QPushButton::released, this, &MainWindow::initQuestionsCount);
    dialog->show();
    dialog->move(x() + width() / 2,
                 y() + height() / 2 - dialog->height());
}

void MainWindow::initQuestionsCount()
{
    ui->qCountSB->setValue(qInitCountSB->value());
    setEnabled(true);
    dialog->close();
    on_qCreateBtn_clicked();
}

void MainWindow::on_qCreateBtn_clicked()
{
    saveCurQuestion();
    clearQuestionFields();

    ui->qCreateGB->setEnabled(true);
    ui->resetTestBtn->setEnabled(true);
    ui->saveTestBtn->setEnabled(true);

    for (auto &btn : questionSwitchers)
        btn->hide();

    questionSwitchers.clear();

    questionsCount = ui->qCountSB->value();

    if (curQuestionIndex >= questionsCount)
        curQuestionIndex = questionsCount - 1;

    ui->questionLabel->setText(QString::asprintf("Вопрос %zu из %zu: ", curQuestionIndex + 1, questionsCount));

    for (size_t i = 0; i < questionsCount; ++i)
    {
        QPushButton *btn = new QPushButton(QString::number(i + 1));
        btn->setStyleSheet("font-size: 18px;");
        connect(btn, &QPushButton::released, this, &MainWindow::switchQuestion);
        questionSwitchersLayout->addWidget(btn, 0, i);
        questionSwitchers.push_back(btn);
    }

    questionSwitchers[curQuestionIndex]->setDisabled(true);
    ui->goPrevBtn->setDisabled(!curQuestionIndex);
    ui->goNextBtn->setDisabled(curQuestionIndex == questionsCount - 1);

    writer.setQuestionsCount(questionsCount);
    TestsWriter::Question curQuestion;

    if (writer.getQuestion(curQuestion, curQuestionIndex))
        restoreCurQuestion(curQuestion);
}

QString MainWindow::truncateFilename(const QString &filename)
{
    QString truncatedFilename = filename;

    if (truncatedFilename.length() > MAX_FILENAME_LENGTH - 4/* && filename.count('/')*/)
    {
        do
            truncatedFilename = truncatedFilename.section('/', 1, -1);
        while (truncatedFilename.length() > MAX_FILENAME_LENGTH - 4/* && filename.count('/')*/);

        truncatedFilename = ".../" + truncatedFilename;
    }

    return truncatedFilename;
}

void MainWindow::addFileWidget(const QString &filename)
{
    for (auto &fileWidget : files)
        if (fileWidget.first->text() == filename)
        {
            QMessageBox::warning(this, "Предупреждение", "Выбранный файл уже прикреплён к вопросу.");

            return;
        }

    filesLayout->removeItem(filesVSpacer);
    delete filesVSpacer;
    filesVSpacer = nullptr;

    QLabel *lbl = new QLabel(filename);
    DeleteButton *btn = new DeleteButton();
    files.push_back(QPair<QLabel *, DeleteButton *>(lbl, btn));

    QSpacerItem *sp = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    filesHSpacers.push_back(sp);

    btn->setFixedWidth(25);
    btn->setMainWindow(this);

    filesLayout->addWidget(lbl, filesCountOfAllTime, 0);
    filesLayout->addItem(sp, filesCountOfAllTime, 1);
    filesLayout->addWidget(btn, filesCountOfAllTime, 2);

    filesVSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    filesLayout->addItem(filesVSpacer, filesCountOfAllTime + 1, 0);

    ++filesCountOfAllTime;
}

void MainWindow::on_addFileBtn_clicked()
{
    QString originalFilename = QFileDialog::getOpenFileName(nullptr,
                                                    "Прикрепить изображение",
                                                    filepath,
                                                    "Image files (*.png *.jpg *.jpeg *.svg *.bmp *.xpm)");

    if (originalFilename == QString(""))
        return;

    filepath = QFileInfo(originalFilename).absolutePath();

    //QString truncatedFilename = truncateFilename(originalFilename);

    addFileWidget(originalFilename);
}

void MainWindow::removeLine(DeleteButton *const btn)
{
    auto answersIterPair = answers.begin();
    auto answersIterSpacer = answersHSpacers.begin();

    for (; answersIterPair < answers.end(); ++answersIterPair, ++answersIterSpacer)
    {
        if (answersIterPair->second == btn)
        {
            answersIterPair->first->hide();
            answersIterPair->second->hide();
            answersLayout->removeItem(*answersIterSpacer);

            if (curAnswerType == TestsWriter::Question::SINGLE_CHOICE || \
                curAnswerType == TestsWriter::Question::MULTIPLE_CHOICE)
                answerButtons->removeButton(static_cast<QAbstractButton *>(answersIterPair->first));

            answers.erase(answersIterPair);
            answersHSpacers.erase(answersIterSpacer);

            //--answersCountOfAllTime;

            return;
        }
    }

    auto filesIterPair = files.begin();
    auto filesIterSpacer = filesHSpacers.begin();

    for (; filesIterPair < files.end(); ++filesIterPair, ++filesIterSpacer)
    {
        if (filesIterPair->second == btn)
        {
            filesIterPair->first->hide();
            filesIterPair->second->hide();
//            answersLayout->removeWidget(filesIterPair->first);
//            answersLayout->removeWidget(filesIterPair->second);
            filesLayout->removeItem(*filesIterSpacer);

            files.erase(filesIterPair);
            filesHSpacers.erase(filesIterSpacer);

            //--filesCountOfAllTime;

            return;
        }
    }
}

void MainWindow::addAnswerWidget(const QString &answer)
{
    for (auto &answerWidget : answers)
        if (curAnswerType == TestsWriter::Question::STRING_ANSWER)
        {
            if (static_cast<QLineEdit *>(answerWidget.first)->text() == answer)
            {
                QMessageBox::warning(this, "Предупреждение", "Введённый вариант ответа был задан ранее.");

                return;
            }
        }
        else
            if (static_cast<QAbstractButton *>(answerWidget.first)->text() == answer)
            {
                QMessageBox::warning(this, "Предупреждение", "Введённый вариант ответа был задан ранее.");

                return;
            }

    if (answersVSpacer)
    {
        answersLayout->removeItem(answersVSpacer);
        delete answersVSpacer;
        answersVSpacer = nullptr;
    }

    //    answersLayout->removeItem(answersHSpacer);
    //    answersLayout->removeWidget(answerField);
    //    delete answersHSpacer;
    //    answersHSpacer = nullptr;

    if (addAnswerBtn)
    {
        addAnswerBtn->hide();
        delete addAnswerBtn;
        addAnswerBtn = nullptr;
    }

    if (answerField)
    {
        answerField->hide();
        delete answerField;
        answerField = nullptr;
    }

    QWidget *answerWidget;

    if (curAnswerType == TestsWriter::Question::SINGLE_CHOICE)
    {
        QRadioButton *rb = new QRadioButton(answer);
        answerButtons->addButton(rb);
        answerWidget = rb;
    }
    else if (curAnswerType == TestsWriter::Question::MULTIPLE_CHOICE)
    {
        QCheckBox *cb = new QCheckBox(answer);
        answerButtons->addButton(cb);
        answerWidget = cb;
    }
    else
        answerWidget = new QLineEdit(answer);

    DeleteButton *btn = new DeleteButton();
    answers.push_back(QPair<QWidget *, DeleteButton *>(answerWidget, btn));

    QSpacerItem *sp = new QSpacerItem(40, 20,
        curAnswerType == TestsWriter::Question::STRING_ANSWER ? QSizePolicy::Fixed : QSizePolicy::Expanding,
        QSizePolicy::Minimum);
    answersHSpacers.push_back(sp);

    btn->setFixedWidth(25);
    btn->setMainWindow(this);

    answersLayout->addWidget(answerWidget, answersCountOfAllTime, 0);
    answersLayout->addItem(sp, answersCountOfAllTime, 1);
    answersLayout->addWidget(btn, answersCountOfAllTime, 2);
    ++answersCountOfAllTime;

    addAnswerBtn = new QPushButton("Добавить ответ");
    addAnswerBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(addAnswerBtn, &QPushButton::released, this, &MainWindow::addAnswerField);
    answersLayout->addWidget(addAnswerBtn, answersCountOfAllTime, 0);

    answersVSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    answersLayout->addItem(answersVSpacer, answersCountOfAllTime + 1, 0);
}

void MainWindow::addAnswer()
{
    QString newAnswer = answerField->text();

    if (newAnswer != QString(""))
        addAnswerWidget(newAnswer);
}

void MainWindow::addAnswerField()
{
    addAnswerBtn->hide();
    answersLayout->removeItem(answersVSpacer);
    delete addAnswerBtn;
    delete answersVSpacer;
    addAnswerBtn = nullptr;
    answersVSpacer = nullptr;

    answerField = new QLineEdit();
    // добавление фонового приглашения к вводу
    answersLayout->addWidget(answerField, answersCountOfAllTime, 0, 1, 3);

    addAnswerBtn = new QPushButton("Добавить");
    addAnswerBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(addAnswerBtn, &QPushButton::released, this, &MainWindow::addAnswer);
    answersLayout->addWidget(addAnswerBtn, answersCountOfAllTime + 1, 0);

    answersVSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    answersLayout->addItem(answersVSpacer, answersCountOfAllTime + 2, 0);
}

void MainWindow::on_singleChoiceRB_clicked()
{
    answerButtons->setExclusive(true);

    if (curAnswerType != TestsWriter::Question::SINGLE_CHOICE)
    {
        int curAnswersCount = answers.count();
        //int deletedAnswersCount = answersCountOfAllTime - curAnswersCount;

        if (curAnswerType == TestsWriter::Question::MULTIPLE_CHOICE)
        {
            for (int i = 0; i < curAnswersCount; ++i)
            {
                QRadioButton *rb = new QRadioButton(static_cast<QCheckBox *>(answers[i].first)->text());
                answersLayout->replaceWidget(answers[i].first, rb);

                answers[i].first->hide();
                answerButtons->removeButton(static_cast<QCheckBox *>(answers[i].first));
                delete answers[i].first;
                answers[i].first = rb;

                answerButtons->addButton(rb);

            }
        }
        else if (curAnswerType == TestsWriter::Question::STRING_ANSWER)
        {
            for (int i = 0; i < curAnswersCount; ++i)
            {
                QRadioButton *rb = new QRadioButton(static_cast<QLineEdit *>(answers[i].first)->text());
                answersLayout->replaceWidget(answers[i].first, rb);

                answersHSpacers[i]->changeSize(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

                answers[i].first->hide();
                delete answers[i].first;
                answers[i].first = rb;

                answerButtons->addButton(rb);

            }
        }

        curAnswerType = TestsWriter::Question::SINGLE_CHOICE;
    }

    if (!addAnswerBtn)
    {
        addAnswerBtn = new QPushButton("Добавить ответ");
        addAnswerBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        connect(addAnswerBtn, &QPushButton::released, this, &MainWindow::addAnswerField);
        answersLayout->addWidget(addAnswerBtn, answersCountOfAllTime, 0);

//        answersHSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
//        answersLayout->addItem(answersHSpacer, answersCountOfAllTime, 1);

        answersVSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
        answersLayout->addItem(answersVSpacer, answersCountOfAllTime + 1, 0);
    }
}


void MainWindow::on_multipleChoiceRB_clicked()
{
    answerButtons->setExclusive(false);

    int curAnswersCount = answers.count();
    //int deletedAnswersCount = answersCountOfAllTime - curAnswersCount;

    if (curAnswerType != TestsWriter::Question::MULTIPLE_CHOICE)
    {
        if (curAnswerType == TestsWriter::Question::SINGLE_CHOICE)
        {
            for (int i = 0; i < curAnswersCount; ++i)
            {
                QCheckBox *cb = new QCheckBox(static_cast<QRadioButton *>(answers[i].first)->text());
                answersLayout->replaceWidget(answers[i].first, cb);

                answers[i].first->hide();
                answerButtons->removeButton(static_cast<QRadioButton *>(answers[i].first));
                delete answers[i].first;
                answers[i].first = cb;

                answerButtons->addButton(cb);
            }
        }
        else if (curAnswerType == TestsWriter::Question::STRING_ANSWER)
        {
            for (int i = 0; i < curAnswersCount; ++i)
            {
                QCheckBox *cb = new QCheckBox(static_cast<QLineEdit *>(answers[i].first)->text());
                answersLayout->replaceWidget(answers[i].first, cb);

                answersHSpacers[i]->changeSize(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

                answers[i].first->hide();
                delete answers[i].first;
                answers[i].first = cb;

                answerButtons->addButton(cb);
            }
        }

        curAnswerType = TestsWriter::Question::MULTIPLE_CHOICE;
    }

    if (!addAnswerBtn)
    {
        addAnswerBtn = new QPushButton("Добавить ответ");
        addAnswerBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        connect(addAnswerBtn, &QPushButton::released, this, &MainWindow::addAnswerField);
        answersLayout->addWidget(addAnswerBtn, answersCountOfAllTime, 0);

//        answersHSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
//        answersLayout->addItem(answersHSpacer, answersCountOfAllTime, 1);

        answersVSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
        answersLayout->addItem(answersVSpacer, answersCountOfAllTime + 1, 0);
    }
}


void MainWindow::on_textAnswerRB_clicked()
{
    int curAnswersCount = answers.count();
    //int deletedAnswersCount = answersCountOfAllTime - curAnswersCount;

    if (curAnswerType != TestsWriter::Question::STRING_ANSWER)
    {
        if (curAnswerType == TestsWriter::Question::SINGLE_CHOICE)
        {
            for (int i = 0; i < curAnswersCount; ++i)
            {
                QLineEdit *le = new QLineEdit(static_cast<QRadioButton *>(answers[i].first)->text());
                le->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
                answersLayout->replaceWidget(answers[i].first, le);

                answersHSpacers[i]->changeSize(40, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

                answers[i].first->hide();
                answerButtons->removeButton(static_cast<QRadioButton *>(answers[i].first));
                delete answers[i].first;
                answers[i].first = le;
            }
        }
        else if (curAnswerType == TestsWriter::Question::MULTIPLE_CHOICE)
        {
            for (int i = 0; i < curAnswersCount; ++i)
            {
                QLineEdit *le = new QLineEdit(static_cast<QCheckBox *>(answers[i].first)->text());
                le->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
                answersLayout->replaceWidget(answers[i].first, le);

                answersHSpacers[i]->changeSize(40, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

                answers[i].first->hide();
                answerButtons->removeButton(static_cast<QCheckBox *>(answers[i].first));
                delete answers[i].first;
                answers[i].first = le;
            }
        }

        curAnswerType = TestsWriter::Question::STRING_ANSWER;
    }

    if (!addAnswerBtn)
    {
        addAnswerBtn = new QPushButton("Добавить ответ");
        addAnswerBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        connect(addAnswerBtn, &QPushButton::released, this, &MainWindow::addAnswerField);
        answersLayout->addWidget(addAnswerBtn, answersCountOfAllTime, 0);

//        answersHSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
//        answersLayout->addItem(answersHSpacer, answersCountOfAllTime, 1);

        answersVSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
        answersLayout->addItem(answersVSpacer, answersCountOfAllTime + 1, 0);
    }
}

void MainWindow::saveCurQuestion()
{
    QVector<QString> answersData;
    QVector<size_t> correctAnswerIndicies;

    if (curAnswerType == TestsWriter::Question::STRING_ANSWER)
        for (int i = 0; i < answers.count(); ++i)
        {
            answersData.push_back(static_cast<QLineEdit *>(answers[i].first)->text());
            correctAnswerIndicies.push_back(i);
        }
    else if (curAnswerType != TestsWriter::Question::NONE)
        for (int i = 0; i < answers.count(); ++i)
        {
            answersData.push_back(static_cast<QAbstractButton *>(answers[i].first)->text());

            if (static_cast<QAbstractButton *>(answers[i].first)->isChecked())
                correctAnswerIndicies.push_back(i);
        }

    QVector<QString> filenames;

    for (auto &fileWidget : files)
        filenames.push_back(fileWidget.first->text());

    TestsWriter::Question question = {ui->textEdit->toPlainText(),
                                      filenames,
                                      curAnswerType,
                                      answersData,
                                      correctAnswerIndicies};

    if (!writer.addQuestion(question, curQuestionIndex))
    {
        // throw Exception(...);
    }
}

void MainWindow::restoreCurQuestion(const TestsWriter::Question &question)
{
    ui->textEdit->setText(question.getQuestionText());

    for (auto &filename : question.getFilenames())
        addFileWidget(filename);

    if (question.getAnswerType() != TestsWriter::Question::NONE)
    {
        if (question.getAnswerType() == TestsWriter::Question::STRING_ANSWER)
        {
            ui->textAnswerRB->setChecked(true);
            on_textAnswerRB_clicked();

            for (auto &answer : question.getAnswers())
                addAnswerWidget(answer);
        }
        else
        {
            if (question.getAnswerType() == TestsWriter::Question::SINGLE_CHOICE)
            {
                ui->singleChoiceRB->setChecked(true);
                on_singleChoiceRB_clicked();
            }
            else
            {
                ui->multipleChoiceRB->setChecked(true);
                on_multipleChoiceRB_clicked();
            }

            auto answersData = question.getAnswers();
            auto correctAnswerIndicies = question.getCorrectAnswerIndicies();

            for (auto &answer : answersData)
                addAnswerWidget(answer);

            for (auto &i : correctAnswerIndicies)
                static_cast<QAbstractButton *>(answers[i].first)->setChecked(true);
        }
    }
}

void MainWindow::showCurQuestion()
{
    ui->questionLabel->setText(QString::asprintf("Вопрос %zu из %zu: ", curQuestionIndex + 1, questionsCount));
    questionSwitchers[curQuestionIndex]->setDisabled(true);
    ui->goPrevBtn->setDisabled(!curQuestionIndex);
    ui->goNextBtn->setDisabled(curQuestionIndex == questionsCount - 1);

    TestsWriter::Question curQuestion;

    if (writer.getQuestion(curQuestion, curQuestionIndex))
        restoreCurQuestion(curQuestion);
}

void MainWindow::switchQuestion()
{
    saveCurQuestion();
    clearQuestionFields();
    questionSwitchers[curQuestionIndex]->setDisabled(false);
    curQuestionIndex = static_cast<QPushButton *>(sender())->text().toULongLong() - 1;
    showCurQuestion();
}

void MainWindow::on_goPrevBtn_clicked()
{
    saveCurQuestion();
    clearQuestionFields();
    questionSwitchers[curQuestionIndex]->setDisabled(false);
    --curQuestionIndex;
    showCurQuestion();
}


void MainWindow::on_goNextBtn_clicked()
{
    saveCurQuestion();
    clearQuestionFields();
    questionSwitchers[curQuestionIndex]->setDisabled(false);
    ++curQuestionIndex;
    showCurQuestion();
}

void MainWindow::clearQuestionFields()
{
    ui->textEdit->setText("");

    for (auto &pair : files)
        emit pair.second->released();

    if (filesVSpacer)
    {
        filesLayout->removeItem(filesVSpacer);
        delete filesVSpacer;
        filesVSpacer = nullptr;
    }

    for (auto &pair : answers)
        emit pair.second->released();

    if (answersVSpacer)
    {
        answersLayout->removeItem(answersVSpacer);
        delete answersVSpacer;
        answersVSpacer = nullptr;
    }

    if (answerField)
    {
        answerField->hide();
        delete answerField;
        answerField = nullptr;
    }

    if (addAnswerBtn)
    {
        addAnswerBtn->hide();
        delete addAnswerBtn;
        addAnswerBtn = nullptr;
    }

    answerTypeButtons->setExclusive(false);
    ui->singleChoiceRB->setChecked(false);
    ui->multipleChoiceRB->setChecked(false);
    ui->textAnswerRB->setChecked(false);
    answerTypeButtons->setExclusive(true);
    curAnswerType = TestsWriter::Question::NONE;
}

void MainWindow::on_resetTestBtn_clicked()
{
    questionsCount = 0;
    curQuestionIndex = 0;
    answersCountOfAllTime = 0;
    filesCountOfAllTime = 0;

    ui->questionLabel->setText("Вопрос: ");

    for (auto &btn : questionSwitchers)
        btn->hide();

    questionSwitchers.clear();

    clearQuestionFields();

    writer.resetTest();

    ui->qCreateGB->setDisabled(true);
    ui->resetTestBtn->setDisabled(true);
    ui->saveTestBtn->setDisabled(true);
}

std::string MainWindow::getVacantFilename()
{
    size_t testIndex = 1;
    std::string filename;

    do
        filename = TESTS_FOLDER + std::string("\\") + TEST_FILENAME + std::to_string(testIndex++) + FILE_EXTENSION;
    while (std::filesystem::exists(filename));

    return filename;
}

void MainWindow::on_saveTestBtn_clicked()
{
    saveCurQuestion();
    QVector<size_t> incompleteQuestionIndicies = writer.getIncompleteQuestionIndicies();

    if (!incompleteQuestionIndicies.empty())
    {
        std::string message = std::to_string(incompleteQuestionIndicies[0] + 1);

        if (incompleteQuestionIndicies.count() > 1)
        {
            for (int i = 1; i < incompleteQuestionIndicies.count(); ++i)
                message += ", " + std::to_string(incompleteQuestionIndicies[i] + 1);

            message = "Вопросы " + message + " заполнены не до конца.\nЗаполните вопросы и повторите попытку.";
        }
        else
            message = "Вопрос " + message + " не заполнен до конца.\nЗаполните вопрос и повторите попытку.";

        QMessageBox::warning(this, "Предупреждение", QString::fromStdString(message));

        emit questionSwitchers[incompleteQuestionIndicies[0]]->released();

        return;
    }

    try
    {
        std::filesystem::create_directory(TESTS_FOLDER);
        std::ofstream file(getVacantFilename(), std::ios::binary);
        writer.saveTest(file);
    }
    catch (const TestsWriterException &error)
    {
        QMessageBox::critical(this, "Ошибка", error.what());
    }
}

