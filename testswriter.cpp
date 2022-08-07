#include <QDebug>
#include <iostream>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>

#include "testswriter.h"
#include "exceptions.h"

using json = nlohmann::json;

TestsWriter::Question::Question()
{
    _isReadable = false;
}

TestsWriter::Question::Question(const QString &question,
         const QVector<QString> &filenames,
         const AnswerType &answerType,
         const QVector<QString> &answers,
         const QVector<size_t> &correctAnswerIndicies)
{
    _question = question;
    _filenames = filenames;
    _answerType = answerType;
    _answers = answers;
    _correctAnswerIndicies = correctAnswerIndicies;
    _isReadable = false;
}

QString TestsWriter::Question::getQuestionText() const
{
    return _question;
}

QVector<QString> TestsWriter::Question::getFilenames() const
{
    return _filenames;
}

TestsWriter::Question::AnswerType TestsWriter::Question::getAnswerType() const
{
    return _answerType;
}

QVector<QString> TestsWriter::Question::getAnswers() const
{
    return _answers;
}

QVector<size_t> TestsWriter::Question::getCorrectAnswerIndicies() const
{
    return _correctAnswerIndicies;
}

void TestsWriter::Question::setReadable(bool flag)
{
    _isReadable = flag;
}

bool TestsWriter::Question::readable() const
{
    return _isReadable;
}

void TestsWriter::setQuestionsCount(size_t count)
{
    _questions.resize(count);
    _questionsCount = count;
}

bool TestsWriter::addQuestion(const Question &question, size_t index)
{
    if (index >= _questionsCount)
        return false;

    _questions[index] = question;
    _questions[index].setReadable(true);

    return true;
}

bool TestsWriter::getQuestion(Question &question, size_t index) const
{
    if (index >= _questionsCount || !_questions[index].readable())
        return false;

    question = _questions[index];

    return true;
}

QVector<size_t> TestsWriter::getIncompleteQuestionIndicies() const
{
    QVector<size_t> incompleteQuestionIndicies;

    for (size_t i = 0; i < _questionsCount; ++i)
        if (!_questions[i].readable() || \
             _questions[i].getQuestionText() == "" || \
             _questions[i].getAnswerType() == TestsWriter::Question::NONE || \
             _questions[i].getAnswers().isEmpty() || \
             _questions[i].getCorrectAnswerIndicies().isEmpty())
        {
            incompleteQuestionIndicies.push_back(i);
        }

    return incompleteQuestionIndicies;
}

void TestsWriter::resetTest()
{
    _questionsCount = 0;
    _questions.clear();
}

void TestsWriter::saveTest(std::ofstream &file)
{
    json test;
    test["count"] = _questionsCount;
    json questionsArray = json::array();

    for (size_t i = 0; i < _questionsCount; ++i)
    {
        json question;
        question["text"] = _questions[i].getQuestionText().toStdString();
        std::vector<std::string> filenames;

        for (auto &filename : _questions[i].getFilenames())
            filenames.push_back(filename.toStdString());

        question["question type"] = filenames.size() ? "with images" : "text only";
        question["images"] = filenames;

        Question::AnswerType answerType = _questions[i].getAnswerType();

        switch (answerType)
        {
        case Question::AnswerType::SINGLE_CHOICE:
            question["answer type"] = "single choice";
            break;
        case Question::AnswerType::MULTIPLE_CHOICE:
            question["answer type"] = "multiple choice";
            break;
        case Question::AnswerType::STRING_ANSWER:
            question["answer type"] = "text answer";
            break;
        default:
            throw SavingException();
            break;
        }

        std::vector<std::string> answers;

        for (auto &answer : _questions[i].getAnswers())
            answers.push_back(answer.toStdString());

        question["answers"] = answers;

        std::vector<size_t> correctAnswerIndicies;

        for (auto &answerIndex : _questions[i].getCorrectAnswerIndicies())
            correctAnswerIndicies.push_back(answerIndex);

        question["correct answers"] = correctAnswerIndicies;
        questionsArray.push_back(question);
    }

    test["questions"] = questionsArray;

    file << test.dump(4) << std::endl;
    std::cout << "Test successfully saved." << std::endl;
}
